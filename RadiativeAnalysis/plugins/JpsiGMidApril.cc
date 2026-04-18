// based on JpsiGStartApril
// new photon energy modification

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
class JpsiGMidApril : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiGMidApril(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiGMidApril();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<pat::Muon> > theMuonToken;
  edm::EDGetTokenT < vector<pat::Photon> > thePhotonToken;
  edm::EDGetTokenT < vector<reco::Vertex> > theVertexToken; 
  edm::EDGetTokenT < vector<pat::PackedCandidate> > thePackedCandidateToken;

  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
  edm::EDGetTokenT < reco::BeamSpot > theBeamSpotToken;

  MuonMVAID* muonMVAIDProducer_;

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

JpsiGMidApril::JpsiGMidApril(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theMuonToken = consumes< vector<pat::Muon>  >( edm::InputTag("slimmedMuons"));
  thePhotonToken = consumes< vector<pat::Photon>  >( edm::InputTag("slimmedPhotons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlineSlimmedPrimaryVertices"));
  thePackedCandidateToken = consumes< vector<pat::PackedCandidate> >(edm::InputTag("packedPFCandidates"));
  theTriggerResultsToken = consumes< edm::TriggerResults > (edm::InputTag("TriggerResults","","HLT"));
  theBeamSpotToken = consumes< reco::BeamSpot >( edm::InputTag("offlineBeamSpot"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

JpsiGMidApril::~JpsiGMidApril()
{
  cout <<" DTOR" << endl;
}

void JpsiGMidApril::beginJob()
{
  muonMVAIDProducer_ = new MuonMVAID(theConfig);

  tOut = new TNtupleD("tOut","Output tuple",
  "fittedDimuonVertexProb:fittedDimuonMass:maxMuonsVertexComp:lXY_fittedDimuon_bSpot:lXY_fittedDimuon_bSpot_sig:dR_photonFittedDimuon:eta:initPhotonEnergy:modScale:candBsMass:candBsModMass:cosAngleBsBSpot2D:cosAngleModBsBSpot2D:l3D_BsPV:l3D_ModBsPV:cosAngleBsPV3D:cosAngleModBsPV3D:lifetimeBs:lifetimeModBs:muon1Id:muon2Id:tight1:tight2:dEta:cosDPhi:cosAnDimuonBSpot2D:triggerRes:bestPVs_dz:dimuonPt:photonPt:photonModPt:candBsPt:candBsModPt");
  cout << "HERE JpsiGMidApril::beginJob()" << endl;
}

void JpsiGMidApril::endJob()
{
  delete muonMVAIDProducer_;

  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write data
  tOut->Write();

  myRootFile.Close();

  delete tOut;

  cout << "HERE JpsiGMidApril::endJob()" << endl;
}

void JpsiGMidApril::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiGMidApril::analyze "<< std::endl;

  const std::vector<pat::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<pat::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  const std::vector<pat::PackedCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);

  auto const& field = es.getData(m_fieldToken);  

  // trigger info
  const edm::TriggerResults & triggerResults = ev.get(theTriggerResultsToken);
  edm::TriggerNames triggerNames = ev.triggerNames(triggerResults);
  
  ////////////////////////////////////////

  if (recoMuons.size() < 2 || recoPhotons.size() < 1)   return;

  // Muon MVA ID
  std::vector<float> muonMVAIDs = muonMVAIDProducer_->produce(recoMuons);

  KalmanVertexFitter kvf(false);
  KinematicParticleFactoryFromTransientTrack pFactory;
  KinematicParticleVertexFitter kinVertexFitter;
  double jpsiMass = 3.097;
  double bsMass = 5.367;
  const ParticleMass muonMass = 0.105658;
  float muonMassSigma = 2E-6;

  // take two oppositely charged recoMuons with tracks
  for (std::vector<pat::Muon>::const_iterator im1 = recoMuons.begin(); im1 < recoMuons.end(); im1++)
  {
    reco::TrackRef mu1Track = im1->track();
    if (!mu1Track)  continue;
    reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);
    RefCountedKinematicParticle muon1KinPar = pFactory.particle(muon1TT, muonMass, float(0), float(0), muonMassSigma);

    for (std::vector<pat::Muon>::const_iterator im2 = im1+1; im2 < recoMuons.end(); im2++)
    {
      if (im1->charge() * im2->charge() != -1)  continue;

      reco::TrackRef mu2Track = im2->track();
      if (!mu2Track) continue;
      reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);
      RefCountedKinematicParticle muon2KinPar = pFactory.particle(muon2TT, muonMass, float(0), float(0), muonMassSigma);

      // fit the two muons common vertex
      std::vector<RefCountedKinematicParticle> muonKinematicParticles = {muon1KinPar, muon2KinPar};
      RefCountedKinematicTree twoMuonsFitTree = kinVertexFitter.fit(muonKinematicParticles);
      if (! twoMuonsFitTree->isValid())  continue;

      // get the fitted decay vertex
      twoMuonsFitTree->movePointerToTheTop();
      RefCountedKinematicVertex fittedDimuonVertex = twoMuonsFitTree->currentDecayVertex();
      if (! fittedDimuonVertex->vertexIsValid())  continue;      
      double fittedDimuonVertexProb = TMath::Prob(fittedDimuonVertex->chiSquared(),fittedDimuonVertex->degreesOfFreedom());
      if (fittedDimuonVertexProb < 0.01)  continue;
      GlobalPoint dmuVPosGP = fittedDimuonVertex->position();
      math::XYZPoint fittedDimuonVertexPoint (dmuVPosGP.x(),dmuVPosGP.y(),dmuVPosGP.z());

      // get the fitted dimuon state
      RefCountedKinematicParticle fittedDimuon = twoMuonsFitTree->currentParticle();
      double fittedDimuonMass = fittedDimuon->currentState().mass();
      if (std::fabs(fittedDimuonMass - jpsiMass) > 0.15) continue;      

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
      if (lXY_fittedDimuon_bSpot_significance < 3.0)  continue;

      // beam spot pointing angle of the dimuon alone
      const math::XYZPoint beamSpotPoint = beamSpot.position(fittedDimuonVertexPoint.z());
      math::XYZVector bSpotToVtx3D = fittedDimuonVertexPoint - beamSpotPoint;
      math::XYZVector bSpotToVtx2D (bSpotToVtx3D.x(),bSpotToVtx3D.y(),0.0);
      double cosAngleDimuonBSpot2D = bSpotToVtx2D.unit().Dot(fittedDimuonLV.Vect()) / fittedDimuonLV.Pt();
    
      ////////////LOOP OVER PHOTONS//////////////////////////////
      for (std::vector<pat::Photon>::const_iterator igamma0 = recoPhotons.begin(); igamma0 < recoPhotons.end(); igamma0++)
      {
        pat::Photon* igamma = igamma0->clone();
        igamma->setVertex(fittedDimuonVertexPoint);

        // photon deltaR
        math::XYZTLorentzVector photonLV = igamma->p4();
        double deltaR_photon_fittedDimuon = reco::deltaR(fittedDimuonLV,photonLV);
        if (deltaR_photon_fittedDimuon > 0.5) continue;
        if (deltaR_photon_fittedDimuon < 0.05)  continue;

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
        if (std::fabs(candBsMass - bsMass) > 1.5
          && std::fabs(candBsModMass - bsMass) > 1.5) continue;

        //
        // PV selection
        const reco::Vertex & bestPV = Tools::bestPV(primaryVertices,
            fittedDimuonVertexPoint, candBsLV.Vect());
        const reco::Vertex & bestPVmod = Tools::bestPV(primaryVertices,
            fittedDimuonVertexPoint, candBsModLV.Vect());

        double bestPVtoBestPVmod_z = bestPVmod.z() - bestPV.z();

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
        std::vector<pat::Muon>::const_iterator imB = recoMuons.begin();
        float muon1Id = muonMVAIDs.at(im1 - imB);
        float muon2Id = muonMVAIDs.at(im2 - imB);
        
        bool tight1 = im1->isTightMuon(bestPV);
        bool tight2 = im2->isTightMuon(bestPV);

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
        const double outArray[33] = {fittedDimuonVertexProb,fittedDimuonMass,maxMuonsVertexComp,lXY_fittedDimuon_bSpot,lXY_fittedDimuon_bSpot_significance,
        deltaR_photon_fittedDimuon,eta,initPhotonEnergy,modScale,candBsMass,candBsModMass,
        cosAngleBsBSpot2D,cosAngleModBsBSpot2D,l3D_BsPV,l3D_ModBsPV,cosAngleBsPV3D,cosAngleModBsPV3D,
        lifetimeBs,lifetimeModBs,muon1Id,muon2Id,(double)tight1,(double)tight2,
        deltaEta_photon_fittedDimuon,cosDeltaPhi_photon_fittedDimuon,cosAngleDimuonBSpot2D,triggerRes,
        bestPVtoBestPVmod_z,fittedDimuonLV.Pt(),photonLV.Pt(),photonModLV.Pt(),candBsLV.Pt(),candBsModLV.Pt()};
        
        tOut->Fill(outArray);


      } // photon

    } // muon2
  } // muon1
    

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(JpsiGMidApril);