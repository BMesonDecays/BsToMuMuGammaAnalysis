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
class MCimpJpsiG : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit MCimpJpsiG(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~MCimpJpsiG();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<reco::Muon> > theMuonToken;
  edm::EDGetTokenT < vector<reco::Photon> > thePhotonToken;
  edm::EDGetTokenT < vector<reco::Vertex> > theVertexToken; 
  //edm::EDGetTokenT < vector<reco::PackedCandidate> > thePackedCandidateToken;

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;

  TH1D* hLxySignificance;
  TH1D* hLifetime_ps;
  TH1D* hCosPointingAngle;

};


MCimpJpsiG::MCimpJpsiG(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlinePrimaryVertices"));
  //thePackedCandidateToken = consumes< vector<reco::PackedCandidate> >(edm::InputTag("packedPFCandidates"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

MCimpJpsiG::~MCimpJpsiG()
{
  cout <<" DTOR" << endl;
}

void MCimpJpsiG::beginJob()
{
  hLxySignificance = new TH1D("hLxySignificance","hLxySignificance",100,0.,1000);
  hLifetime_ps = new TH1D("hLifetime_ps","hLifetime [ps]",1000,0.,20.);
  hCosPointingAngle = new TH1D("hCosPointingAngle","hCosPointingAngle",1000,-1.,1.);

  cout << "HERE MCimpJpsiG::beginJob()" << endl;
}

void MCimpJpsiG::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  hLxySignificance->Write();
  hLifetime_ps->Write();
  hCosPointingAngle->Write();

  myRootFile.Close();

  delete hLxySignificance;
  delete hLifetime_ps;
  delete hCosPointingAngle;
  
  cout << "HERE MCimpJpsiG::endJob()" << endl;
}

void MCimpJpsiG::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE MCimpJpsiG::analyze "<< std::endl;

  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  //const std::vector<reco::PackedCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  
  auto const& field = es.getData(m_fieldToken);  
  
  ////////////////////////////////////////

  if (recoMuons.size() < 2 || recoPhotons.size() < 1)   return;

  KalmanVertexFitter kvf(false);
  double jpsiMass = 3.097;
  double bsMass = 5.367;

  // take two oppositely charged recoMuons
  for (std::vector<reco::Muon>::const_iterator im1 = recoMuons.begin(); im1 < recoMuons.end(); im1++)
  {
    reco::TrackRef mu1Track = im1->track();
    if (!mu1Track)  continue;
    reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);

    for (std::vector<reco::Muon>::const_iterator im2 = im1+1; im2 < recoMuons.end(); im2++)
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

      

      // find the photon closest in dR to the two muons LV
      // (however dR > 0.05)
      const reco::Photon* min_dR_photon = &recoPhotons.at(0);
      double min_dR_photon_twoMuons = 99.0;
      for (const auto& photon : recoPhotons)
      {
        math::XYZTLorentzVector recoGammaLV = photon.p4();
        double dR = reco::deltaR(recoGammaLV, twoMuonsLV);
        if (dR < min_dR_photon_twoMuons && dR > 0.05){
          min_dR_photon = &photon;
          min_dR_photon_twoMuons = dR;
        }
      }
      if (min_dR_photon_twoMuons > 0.4) continue;
      math::XYZTLorentzVector photonLV = min_dR_photon->p4();
      // got Lorentz vector of the selected photon

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

      // muonsKalmanVertex 3D displacement from bestPV
      math::XYZVector lXYZ_muonsKalman_PV_Vector = muonsKalmanVertex.position() - bestPV.position();
      double lXYZ_muonsKalman_PV = TMath::Sqrt(lXYZ_muonsKalman_PV_Vector.Mag2());
      double cosPointingAngle_muonsKalman_PV = twoMuonsPhotonLV.Vect().unit().Dot(lXYZ_muonsKalman_PV_Vector.unit());
      hCosPointingAngle->Fill(cosPointingAngle_muonsKalman_PV);

      // B0s lifetime based on lXY
      double lifetimeB0s = (lXY_muonsKalman_PV * bsMass) / twoMuonsPhotonLV.Vect().Rho();
      hLifetime_ps->Fill(lifetimeB0s*10/3); // should be 100, not 10




    } // muon2
    

  } // muon1


}

DEFINE_FWK_MODULE(MCimpJpsiG);
