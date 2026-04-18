// JpsiGStartApril_MC with pT balance at PV

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
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/PhotonEnergyModifier.h"
#include "BsToMuMuGammaAnalysis/run3mvaid/interface/MuonMVAID.h"

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
class JpsiGMidApril_MC : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiGMidApril_MC(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiGMidApril_MC();

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

  MuonMVAID* muonMVAIDProducer_;

  std::vector<int> JpsiG = {443, 22};
  std::vector<int> MuMu = {13, -13};

  TNtupleD* tOut;
  TNtupleD* tPtBalance;

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

JpsiGMidApril_MC::JpsiGMidApril_MC(const edm::ParameterSet& conf)
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

JpsiGMidApril_MC::~JpsiGMidApril_MC()
{
  cout <<" DTOR" << endl;
}

void JpsiGMidApril_MC::beginJob()
{
  muonMVAIDProducer_ = new MuonMVAID(theConfig);

  tOut = new TNtupleD("tOut","Output tuple",
  "fittedDimuonVertexProb:fittedDimuonMass:maxMuonsVertexComp:lXY_fittedDimuon_bSpot:lXY_fittedDimuon_bSpot_sig:dR_photonFittedDimuon:etaPhoton:initPhotonEnergy:modScale:candBsMass:candBsModMass:cosAngleBsBSpot2D:cosAngleModBsBSpot2D:l3D_BsPV:l3D_ModBsPV:cosAngleBsPV3D:cosAngleModBsPV3D:lifetimeBs:lifetimeModBs:muon1Id:muon2Id:tight1:tight2:dEta:cosdPhi:cosAnDimuonBSpot2D:pTdimuon:pTphoton:etaDimuon:triggerRes:tight1selPV:tight2selPV");

  tPtBalance = new TNtupleD("tPtBalance","pT balance at PV",
  "pTMiss:pTMissMod:cosDimuonPtMiss:cosDimuonPtMissMod:cosCandBsPtMiss:cosCandBsPtMissMod:pTMissMinusDimuon:pTMissModMinusDimuon:pTMissMinusDimuonMinusPhoton:pTMissMinusDimuonMinusPhotonMod");

  cout << "HERE JpsiGMidApril_MC::beginJob()" << endl;
}

void JpsiGMidApril_MC::endJob()
{
  delete muonMVAIDProducer_;

  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write data
  tOut->Write();
  tPtBalance->Write();

  myRootFile.Close();

  delete tOut;
  delete tPtBalance;

  cout << "HERE JpsiGMidApril_MC::endJob()" << endl;
}

void JpsiGMidApril_MC::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiGMidApril_MC::analyze "<< std::endl;

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

  KalmanVertexFitter kvf(false);
  KinematicParticleFactoryFromTransientTrack pFactory;
  KinematicParticleVertexFitter kinVertexFitter;
  double jpsiMass = 3.097;
  double bsMass = 5.367;
  const ParticleMass muonMass = 0.105658;
  float muonMassSigma = 2E-6;

  // take both recoMatchedMuons
  const reco::Muon* & im1 = recoMatchedMuons.at(0);
  const reco::Muon* & im2 = recoMatchedMuons.at(1);

  reco::TrackRef mu1Track = im1->track();
  if (!mu1Track)  return;
  reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);
  RefCountedKinematicParticle muon1KinPar = pFactory.particle(muon1TT, muonMass, float(0), float(0), muonMassSigma);

  if (im1->charge() * im2->charge() != -1)  return;

  reco::TrackRef mu2Track = im2->track();
  if (!mu2Track) return;
  reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);
  RefCountedKinematicParticle muon2KinPar = pFactory.particle(muon2TT, muonMass, float(0), float(0), muonMassSigma);

  // fit the two muons common vertex
  std::vector<RefCountedKinematicParticle> muonKinematicParticles = {muon1KinPar, muon2KinPar};
  RefCountedKinematicTree twoMuonsFitTree = kinVertexFitter.fit(muonKinematicParticles);
  if (! twoMuonsFitTree->isValid())  return;

  // get the fitted decay vertex
  twoMuonsFitTree->movePointerToTheTop();
  RefCountedKinematicVertex fittedDimuonVertex = twoMuonsFitTree->currentDecayVertex();
  if (! fittedDimuonVertex->vertexIsValid())  return;      
  double fittedDimuonVertexProb = TMath::Prob(fittedDimuonVertex->chiSquared(),fittedDimuonVertex->degreesOfFreedom());
  if (fittedDimuonVertexProb < 0.01)  return;
  GlobalPoint dmuVPosGP = fittedDimuonVertex->position();
  math::XYZPoint fittedDimuonVertexPoint (dmuVPosGP.x(),dmuVPosGP.y(),dmuVPosGP.z());

  // get the fitted dimuon state
  RefCountedKinematicParticle fittedDimuon = twoMuonsFitTree->currentParticle();
  double fittedDimuonMass = fittedDimuon->currentState().mass();
  //if (std::fabs(fittedDimuonMass - jpsiMass) > 0.15) return;      

  // construct the fittedDimuon Lorentz Vector
  GlobalVector fittedDimuonMom = fittedDimuon->currentState().globalMomentum();
  double fittedDimuonEnergy = fittedDimuon->currentState().kinematicParameters().energy();
  math::XYZTLorentzVector fittedDimuonLV (fittedDimuonMom.x(),fittedDimuonMom.y(),fittedDimuonMom.z(),fittedDimuonEnergy);

  // check if no other packed candidate is compatible with the two muons (get the maximal probability)
  std::vector<reco::TransientTrack> trackTTs = {muon1TT,muon2TT};
  double maxMuonsVertexComp = Tools::getMaxCompatibility (packedCandidates, mu1Track, mu2Track, field, trackTTs);      

  //////////////END OF THE MUON FITTING//////////////////////////////

  // consider Vertex Lxy from the beam spot     
  std::vector<double> lXY_fittedDimuon_bSpot_Vec = Tools::beamSpotVertexLxy(beamSpot, *fittedDimuonVertex);
  double lXY_fittedDimuon_bSpot = lXY_fittedDimuon_bSpot_Vec.at(0);
  double lXY_fittedDimuon_bSpot_significance = lXY_fittedDimuon_bSpot / lXY_fittedDimuon_bSpot_Vec.at(1);
  //if (lXY_fittedDimuon_bSpot_significance < 3.0)  return;

  // beam spot pointing angle of the dimuon alone
  const math::XYZPoint beamSpotPoint = beamSpot.position(fittedDimuonVertexPoint.z());
  math::XYZVector bSpotToVtx3D = fittedDimuonVertexPoint - beamSpotPoint;
  math::XYZVector bSpotToVtx2D (bSpotToVtx3D.x(),bSpotToVtx3D.y(),0.0);
  double cosAngleDimuonBSpot2D = bSpotToVtx2D.unit().Dot(fittedDimuonLV.Vect()) / fittedDimuonLV.Pt();
  
  ////////////PHOTON//////////////////////////////
  reco::Photon* igamma = recoMatchedPhotons.at(0)->clone();
  igamma->setVertex(fittedDimuonVertexPoint);

  // photon deltaR
  math::XYZTLorentzVector photonLV = igamma->p4();
  double deltaR_photon_fittedDimuon = reco::deltaR(fittedDimuonLV,photonLV);
  //if (deltaR_photon_fittedDimuon > 0.5) return;
  //if (deltaR_photon_fittedDimuon < 0.05)  return;

  // photon deltaEta and deltaPhi wrt. dimuon
  double deltaEta_photon_fittedDimuon = photonLV.Eta() - fittedDimuonLV.Eta();
  double cosDeltaPhi_photon_fittedDimuon = TMath::Cos(photonLV.Phi() - fittedDimuonLV.Phi());

  // get Lorentz Vector for modified photon energy
  double initPhotonEnergy = photonLV.energy();
  double eta = igamma->caloPosition().Eta();
  double modPhotonEnergy = PhotonEnergyModifier::getModPhotonEnergy(initPhotonEnergy, eta);
  double modScale = modPhotonEnergy / initPhotonEnergy;
  math::XYZTLorentzVector photonModLV = modScale * photonLV;
  ///////////////////////
  // What if == 0.0
  ///////////////////////

  // construct B0s candidate Lorentz Vector
  math::XYZTLorentzVector candBsLV = fittedDimuonLV + photonLV;
  math::XYZTLorentzVector candBsModLV = fittedDimuonLV + photonModLV;

  // initial check of B0s mass
  double candBsMass = candBsLV.M();
  double candBsModMass = candBsModLV.M();
  //if (std::fabs(candBsMass - bsMass) > 1.5
  //  && std::fabs(candBsModMass - bsMass) > 1.5) return;

  //
  // PV selection
  const reco::Vertex & bestPV = Tools::bestPV(primaryVertices,
      fittedDimuonVertexPoint, candBsLV.Vect());
  const reco::Vertex & bestPVmod = Tools::bestPV(primaryVertices,
      fittedDimuonVertexPoint, candBsModLV.Vect());
  
  //
  // 2D pointing angle to beam spot
  double cosAngleBsBSpot2D = bSpotToVtx2D.unit().Dot(candBsLV.Vect()) / candBsLV.Pt();
  double cosAngleModBsBSpot2D = bSpotToVtx2D.unit().Dot(candBsModLV.Vect()) / candBsModLV.Pt();
  
  // pointing angle and the 3D displacement from the corresponding bestPV
  // UNmod momentum
  math::XYZVector v3D_BsPV = fittedDimuonVertexPoint - bestPV.position();
  double l3D_BsPV = TMath::Sqrt(v3D_BsPV.Mag2());
  double cosAngleBsPV3D = v3D_BsPV.unit().Dot(candBsLV.Vect().unit());
  // MOD momentum
  math::XYZVector v3D_ModBsPV = fittedDimuonVertexPoint - bestPVmod.position();
  double l3D_ModBsPV = TMath::Sqrt(v3D_ModBsPV.Mag2());
  double cosAngleModBsPV3D = v3D_ModBsPV.unit().Dot(candBsModLV.Vect().unit());
  
  // B0s lifetime [ps] based on lXY_fittedDimuon_beamSpot
  double lifetimeBs = (lXY_fittedDimuon_bSpot * bsMass) / candBsLV.Pt();
  lifetimeBs *= 100/3;  // for ps (unit)
  double lifetimeModBs = (lXY_fittedDimuon_bSpot * bsMass) / candBsModLV.Pt();
  lifetimeModBs *= 100/3;

  // muon IDs
  std::vector<reco::Muon> muonsForID = {*im1,*im2};
  std::vector<float> muonMVAIDs = muonMVAIDProducer_->produce(muonsForID);
  float muon1Id = muonMVAIDs.at(0);
  float muon2Id = muonMVAIDs.at(1);
  
  pat::Muon m1pat (*im1);
  pat::Muon m2pat (*im2);
  bool tight1 = m1pat.isTightMuon(primaryVertices.at(0));
  bool tight2 = m2pat.isTightMuon(primaryVertices.at(0));
  bool tight1_selPV = m1pat.isTightMuon(bestPV);
  bool tight2_selPV = m2pat.isTightMuon(bestPV);

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


  // output
  const double outArray[32] = {fittedDimuonVertexProb,fittedDimuonMass,maxMuonsVertexComp,lXY_fittedDimuon_bSpot,lXY_fittedDimuon_bSpot_significance,
  deltaR_photon_fittedDimuon,eta,initPhotonEnergy,modScale,candBsMass,candBsModMass,
  cosAngleBsBSpot2D,cosAngleModBsBSpot2D,l3D_BsPV,l3D_ModBsPV,cosAngleBsPV3D,cosAngleModBsPV3D,
  lifetimeBs,lifetimeModBs,muon1Id,muon2Id,(double)tight1,(double)tight2,
  deltaEta_photon_fittedDimuon,cosDeltaPhi_photon_fittedDimuon,cosAngleDimuonBSpot2D,
  fittedDimuonLV.Pt(),photonLV.Pt(),fittedDimuonLV.Eta(),triggerRes,(double)tight1_selPV,(double)tight2_selPV};
  
  tOut->Fill(outArray);

  // pT balance at PV
  std::vector<double> ptBalanceVars = Tools::getPtBalanceVars(bestPV,bestPVmod,fittedDimuonLV,photonLV,photonModLV);
  auto pTBalArray = ptBalanceVars.data();

  tPtBalance->Fill(pTBalArray);
  

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(JpsiGMidApril_MC);