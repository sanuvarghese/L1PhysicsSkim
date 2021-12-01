# L1PhysicsSkim WorkFlow
An L1 Skim config setup to save events that  pass the L1 Physics trigger bits based on the new L1 Menu.  

The targeted environment is Lxplus and assumes a standard session:
```
ssh -XY <username>@lxplus.cern.ch
```

## Environment Setup
Setup the environment according to the official instructions.
```
cmsrel CMSSW_11_2_0
cd CMSSW_11_2_0/src
cmsenv
git cms-init
git remote add cms-l1t-offline git@github.com:cms-l1t-offline/cmssw.git
git fetch cms-l1t-offline l1t-integration-CMSSW_11_2_0
git cms-merge-topic -u cms-l1t-offline:l1t-integration-v106.0
git cms-addpkg L1Trigger/Configuration
git cms-addpkg L1Trigger/L1TMuon
git clone https://github.com/cms-l1t-offline/L1Trigger-L1TMuon.git L1Trigger/L1TMuon/data
git cms-addpkg L1Trigger/L1TCalorimeter
git clone https://github.com/cms-l1t-offline/L1Trigger-L1TCalorimeter.git L1Trigger/L1TCalorimeter/data
git cms-addpkg HLTrigger/HLTcore
git cms-addpkg DataFormats/L1TGlobal
git cms-checkdeps -A -a

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
+ process.TriggerMenu.L1TriggerMenuFile = cms.string('L1Menu_Collisions2022_v0_1_1_modified.xml')

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

#### The L1T emulation is invoked via cmsDriver.py command step from the L1Trigger directory. for more deatils about cmsDriver and its options, follow this [twiki](https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideCmsDriver) .

```
cmsDriver.py l1Ntuple -s RAW2DIGI --python_filename=data.py -n 2000 --no_output --era=Run2_2018 --data --conditions=\
112X_dataRun2_v7 --customise=L1Trigger/Configuration/customiseReEmul.L1TReEmulFromRAW --customise=L1Trigger/L1TNtuples/customiseL1\
Ntuple.L1NtupleRAWEMU --customise=L1Trigger/Configuration/customiseUtils.L1TGlobalMenuXML --customise=L1Trigger/Configuration/cust\
omiseSettings.L1TSettingsToCaloParams_2018_v1_3 --filein=/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/08D7B1A7\
-B8C5-0944-9A69-B698A2BF52EB.root 
```  
Note that our purpose here is not to get the Emulated L1 Ntuples, but to get the data.py config file on which we will apply the L1 Skim Filter(which is why we omitted the --customise=L1Trigger/L1TNtuples/customiseL1Ntuple.L1NtupleRAWEMU option).  

After cmsDriver finishes running, comment out the following lines of the newly created data.py file

```diff
+ # Automatic addition of the customisation function from L1Trigger.L1TNtuples.customiseL1Ntuple                                     
+ #from L1Trigger.L1TNtuples.customiseL1Ntuple import L1NtupleRAWEMU   
+ #call to customisation function L1NtupleRAWEMU imported from L1Trigger.L1TNtuples.customiseL1Ntuple                                
+ #process = L1NtupleRAWEMU(process)  
```
comment out also the lines at the end
```diff 
+ # Add early deletion of temporary data products to reduce peak memory need                                                         
+ #from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete                                          
+ #process = customiseEarlyDelete(process)                                                                                           
+ # End adding early deletion 
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
- The -n option allows you to set the number of input root files processed per job. For data, the default -n 1 is recommended because other values could potentially lead to normalization problems in Step 2.
- The -q option allows you to set the "flavour" of your job. Each flavour corresponds to a different maximum running time. The default is "workday" (= 8h), but "longlunch" (= 2h) is usually enough.
- You can use the -p option to attach your grid proxy to your jobs (specify the path to your proxy after -p). When running over data, this is only necessary if the files aren't available at CERN  

```
./cmsCondorData.py runFilter_cfg.py <path to your CMSSW src directory> <path to your output directory >  -n 1 -q longlunch -p /afs/cern.ch/user/<first letter>/<username>/private/x509up_<user proxy>
```
You can try running one test job locally
```
source Jobs/Job_0/sub_0.sh
```
If everything works out fine, Submit All Jobs on Condor

```
./sub_total.jobb

```
When the Jobs are done, New Filtered Raw root files (containing only events that pass the L1 Trigger with the new menu named L1_0.root, L1_1.root etc) will be produced in the output directory.








