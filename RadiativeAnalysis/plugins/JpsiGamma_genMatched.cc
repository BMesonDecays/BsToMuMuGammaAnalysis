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
class JpsiGamma : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit JpsiGamma(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~JpsiGamma();

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

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
    
  std::vector<int> JpsiG = {443, 22};
  std::vector<int> MuMu = {13, -13};

  TH1D* hGenDeltaR_dimuon_photon;
  TH1D* hRecoDeltaR_dimuon_photon;

  TNtupleD* tMuMuGamma;

};


JpsiGamma::JpsiGamma(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));

  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();

}

JpsiGamma::~JpsiGamma()
{
  cout <<" DTOR" << endl;
}

void JpsiGamma::beginJob()
{
    hGenDeltaR_dimuon_photon = new TH1D("hGenDeltaR_dimuon_photon","hGenDeltaR_dimuon_photon",100,0.,3.);
    hRecoDeltaR_dimuon_photon = new TH1D("hRecoDeltaR_dimuon_photon","hRecoDeltaR_dimuon_photon",100,0.,3.);

    tMuMuGamma = new TNtupleD("tMuMuGamma","tMuMuGamma","M_MuMu:eta_Gamma:M_MuMuGamma");
    
    cout << "HERE JpsiGamma::beginJob()" << endl;
}

void JpsiGamma::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  hGenDeltaR_dimuon_photon->Write();
  hRecoDeltaR_dimuon_photon->Write();

  tMuMuGamma->Write();

  myRootFile.Close();

  delete hGenDeltaR_dimuon_photon;
  delete hRecoDeltaR_dimuon_photon;
  delete tMuMuGamma;
  
  cout << "HERE JPsiGamma::endJob()" << endl;
}

void JpsiGamma::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE JpsiGamma::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  
  auto const& field = es.getData(m_fieldToken);

  vector<const reco::Candidate*> genMuons;
  vector<const reco::Muon*> recoMatchedMuons;
  vector<const reco::Candidate*> genMatchedMuons;

  vector<const reco::Candidate*> genPhotons;
  vector<const reco::Photon*> recoMatchedPhotons;
  vector<const reco::Candidate*> genMatchedPhotons;  
  
  ////////////////////////////////////////
  
  const reco::GenParticle* genB0sPtr = &genPar.at(0); //generated B0s decaying into mmg
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

  //if(recoMatchedPhotons[0]->isEB() == 0) return; // only EB photons

  // get deltaR (dimuon, photon)
  math::XYZVector genDimuonMomentum = genMuons.at(0)->momentum() + genMuons.at(1)->momentum();
  double deltaR_gen_dimuon_photon = reco::deltaR(genDimuonMomentum, *genPhotons.at(0));
  hGenDeltaR_dimuon_photon->Fill(deltaR_gen_dimuon_photon);

  math::XYZVector recoDimuonMomentum = recoMatchedMuons.at(0)->momentum() + recoMatchedMuons.at(1)->momentum();
  double deltaR_reco_dimuon_photon = reco::deltaR(recoDimuonMomentum, *recoMatchedPhotons.at(0));
  hRecoDeltaR_dimuon_photon->Fill(deltaR_reco_dimuon_photon);

  // eta of the photon's calorimeter
  double photonCaloEta = recoMatchedPhotons.at(0)->superCluster()->eta();

  
  ////////////////////////  FITTING /////////////////////

  // kinematic particle creation  
  vector<RefCountedKinematicParticle> muonKinematicParticles;
  for(const auto& recoMuPtr : recoMatchedMuons)
  {
    reco::Muon recoMu = *recoMuPtr;
    reco::TrackRef muTrack = recoMu.track();
    if(!muTrack) continue;
    reco::TransientTrack muonTT = reco::TransientTrack(muTrack, &field);

    const ParticleMass muon_mass(0.105658);
    float muon_sigma = 23E-10;

    KinematicParticleFactoryFromTransientTrack pFactory;
    muonKinematicParticles.push_back(pFactory.particle(muonTT, muon_mass, float(0), float(0), muon_sigma));
  }

  if (muonKinematicParticles.size() != 2) return;

  RefCountedKinematicParticle mu1 = muonKinematicParticles.at(0);
  RefCountedKinematicParticle mu2 = muonKinematicParticles.at(1);
  std::vector<RefCountedKinematicParticle> allParticles;
  allParticles.push_back(mu1);
  allParticles.push_back(mu2);

  // SV fit using only two muons
  KinematicParticleVertexFitter fitter;
  RefCountedKinematicTree vertexFitTree = fitter.fit(allParticles);
  if (!vertexFitTree->isValid()) return;

  // get the fitted particle (i.e. dimuon) and vertex
  vertexFitTree->movePointerToTheTop();
  RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
  RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
  if (!fitVertex->vertexIsValid()) return;

  double M_MuMu = fitParticle->currentState().mass();

  GlobalPoint fittedGlobalPoint = fitVertex->position();
  math::XYZPoint fittedSV (fittedGlobalPoint.x(),fittedGlobalPoint.y(),fittedGlobalPoint.z());

  KinematicParameters dimuonKinPar = fitParticle->currentState().kinematicParameters();
  GlobalVector dimuonMomentum = dimuonKinPar.momentum();
  math::XYZVector dimuonMomentum_math(dimuonMomentum.x(),dimuonMomentum.y(),dimuonMomentum.z());
  math::XYZTLorentzVector fitDimuonLVec (dimuonMomentum.x(),dimuonMomentum.y(),dimuonMomentum.z(), dimuonKinPar.energy());

  // photon enters
  reco::Photon recoPhoton = *recoMatchedPhotons.at(0);
  //recoPhoton.setVertex(reco::Candidate::Point(fittedSV.x(), fittedSV.y(), fittedSV.z()));
  math::XYZTLorentzVector recoPhotonLVec = recoPhoton.p4();

  // invariant mass of fitDimuonRecoPhoton
  auto fitDimuonRecoPhotonLVec = fitDimuonLVec + recoPhotonLVec;
  double M_MuMuGamma = fitDimuonRecoPhotonLVec.M();

  tMuMuGamma->Fill(M_MuMu, photonCaloEta, M_MuMuGamma);  
  

  /*
  //////////////PCA(mmg)_reco to the genPV////////////////////////////

  const reco::Candidate* firstB0sPtr = Tools::findFirstB0s(genB0sPtr);    //first produced B0s
  const math::XYZPoint genPV = firstB0sPtr->vertex();

  reco::Photon recoPhoton = *recoMatchedPhotons.at(0);
  recoPhoton.setVertex(reco::Candidate::Point(fittedSV.x(), fittedSV.y(), fittedSV.z()));

  // find the PCA
  math::XYZVector fitDimuonRecoPhotonMomentum = dimuonMomentum_math + recoPhoton.momentum();
  math::XYZPoint pca = Tools::pca(genPV, fittedSV, fitDimuonRecoPhotonMomentum);

  hRecoPCA_GenPV_z->Fill(std::abs(pca.z() - genPV.z()));  

  */


  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(JpsiGamma);
