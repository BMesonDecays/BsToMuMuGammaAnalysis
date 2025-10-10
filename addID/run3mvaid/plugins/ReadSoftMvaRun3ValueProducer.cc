#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include <iostream>
#include <vector>

class ReadSoftMvaRun3ValueProducer : public edm::stream::EDAnalyzer<> {
public:
  explicit ReadSoftMvaRun3ValueProducer(const edm::ParameterSet&);
  ~ReadSoftMvaRun3ValueProducer() override = default;

  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  edm::EDGetTokenT<std::vector<pat::Muon>> muonsToken_;
};

ReadSoftMvaRun3ValueProducer::ReadSoftMvaRun3ValueProducer(const edm::ParameterSet& iConfig)
    : muonsToken_(consumes<std::vector<pat::Muon>>(iConfig.getParameter<edm::InputTag>("src"))) {}

void ReadSoftMvaRun3ValueProducer::analyze(const edm::Event& iEvent, const edm::EventSetup&) {
  // Retrieve the slimmedMuons collection
  edm::Handle<std::vector<pat::Muon>> muons;
  iEvent.getByToken(muonsToken_, muons);

  if (!muons.isValid()) {
    edm::LogError("ReadSoftMvaRun3ValueProducer") << "Muon collection not found!";
    return;
  }

  // Loop over the muons and read the 'tempt' variable
  for (const auto& muon : *muons) {
    if (muon.hasUserFloat("run3muonmva")) {  // Check if 'tempt' exists
         //float value = muon.softMvaRun3Value();
         float temmva = muon.userFloat("run3muonmva");
         std::cout << " custom mva=" << temmva << std::endl;
    } else {
      std::cout << "Muon pt: " << muon.pt() << " GeV, run3muonmva: <not available>" << std::endl;
    }
  }
}

// Define this as a plug-in
DEFINE_FWK_MODULE(ReadSoftMvaRun3ValueProducer);

