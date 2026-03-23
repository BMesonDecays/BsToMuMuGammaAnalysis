// based on JpsiMassG
// started on March 22 2026

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
class JpsiGEndMarch : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiGEndMarch(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiGEndMarch();

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

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
  edm::EDGetTokenT < reco::BeamSpot > theBeamSpotToken;

  MuonMVAID* muonMVAIDProducer_;

  TNtupleD* tOut;

};

JpsiGEndMarch::JpsiGEndMarch(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theMuonToken = consumes< vector<pat::Muon>  >( edm::InputTag("slimmedMuons"));
  thePhotonToken = consumes< vector<pat::Photon>  >( edm::InputTag("slimmedPhotons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlineSlimmedPrimaryVertices"));
  thePackedCandidateToken = consumes< vector<pat::PackedCandidate> >(edm::InputTag("packedPFCandidates"));
  theBeamSpotToken = consumes< reco::BeamSpot >( edm::InputTag("offlineBeamSpot"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

JpsiGEndMarch::~JpsiGEndMarch()
{
  cout <<" DTOR" << endl;
}

void JpsiGEndMarch::beginJob()
{
  tOut = new TNtupleD("tOut","Output tuple",
    "twoMuonsMass:muonsKalmanVxProb:maxMuonsComp:fittedJpsiVxProb:lXY_twoMuons_bSpot:lXY_fittedJpsi_bSpot:dR_photon_twoMuons:dR_photon_Jpsi:twoMuonsPhotonMass:fittedJpsiPhotonMass:lXY_twoMuons_PV:lXY_fittedJpsi_PV:lXY_twoMuons_PV_sign:lXY_fittedJpsi_PV_sign:lXYZ_twoMuons_bestPV:lXYZ_fittedJpsi_bestPV:cosPointingAngle_twoMuons:cosPointingAngle_fittedJpsi:BsXYlifetime_twoMuons:BsXYlifetime_fittedJpsi");
  
  cout << "HERE JpsiGEndMarch::beginJob()" << endl;
}

void JpsiGEndMarch::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write data
  tOut->Write();

  myRootFile.Close();

  delete tOut;

  cout << "HERE JpsiGEndMarch::endJob()" << endl;
}

void JpsiGEndMarch::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiGEndMarch::analyze "<< std::endl;

  const std::vector<pat::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<pat::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  const std::vector<pat::PackedCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);

  auto const& field = es.getData(m_fieldToken);  
  
  ////////////////////////////////////////

  if (recoMuons.size() < 2 || recoPhotons.size() < 1)   return;

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
      if (! fittedDimuonVertex->isValid())  continue;      
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
      
      ////////////LOOP OVER PHOTONS//////////////////////////////
      for (std::vector<pat::Photon>::const_iterator igamma = recoPhotons.begin(); igamma < recoPhotons.end(); igamma++)
      {
        igamma->setVertex(fittedDimuonVertexPoint);

        // photon deltaR
        math::XYZTLorentzVector photonLV = igamma->p4();
        double deltaR_photon_fittedDimuon = reco::deltaR(fittedDimuonLV,photonLV);
        if (deltaR_photon_fittedDimuon > 0.5) continue;
        if (deltaR_photon_fittedDimuon < 0.05)  continue;

        // get Lorentz Vector for modified photon energy

        ///////////////////////
        ///////////////////////
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

        const double outArray[20] = {twoMuonsM,muonsKalmanVxProb,maxMuonsVertexComp,fittedJpsiVxProb,
            lXY_muonsKalman_bSpot,lXY_fittedJpsi_bSpot,deltaR_photon_twoMuons,deltaR_photon_Jpsi,
            twoMuonsPhotonMass,fittedJpsiPhotonMass,lXY_muonsKalman_PV,lXY_fittedJpsi_PV,
            lXY_muonsKalman_PV_significance,lXY_fittedJpsi_PV_significance,lXYZ_muonsKalman_PV,lXYZ_fittedJpsi_PV,
            cosPointingAngle_muonsKalman_PV,cosPointingAngle_fittedJpsi_PV,lifetimeB0s_muonsKalman,lifetimeB0s_fittedJpsi};

        tOut->Fill(outArray);


      } // photon

    } // muon2
  } // muon1
    

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(JpsiGEndMarch);