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


#include <sstream>
#include <iomanip> 
#include <utility>
#include <numeric>
#include <vector>
using namespace std;

class PrimVertices : public edm::one::EDAnalyzer<>
{
public:
  //constructor, function is called when new object is created
  explicit PrimVertices(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~PrimVertices();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

private:
  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<reco::Vertex> > thePrimaryVerticesToken;

  // histograms
  TH1I* hNOfPV;
};

PrimVertices::PrimVertices(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  thePrimaryVerticesToken = consumes< vector<reco::Vertex> >(edm::InputTag("offlinePrimaryVertices"));
}

PrimVertices::~PrimVertices()
{
  cout <<" DTOR" << endl;
}

void PrimVertices::beginJob()
{
  hNOfPV = new TH1I("hNOfPV", "# PV", 10, 0, 10);

  cout << "HERE PrimVertices::beginJob()" << endl;
}

void PrimVertices::endJob()
{
  //make a new Root file
  TFile myRootFile(theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  hNOfPV->Write();
  
  myRootFile.Close();

  delete hNOfPV;

  cout << "HERE PrimVertices::endJob()" << endl;
}

void PrimVertices::analyze(const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE PrimVertices::analyze "<< std::endl;

  const std::vector<reco::Vertex> & primVertices = ev.get(thePrimaryVerticesToken);

  hNOfPV->Fill(primVertices.size());

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}


DEFINE_FWK_MODULE(PrimVertices);