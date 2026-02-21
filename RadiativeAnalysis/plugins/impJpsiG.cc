// based on JpsiGammaMiniAOD
// created after CMS talk in January


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
class impJpsiG : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit impJpsiG(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~impJpsiG();

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

  TH1D* hLxySignificance;
  TH1D* hLifetime_ps;

};


impJpsiG::impJpsiG(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theMuonToken = consumes< vector<pat::Muon>  >( edm::InputTag("slimmedMuons"));
  thePhotonToken = consumes< vector<pat::Photon>  >( edm::InputTag("slimmedPhotons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlineSlimmedPrimaryVertices"));
  thePackedCandidateToken = consumes< vector<pat::PackedCandidate> >(edm::InputTag("packedPFCandidates"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

impJpsiG::~impJpsiG()
{
  cout <<" DTOR" << endl;
}

void impJpsiG::beginJob()
{
  hLxySignificance = new TH1D("hLxySignificance","hLxySignificance",100,0.,1000);
  hLifetime_ps = new TH1D("hLifetime_ps","hLifetime [ps]",1000,0.,20.);

  cout << "HERE impJpsiG::beginJob()" << endl;
}

void impJpsiG::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  hLxySignificance->Write();
  hLifetime_ps->Write();

  myRootFile.Close();

  delete hLxySignificance;
  delete hLifetime_ps;
  
  cout << "HERE impJpsiG::endJob()" << endl;
}

void impJpsiG::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE impJpsiG::analyze "<< std::endl;

  const std::vector<pat::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<pat::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  const std::vector<pat::PackedCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  
  auto const& field = es.getData(m_fieldToken);  
  
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
      double prob = TMath::Prob(muonsKalmanVertex.chi2(), muonsKalmanVertex.ndof());
      if (prob < 0.1)  continue;

      // got two muons Lorentz Vector - candidate for J/psi

      // check if no other packed candidate is compatible with the two muons vertex (get the maximal probability)
      double maxMuonsVertexComp = Tools::getMaxCompatibility (packedCandidates, mu1Track, mu2Track, field, trackTTs);      

      // find the photon closest in dR to the two muons LV
      // (however dR > 0.05)
      const pat::Photon* min_dR_photon = Tools::findPhotonWithMinDR(recoPhotons,twoMuonsLV, 0.05);
      math::XYZTLorentzVector photonLV = min_dR_photon->p4();
      double min_dR_photon_twoMuons = reco::deltaR(photonLV, twoMuonsLV);
      if (min_dR_photon_twoMuons > 0.4) continue;
      
      // add the vectors of the two muons and the photon
      math::XYZTLorentzVector twoMuonsPhotonLV = twoMuonsLV + photonLV;
      if (std::fabs(twoMuonsPhotonLV.M() - bsMass) > 0.5)  continue;
      // got twoMuonsPhoton Lorentz vector - candidate for B0s

      // PV selection
      const reco::Vertex & bestPV = Tools::bestPV(primaryVertices, muonsKalmanVertex.position(), 
                            twoMuonsPhotonLV.Vect());
      
      // muonsKalmanVertex 2D displacement from bestPV
      Tools::displacementXY displXY_muonsKalman_PV = Tools::getDisplXY (muonsKalmanVertex, bestPV);
      double lXY_muonsKalman_PV = TMath::Sqrt(displXY_muonsKalman_PV.vector.Mag2());
      double lXY_muonsKalman_PV_significance = lXY_muonsKalman_PV / displXY_muonsKalman_PV.error;
      hLxySignificance->Fill(lXY_muonsKalman_PV_significance);

      if (lXY_muonsKalman_PV_significance < 3.0)  continue;

      // muonsKalmanVertex 3D displacement from bestPV
      math::XYZVector lXYZ_muonsKalman_PV_Vector = muonsKalmanVertex.position() - bestPV.position();
      double lXYZ_muonsKalman_PV = TMath::Sqrt(lXYZ_muonsKalman_PV_Vector.Mag2());
      double cosPointingAngle_muonsKalman_PV = twoMuonsPhotonLV.Vect().unit().Dot(lXYZ_muonsKalman_PV_Vector.unit());

      // B0s lifetime based on lXY
      double lifetimeB0s = (lXY_muonsKalman_PV * bsMass) / twoMuonsPhotonLV.Vect().Rho();
      hLifetime_ps->Fill(lifetimeB0s*100/3);


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
      GlobalPoint position_GP = fittedJpsiVertex->position();
      math::XYZPoint fittedJpsiVertexPosition (position_GP.x(),position_GP.y(),position_GP.z());

      // create a Lorentz Vector for the fitted J/psi
      GlobalVector fittedJpsiMom = fittedJpsi->currentState()->globalMomentum();
      double fittedJpsiEnergy = fittedJpsi->currentState()->kinematicParameters()->energy();
      math::XYZTLorentzVector fittedJpsiLV (fittedJpsiMom.x(),fittedJpsiMom.y(),fittedJpsiMom.z(),fittedJpsiEnergy);

      // find the photon again, this time the closest in dR to the fittedJpsi
      // (however dR > 0.05)
      const pat::Photon* min_dR_photon_Jpsi = Tools::findPhotonWithMinDR(recoPhotons,fittedJpsiLV, 0.05);
      math::XYZTLorentzVector photon_JpsiLV = min_dR_photon_Jpsi->p4();
      double min_dR_photon_Jpsi = reco::deltaR(photon_JpsiLV, fittedJpsiLV);
      
      // find the best PV again, based on fittedJpsiLV + photon_JpsiLV
      math::XYZTLorentzVector fittedJpsiPhotonLV = fittedJpsiLV + photon_JpsiLV;
      const reco::Vertex & bestPV_Jpsi = Tools::bestPV(primaryVertices, fittedJpsiVertexPosition, 
                            fittedJpsiPhotonLV.Vect());
      bool sameBestPV = (bestPV_Jpsi == bestPV);
      
      /*
      add photon, get LV and vertex position
      find the best PV again
      global fit with the pointing constraint
      */

    } // muon2
    

  } // muon1


}

DEFINE_FWK_MODULE(impJpsiG);
