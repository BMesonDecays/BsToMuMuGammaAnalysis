// MC version of JpsiMassG

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
class JpsiMassG_MC : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiMassG_MC(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiMassG_MC();

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

  TH1D* hCompAll;
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

JpsiMassG_MC::JpsiMassG_MC(const edm::ParameterSet& conf)
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

JpsiMassG_MC::~JpsiMassG_MC()
{
  cout <<" DTOR" << endl;
}

void JpsiMassG_MC::beginJob()
{
    hCompAll = new TH1D("hCompAll","Third candidate compatibility (one entry per candidate)",1000,0.,1.);
    tOut = new TNtupleD("tOut","Output tuple",
        "twoMuonsMass:muonsKalmanVxProb:maxMuonsComp:fittedJpsiVxProb:lXY_twoMuons_bSpot:lXY_fittedJpsi_bSpot:dR_photon_twoMuons:dR_photon_Jpsi:twoMuonsPhotonMass:fittedJpsiPhotonMass:lXY_twoMuons_PV:lXY_fittedJpsi_PV:lXY_twoMuons_PV_sign:lXY_fittedJpsi_PV_sign:lXYZ_twoMuons_bestPV:lXYZ_fittedJpsi_bestPV:cosPointingAngle_twoMuons:cosPointingAngle_fittedJpsi:BsXYlifetime_twoMuons:BsXYlifetime_fittedJpsi:hltRes");
  
  cout << "HERE JpsiMassG_MC::beginJob()" << endl;
}

void JpsiMassG_MC::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write data
  hCompAll->Write();
  tOut->Write();

  myRootFile.Close();

  delete hCompAll;
  delete tOut;

  cout << "HERE JpsiMassG_MC::endJob()" << endl;
}

void JpsiMassG_MC::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiMassG_MC::analyze "<< std::endl;

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
  double jpsiMass = 3.097;
  double bsMass = 5.367;
  const ParticleMass muonMass = 0.105658;
  float muonMassSigma = 2E-6;

  // take both recoMatchedMuons
  reco::TrackRef mu1Track = recoMatchedMuons.at(0)->track();
  if (!mu1Track)  return;
  reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);

  if (recoMatchedMuons.at(0)->charge() * recoMatchedMuons.at(1)->charge() != -1)  return;

  math::XYZTLorentzVector twoMuonsLV = recoMatchedMuons.at(0)->p4() + recoMatchedMuons.at(1)->p4();
  double twoMuonsM = twoMuonsLV.M();
  if (std::fabs(twoMuonsM - jpsiMass) > 0.15) return;
  // got two muons Lorentz Vector - candidate for J/psi

  reco::TrackRef mu2Track = recoMatchedMuons.at(1)->track();
  if (!mu2Track) return;
  reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);

  std::vector<reco::TransientTrack> trackTTs;
  trackTTs.push_back(muon1TT);
  trackTTs.push_back(muon2TT);

  reco::Vertex muonsKalmanVertex (TransientVertex(kvf.vertex(trackTTs)));
  double muonsKalmanVxProb = TMath::Prob(muonsKalmanVertex.chi2(), muonsKalmanVertex.ndof());
  if (muonsKalmanVxProb < 0.01)  return;
  // got muonsKalmanVertex

  // check if no other packed candidate is compatible with the two muons vertex (get the maximal probability)
  double maxMuonsVertexComp = Tools::getMaxCompatibility (packedCandidates, mu1Track, mu2Track, field, trackTTs);      


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

  ////////////END OF THE CONSTRAINED FIT///////////
  // from now on all operations both for twoMuons and fittedJpsi

  // consider Vertex Lxy from the beam spot
  // muonsKalmanVertex
  std::vector<double> lXY_muonsKalman_bSpot_Vec = Tools::beamSpotVertexLxy(beamSpot, muonsKalmanVertex);
  double lXY_muonsKalman_bSpot = lXY_muonsKalman_bSpot_Vec.at(0);
  double lXY_muonsKalman_bSpot_significance = lXY_muonsKalman_bSpot / lXY_muonsKalman_bSpot_Vec.at(1);
  // fittedJpsiVertex
  std::vector<double> lXY_fittedJpsi_bSpot_Vec = Tools::beamSpotVertexLxy(beamSpot, *fittedJpsiVertex);
  double lXY_fittedJpsi_bSpot = lXY_fittedJpsi_bSpot_Vec.at(0);
  double lXY_fittedJpsi_bSpot_significance = lXY_fittedJpsi_bSpot / lXY_fittedJpsi_bSpot_Vec.at(1);

  if (lXY_muonsKalman_bSpot_significance < 3.0
    && lXY_fittedJpsi_bSpot_significance < 3.0)  return;
  
  //////PHOTON/////////
  
  // photon deltaR
  math::XYZTLorentzVector photonLV = recoMatchedPhotons.at(0)->p4();
  double deltaR_photon_twoMuons = reco::deltaR(twoMuonsLV, photonLV);
  double deltaR_photon_Jpsi = reco::deltaR(fittedJpsiLV, photonLV);

  if (deltaR_photon_twoMuons > 0.4
      && deltaR_photon_Jpsi > 0.4)   return;
  if (deltaR_photon_twoMuons < 0.05
      && deltaR_photon_Jpsi < 0.05)  return;

  // addition of the photon LorentzVector
  // to get (twoMuons/fittedJpsi)Photon Lorentz vector - candidate for B0s
  math::XYZTLorentzVector twoMuonsPhotonLV = twoMuonsLV + photonLV;
  double twoMuonsPhotonMass = twoMuonsPhotonLV.M();
  math::XYZTLorentzVector fittedJpsiPhotonLV = fittedJpsiLV + photonLV;
  double fittedJpsiPhotonMass = fittedJpsiPhotonLV.M();

  // initial check of B0s mass
  if (std::fabs(twoMuonsPhotonMass - bsMass) > 1.5
      && std::fabs(fittedJpsiPhotonMass - bsMass) > 1.5) return;        

  // PV selection
  const reco::Vertex & bestPV_twoMuons = Tools::bestPV(primaryVertices, muonsKalmanVertex.position(), 
                        twoMuonsPhotonLV.Vect());
  const reco::Vertex & bestPV_fittedJpsi = Tools::bestPV(primaryVertices, fittedJpsiVertexPosition, 
                                      fittedJpsiPhotonLV.Vect());
  
  // 2D displacement from the corresponding bestPV
  // muonsKalmanVertex
  Tools::displacementXY displXY_muonsKalman_PV = Tools::getDisplXY (muonsKalmanVertex, bestPV_twoMuons);
  double lXY_muonsKalman_PV = TMath::Sqrt(displXY_muonsKalman_PV.vector.Mag2());
  double lXY_muonsKalman_PV_significance = lXY_muonsKalman_PV / displXY_muonsKalman_PV.error;
  // fittedJpsiVertex
  Tools::displacementXY displXY_fittedJpsi_PV = Tools::getDisplXY (*fittedJpsiVertex, bestPV_fittedJpsi);
  double lXY_fittedJpsi_PV = TMath::Sqrt(displXY_fittedJpsi_PV.vector.Mag2());
  double lXY_fittedJpsi_PV_significance = lXY_fittedJpsi_PV / displXY_fittedJpsi_PV.error;

  if (lXY_muonsKalman_PV_significance < 3.0
      && lXY_fittedJpsi_PV_significance < 3.0)   return;

  // pointing angle and the 3D displacement from the corresponding bestPV
  // muonsKalmanVertex
  math::XYZVector lXYZ_muonsKalman_PV_Vector = muonsKalmanVertex.position() - bestPV_twoMuons.position();
  double lXYZ_muonsKalman_PV = TMath::Sqrt(lXYZ_muonsKalman_PV_Vector.Mag2());
  double cosPointingAngle_muonsKalman_PV = twoMuonsPhotonLV.Vect().unit().Dot(lXYZ_muonsKalman_PV_Vector.unit());
  // fittedJpsiVertex
  math::XYZVector lXYZ_fittedJpsi_PV_Vector = fittedJpsiVertexPosition - bestPV_fittedJpsi.position();
  double lXYZ_fittedJpsi_PV = TMath::Sqrt(lXYZ_fittedJpsi_PV_Vector.Mag2());
  double cosPointingAngle_fittedJpsi_PV = fittedJpsiPhotonLV.Vect().unit().Dot(lXYZ_fittedJpsi_PV_Vector.unit());

  // B0s lifetime [ps] based on lXY
  // muonsKalmanVertex
  double lifetimeB0s_muonsKalman = (lXY_muonsKalman_PV * bsMass) / twoMuonsPhotonLV.Vect().Rho();
  lifetimeB0s_muonsKalman *= 100/3;
  // fittedJpsiVertex
  double lifetimeB0s_fittedJpsi = (lXY_fittedJpsi_PV * bsMass) / fittedJpsiPhotonLV.Vect().Rho();
  lifetimeB0s_fittedJpsi *= 100/3;

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


  const double outArray[21] = {twoMuonsM,muonsKalmanVxProb,maxMuonsVertexComp,fittedJpsiVxProb,
      lXY_muonsKalman_bSpot,lXY_fittedJpsi_bSpot,deltaR_photon_twoMuons,deltaR_photon_Jpsi,
      twoMuonsPhotonMass,fittedJpsiPhotonMass,lXY_muonsKalman_PV,lXY_fittedJpsi_PV,
      lXY_muonsKalman_PV_significance,lXY_fittedJpsi_PV_significance,lXYZ_muonsKalman_PV,lXYZ_fittedJpsi_PV,
      cosPointingAngle_muonsKalman_PV,cosPointingAngle_fittedJpsi_PV,lifetimeB0s_muonsKalman,lifetimeB0s_fittedJpsi,
      triggerRes};

  tOut->Fill(outArray);



  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(JpsiMassG_MC);