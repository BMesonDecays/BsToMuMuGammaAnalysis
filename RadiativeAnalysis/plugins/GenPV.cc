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
#include "TH1I.h"


#include <sstream>
#include <iomanip> 
#include <utility>
#include <numeric>


using namespace std;



//object definition
class GenPV : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit GenPV(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~GenPV();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  bool isSameDecay(const std::vector<int>&, const std::vector<int>&);

private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<reco::GenParticle> > theGenParticleToken;
  edm::EDGetTokenT < vector<reco::Muon> > theMuonToken;
  edm::EDGetTokenT < vector<reco::Photon> > thePhotonToken;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;
  edm::ESGetToken <TransientTrackBuilder, TransientTrackRecord> theTransientTrackBuilderToken;

  edm::EDGetTokenT < reco::BeamSpot > theBeamSpotToken;
  edm::EDGetTokenT < vector<reco::Vertex> > theVertexToken;
    
  std::vector<int> MuMuG = {22, 13, -13};

  TH1D* hRecoPca_bestRecoPV_z;
  TH1D* hBestRecoPV_GenPV_z;
  TH1I* hLargestPt;
  TH1I* hLargestNTr;
  TH1D* hPtClosOverLarg;
  TH1D* hNTrClosOverLarg;
  TH1I* hLargestPtAndNTr;

};


GenPV::GenPV(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();
  theTriggerResultsToken = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
  theTransientTrackBuilderToken = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlinePrimaryVertices"));
  theBeamSpotToken = consumes< reco::BeamSpot >( edm::InputTag("offlineBeamSpot"));

}

GenPV::~GenPV()
{
  cout <<" DTOR" << endl;
}


bool GenPV::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}

void GenPV::beginJob()
{
    hRecoPca_bestRecoPV_z = new TH1D("hRecoPca_bestRecoPV_z","hRecoPca_bestRecoPV_z",100,0.,0.012);
    hBestRecoPV_GenPV_z = new TH1D("hBestRecoPV_GenPV_z","hBestRecoPV_GenPV_z",100,0.,0.012);
    hLargestPt = new TH1I("hLargestPt","Has closest PV largest p_t",2,0,2);
    hLargestNTr = new TH1I("hLargestNTr","Has closest PV largest n_tracks",2,0,2);
    hPtClosOverLarg = new TH1D("hPtClosOverLarg","p_t (closest/largest)",100,0.,1.);
    hNTrClosOverLarg = new TH1D("hNTrClosOverLarg","n_tracks (closest/largest)",100,0.,1.);
    hLargestPtAndNTr = new TH1I("hLargestPtAndNTr","Has closest PV largest p_t and n_tracks",2,0,2);

    cout << "HERE GenPV::beginJob()" << endl;
}

void GenPV::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  hRecoPca_bestRecoPV_z->Write();
  hBestRecoPV_GenPV_z->Write();
  hLargestPt->Write();
  hLargestNTr->Write();
  hPtClosOverLarg->Write();
  hNTrClosOverLarg->Write();
  hLargestPtAndNTr->Write();

  myRootFile.Close();

  delete hRecoPca_bestRecoPV_z;
  delete hBestRecoPV_GenPV_z;
  delete hLargestPt;
  delete hLargestNTr;
  delete hPtClosOverLarg;
  delete hNTrClosOverLarg;
  delete hLargestPtAndNTr;


  cout << "HERE GenPV::endJob()" << endl;
}

void GenPV::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE GenPV::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);

  auto const& field = es.getData(m_fieldToken);

  vector<const reco::Candidate*> genMuons;
  vector<const reco::Muon*> recoMatchedMuons;
  vector<const reco::Candidate*> genMatchedMuons;

  vector<const reco::Candidate*> genPhotons;
  vector<const reco::Photon*> recoMatchedPhotons;
  vector<const reco::Candidate*> genMatchedPhotons;  
  
  ////////////////////////////////////////
  
  GlobalPoint genSV;
  const reco::GenParticle* genB0sPtr = &genPar.at(0); //generated B0s decaying into mmg
  
  // find B0s decaying into mmg
  for(const auto& genP : genPar)
  {
    if (abs(genP.pdgId()) == 531)
    {
      vector<int> daughters;
      for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
      {
        daughters.push_back(genP.daughter(i)->pdgId());
      }
      if(isSameDecay(daughters, MuMuG))
      {
        genB0sPtr = &genP;
        genSV = GlobalPoint(genP.daughter(0)->vx(), genP.daughter(0)->vy(), genP.daughter(0)->vz());
        
        for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
        {
          if(abs(genP.daughter(i)->pdgId()) == 13) genMuons.push_back(genP.daughter(i));
          if(abs(genP.daughter(i)->pdgId()) == 22) genPhotons.push_back(genP.daughter(i));
        }
      }
    }
  }
  if(genPhotons.size() == 0) return;  //no 'SameDecay' found  


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
    if (matched && minDR < 0.01)
    {
      recoMatchedMuons.push_back(bestMatchedMuon);
      genMatchedMuons.push_back(genMu);
    }
  }
  if(recoMatchedMuons.size() != 2)  return;


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
    if (matched && minDR < 0.03)
    {
      recoMatchedPhotons.push_back(bestMatchedPhoton);
      genMatchedPhotons.push_back(genPh);
    }
  }
  if(recoMatchedPhotons.size() != 1) return;

  if(recoMatchedPhotons[0]->isEB() == 0) return; // only EB photons


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

  GlobalPoint fittedGlobalPoint = fitVertex->position();
  math::XYZPoint fittedSV (fittedGlobalPoint.x(),fittedGlobalPoint.y(),fittedGlobalPoint.z());

  GlobalVector dimuonMomentum = fitParticle->currentState().kinematicParameters().momentum();
  math::XYZVector dimuonMomentum_math(dimuonMomentum.x(),dimuonMomentum.y(),dimuonMomentum.z());
  

  //////////////PCA(mmg)_reco to the bestRecoPV////////////////////////////

  const reco::Candidate* firstB0sPtr = Tools::findFirstB0s(genB0sPtr);    //first produced B0s
  const math::XYZPoint genPV = firstB0sPtr->vertex();

  reco::Photon recoPhoton = *recoMatchedPhotons.at(0);
  recoPhoton.setVertex(reco::Candidate::Point(fittedSV.x(), fittedSV.y(), fittedSV.z()));

  math::XYZVector fitDimuonRecoPhotonMomentum = dimuonMomentum_math + recoPhoton.momentum();

  double minDistPcaPV = 1000.0;
  unsigned int bestPrimVertexIndex = 0;
  math::XYZPoint pca(0.,0.,0.);
  // find the best reco PV
  for(long unsigned int i=0; i<primaryVertices.size(); i++)
  {
    math::XYZPoint tempPca = Tools::pca(primaryVertices.at(i).position(), fittedSV, fitDimuonRecoPhotonMomentum);
    math::XYZVector tempDiffPcaPV = tempPca - primaryVertices.at(i).position();
    double tempDistPcaPV = TMath::Sqrt(tempDiffPcaPV.mag2());

    if (tempDistPcaPV < minDistPcaPV)
    {
      minDistPcaPV = tempDistPcaPV;
      bestPrimVertexIndex = i;
      pca = tempPca;
    }
  }
  if(minDistPcaPV > 0.01) return;

  reco::Vertex bestPV = primaryVertices.at(bestPrimVertexIndex);

  hRecoPca_bestRecoPV_z->Fill(std::abs(pca.z() - bestPV.position().z()) );
  hBestRecoPV_GenPV_z->Fill(std::abs(bestPV.position().z() - genPV.z()) );

  // more than one recoPV
  if (primaryVertices.size() > 1)
  {
    unsigned int closestPVIndex = bestPrimVertexIndex;
    double minRecoPV_GenPV_z = std::abs(bestPV.position().z() - genPV.z());

    // find the recoPV closest (in z) to the genPV
    for(unsigned int i=0; i<primaryVertices.size();i++)
    {
        double tempRecoPV_GenPV_z = std::abs(primaryVertices.at(i).position().z() - genPV.z());
        if (tempRecoPV_GenPV_z < minRecoPV_GenPV_z)
        {
            minRecoPV_GenPV_z = tempRecoPV_GenPV_z;
            closestPVIndex = i;
        }
    }
    reco::Vertex closestPV = primaryVertices.at(closestPVIndex);

    // check the p_t and n_tr of recoPVs
    std::vector<double> pTVector;
    std::vector<unsigned int> nTrVector;
    for (auto &pv : primaryVertices)
    {
        pTVector.push_back(pv.p4().pt());
        nTrVector.push_back(pv.nTracks());
    }

    auto max_it = std::max_element(pTVector.begin(),pTVector.end());
    unsigned int largestPtIndex = std::distance(pTVector.begin(), max_it);

    auto max_it2 = std::max_element(nTrVector.begin(),nTrVector.end());
    unsigned int largestNTrIndex = std::distance(nTrVector.begin(), max_it2);

    // fill histograms
    if (largestPtIndex == closestPVIndex)
        hLargestPt->Fill(1);
    else
    {
        hLargestPt->Fill(0);
        hPtClosOverLarg->Fill(closestPV.p4().pt() / pTVector.at(largestPtIndex));
    }
    if (largestNTrIndex == closestPVIndex)
        hLargestNTr->Fill(1);
    else
    {
        hLargestNTr->Fill(0);
        hNTrClosOverLarg->Fill(closestPV.nTracks() / nTrVector.at(largestNTrIndex));
    }

    hLargestPtAndNTr->Fill(int(largestPtIndex == closestPVIndex && largestNTrIndex == closestPVIndex));    

  }




  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(GenPV);
