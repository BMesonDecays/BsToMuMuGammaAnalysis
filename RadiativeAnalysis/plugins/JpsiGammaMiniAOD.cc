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
class JpsiGammaMiniAOD : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiGammaMiniAOD(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiGammaMiniAOD();

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

  TNtupleD* tMuMuGamma;

};


JpsiGammaMiniAOD::JpsiGammaMiniAOD(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theMuonToken = consumes< vector<pat::Muon>  >( edm::InputTag("slimmedMuons"));
  thePhotonToken = consumes< vector<pat::Photon>  >( edm::InputTag("slimmedPhotons"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlineSlimmedPrimaryVertices"));
  thePackedCandidateToken = consumes< vector<pat::PackedCandidate> >(edm::InputTag("packedPFCandidates"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

JpsiGammaMiniAOD::~JpsiGammaMiniAOD()
{
  cout <<" DTOR" << endl;
}

void JpsiGammaMiniAOD::beginJob()
{
    tMuMuGamma = new TNtupleD("tMuMuGamma","tMuMuGamma",
      "M_dimuon:ProbOfCommonMuonVertex:eta_photon:deltaR_dimuon_photon:minPCA_distance:M_dimuonGamma:minFlightPath:maxProb_MuMuNotGamma:nOfPhotons0_4:nOfPhotons0_1_0_4:nOfPhotons0_2_0_4");
    
    cout << "HERE JpsiGammaMiniAOD::beginJob()" << endl;
}

void JpsiGammaMiniAOD::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  tMuMuGamma->Write();

  myRootFile.Close();

  delete tMuMuGamma;
  
  cout << "HERE JpsiGammaMiniAOD::endJob()" << endl;
}

void JpsiGammaMiniAOD::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiGammaMiniAOD::analyze "<< std::endl;

  const std::vector<pat::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<pat::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);
  const std::vector<pat::PackedCandidate> & packedCandidates = ev.get(thePackedCandidateToken);
  
  auto const& field = es.getData(m_fieldToken);  
  
  ////////////////////////////////////////

  const ParticleMass muon_mass(0.105658);
  float muon_sigma = 23E-10;
  KinematicParticleFactoryFromTransientTrack pFactory;
  KinematicParticleVertexFitter fitter;

  // take two oppositely charged recoMuons
  for (std::vector<pat::Muon>::const_iterator im1 = recoMuons.begin(); im1 < recoMuons.end(); im1++)
  {
    reco::TrackRef mu1Track = im1->track();
    if (!mu1Track)  continue;
    reco::TransientTrack muon1TT = reco::TransientTrack(mu1Track, &field);
    
    for (std::vector<pat::Muon>::const_iterator im2 = im1+1; im2 < recoMuons.end(); im2++)
    {
      if (im1->charge() * im2->charge() != -1)  continue;

      // fit the vertex of two muons using the KinematicFitter
      reco::TrackRef mu2Track = im2->track();
      if (!mu2Track) continue;
      reco::TransientTrack muon2TT = reco::TransientTrack(mu2Track, &field);

      std::vector<RefCountedKinematicParticle> muonKinematicParticles;
      muonKinematicParticles.push_back(pFactory.particle(muon1TT, muon_mass, float(0), float(0), muon_sigma));
      muonKinematicParticles.push_back(pFactory.particle(muon2TT, muon_mass, float(0), float(0), muon_sigma));
      if (muonKinematicParticles.size() != 2) continue;

      RefCountedKinematicTree vertexFitTree = fitter.fit(muonKinematicParticles); // was "allParticles"
      if (!vertexFitTree->isValid()) continue;

      // get the fitted particle (i.e. dimuon) and vertex
      vertexFitTree->movePointerToTheTop();
      RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
      RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
      if (!fitVertex->vertexIsValid()) return;
      double commonVertexProb = TMath::Prob(fitVertex->chiSquared(), fitVertex->degreesOfFreedom());
      if (commonVertexProb < 0.1) continue; // cut on the common vertex prob. of two muons

      double M_dimuon = fitParticle->currentState().mass(); // invariant mass of the fitted dimuon

      if (std::fabs(M_dimuon - 3.097) > 0.2)  continue; // constraint on M_mumu ~ M_Jpsi

      GlobalPoint fittedGlobalPoint = fitVertex->position();
      math::XYZPoint fittedSV (fittedGlobalPoint.x(),fittedGlobalPoint.y(),fittedGlobalPoint.z());

      KinematicParameters dimuonKinPar = fitParticle->currentState().kinematicParameters();
      GlobalVector dimuonMomentum = dimuonKinPar.momentum();
      math::XYZTLorentzVector lFitDimuonVector (dimuonMomentum.x(),dimuonMomentum.y(),dimuonMomentum.z(), dimuonKinPar.energy());

      // check if no other packed candidate is compatible with the two muons vertex (get the maximal probability)
      double maxVertexProb = 0.0;
      std::vector<reco::TransientTrack> trackTTs;
      trackTTs.push_back(muon1TT);
      trackTTs.push_back(muon2TT);
      KalmanVertexFitter kvf(true);
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
        if (prob3part > maxVertexProb)
          maxVertexProb = prob3part;
        
        trackTTs.pop_back();
      }
      
      // take a photon
      int nOfPhotons0_4 = 0;
      int nOfPhotons0_1_0_4 = 0;
      int nOfPhotons0_2_0_4 = 0;
      for (std::vector<pat::Photon>::const_iterator igamma = recoPhotons.begin(); igamma < recoPhotons.end(); igamma++)
      {
        double photonCaloEta = igamma->superCluster()->eta();

        math::XYZTLorentzVector lRecoGammaVector = igamma->p4();
        double deltaR_fitDimuon_recoPhoton = reco::deltaR(lFitDimuonVector, lRecoGammaVector);

        // count number of photons in the event with deltaR < 0.4 (and >0.1/0/2)
        if (deltaR_fitDimuon_recoPhoton < 0.4)
        {
          nOfPhotons0_4 ++;
          nOfPhotons0_1_0_4 += (int)(deltaR_fitDimuon_recoPhoton > 0.1);
          nOfPhotons0_2_0_4 += (int)(deltaR_fitDimuon_recoPhoton > 0.2);
        }

        // consider the closest approach distance of fitDimuonRecoPhoton to the best (closest) recoPV
        math::XYZTLorentzVector lFitDimuonRecoPhotonVector = lFitDimuonVector + lRecoGammaVector;

        std::vector<math::XYZPoint> closestPoints = Tools::points_PV_pca (primaryVertices, fittedSV,
                                                                lFitDimuonRecoPhotonVector.Vect());
        auto bestPVposition = closestPoints.at(0);
        auto bestPCA = closestPoints.at(1);
        
        double closestAppDistance = TMath::Sqrt((bestPVposition-bestPCA).Mag2());

        double M_dimuonGamma = lFitDimuonRecoPhotonVector.M();

        double flightLength = TMath::Sqrt((fittedSV - bestPVposition).Mag2());

        // fill the Ntuple
        tMuMuGamma->Fill(M_dimuon, commonVertexProb, photonCaloEta, deltaR_fitDimuon_recoPhoton, closestAppDistance, 
          M_dimuonGamma, flightLength, maxVertexProb, (double)nOfPhotons0_4,(double)nOfPhotons0_1_0_4,(double)nOfPhotons0_2_0_4);

      }


    } 
  }


  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(JpsiGammaMiniAOD);
