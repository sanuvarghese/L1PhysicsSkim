//         Created:  Wed, 15 Dec 2021 17:36:13 GMT
//         Original Author: Sanu Varghese
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include<vector>
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
// class declaration
//

class L1PhysicsFilter : public edm::stream::EDFilter<> {
private:
    
  std::string hltProcess_; //name of HLT process, usually "HLT"
  std::vector<unsigned int> triggerCounts_; // Counter for each trigger bit 
  
public:
  explicit L1PhysicsFilter(const edm::ParameterSet&);
  ~L1PhysicsFilter();

  //  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginRun(const edm::Run& ,const edm::EventSetup& ) override;
  //virtual void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup);
  virtual bool filter(edm::Event&, const edm::EventSetup&) override;
  //GlobalAlgBlk const *results_;
  const edm::EDGetTokenT<GlobalAlgBlkBxCollection> ugt_token_;
  //  unsigned long long cache_id_;
};
L1PhysicsFilter::L1PhysicsFilter(const edm::ParameterSet& iConfig):
  hltProcess_(iConfig.getParameter<std::string>("hltProcess")),
  //results_(nullptr),
  ugt_token_(consumes<GlobalAlgBlkBxCollection>(iConfig.getParameter<edm::InputTag>("ugtToken")))
  //cache_id_(0)
{
triggerCounts_.resize(512, 0);
}
L1PhysicsFilter::~L1PhysicsFilter() {
  // Print the trigger counts at the end                                                                                                  
  for (size_t i = 0; i < triggerCounts_.size(); ++i) {
    std::cout << "Trigger " << i << " fired " << triggerCounts_[i] << " times." << std::endl;
  }
}
void L1PhysicsFilter::beginRun(const edm::Run& run,const edm::EventSetup& setup)
{
}
//bool L1PhysicsFilter::filter(edm::Event const &event, edm::EventSetup const &setup) { 
bool L1PhysicsFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)  {

  // using namespace edm;
  bool passEvents = false;
  //unsigned long long id = iSetup.get<L1TUtmTriggerMenuRcd>().cacheIdentifier();
  // if (id != cache_id_) {
  //   cache_id_ = id;
  //   edm::ESHandle<L1TUtmTriggerMenu> menu;
  //  iSetup.get<L1TUtmTriggerMenuRcd>().get(menu);
  edm::Handle<GlobalAlgBlkBxCollection> ugt;
  iEvent.getByToken(ugt_token_, ugt);
  const GlobalAlgBlk* L1uGT(nullptr);
    if (ugt.isValid()) {
    L1uGT = &ugt->at(0, 0);
    //cout<<"The ugtValid is working"<<endl;
  }
  //  }
  //GlobalAlgBlk* L1uGT = new GlobalAlgBlk();

  //bool passEvents = false;
    if(L1uGT != 0){
      //cout<<"The pointer is working"<<endl;
      std::vector<bool> m_algoDecisionFinal = L1uGT->getAlgoDecisionFinal();
      // // cout<<m_algoDecisionFinal<<endl;
      for(size_t s = 0; s < m_algoDecisionFinal.size(); s++){
	//   //  cout<<m_algoDecisionFinal[s]<<endl;
	if(m_algoDecisionFinal[s] > 0){
	  passEvents = true;
	  triggerCounts_[s]++; 
	  //	  cout<<"success"<<" "<<s<<endl; 
	           break;
	}
      }          
    }
//   }
   return passEvents;

}

  
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
DEFINE_FWK_MODULE(L1PhysicsFilter);
