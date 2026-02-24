// based on impJpsiG
// full kinematic fit with kinematic photons

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
class fullFitJpsiG : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit fullFitJpsiG(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~fullFitJpsiG();

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

  edm::EDGetTokenT<EcalRecHitCollection> ebRecHitsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> eeRecHitsToken_;
  unique_ptr<EcalClusterLazyToolsBase::ESGetTokens> esGetTokens;

  TNtupleD* tKalman;
  TNtupleD* tJpsi;

};


fullFitJpsiG::fullFitJpsiG(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theMuonToken = consumes< vector<pat::Muon>  >( edm::InputTag("slimmedMuons"));
  thePhotonToken = consumes< vector<pat::Photon>  >( edm::InputTag("slimmedPhotons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlineSlimmedPrimaryVertices"));
  thePackedCandidateToken = consumes< vector<pat::PackedCandidate> >(edm::InputTag("packedPFCandidates"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

  ebRecHitsToken_ = consumes<EcalRecHitCollection>(edm::InputTag("reducedEcalRecHitsEB"));
  eeRecHitsToken_ = consumes<EcalRecHitCollection>(edm::InputTag("reducedEcalRecHitsEB"));

  esGetTokens = make_unique<EcalClusterLazyToolsBase::ESGetTokens>(consumesCollector());

}

fullFitJpsiG::~fullFitJpsiG()
{
  cout <<" DTOR" << endl;
}

void fullFitJpsiG::beginJob()
{
  tKalman = new TNtupleD("tKalman","Unconstrained Kalman fit",
    "twoMuonsMass:muonsKalmanVxProb:maxMuonsComp:deltaR_photon_twoMuons:twoMuonsPhotonMass:displXY_bestPV:displXY_bestPV_significance:displ3D_bestPV:cosPointingAngle:BsXYlifetime");

  tJpsi = new TNtupleD("tJpsi","J/psi mass constrained fit",
    "twoMuonsMass:muonsKalmanVxProb:maxMuonsComp:fittedJpsiVxProb:deltaR_photon_Jpsi:fittedJpsiPhotonMass:lXY_fittedJpsi_PV:lXY_fittedJpsi_PV_significance:lXYZ_fittedJpsi_PV:cosPointingAngle:BsXYlifetime");

  cout << "HERE fullFitJpsiG::beginJob()" << endl;
}

void fullFitJpsiG::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  tKalman->Write();
  tJpsi->Write();

  myRootFile.Close();

  delete tKalman;
  delete tJpsi;

  cout << "HERE fullFitJpsiG::endJob()" << endl;
}

void fullFitJpsiG::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE fullFitJpsiG::analyze "<< std::endl;

  const std::vector<pat::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<pat::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  const std::vector<pat::PackedCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  
  auto const& field = es.getData(m_fieldToken);  

  EcalClusterLazyTools lazyTools(ev, esGetTokens->get(es), ebRecHitsToken_, eeRecHitsToken_);

  
  ////////////////////////////////////////

  if (recoMuons.size() < 2 || recoPhotons.size() < 1)   return;

  KalmanVertexFitter kvf(false);
  double jpsiMass = 3.097;
  double bsMass = 5.367;
  const ParticleMass muonMass = 0.105658;
  float muonMassSigma = 2E-6;

  // take two oppositely charged recoMuons
  for (std::vector<pat::Muon>::const_iterator im1 = recoMuons.begin(); im1 < recoMuons.end(); im1++)
  {
    reco::TrackRef mu1Track = im1->track();
    if (!mu1Track)  continue;
    reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);

    for (std::vector<pat::Muon>::const_iterator im2 = im1+1; im2 < recoMuons.end(); im2++)
    {
      if (im1->charge() * im2->charge() != -1)  continue;

      math::XYZTLorentzVector twoMuonsLV = im1->p4() + im2->p4();
      double twoMuonsM = twoMuonsLV.M();
      if (std::fabs(twoMuonsM - jpsiMass) > 0.15) continue;

      reco::TrackRef mu2Track = im2->track();
      if (!mu2Track) continue;
      reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);

      std::vector<reco::TransientTrack> trackTTs;
      trackTTs.push_back(muon1TT);
      trackTTs.push_back(muon2TT);

      reco::Vertex muonsKalmanVertex (TransientVertex(kvf.vertex(trackTTs)));
      double muonsKalmanVxProb = TMath::Prob(muonsKalmanVertex.chi2(), muonsKalmanVertex.ndof());
      if (muonsKalmanVxProb < 0.01)  continue;

      // got two muons Lorentz Vector - candidate for J/psi

      // check if no other packed candidate is compatible with the two muons vertex (get the maximal probability)
      double maxMuonsVertexComp = Tools::getMaxCompatibility (packedCandidates, mu1Track, mu2Track, field, trackTTs);      

      // photons that have passed the loop
      std::vector<pat::Photon> passingPhotons;

      // loop over photons
      for (std::vector<pat::Photon>::const_iterator igamma = recoPhotons.begin(); igamma < recoPhotons.end(); igamma++)
      {
        math::XYZTLorentzVector photonLV = igamma->p4();
        double deltaR_photon_twoMuons = reco::deltaR(twoMuonsLV, photonLV);
        if (deltaR_photon_twoMuons > 0.4) continue;
        if (deltaR_photon_twoMuons < 0.05) continue;

        // add the vectors of the two muons and the photon
        math::XYZTLorentzVector twoMuonsPhotonLV = twoMuonsLV + photonLV;
        double twoMuonsPhotonMass = twoMuonsPhotonLV.M();
        if (std::fabs(twoMuonsPhotonMass - bsMass) > 0.5)  continue;
        // got twoMuonsPhoton Lorentz vector - candidate for B0s

        // PV selection
        const reco::Vertex & bestPV = Tools::bestPV(primaryVertices, muonsKalmanVertex.position(), 
                              twoMuonsPhotonLV.Vect());
        
        // muonsKalmanVertex 2D displacement from bestPV
        Tools::displacementXY displXY_muonsKalman_PV = Tools::getDisplXY (muonsKalmanVertex, bestPV);
        double lXY_muonsKalman_PV = TMath::Sqrt(displXY_muonsKalman_PV.vector.Mag2());
        double lXY_muonsKalman_PV_significance = lXY_muonsKalman_PV / displXY_muonsKalman_PV.error;

        if (lXY_muonsKalman_PV_significance < 3.0)  continue;

        // muonsKalmanVertex 3D displacement from bestPV
        math::XYZVector lXYZ_muonsKalman_PV_Vector = muonsKalmanVertex.position() - bestPV.position();
        double lXYZ_muonsKalman_PV = TMath::Sqrt(lXYZ_muonsKalman_PV_Vector.Mag2());
        double cosPointingAngle_muonsKalman_PV = twoMuonsPhotonLV.Vect().unit().Dot(lXYZ_muonsKalman_PV_Vector.unit());

        // B0s lifetime based on lXY
        double lifetimeB0s = (lXY_muonsKalman_PV * bsMass) / twoMuonsPhotonLV.Vect().Rho();
        lifetimeB0s *= 100/3;

        tKalman->Fill(twoMuonsM,muonsKalmanVxProb,maxMuonsVertexComp, deltaR_photon_twoMuons,
                    twoMuonsPhotonMass,lXY_muonsKalman_PV,lXY_muonsKalman_PV_significance,
                    lXYZ_muonsKalman_PV,cosPointingAngle_muonsKalman_PV,lifetimeB0s);

        passingPhotons.push_back(*igamma);

      }


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
      if (! jpsiFitTree->isValid()) continue;

      // get the fitted J/psi and vertex
      jpsiFitTree->movePointerToTheTop();
      RefCountedKinematicParticle fittedJpsi = jpsiFitTree->currentParticle();
      RefCountedKinematicVertex fittedJpsiVertex = jpsiFitTree->currentDecayVertex();
      if (! fittedJpsiVertex->vertexIsValid())  continue;

      double fittedJpsiVxProb = TMath::Prob(fittedJpsiVertex->chiSquared(), fittedJpsiVertex->degreesOfFreedom());

      GlobalPoint position_GP = fittedJpsiVertex->position();
      math::XYZPoint fittedJpsiVertexPosition (position_GP.x(),position_GP.y(),position_GP.z());

      // create a Lorentz Vector for the fitted J/psi
      GlobalVector fittedJpsiMom = fittedJpsi->currentState().globalMomentum();
      double fittedJpsiEnergy = fittedJpsi->currentState().kinematicParameters().energy();
      math::XYZTLorentzVector fittedJpsiLV (fittedJpsiMom.x(),fittedJpsiMom.y(),fittedJpsiMom.z(),fittedJpsiEnergy);

      // loop over photons that have passed previous stage
      for (std::vector<pat::Photon>::const_iterator igamma = passingPhotons.begin(); igamma < passingPhotons.end(); igamma++)
      {
        math::XYZTLorentzVector photonLV = igamma->p4();
        double deltaR_photon_Jpsi = reco::deltaR(fittedJpsiLV, photonLV);
        if (deltaR_photon_Jpsi > 0.4) continue;
        if (deltaR_photon_Jpsi < 0.05) continue;

        // find the best PV, based on fittedJpsiLV + photon_JpsiLV
        math::XYZTLorentzVector fittedJpsiPhotonLV = fittedJpsiLV + photonLV;
        const reco::Vertex & bestPV_Jpsi = Tools::bestPV(primaryVertices, fittedJpsiVertexPosition, 
                                            fittedJpsiPhotonLV.Vect());

        double fittedJpsiPhotonMass = fittedJpsiPhotonLV.M();

        // fittedJpsiVertex 2D displacement from bestPV_Jpsi
        Tools::displacementXY displXY_fittedJpsi_PV = Tools::getDisplXY (*fittedJpsiVertex, bestPV_Jpsi);
        double lXY_fittedJpsi_PV = TMath::Sqrt(displXY_fittedJpsi_PV.vector.Mag2());
        double lXY_fittedJpsi_PV_significance = lXY_fittedJpsi_PV / displXY_fittedJpsi_PV.error;

        if (lXY_fittedJpsi_PV_significance < 3.0)  continue;

        // fittedJpsiVertex 3D displacement from bestPV_Jpsi
        math::XYZVector lXYZ_fittedJpsi_PV_Vector = fittedJpsiVertexPosition - bestPV_Jpsi.position();
        double lXYZ_fittedJpsi_PV = TMath::Sqrt(lXYZ_fittedJpsi_PV_Vector.Mag2());
        double cosPointingAngle_fittedJpsi_PV = fittedJpsiPhotonLV.Vect().unit().Dot(lXYZ_fittedJpsi_PV_Vector.unit());

        // B0s lifetime based on lXY
        double lifetimeB0s = (lXY_fittedJpsi_PV * bsMass) / fittedJpsiPhotonLV.Vect().Rho();
        lifetimeB0s *= 100/3;


        tJpsi->Fill(twoMuonsM,muonsKalmanVxProb,maxMuonsVertexComp,fittedJpsiVxProb,
                    deltaR_photon_Jpsi,fittedJpsiPhotonMass,lXY_fittedJpsi_PV,lXY_fittedJpsi_PV_significance,
                    lXYZ_fittedJpsi_PV,cosPointingAngle_fittedJpsi_PV,lifetimeB0s);

        
                                          
      }
      

      /*
      // create the pointing constraint and perform the full global (kinematic) fit
      GlobalPoint bestPV_JpsiPosition (bestPV_Jpsi.position().x(),bestPV_Jpsi.position().y(),bestPV_Jpsi.position().z());
      MultiTrackKinematicConstraint* pointingConstraint = new MultiTrackKinematicConstraint(bestPV_JpsiPosition);
      RefCountedKinematicTree bsFitTree = constrainedFitter

      
      add photon, get LV and vertex position
      find the best PV again
      global fit with the pointing constraint
      */

    } // muon2
    

  } // muon1


}

DEFINE_FWK_MODULE(fullFitJpsiG);
