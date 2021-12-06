# L1PhysicsSkim WorkFlow
An L1 Skim config setup to save events that  pass the L1 Physics trigger bits based on the new L1 Menu.  

The targeted environment is Lxplus and assumes a standard session:
```
ssh -XY <username>@lxplus.cern.ch
```

## Environment Setup
Setup the environment according to the [official instructions](https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideL1TStage2Instructions).
```
cmsrel CMSSW_12_0_2
cd CMSSW_12_0_2/src
cmsenv
git cms-init
git remote add cms-l1t-offline git@github.com:cms-l1t-offline/cmssw.git
git fetch cms-l1t-offline l1t-integration-CMSSW_12_0_2
git cms-merge-topic -u cms-l1t-offline:l1t-integration-v110.0
git cms-addpkg HLTrigger/HLTcore
git cms-checkdeps -A -a
git cms-addpkg DataFormats/L1TGlobal  

scram b -j 8

```
## Customisation of L1 Menu 
Here you will be customising the L1 emulator using the your new L1 menu. For more details follow Elisa Fontanesi's [L1 emulator tutorial](https://indico.cern.ch/event/1060362/contributions/4455932/attachments/2286815/3937192/L1T_Tutorial_Emulator.pdf) 
```diff
★ Check if you have the package L1Trigger/L1TCommon, otherwise add it:
git cms-addpkg L1Trigger/L1TCommon
★ Add the latest L1Trigger/L1TGlobal package:
git cms-addpkg L1Trigger/L1TGlobal
mkdir -p L1Trigger/L1TGlobal/data/Luminosity/startup/
★ Upload the XML file into the directory L1Trigger/L1TGlobal/data/Luminosity/startup/
★ Edit the file L1Trigger/Configuration/python/customiseUtils.py by changing the L1TriggerMenuFile:
- process.TriggerMenu.L1TriggerMenuFile = cms.string('L1Menu_Collisions2016_v2c.xml') 
+ process.TriggerMenu.L1TriggerMenuFile = cms.string('L1Menu_Collisions2022_v0_1_1.xml')

scram b -j 8
```
## Running the L1 Skim  

 Now you are ready with the L1 setup.The next step is to git clone the L1Physics Skim repository and run the L1emulation using cmsDriver(eg.running on Zero Bias Data for run 323755).
```
git clone https://github.com/sanuvarghese/L1PhysicsSkim.git
scram b -j 8
cd L1PhysicsSkim/L1PhysicsFilter/test/
```  
### Input Files
The L1 Skim should be run either on Zero Bias samples or MC. Do not run the skimmer on EphemeralHLTPhysics dataset because an L1 menu is already applied on them.Here we will be considering ZB. Since most(if not all) Zero Bias Datasets are not available locally on eos, you need to create your own list_cff.py containing the paths of the runs you are considering from DAS. You can obtain the file names directly from the command line using dasgoclient query
```
voms-proxy-init --voms cms --valid 168:00
cp /tmp/x509up_<user proxy> /afs/cern.ch/user/<letter>/<username>/private/  

dasgoclient --query="file dataset=/EphemeralZeroBias1/Run2018D-v1/RAW and run=323755" > ZB1.txt
```  
You need to repeat it for EphemeralZeroBias{1-8} and combine the file paths into a single txt file   
(eg $ cat ZB1.txt ZB2.txt .. ZB8.txt > ZB.txt)  

Next step is to create a list_cff.py file in the format (edit the ZB.txt accordingly and rename to list_cff.py)
```
inputFileNames=[
'/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/02506E54-CE47-A649-9F80-117E978DC69E.root',
'/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/08D7B1A7-B8C5-0944-9A69-B698A2BF52EB.root',
'/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/11FC721B-C288-2342-B356-317FD2457444.root',
'/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/1507559B-D021-E648-8100-010C4698D4DB.root',

...

]
```
As an example, the list_cff.py for the EphemeralZeroBias samples for run 323755 is already available in the test directory. 
### L1T emulation
The L1T emulation is invoked via cmsDriver.py command step from the L1Trigger directory. for more deatils about cmsDriver and its options, follow this [twiki](https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideCmsDriver) .

```
cmsDriver.py l1Ntuple -s RAW2DIGI --python_filename=data.py -n 500 --no_output --era=Run2_2018 --data --conditions=120X_dataRun2_v2 --customise=L1Trigger/Configuration/customiseReEmul.L1TReEmulFromRAW --customise=L1Trigger/Configuration/customiseUtils.L1TGlobalMenuXML --customise=L1Trigger/Configuration/customiseSettings.L1TSettingsToCaloParams_2018_v1_3 --filein=/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/08D7B1A7-B8C5-0944-9A69-B698A2BF52EB.root


```  
If you get an import Command warning(import Command is deprecated for python 3), replace the "import command" line in L1Trigger/Configuration/python/customiseUtils.py with "import subprocess" (I dont think it is used anywhere!)  

Note that our purpose here is not to get the Emulated L1 Ntuples, but to get the data.py config file on which we will apply the L1 Skim Filter(which is why we omitted the --customise=L1Trigger/L1TNtuples/customiseL1Ntuple.L1NtupleRAWEMU option).  

#### After cmsDriver finishes running, make the following changes in the newly created data.py file.  
Change the process name from "RAW2DIGI" to "HLT2"
```diff
- process = cms.Process('RAW2DIGI',Run2_2018)
+ process = cms.Process('HLT2',Run2_2018)
```  

comment out the lines at the end
```diff
  # Add early deletion of temporary data products to reduce peak memory need                                                         
+ #from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete                                          
+ #process = customiseEarlyDelete(process)                                                                                           
  # End adding early deletion 
``` 

## Creating and submiting Jobs on Condor

Create an output directory for your future root files:  
```
mkdir /path/to/output/dir
```
Create condor jobs for data by running the cmsCondorData.py script, for MC by running the cmsCondorMC.py(will be available soon). There are 3 mandatory arguments and 3 options:
- The 1st argument is always runFilter_cfg.py.
- The 2nd argument is the path to the top of your CMSSW release.
- The 3rd argument is the path to the output directory for your root files.
- The -n option allows you to set the number of input root files processed per job. 
- The -q option allows you to set the "flavour" of your job. Each flavour corresponds to a different maximum running time. The default is "workday" (= 8h), but "longlunch" (= 2h) is usually enough.
- You can use the -p option to attach your grid proxy to your jobs (specify the path to your proxy after -p).

```
./cmsCondorData.py runFilter_cfg.py <path to your CMSSW src directory> <path to your output directory >  -n 1 -q longlunch -p /afs/cern.ch/user/<first letter>/<username>/private/x509up_<user proxy>
```
Submit All Jobs on Condor

```
./sub_total.jobb

```
When the Jobs are done, New Filtered Raw root files (containing only events that pass the L1 Trigger with the new menu named L1_0.root, L1_1.root etc) will be produced in the output directory.

### Some Comments about the Skimmer
The skimmer uses the L1GT utils tool to access L1 Trigger decisions and saves events that pass the L1 Trigger bits less than a maxBitNr(maxBitNr==458 for the modified [L1 menu](https://twiki.cern.ch/twiki/bin/viewauth/CMS/L1Menu_Collisions2022_v0_1_1) corresponding to the physics based trigger bits). If you want to apply your customised L1 menu with new L1 seeds, you need to change this parameter(maxBitNr) accordingly in the run_Filter_cfg.py config file.
```diff
 process.L1PhysicsFilter = cms.EDFilter("L1PhysicsFilter",
                                       hltProcess=cms.string("HLT2"),
                                       stageL1Trigger=cms.uint32(2),
++                                       maxBitNr=cms.uint32(458)
                                       )

```

### Additional Links
1. [L1 Menu for Run 3](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideL1TriggerMenu#L1_Menus_for_Run_3)








