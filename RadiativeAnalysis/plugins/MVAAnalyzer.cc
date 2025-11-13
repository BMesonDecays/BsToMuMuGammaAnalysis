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


#include "BsToMuMuGammaAnalysis/run3mvaid/interface/MuonMVAID.h"


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
class MVAAnalyzer : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit MVAAnalyzer(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~MVAAnalyzer();

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
  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;


  // histograms

  TH1D* hMVASCore;

  int nConvPhotons = 0;
  std::vector<int> MuMuG = {22, 13, -13};
};


MVAAnalyzer::MVAAnalyzer(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  theTriggerResultsToken = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
}

MVAAnalyzer::~MVAAnalyzer()
{
  cout <<" DTOR" << endl;
}

bool MVAAnalyzer::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}


void MVAAnalyzer::beginJob()
{
  //create a histogram

  hMVASCore = new TH1D("hMVASCore", "MVA Score for Muons", 100, 0, 1);

  cout << "HERE MVAAnalyzer::beginJob()" << endl;
}

void MVAAnalyzer::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data

  hMVASCore->Write();
  
  myRootFile.Close();

  delete hMVASCore;


  cout << "HERE MVAAnalyzer::endJob()" << endl;
}


void MVAAnalyzer::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE MVAAnalyzer::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);

  vector<const reco::Candidate*> genMuons;
  vector<reco::Muon> recoMatchedMuons;
  vector<const reco::Candidate*> genMatchedMuons;



  for(const auto& genP : genPar)
  {
    if (abs(genP.pdgId()) == 531)
    {
      vector<int> daughters;
      for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
      {
        daughters.push_back(genP.daughter(i)->pdgId());
      }
      if(isSameDecay(daughters, MuMuG))
      {
        for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
        {
          if(abs(genP.daughter(i)->pdgId()) == 13) genMuons.push_back(genP.daughter(i));
        }

      }
    }
  }



 for (const reco::Candidate* genMu : genMuons)
  {
    float minDR = 10;
    const reco::Muon* bestMatchedMuon;
    bool matched = false;
    for (const auto& recoMu : recoMuons)
    {
      float dR = reco::deltaR(recoMu, *genMu);
      if (dR < minDR)
      {
        minDR = dR;
        bestMatchedMuon = &recoMu;
        matched = true;
      }
    }
    // if (matched) hMuDeltaR->Fill(minDR);
    if (matched && minDR < 0.01)
    {
      recoMatchedMuons.push_back(*bestMatchedMuon);
      genMatchedMuons.push_back(genMu);
      // hRecoVsGenMuPt->Fill(genMu->pt(), bestMatchedMuon->pt());
      // hMuPtError->Fill((bestMatchedMuon->pt() - genMu->pt())/genMu->pt());
    }
  }

  MuonMVAID muonMVAID(theConfig);
  vector<float> mvaScores = muonMVAID.produce(recoMatchedMuons);

  for(unsigned int i=0; i < recoMatchedMuons.size(); i++)
  {
    hMVASCore->Fill(mvaScores.at(i));
  }

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(MVAAnalyzer);

