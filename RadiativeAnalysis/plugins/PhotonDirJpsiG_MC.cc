// correction of the photon momentum direction

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
#include "DataFormats/PatCandidates/interface/Photon.h"

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

#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"

#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "TrackingTools/TrajectoryParametrization/interface/CartesianTrajectoryError.h"
#include "DataFormats/TrajectoryState/interface/TrackCharge.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/Math/interface/AlgebraicROOTObjects.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

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
class PhotonDirJpsiG_MC : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit PhotonDirJpsiG_MC(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~PhotonDirJpsiG_MC();

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

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;

  std::vector<int> JpsiG = {443, 22};
  std::vector<int> MuMu = {13, -13};

  TNtupleD* tOut;
};


PhotonDirJpsiG_MC::PhotonDirJpsiG_MC(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlinePrimaryVertices"));
  
  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

PhotonDirJpsiG_MC::~PhotonDirJpsiG_MC()
{
  cout <<" DTOR" << endl;
}

void PhotonDirJpsiG_MC::beginJob()
{
  tOut = new TNtupleD("tOut","Output tuple","muonsM:probKalman:probMassConstr:dRstd:dRmod:Mstd:Mmod");
 
  cout << "HERE PhotonDirJpsiG_MC::beginJob()" << endl;
}

void PhotonDirJpsiG_MC::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  tOut->Write();

  myRootFile.Close();

  delete tOut;
  
  cout << "HERE PhotonDirJpsiG_MC::endJob()" << endl;
}

void PhotonDirJpsiG_MC::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE PhotonDirJpsiG_MC::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  
  auto const& field = es.getData(m_fieldToken);  

  
  ////////////////////////////////////////

  if (recoMuons.size() < 2 || recoPhotons.size() < 1)   return;

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

  KalmanVertexFitter kvf(false);
  double jpsiMass = 3.097;
  double bsMass = 5.367;
  const ParticleMass muonMass = 0.105658;
  float muonMassSigma = 2E-6;

  // take both recoMatchedMuons, apply J/psi mass window
  math::XYZTLorentzVector twoMuonsLV = recoMatchedMuons.at(0)->p4() + recoMatchedMuons.at(1)->p4();
  double twoMuonsM = twoMuonsLV.M();

  if (std::fabs(twoMuonsM - jpsiMass) > 0.15) return;

  // create transient tracks
  reco::TrackRef mu1Track = recoMatchedMuons.at(0)->track();
  if (!mu1Track)  return;
  reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);
  reco::TrackRef mu2Track = recoMatchedMuons.at(1)->track();
  if (!mu2Track) return;
  reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);
  
  // get the muons common vertex with Kalman Vertex Fitter
  std::vector<reco::TransientTrack> trackTTs;
  trackTTs.push_back(muon1TT);
  trackTTs.push_back(muon2TT);
  reco::Vertex muonsKalmanVertex (TransientVertex(kvf.vertex(trackTTs)));
  double muonsKalmanVxProb = TMath::Prob(muonsKalmanVertex.chi2(), muonsKalmanVertex.ndof());

  //if (muonsKalmanVxProb < 0.01)  return;

  //////////////J/PSI MASS CONSTRAINED KINEMATIC FIT////////////////////
  // prepare kinematic muons
  std::vector<RefCountedKinematicParticle> muonKinematicParticles;
  KinematicParticleFactoryFromTransientTrack pFactory;
  muonKinematicParticles.push_back(pFactory.particle(muon1TT, muonMass, float(0), float(0), muonMassSigma));
  muonKinematicParticles.push_back(pFactory.particle(muon2TT, muonMass, float(0), float(0), muonMassSigma));

  // create the fitter and the mass constraint
  KinematicConstrainedVertexFitter constrainedFitter;
  MultiTrackMassKinematicConstraint* jpsiMassKinematicConstraint = new MultiTrackMassKinematicConstraint(jpsiMass, 2);  // 2 particles - muons

  // perform the fit
  RefCountedKinematicTree jpsiFitTree = constrainedFitter.fit(muonKinematicParticles, jpsiMassKinematicConstraint);
  if (! jpsiFitTree->isValid()) return;

  // get the fitted J/psi and vertex
  jpsiFitTree->movePointerToTheTop();
  RefCountedKinematicParticle fittedJpsi = jpsiFitTree->currentParticle();
  RefCountedKinematicVertex fittedJpsiVertex = jpsiFitTree->currentDecayVertex();
  if (! fittedJpsiVertex->vertexIsValid())  return;

  double fittedJpsiVxProb = TMath::Prob(fittedJpsiVertex->chiSquared(), fittedJpsiVertex->degreesOfFreedom());

  GlobalPoint position_GP = fittedJpsiVertex->position();
  math::XYZPoint fittedJpsiVertexPosition (position_GP.x(),position_GP.y(),position_GP.z());

  // create a Lorentz Vector for the fitted J/psi
  GlobalVector fittedJpsiMom = fittedJpsi->currentState().globalMomentum();
  double fittedJpsiEnergy = fittedJpsi->currentState().kinematicParameters().energy();
  math::XYZTLorentzVector fittedJpsiLV (fittedJpsiMom.x(),fittedJpsiMom.y(),fittedJpsiMom.z(),fittedJpsiEnergy);

  ///////////STANDARD PHOTON MOMENTUM////////////////
  // add the photon
  math::XYZTLorentzVector photonLV = recoMatchedPhotons.at(0)->p4();
  double deltaR_photon_Jpsi = reco::deltaR(fittedJpsiLV, photonLV);

  // find the best PV, based on fittedJpsiLV + photonLV
  math::XYZTLorentzVector fittedJpsiPhotonLV = fittedJpsiLV + photonLV;
  const reco::Vertex & bestPV = Tools::bestPV(primaryVertices, fittedJpsiVertexPosition, 
                                      fittedJpsiPhotonLV.Vect());

  double fittedJpsiPhotonMass = fittedJpsiPhotonLV.M();

  ///////////MODIFIED PHOTON MOMENTUM/////////////////
  float photonEnergy = recoMatchedPhotons.at(0)->getCorrectedEnergy(recoMatchedPhotons.at(0)->getCandidateP4type());
  const math::XYZPoint sClusterPos = recoMatchedPhotons.at(0)->superCluster()->position();
  math::XYZVector photonMomDir = (sClusterPos - fittedJpsiVertexPosition).unit();
  math::XYZVector photonModMomentum = photonEnergy*photonMomDir;

  math::XYZTLorentzVector modPhotonLV = 
            math::XYZTLorentzVector(photonModMomentum.x(),photonModMomentum.y(),photonModMomentum.z(),photonEnergy);

  double deltaR_modPhoton_Jpsi = reco::deltaR(fittedJpsiLV, modPhotonLV);

  // find the best PV, based on fittedJpsiLV + modPhotonLV
  math::XYZTLorentzVector fittedJpsiModPhotonLV = fittedJpsiLV + modPhotonLV;
  const reco::Vertex & bestPV_modPhoton = Tools::bestPV(primaryVertices, fittedJpsiVertexPosition, 
                                      fittedJpsiModPhotonLV.Vect());

  double fittedJpsiModPhotonMass = fittedJpsiModPhotonLV.M();

  tOut->Fill(twoMuonsM,muonsKalmanVxProb,fittedJpsiVxProb,deltaR_photon_Jpsi,deltaR_modPhoton_Jpsi,fittedJpsiPhotonMass,fittedJpsiModPhotonMass);
}

DEFINE_FWK_MODULE(PhotonDirJpsiG_MC);
