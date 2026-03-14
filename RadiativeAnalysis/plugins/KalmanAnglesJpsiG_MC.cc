// Analyzer to check angle distributions in MC using only simple Kalman Vertex Fitter

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
class KalmanAnglesJpsiG_MC : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit KalmanAnglesJpsiG_MC(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~KalmanAnglesJpsiG_MC();

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
  edm::EDGetTokenT < vector<reco::PFCandidate> > thePackedCandidateToken;

  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
  edm::EDGetTokenT < reco::BeamSpot > theBeamSpotToken;

  std::vector<int> JpsiG = {443, 22};
  std::vector<int> MuMu = {13, -13};

  TNtupleD* tOut;
  
  // HLT paths
  std::string path0 = "HLT_DoubleMu2_Jpsi_LowPt_v";
  std::string path1 = "HLT_DoubleMu4_3_Jpsi_v";
  std::string path2 = "HLT_DoubleMu4_3_LowMass_v";
  std::string path3 = "HLT_DoubleMu4_JpsiTrkTrk_Displaced_v";
  std::string path4 = "HLT_DoubleMu4_Jpsi_Displaced_v";
  std::string path5 = "HLT_DoubleMu4_Jpsi_NoVertexing_v";
  std::string path6 = "HLT_DoubleMu4_LowMass_Displaced_v";
  std::string path7 = "HLT_DoubleMu4_MuMuTrk_Displaced_v";

};

KalmanAnglesJpsiG_MC::KalmanAnglesJpsiG_MC(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlinePrimaryVertices"));
  thePackedCandidateToken = consumes< vector<reco::PFCandidate> >(edm::InputTag("particleFlow"));
  theTriggerResultsToken = consumes< edm::TriggerResults > (edm::InputTag("TriggerResults","","HLT"));
  theBeamSpotToken = consumes< reco::BeamSpot >( edm::InputTag("offlineBeamSpot"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

KalmanAnglesJpsiG_MC::~KalmanAnglesJpsiG_MC()
{
  cout <<" DTOR" << endl;
}

void KalmanAnglesJpsiG_MC::beginJob()
{
    tOut = new TNtupleD("tOut","Output tuple",
        "muonsKalmanVxProb:twoMuonsM:lXY_muonsKalman_bSpot:lXY_muonsKalman_bSpot_significance:cosPointingAngleTwoMuonsBS2D:muon1VtxDisplVal:muon2VtxDisplVal:muon1VtxDisplVal2D:muon2VtxDisplVal2D:dimuonMassDiff:deltaR_photon_dimuon:photonPathLength:photonEnergy:photonDirChange:photonDirChange2D:triggerRes");

  cout << "HERE KalmanAnglesJpsiG_MC::beginJob()" << endl;
}

void KalmanAnglesJpsiG_MC::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write data
  tOut->Write();

  myRootFile.Close();

  delete tOut;

  cout << "HERE KalmanAnglesJpsiG_MC::endJob()" << endl;
}

void KalmanAnglesJpsiG_MC::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE KalmanAnglesJpsiG_MC::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  const std::vector<reco::PFCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);

  auto const& field = es.getData(m_fieldToken);  

  // trigger info
  const edm::TriggerResults & triggerResults = ev.get(theTriggerResultsToken);
  edm::TriggerNames triggerNames = ev.triggerNames(triggerResults);
  
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

  KalmanVertexFitter kvf(true);
  double jpsiMass = 3.097;
  double bsMass = 5.367;

  // take both recoMatchedMuons
  if (recoMatchedMuons.at(0)->charge() * recoMatchedMuons.at(1)->charge() != -1)  return;

  reco::TrackRef mu1Track = recoMatchedMuons.at(0)->track();
  if (!mu1Track)  return;
  reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);
  reco::TrackRef mu2Track = recoMatchedMuons.at(1)->track();
  if (!mu2Track) return;
  reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);

  std::vector<reco::TransientTrack> trackTTs;
  trackTTs.push_back(muon1TT);
  trackTTs.push_back(muon2TT);

  // fit a common muons vertex
  TransientVertex muonsKalmanTVertex = kvf.vertex(trackTTs);
  double muonsKalmanVxProb = TMath::Prob(muonsKalmanTVertex.totalChiSquared(), muonsKalmanTVertex.degreesOfFreedom());
  if (muonsKalmanVxProb < 0.1)  return;

  // twoMuons mass
  math::XYZTLorentzVector twoMuonsLV = recoMatchedMuons.at(0)->p4() + recoMatchedMuons.at(1)->p4();
  double twoMuonsM = twoMuonsLV.M();
  if (std::fabs(twoMuonsM - jpsiMass) > 0.15) return;

  // get the momentum of the refitted tracks
  if (! muonsKalmanTVertex.hasRefittedTracks())  return;
  reco::TransientTrack muon1TTrefit = muonsKalmanTVertex.refittedTrack(muon1TT);
  reco::TransientTrack muon2TTrefit = muonsKalmanTVertex.refittedTrack(muon2TT);
  math::XYZVector refitDimuonMom = muon1TTrefit.track().momentum() + muon2TTrefit.track().momentum();

  // lXY to the beam spot with significance
  reco::Vertex muonsKalmanVertex (muonsKalmanTVertex);
  std::vector<double> lXY_muonsKalman_bSpot_Vec = Tools::beamSpotVertexLxy(beamSpot, muonsKalmanVertex);
  double lXY_muonsKalman_bSpot = lXY_muonsKalman_bSpot_Vec.at(0);
  double lXY_muonsKalman_bSpot_significance = lXY_muonsKalman_bSpot / lXY_muonsKalman_bSpot_Vec.at(1);
  
  // pointing angle to the beam spot in 2D
  const math::XYZPoint bSpotPoint = beamSpot.position(muonsKalmanVertex.z());
  math::XYZVector displ3DVec = muonsKalmanVertex.position() - bSpotPoint;
  math::XYZVector displ2DVec (displ3DVec.x(), displ3DVec.y(), 0);
  math::XYZVector refitDimuonMom2D (refitDimuonMom.x(),refitDimuonMom.y(),0.0);
  double cosPointingAngleTwoMuonsBS2D = displ2DVec.unit().Dot(refitDimuonMom2D.unit());

  // displacement of the muons vertex position after the common vertex constraint
  math::XYZPoint muon1Vtx = recoMatchedMuons.at(0)->vertex();
  math::XYZPoint muon2Vtx = recoMatchedMuons.at(1)->vertex();
  math::XYZVector muon1VtxDispl = muon1Vtx - muonsKalmanVertex.position();
  math::XYZVector muon2VtxDispl = muon2Vtx - muonsKalmanVertex.position();
  double muon1VtxDisplVal = TMath::Sqrt(muon1VtxDispl.Mag2());
  double muon2VtxDisplVal = TMath::Sqrt(muon2VtxDispl.Mag2());
  double muon1VtxDisplVal2D = TMath::Sqrt(muon1VtxDispl.Perp2());
  double muon2VtxDisplVal2D = TMath::Sqrt(muon2VtxDispl.Perp2());

  // modified dimuon LV
  math::XYZTLorentzVector refitDimuonLV (refitDimuonMom.x(),refitDimuonMom.y(),refitDimuonMom.z(),twoMuonsLV.energy());
  double dimuonMassDiff = refitDimuonLV.M() - twoMuonsLV.M();


  //////PHOTON/////////
  
  // photon deltaR
  math::XYZTLorentzVector photonLV = recoMatchedPhotons.at(0)->p4();
  double deltaR_photon_dimuon = reco::deltaR(refitDimuonMom, photonLV.Vect());

  // photon 'path length'
  const math::XYZPoint clusterPos = recoMatchedPhotons.at(0)->superCluster()->position();
  math::XYZVector photonPath = clusterPos - primaryVertices.at(0).position();
  double photonPathLength = TMath::Sqrt(photonPath.Mag2());

  // photon direction change
  math::XYZVector realPhotonPath = clusterPos - muonsKalmanVertex.position();
  double photonDirChange = realPhotonPath.unit().Dot(photonPath.unit());
  // XY 2D plane
  math::XYZVector photonPath2D (photonPath.x(),photonPath.y(),0.0);
  math::XYZVector realPhotonPath2D (realPhotonPath.x(),realPhotonPath.y(),0.0);
  double photonDirChange2D = realPhotonPath2D.unit().Dot(photonPath2D.unit());
  
  // photon energy
  double photonEnergy = photonLV.energy();


  // HLT paths
  double triggerRes = 0.0;
  for (unsigned int i=0; i < triggerResults.size();i++)
  {
    if (!triggerResults.accept(i))  continue;
    std::string name = triggerNames.triggerName(i);
    
    if (!name.compare(0,26,path0))  triggerRes += 1.E0;
    else if (!name.compare(0,22,path1))  triggerRes += 1.E1;
    else if (!name.compare(0,25,path2))  triggerRes += 1.E2;
    else if (!name.compare(0,36,path3))  triggerRes += 1.E3;
    else if (!name.compare(0,30,path4))  triggerRes += 1.E4;
    else if (!name.compare(0,32,path5))  triggerRes += 1.E5;
    else if (!name.compare(0,33,path6))  triggerRes += 1.E6;
    else if (!name.compare(0,33,path7))  triggerRes += 1.E7;      
  }

  const double outArray[16] = {muonsKalmanVxProb,twoMuonsM,lXY_muonsKalman_bSpot,lXY_muonsKalman_bSpot_significance,
    cosPointingAngleTwoMuonsBS2D,muon1VtxDisplVal,muon2VtxDisplVal,muon1VtxDisplVal2D,muon2VtxDisplVal2D,dimuonMassDiff,
    deltaR_photon_dimuon,photonPathLength,photonEnergy,photonDirChange,photonDirChange2D,triggerRes};


  tOut->Fill(outArray);



  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(KalmanAnglesJpsiG_MC);