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
#### After the Initial setup is done, edit the end part of HLTrigger/HLTcore/interface/HLTPreScaleProvider.h like this( needed for CMSSW 11_X and above)

```
HLTPrescaleProvider::HLTPrescaleProvider(edm::ParameterSet const& pset, edm::ConsumesCollector& iC, T& module) {
  unsigned int stageL1Trigger = 2;//pset.getParameter<unsigned int>("stageL1Trigger");                                             
  if (stageL1Trigger <= 1) {
    l1GtUtils_ = std::make_unique<L1GtUtils>(pset, iC, false, module, L1GtUtils::UseEventSetupIn::Run);
  } else {
    l1tGlobalUtil_ = std::make_unique<l1t::L1TGlobalUtil>(pset, iC, module, l1t::UseEventSetupIn::Run);
  }
}
#endif
```
### Customisation of L1Menu 
Here you will be customising the L1 emulator using the your new L1 menu. For more details follow Elisa Fontanesi's L1 emulator tutorial https://indico.cern.ch/event/1060362/contributions/4455932/attachments/2286815/3937192/L1T_Tutorial_Emulator.pdf .
```
★ Check if you have the package L1Trigger/L1TCommon, otherwise add it:
git cms-addpkg L1Trigger/L1TCommon
★ Add the latest L1Trigger/L1TGlobal package:
git cms-addpkg L1Trigger/L1TGlobal
mkdir -p L1Trigger/L1TGlobal/data/Luminosity/startup/
★ Upload the XML file into the directory L1Trigger/L1TGlobal/data/Luminosity/startup/
★ Edit the file L1Trigger/Configuration/python/customiseUtils.py by changing the L1TriggerMenuFile:
process.TriggerMenu.L1TriggerMenuFile = cms.string('L1Menu_Collisions2016_v2c.xml') → L1Menu_Collisions2022_v0_1_1_modified.xml
scram b - j 8
```
#### Now you are ready with the L1 setup.The next step is to git clone the L1Physics Skim repository and run the L1emulation using cmsDriver(eg.running on Zero Bias Data for run 323755).
```
git clone https://github.com/sanuvarghese/L1PhysicsSkim -b master
scram b -j 8
cd L1 PhysicsSkim/L1PhysicsFilter/test/
voms-proxy-init --voms cms --valid 168:00
cp /tmp/x509up_<user proxy> /afs/cern.ch/user/<letter>/<username>/private/
cmsDriver.py l1Ntuple -s RAW2DIGI --python_filename=data.py -n 2000 --no_output --era=Run2_2018 --data --conditions=\
112X_dataRun2_v7 --customise=L1Trigger/Configuration/customiseReEmul.L1TReEmulFromRAW --customise=L1Trigger/L1TNtuples/customiseL1\
Ntuple.L1NtupleRAWEMU --customise=L1Trigger/Configuration/customiseUtils.L1TGlobalMenuXML --customise=L1Trigger/Configuration/cust\
omiseSettings.L1TSettingsToCaloParams_2018_v1_3 --filein=/store/data/Run2018D/EphemeralZeroBias1/RAW/v1/000/323/755/00000/08D7B1A7\
-B8C5-0944-9A69-B698A2BF52EB.root 
```
#### Running the L1Skim (making and submiting Jobs on Condor)
```
./cmsCondorData.py runFilter_cfg.py <path to your CMSSW src directory> <path to your output directory >  -n 1 -q longlunch -p /afs/cern.ch/user/s/savarghe/private/x509up_<user proxy>
```
You can try running one test job locally
```
source Jobs/Job_0/sub_0.sh
```
If everything works out fine, Submit All Jobs on Condor

```
./sub_total.jobb

```

#### When the Jobs are done ,New Filtered Ntuples (containing only events that pass the L1 Trigger with the new menu named L1_0.root, L1_1.root etc) will be produced in the output directory.








