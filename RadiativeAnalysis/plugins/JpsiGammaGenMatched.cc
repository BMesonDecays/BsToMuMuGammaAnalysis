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

#include "DataFormats/Math/interface/deltaR.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoVertex/KinematicFitPrimitives/interface/ParticleMass.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MultiTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"


#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "TrackingTools/TrajectoryParametrization/interface/CartesianTrajectoryError.h"
#include "DataFormats/TrajectoryState/interface/TrackCharge.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/Math/interface/AlgebraicROOTObjects.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"


#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Math/interface/LorentzVector.h"

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Tools.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "Math/Vector3D.h"
#include "Math/SMatrix.h"
#include "Math/SVector.h"
#include "TTree.h"
#include "TBranch.h"
#include "TNtupleD.h"


#include <sstream>
#include <iomanip> 
#include <utility>
#include <numeric>


using namespace std;



//object definition
class JpsiGammaGenMatched : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiGammaGenMatched(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiGammaGenMatched();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<reco::GenParticle> > theGenParticleToken;
  edm::EDGetTokenT < vector<reco::Muon> > theMuonToken;
  edm::EDGetTokenT < vector<reco::Photon> > thePhotonToken;
  edm::EDGetTokenT < vector<reco::Vertex> > theVertexToken; 
  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;

  std::vector<int> JpsiG = {443, 22};
  std::vector<int> MuMu = {13, -13};

  TNtupleD* tMuMuGamma;

};


JpsiGammaGenMatched::JpsiGammaGenMatched(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlinePrimaryVertices"));
  theTriggerResultsToken = consumes< edm::TriggerResults > (edm::InputTag("TriggerResults","","HLT"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

JpsiGammaGenMatched::~JpsiGammaGenMatched()
{
  cout <<" DTOR" << endl;
}

void JpsiGammaGenMatched::beginJob()
{
    tMuMuGamma = new TNtupleD("tMuMuGamma","tMuMuGamma","M_dimuon:ProbOfCommonMuonVertex:eta_photon:deltaR_dimuon_photon:minPCA_distance:M_dimuonGamma:minFlightPath");
    
    cout << "HERE JpsiGammaGenMatched::beginJob()" << endl;
}

void JpsiGammaGenMatched::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  tMuMuGamma->Write();

  myRootFile.Close();

  delete tMuMuGamma;
  
  cout << "HERE JpsiGammaGenMatched::endJob()" << endl;
}

void JpsiGammaGenMatched::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiGammaGenMatched::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);

  // trigger info
  const edm::TriggerResults & triggerResults = ev.get(theTriggerResultsToken);
  edm::TriggerNames triggerNames = ev.triggerNames(triggerResults);
  
  auto const& field = es.getData(m_fieldToken);  

  //////////////////BEGINNING OF THE GENPARTICLE SECTION//////////////////////

  vector<const reco::Candidate*> genMuons;
  vector<const reco::Muon*> recoMatchedMuons;
  vector<const reco::Candidate*> genMatchedMuons;

  vector<const reco::Candidate*> genPhotons;
  vector<const reco::Photon*> recoMatchedPhotons;
  vector<const reco::Candidate*> genMatchedPhotons;

  const reco::Candidate* genJpsiPtr = &genPar.at(0); // generated Jpsi
  const reco::Candidate* genGammaPtr = &genPar.at(0);  // generated photon
  
  // find B0s decaying into Jpsi(MuMu) + gamma
  // fill genMuons with muons from Jpsi decay
  for(const auto& genP : genPar)
  {
    if (abs(genP.pdgId()) == 531) // B0s
    {
      vector<int> daughters;
      for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
      {
        if (genP.daughter(i)->pdgId() == 443)  {genJpsiPtr = genP.daughter(i);}
        if (genP.daughter(i)->pdgId() == 22)  {genGammaPtr = genP.daughter(i);}
        daughters.push_back(genP.daughter(i)->pdgId());
      }
      if(Tools::isSameDecay(daughters, JpsiG))
      {
        vector<int> JpsiDaughters;
        for (unsigned int i=0; i < genJpsiPtr->numberOfDaughters(); i++)
        {
          JpsiDaughters.push_back(genJpsiPtr->daughter(i)->pdgId());
        }

        if(Tools::isSameDecay(JpsiDaughters, MuMu)) // Jpsi -> MuMu ?
        {
          genMuons.push_back(genJpsiPtr->daughter(0));
          genMuons.push_back(genJpsiPtr->daughter(1));
          genPhotons.push_back(genGammaPtr);
          break;  // assert that only one B0s -> Jpsi(MuMu)Gamma decay per event
        }

      }
    }
  }
  if(genPhotons.size() == 0) return;  //no 'SameDecay' found  

  if(recoMuons.size() < 2 || recoPhotons.size() < 1) return;  // insufficient number of reco muons and photons
  
  // reco muon matching
  for (const reco::Candidate* genMu : genMuons)
  {
    float minDR = 10;
    const reco::Muon* bestMatchedMuon = &recoMuons.at(0); //initialization to suppress warning
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
    if (matched && minDR < 0.01)  //cut on dR(reco,gen)
    {
      recoMatchedMuons.push_back(bestMatchedMuon);
      genMatchedMuons.push_back(genMu);
    }
  }

  // reco photon matching
  for (const reco::Candidate* genPh : genPhotons)
  {
    float minDR = 10;
    const reco::Photon* bestMatchedPhoton = &recoPhotons.at(0); //initialization to suppress warning
    bool matched = false;
    for (const auto& recoPh : recoPhotons)
    {
      float dR = reco::deltaR(recoPh, *genPh);
      if (dR < minDR)
      {
        minDR = dR;
        bestMatchedPhoton = &recoPh;
        matched = true;
      }
    }
    if (matched && minDR < 0.03)  // cut on dR(reco,gen)
    {
      recoMatchedPhotons.push_back(bestMatchedPhoton);
      genMatchedPhotons.push_back(genPh);
    }
  }
  if(recoMatchedMuons.size() != 2)  return;
  if(recoMatchedPhotons.size() != 1) return;


  ///////////////END OF THE GENPARTICLE SECTION/////////////////////////

  const ParticleMass muon_mass(0.105658);
  float muon_sigma = 23E-10;
  KinematicParticleFactoryFromTransientTrack pFactory;
  KinematicParticleVertexFitter fitter;

  // take two recoMatchedMuons
  reco::TrackRef mu1Track = recoMatchedMuons.at(0)->track();
  if (!mu1Track)  return;
  reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);

  reco::TrackRef mu2Track = recoMatchedMuons.at(1)->track();
  if (!mu2Track) return;
  reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);

  // fit the vertex of two muons using the KinematicFitter
  std::vector<RefCountedKinematicParticle> muonKinematicParticles;
  muonKinematicParticles.push_back(pFactory.particle(muon1TT, muon_mass, float(0), float(0), muon_sigma));
  muonKinematicParticles.push_back(pFactory.particle(muon2TT, muon_mass, float(0), float(0), muon_sigma));
  if (muonKinematicParticles.size() != 2) return;
  RefCountedKinematicTree vertexFitTree = fitter.fit(muonKinematicParticles); // was "allParticles"
  if (!vertexFitTree->isValid()) return;

  // get the fitted particle (i.e. dimuon) and vertex
  vertexFitTree->movePointerToTheTop();
  RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
  RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
  if (!fitVertex->vertexIsValid()) return;
  double commonVertexProb = TMath::Prob(fitVertex->chiSquared(), fitVertex->degreesOfFreedom());  // potential problem: degreesOfFreedom type
  if (commonVertexProb < 0.1) return; // cut on the common vertex prob. of two muons

  double M_dimuon = fitParticle->currentState().mass(); // invariant mass of the fitted dimuon

  GlobalPoint fittedGlobalPoint = fitVertex->position();
  math::XYZPoint fittedSV (fittedGlobalPoint.x(),fittedGlobalPoint.y(),fittedGlobalPoint.z());

  KinematicParameters dimuonKinPar = fitParticle->currentState().kinematicParameters();
  GlobalVector dimuonMomentum = dimuonKinPar.momentum();
  math::XYZTLorentzVector lFitDimuonVector (dimuonMomentum.x(),dimuonMomentum.y(),dimuonMomentum.z(), dimuonKinPar.energy());

  // take a photon
  double photonCaloEta = recoMatchedPhotons.at(0)->superCluster()->eta();

  math::XYZTLorentzVector lRecoGammaVector = recoMatchedPhotons.at(0)->p4();
  double deltaR_fitDimuon_recoPhoton = reco::deltaR(lFitDimuonVector, lRecoGammaVector);

  // consider the closest approach distance of fitDimuonRecoPhoton to the best (closest) recoPV
  math::XYZTLorentzVector lFitDimuonRecoPhotonVector = lFitDimuonVector + lRecoGammaVector;

  std::vector<math::XYZPoint> closestPoints = Tools::points_PV_pca (primaryVertices, fittedSV,
                                                                    lFitDimuonRecoPhotonVector.Vect());
  auto bestPVposition = closestPoints.at(0);
  auto bestPCA = closestPoints.at(1);
        
  double closestAppDistance = TMath::Sqrt((bestPVposition-bestPCA).Mag2());
  double M_dimuonGamma = lFitDimuonRecoPhotonVector.M();
  double minFlightPath = Tools::minDistPV(fittedSV, primaryVertices);

  // fill the Ntuple
  tMuMuGamma->Fill(M_dimuon, commonVertexProb, photonCaloEta, deltaR_fitDimuon_recoPhoton, closestAppDistance, M_dimuonGamma, minFlightPath);

  /*
  // print the trigger info
  for (unsigned int i=0; i < triggerResults.size();i++)
  {
    if (triggerResults.accept(i))
      std::cout << triggerNames.triggerName(i) << std::endl;
  }
  */


  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(JpsiGammaGenMatched);
