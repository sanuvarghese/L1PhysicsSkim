# L1PhysicsSkim WorkFlow
An L1 Skim config setup to save events that  pass the L1 Physics trigger bits based on the new L1 Menu.  

The targeted environment is Lxplus and assumes a standard session:
```
ssh -XY <username>@lxplus.cern.ch
```

## Environment Setup
Setup the environment according to the [official instructions](https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideL1TStage2Instructions).
```
cmsrel CMSSW_12_3_0_pre6
cd CMSSW_12_3_0_pre6/src/
cmsenv
git cms-init
git cms-addpkg L1Trigger/L1TGlobal
git cms-merge-topic Sam-Harper:L1PSLumiSecFix_1230pre6
mkdir -p L1Trigger/L1TGlobal/data/Luminosity/startup
cd L1Trigger/L1TGlobal/data/Luminosity/startup
wget https://raw.githubusercontent.com/cms-l1-dpg/L1MenuRun3/master/development/L1Menu_Collisions2022_v1_0_0/L1Menu_Collisions2022_v1_0_0.xml
wget https://raw.githubusercontent.com/cms-l1-dpg/L1MenuRun3/master/development/L1Menu_Collisions2022_v1_0_0/PrescaleTable/UGT_BASE_RS_FINOR_MASK_L1MenuCollisions2022_v1_0_0.xml
cp /afs/cern.ch/work/s/savarghe/public/L1Skim/UGT_BASE_RS_PRESCALES_L1MenuCollisions2022_v1_0_0.xml .
cd -
```
```diff
git cms-addpkg L1Trigger/Configuration
★ Edit the file L1Trigger/Configuration/python/customiseUtils.py by changing the L1TriggerMenuFile:
- process.TriggerMenu.L1TriggerMenuFile = cms.string('L1Menu_Collisions2016_v2c.xml') 
+ process.TriggerMenu.L1TriggerMenuFile = cms.string('L1Menu_Collisions2022_v1_0_0.xml')

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
cmsDriver.py l1Ntuple -s L1REPACK:uGT,RAW2DIGI --python_filename=data.py -n 500 --no_output --era=Run2_2018 --data --conditions=123X_dataRun2_v1 --customise=L1Trigger/Configuration/customiseReEmul.L1TReEmulFromRAW --customise=L1Trigger/Configuration/customiseUtils.L1TGlobalMenuXML  --filein=/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/08D7B1A7-B8C5-0944-9A69-B698A2BF52EB.root --lumiToProcess=/afs/cern.ch/work/s/savarghe/public/L1Skim/myRealjson.txt --nThreads=4 --processName=HLT2


```  
If you get an "import commands" error(You will get this error if you use 12_0_X, taken care of in higher releases),replace the "import commands" line in L1Trigger/Configuration/python/customiseUtils.py with "import subprocess" ("commands" is deprecated for python 3 but I dont think it is used anywhere!)  

Note that our purpose here is not to get the Emulated L1 Ntuples, but to get the data.py config file on which we will apply the L1 Skim Filter(which is why we omitted the --customise=L1Trigger/L1TNtuples/customiseL1Ntuple.L1NtupleRAWEMU option).  
 
#### Applying Prescales
Add the following lines at the end of the newly created data.py config  file
```process.load('L1Trigger.L1TGlobal.PrescalesVetosFract_cff')
process.load('L1Trigger.L1TGlobal.simGtStage2Digis_cfi')
process.load('L1Trigger.L1TGlobal.hackConditions_cff')                                                                                                       
process.L1TGlobalPrescalesVetosFract.PrescaleXMLFile = cms.string('UGT_BASE_RS_PRESCALES_L1MenuCollisions2022_v1_0_0.xml')   
process.L1TGlobalPrescalesVetosFract.FinOrMaskXMLFile = cms.string('UGT_BASE_RS_FINOR_MASK_L1MenuCollisions2022_v1_0_0.xml')  
process.simGtStage2Digis.AlgorithmTriggersUnmasked = cms.bool(False)
process.simGtStage2Digis.AlgorithmTriggersUnprescaled = cms.bool(False)
process.simGtStage2Digis.PrescaleSet = cms.uint32(4) #5 corresponds to Prescale column at 1.5e34 (At the moment,It is advised to run the skim at this L1 PS column for EZB dataset)
process.simGtStage2Digis.resetPSCountersEachLumiSec = cms.bool(False)

```  
#### Now run the Skimmer
```
cmsRun runFilter_cfg.py
```
You will get as output the Skimmed File L1.root. 

Verify that the filter actually worked!
```
edmFileUtil L1.root
```
## Creating and submiting Jobs on Condor

Create an output directory for your future root files:  
```
mkdir -p /path/to/output/dir
```
Change the nEvents in runFilter_cfg.py to -1
 
Create condor jobs for data by running the cmsCondorData.py script, for MC by running the cmsCondorMC.py(will be available soon). There are 3 mandatory arguments and 3 options:
- The 1st argument is always runFilter_cfg.py.
- The 2nd argument is the path to the top of your CMSSW release.
- The 3rd argument is the path to the output directory for your root files.
- The -n option allows you to set the number of input root files processed per job. 
- The -q option allows you to set the "flavour" of your job. Each flavour corresponds to a different maximum running time. The default is "workday" (= 8h)(It is recommended to set n = 10-20 and flavor=workday to get decent stats for each output root file).
- You can use the -p option to attach your grid proxy to your jobs (specify the path to your proxy after -p).

```
./cmsCondorData.py runFilter_cfg.py <path to your CMSSW src directory> <path to your output directory >  -n 20 -q workday -p /afs/cern.ch/user/<first letter>/<username>/private/x509up_<user proxy>
```
Submit All Jobs on Condor

```
./sub_total.jobb

```
When the Jobs are done, New Filtered Raw root files (containing only events that pass the L1 Trigger with the new menu named L1_0.root, L1_1.root etc) will be produced in the output directory.



### Additional Links
1. [L1 Menu for Run 3](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideL1TriggerMenu#L1_Menus_for_Run_3)








