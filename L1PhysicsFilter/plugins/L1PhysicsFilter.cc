//         Created:  Wed, 29 Sep 2021 09:58:17 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
//#include "L1Trigger/GlobalTriggerAnalyzer/interface/L1GtUtils.h"
//
// class declaration
//

class L1PhysicsFilter : public edm::stream::EDFilter<> {
private:
  
  HLTPrescaleProvider hltPSProv_;  
  std::string hltProcess_; //name of HLT process, usually "HLT"
public:
  explicit L1PhysicsFilter(const edm::ParameterSet&);
  ~L1PhysicsFilter(){};

  //  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  //  virtual void beginStream(edm::StreamID) override;
  //virtual void beginJob(){}
  virtual void beginRun(const edm::Run& run,const edm::EventSetup& iSetup) override;
  //virtual void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup);
  virtual bool filter(edm::Event&, const edm::EventSetup&) override;
  // virtual void endStream() override;
  //virtual void endJob(){}
};
  //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
L1PhysicsFilter::L1PhysicsFilter(const edm::ParameterSet& iConfig):

  hltPSProv_(iConfig,consumesCollector(),*this), //it needs a referernce to the calling module for some reason, hence the *this
  hltProcess_(iConfig.getParameter<std::string>("hltProcess"))
{

}
void L1PhysicsFilter::beginRun(const edm::Run& run,const edm::EventSetup& setup)
{
  bool changed=false;
  hltPSProv_.init(run,setup,hltProcess_,changed);
  const l1t::L1TGlobalUtil& l1GtUtils = hltPSProv_.l1tGlobalUtil();
  std::cout <<"l1 menu "<<l1GtUtils.gtTriggerMenuName()<<" version "<<l1GtUtils.gtTriggerMenuVersion()<<" comment "<<std::endl;
  std::cout <<"hlt name "<<hltPSProv_.hltConfigProvider().tableName()<<std::endl;
}
 bool L1PhysicsFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    //I seem to recall this function being slow so perhaps cache for a given lumi
    //(it only changes on lumi boundaries)
    int psColumn = hltPSProv_.prescaleSet(iEvent,iSetup);
    std::cout <<"PS column "<<psColumn<<std::endl;
    if(psColumn==0 && iEvent.isRealData()){
      std::cout <<"PS column zero detected for data, this is unlikely (almost all triggers are disabled in normal menus here) and its more likely that you've not loaded the correct global tag in "<<std::endl;
    }
//using namespace edm;
    l1t::L1TGlobalUtil& l1GtUtils = const_cast<l1t::L1TGlobalUtil&> (hltPSProv_.l1tGlobalUtil());
    std::cout <<"l1 menu: name decisions prescale "<<std::endl;
    bool passEvents = false;
    for(size_t bitNr=0;bitNr<l1GtUtils.decisionsFinal().size();bitNr++){
      if(bitNr >= 458) continue;
      //const std::string& bitName = l1GtUtils.decisionsFinal()[bitNr].first; // l1GtUtils.decisionsFinal() is of type std::vector<std::pair<std::string,bool> >
      
      bool passFinal = l1GtUtils.decisionsFinal()[bitNr].second; //after masks & prescales, true means it gives a L1 accept to the HLT
      if(passFinal) passEvents = true;
      //      int prescale = l1GtUtils.prescales()[bitNr].second;
      //     if(bitNr<458){
      //      std::cout <<"   "<<bitNr<<" "<<bitName<<" "<<passFinal<<std::endl;
		 //      }
    
       }
      return passEvents;  
 }

  
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
DEFINE_FWK_MODULE(L1PhysicsFilter);
