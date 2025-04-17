#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"


#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MMGVertexFit.h"



#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"

#include <sstream>
#include <iomanip> 
#include <utility>
#include <numeric>
#include <vector>


using namespace std;


//object definition
class MMGVertexFitExample : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit MMGVertexFitExample(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~MMGVertexFitExample();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  bool isSameDecay(const std::vector<int>&, const std::vector<int>&);
private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<reco::GenParticle> > theGenParticleToken;
  edm::EDGetTokenT < vector<reco::Muon> > theMuonToken;
  edm::EDGetTokenT < vector<reco::Photon> > thePhotonToken;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;
  edm::ESGetToken <TransientTrackBuilder, TransientTrackRecord> theTransientTrackBuilderToken;

  edm::EDGetTokenT<EcalRecHitCollection> ebRecHitsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> eeRecHitsToken_;

  edm::EDGetTokenT < reco::BeamSpot > theBeamSpotToken;
  edm::EDGetTokenT < vector<reco::Vertex> > theVertexToken;

  unique_ptr<EcalClusterLazyToolsBase::ESGetTokens> esGetTokens;

  // histograms

  TH1D* hBsMass;

  int nConvPhotons = 0;
  std::vector<int> MuMuG = {22, 13, -13};
};


MMGVertexFitExample::MMGVertexFitExample(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();
  theTriggerResultsToken = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
  theTransientTrackBuilderToken = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));
  ebRecHitsToken_ = consumes<EcalRecHitCollection>(edm::InputTag("reducedEcalRecHitsEB"));
  eeRecHitsToken_ = consumes<EcalRecHitCollection>(edm::InputTag("reducedEcalRecHitsEB"));

  esGetTokens = make_unique<EcalClusterLazyToolsBase::ESGetTokens>(consumesCollector());

  theBeamSpotToken = consumes< reco::BeamSpot >( edm::InputTag("offlineBeamSpot"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlinePrimaryVertices"));

}

MMGVertexFitExample::~MMGVertexFitExample()
{
  cout <<" DTOR" << endl;
}

bool MMGVertexFitExample::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}


void MMGVertexFitExample::beginJob()
{
  //create a histogram

  hBsMass = new TH1D("hBsMass", "hBsMass", 50, 3, 7);

  cout << "HERE MMGVertexFitExample::beginJob()" << endl;
}

void MMGVertexFitExample::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data

  hBsMass->Write();
  
  myRootFile.Close();

  delete hBsMass;


  cout << "HERE MMGVertexFitExample::endJob()" << endl;
}


void MMGVertexFitExample::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE MMGVertexFitExample::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  auto const& field = es.getData(m_fieldToken);

  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);

  const TransientTrackBuilder* TTBuilder = &es.getData(theTransientTrackBuilderToken);

  EcalClusterLazyTools lazyTools(ev, esGetTokens->get(es), ebRecHitsToken_, eeRecHitsToken_);

  MMGVertexFit mmgVertexFit;
  std::vector<double> mass = mmgVertexFit.fit(recoMuons, recoPhotons, lazyTools, primaryVertices, TTBuilder);

  for (double m : mass) {
    hBsMass->Fill(m);
  }
  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(MMGVertexFitExample);

