// based on impJpsiG
// after presentation on March 4th

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
class JpsiMassG : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiMassG(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiMassG();

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

  TH1D* hCompAll;
  TNtupleD* tOut;

};

JpsiMassG::JpsiMassG(const edm::ParameterSet& conf)
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

JpsiMassG::~JpsiMassG()
{
  cout <<" DTOR" << endl;
}

void JpsiMassG::beginJob()
{
    hCompAll = new TH1D("hCompAll","Third candidate compatibility (one entry per candidate)",1000,0.,1.);
    tOut = new TNtupleD("tOut","Output tuple",
        "twoMuonsMass:muonsKalmanVxProb:maxMuonsComp:fittedJpsiVxProb:lXY_twoMuons_bSpot:lXY_fittedJpsi_bSpot:dR_photon_twoMuons:dR_photon_Jpsi:twoMuonsPhotonMass:fittedJpsiPhotonMass:lXY_twoMuons_PV:lXY_fittedJpsi_PV:lXY_twoMuons_PV_sign:lXY_fittedJpsi_PV_sign:lXYZ_twoMuons_bestPV:lXYZ_fittedJpsi_bestPV:cosPointingAngle_twoMuons:cosPointingAngle_fittedJpsi:BsXYlifetime_twoMuons:BsXYlifetime_fittedJpsi");
  
  cout << "HERE JpsiMassG::beginJob()" << endl;
}

void JpsiMassG::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write data
  hCompAll->Write();
  tOut->Write();

  myRootFile.Close();

  delete hCompAll;
  delete tOut;

  cout << "HERE JpsiMassG::endJob()" << endl;
}

void JpsiMassG::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiMassG::analyze "<< std::endl;

  const std::vector<pat::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<pat::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  const std::vector<pat::PackedCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);

  auto const& field = es.getData(m_fieldToken);  
  
  ////////////////////////////////////////

  if (recoMuons.size() < 2 || recoPhotons.size() < 1)   return;

  KalmanVertexFitter kvf(false);
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

    for (std::vector<pat::Muon>::const_iterator im2 = im1+1; im2 < recoMuons.end(); im2++)
    {
      if (im1->charge() * im2->charge() != -1)  continue;

      math::XYZTLorentzVector twoMuonsLV = im1->p4() + im2->p4();
      double twoMuonsM = twoMuonsLV.M();
      if (std::fabs(twoMuonsM - jpsiMass) > 0.15) continue;
      // got two muons Lorentz Vector - candidate for J/psi

      reco::TrackRef mu2Track = im2->track();
      if (!mu2Track) continue;
      reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);

      std::vector<reco::TransientTrack> trackTTs;
      trackTTs.push_back(muon1TT);
      trackTTs.push_back(muon2TT);

      reco::Vertex muonsKalmanVertex (TransientVertex(kvf.vertex(trackTTs)));
      double muonsKalmanVxProb = TMath::Prob(muonsKalmanVertex.chi2(), muonsKalmanVertex.ndof());
      if (muonsKalmanVxProb < 0.01)  continue;
      // got muonsKalmanVertex

      // check if no other packed candidate is compatible with the two muons vertex (get the maximal probability)
      double maxMuonsVertexComp = Tools::getMaxCompatibility (packedCandidates, mu1Track, mu2Track, field, trackTTs);      

      // histogram all compatibilities
      for (std::vector<pat::PackedCandidate>::const_iterator icand = packedCandidates.begin(); icand < packedCandidates.end(); icand++)
      {
        if (! icand->hasTrackDetails()) continue;
        const reco::Track candTrack = icand->pseudoTrack();

        // deltaR check to eliminate the already used two muons
        if(std::min(reco::deltaR(candTrack,*mu1Track),reco::deltaR(candTrack,*mu2Track))<0.0003) continue;

        reco::TransientTrack candTT = reco::TransientTrack(candTrack, &field);
        trackTTs.push_back(candTT);
        reco::Vertex v3part (TransientVertex(kvf.vertex(trackTTs)));
        double prob3part = TMath::Prob(v3part.chi2(),v3part.ndof());
        
        hCompAll->Fill(prob3part);
        trackTTs.pop_back();
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
        && lXY_fittedJpsi_bSpot_significance < 3.0)  continue;
     
      //////LOOP OVER PHOTONS/////////
      for (std::vector<pat::Photon>::const_iterator igamma = recoPhotons.begin(); igamma < recoPhotons.end(); igamma++)
      {
        // photon deltaR
        math::XYZTLorentzVector photonLV = igamma->p4();
        double deltaR_photon_twoMuons = reco::deltaR(twoMuonsLV, photonLV);
        double deltaR_photon_Jpsi = reco::deltaR(fittedJpsiLV, photonLV);

        if (deltaR_photon_twoMuons > 0.4
            && deltaR_photon_Jpsi > 0.4)   continue;
        if (deltaR_photon_twoMuons < 0.05
            && deltaR_photon_Jpsi < 0.05)  continue;

        // addition of the photon LorentzVector
        // to get (twoMuons/fittedJpsi)Photon Lorentz vector - candidate for B0s
        math::XYZTLorentzVector twoMuonsPhotonLV = twoMuonsLV + photonLV;
        double twoMuonsPhotonMass = twoMuonsPhotonLV.M();
        math::XYZTLorentzVector fittedJpsiPhotonLV = fittedJpsiLV + photonLV;
        double fittedJpsiPhotonMass = fittedJpsiPhotonLV.M();

        // initial check of B0s mass
        if (std::fabs(twoMuonsPhotonMass - bsMass) > 1.5
            && std::fabs(fittedJpsiPhotonMass - bsMass) > 1.5) continue;        

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
            && lXY_fittedJpsi_PV_significance < 3.0)   continue;

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

DEFINE_FWK_MODULE(JpsiMassG);