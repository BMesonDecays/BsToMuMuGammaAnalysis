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
#include "TTree.h"
#include "TBranch.h"

#include <sstream>
#include <iomanip> 
#include <utility>
#include <numeric>


using namespace std;


//object definition
class BsPtBalance : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit BsPtBalance(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~BsPtBalance();

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

  // histograms

  TH1D* hBsMass;
  TH1D* hBsMassFromP4;
  TH1D* hMuPt;
  TH1D* hGammaPt;
  TH1D* hGammaDeltaR;
  TH1D* hGammaPtWithTrigger;

  TH1D* hBsMassResidual;

  TH1D* hBsMassResidualCorrected;

  TH1D* hDimuonMassResidualNoFit;
  TH1D* hDimuonMassResidualWithFit;
  TH1D* hMuonFitGenGammaResidual;

  TH1D* hScaledBsMassResidual;

  TH1D* hDimuonGammaCosInBsFrame;

  TH1D* hScale;
  TH1D* hCorrScale;

  TProfile* hRecoVsGenEnergyProfile;
  TProfile* hRecoVsGenPtBsFrameMuonProfile;
  TProfile* hRecoVsGenPtBsFramePhotonProfile;
  TProfile* hRecoVsGenPtBsFramePhotonProfileCorrected;


  TH1D* hDimuonAngleInBsFrame;
  TH1D* hPhotonAngleInBsFrame;

  TH1D* hDimuonVertexXResidual;
  TH1D* hDimuonVertexYResidual;
  TH1D* hDimuonVertexZResidual;

  TH1D* hUnscaledRecoVsGenPhotonEnergy;
  TH1D* hScaledRecoVsGenPhotonEnergy;
  TH1D* hUnscaledCorrRecoVsGenPhotonEnergy;
  TH1D* hScaledCorrRecoVsGenPhotonEnergy;

  TH1D* hMuondR;
  TH1D* hPhotondR;
  TH1D* hPcaPV;
  TH1D* hBestPcaPV;
  TH1D* hCos_GenB_PvToSv;

  TTree* theTree;
  std::vector<float> vPVToSV;
  std::vector<float> vPhotonMomentum;
  std::vector<float> vDimuonMomentum;


  int nConvPhotons = 0;
  std::vector<int> MuMuG = {22, 13, -13};
};


BsPtBalance::BsPtBalance(const edm::ParameterSet& conf)
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

BsPtBalance::~BsPtBalance()
{
  cout <<" DTOR" << endl;
}

bool BsPtBalance::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}


void BsPtBalance::beginJob()
{
  //create a histogram

  hBsMass = new TH1D("hBsMass", "hBsMass", 50, 3, 7);
  hBsMassFromP4 = new TH1D("hBsMassFromP4", "hBsMassFromP4", 50, 3, 7);
  hMuPt = new TH1D("hMuPt", "hMuPt", 100, 0, 30);
  hGammaPt = new TH1D("hGammaPt", "hGammaPt", 100, 0, 30);
  hGammaDeltaR = new TH1D("hGammaDeltaR", "hGammaDeltaR", 100, 0, 0.05);
  hGammaPtWithTrigger = new TH1D("hGammaPtWithTrigger", "hGammaPtWithTrigger", 100, 0, 30);

  hBsMassResidual = new TH1D("hBsMassResidual", "hBsMassResidual", 100, -0.5, 0.5);

  hBsMassResidualCorrected = new TH1D("hBsMassResidualCorrected", "hBsMassResidualCorrected", 100, -0.5, 0.5);

  hDimuonMassResidualNoFit = new TH1D("hDimuonMassResidualNoFit", "hDimuonMassResidualNoFit", 100, -0.2, 0.2);
  hDimuonMassResidualWithFit = new TH1D("hDimuonMassResidualWithFit", "hDimuonMassResidualWithFit", 100, -0.2, 0.2);
  hMuonFitGenGammaResidual = new TH1D("hMuonFitGenGammaResidual", "hMuonFitGenGammaResidual", 100, -0.2, 0.2);

  hScaledBsMassResidual = new TH1D("hScaledBsMassResidual", "hScaledBsMassResidual", 100, -0.5, 0.5);

  hDimuonGammaCosInBsFrame = new TH1D("hDimuonGammaCosInBsFrame", "hDimuonGammaCosInBsFrame", 100, -1, 1);

  hScale = new TH1D("hScale", "hScale", 100, -2, 2);
  hCorrScale = new TH1D("hCorrScale", "hCorrScale", 100, -2, 2);

  hRecoVsGenEnergyProfile = new TProfile("hRecoVsGenEnergyProfile", "hRecoVsGenEnergyProfile; gen Energy; reco Energy", 35, 0, 35, "s");

  hRecoVsGenPtBsFrameMuonProfile = new TProfile("hRecoVsGenPtBsFrameMuonProfile", "hRecoVsGenPtBsFrameMuonProfile; gen Pt in Bs frame; reco Pt in Bs frame", 15, 0, 3, "s");
  hRecoVsGenPtBsFramePhotonProfile = new TProfile("hRecoVsGenPtBsFramePhotonProfile", "hRecoVsGenPtBsFramePhotonProfile; gen Pt in Bs frame; reco Pt in Bs frame", 15, 0, 3, "s");
  hRecoVsGenPtBsFramePhotonProfileCorrected = new TProfile("hRecoVsGenPtBsFramePhotonProfileCorrected", "hRecoVsGenPtBsFramePhotonProfileCorrected; gen Pt in Bs frame; reco Pt in Bs frame", 15, 0, 3, "s");

  hDimuonAngleInBsFrame = new TH1D("hDimuonAngleInBsFrame", "hDimuonAngleInBsFrame; dimuon in Bs frame, reco vs gen [degrees]", 100, 0, 10);
  hPhotonAngleInBsFrame = new TH1D("hPhotonAngleInBsFrame", "hPhotonAngleInBsFrame; gamma in Bs frame, reco vs gen [degrees]", 100, 0, 10);

  hDimuonVertexXResidual = new TH1D("hDimuonVertexXResidual", "hDimuonVertexXResidual; [cm]", 100, -0.05, 0.05);
  hDimuonVertexYResidual = new TH1D("hDimuonVertexYResidual", "hDimuonVertexYResidual; [cm]", 100, -0.05, 0.05);
  hDimuonVertexZResidual = new TH1D("hDimuonVertexZResidual", "hDimuonVertexZResidual; [cm]", 100, -0.05, 0.05);


  hUnscaledRecoVsGenPhotonEnergy = new TH1D("hUnscaledRecoVsGenPhotonEnergy","hUnscaledRecoVsGenPhotonEnergy", 500, -50.,50.);
  hScaledRecoVsGenPhotonEnergy = new TH1D("hScaledRecoVsGenPhotonEnergy","hScaledRecoVsGenPhotonEnergy", 500, -50.,50.);
  hUnscaledCorrRecoVsGenPhotonEnergy = new TH1D("hUnscaledCorrRecoVsGenPhotonEnergy","hUnscaledCorrRecoVsGenPhotonEnergy", 500, -50.,50.);
  hScaledCorrRecoVsGenPhotonEnergy = new TH1D("hScaledCorrRecoVsGenPhotonEnergy","hScaledCorrRecoVsGenPhotonEnergy", 500, -50.,50.);

  hMuondR = new TH1D("hMuondR","all reco vs one gen muon; dR; ", 100, 0., 0.1);
  hPhotondR = new TH1D("hPhotondR","all reco vs one gen photon; dR; ", 100, 0., 0.1);
  hPcaPV = new TH1D("hPcaPV","pca-pv distance for fittedDimuonGenPhoton (every PV); distance;", 100,0.,0.3);
  hBestPcaPV = new TH1D("hBestPcaPV","pca-pv distance for fittedDimuonGenPhoton (best PV); distance;", 100,0.,0.05);
  hCos_GenB_PvToSv = new TH1D("hCos_GenB_PvToSv","hCos_GenB_PvToSv",100,0.9,1.);


  theTree = new TTree("theTree", "theTree");
  theTree->Branch("vPVToSV", &vPVToSV);
  theTree->Branch("vPhotonMomentum", &vPhotonMomentum);
  theTree->Branch("vDimuonMomentum", &vDimuonMomentum);

  cout << "HERE BsPtBalance::beginJob()" << endl;
}

void BsPtBalance::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data

  hBsMass->Write();
  hBsMassFromP4->Write();
  hMuPt->Write();
  hGammaPt->Write();
  hGammaDeltaR->Write();
  hGammaPtWithTrigger->Write();

  hBsMassResidual->Write();

  hBsMassResidualCorrected->Write();

  hDimuonMassResidualNoFit->Write();
  hDimuonMassResidualWithFit->Write();
  hMuonFitGenGammaResidual->Write();

  hScaledBsMassResidual->Write();

  hDimuonGammaCosInBsFrame->Write();

  hScale->Write();
  hCorrScale->Write();

  hRecoVsGenEnergyProfile->Write();

  hRecoVsGenPtBsFrameMuonProfile->Write();
  hRecoVsGenPtBsFramePhotonProfile->Write();
  hRecoVsGenPtBsFramePhotonProfileCorrected->Write();

  hDimuonAngleInBsFrame->Write();
  hPhotonAngleInBsFrame->Write();

  hDimuonVertexXResidual->Write();
  hDimuonVertexYResidual->Write();
  hDimuonVertexZResidual->Write();

  hUnscaledRecoVsGenPhotonEnergy->Write();
  hScaledRecoVsGenPhotonEnergy->Write();
  hUnscaledCorrRecoVsGenPhotonEnergy->Write();
  hScaledCorrRecoVsGenPhotonEnergy->Write();

  hMuondR->Write();
  hPhotondR->Write();
  hPcaPV->Write();
  hBestPcaPV->Write();
  hCos_GenB_PvToSv->Write();
  
  // theTree->Write();

  myRootFile.Close();

  delete hBsMass;
  delete hBsMassFromP4;
  delete hMuPt;
  delete hGammaPt;
  delete hGammaDeltaR;
  delete hGammaPtWithTrigger;

  delete hBsMassResidual;

  delete hBsMassResidualCorrected;

  delete hDimuonMassResidualNoFit;
  delete hDimuonMassResidualWithFit;
  delete hMuonFitGenGammaResidual;

  delete hScaledBsMassResidual;

  delete hDimuonGammaCosInBsFrame;

  delete hScale;
  delete hCorrScale;

  delete hRecoVsGenEnergyProfile;

  delete hRecoVsGenPtBsFrameMuonProfile;
  delete hRecoVsGenPtBsFramePhotonProfile;
  delete hRecoVsGenPtBsFramePhotonProfileCorrected;

  delete hDimuonAngleInBsFrame;
  delete hPhotonAngleInBsFrame;

  delete hDimuonVertexXResidual;
  delete hDimuonVertexYResidual;
  delete hDimuonVertexZResidual;

  delete hUnscaledRecoVsGenPhotonEnergy;
  delete hScaledRecoVsGenPhotonEnergy;
  delete hUnscaledCorrRecoVsGenPhotonEnergy;
  delete hScaledCorrRecoVsGenPhotonEnergy;

  delete hMuondR;
  delete hPhotondR;
  delete hPcaPV;
  delete hBestPcaPV;
  delete hCos_GenB_PvToSv;

  delete theTree;

  cout << "HERE BsPtBalance::endJob()" << endl;
}


void BsPtBalance::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE BsPtBalance::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  auto const& field = es.getData(m_fieldToken);

  const TransientTrackBuilder* theB = &es.getData(theTransientTrackBuilderToken);

  const edm::TriggerResults & triggerResults = ev.get(theTriggerResultsToken);
  edm::TriggerNames triggerNames = ev.triggerNames(triggerResults);

  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);


  vector<const reco::Candidate*> genMuons;
  vector<const reco::Muon*> recoMatchedMuons;
  vector<const reco::Candidate*> genMatchedMuons;

  vector<const reco::Candidate*> genPhotons;
  vector<const reco::Photon*> recoMatchedPhotons;
  vector<const reco::Candidate*> genMatchedPhotons;


  GlobalPoint genSV;
  reco::GenParticle genB0s = genPar.at(0); //generated B0s decaying into mmg
  // genMuons and genPhotons from BsToMuMuGamma decay channel
  // checked that only one such decay per event
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
        for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
        {
          if(abs(genP.daughter(i)->pdgId()) == 13) genMuons.push_back(genP.daughter(i));
          if(abs(genP.daughter(i)->pdgId()) == 22) genPhotons.push_back(genP.daughter(i));
        }
        genSV = GlobalPoint(genP.daughter(0)->vx(), genP.daughter(0)->vy(), genP.daughter(0)->vz());
        genB0s = genP;
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
    if(matched) 
      hMuondR->Fill(minDR);
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
    if(matched) 
      hPhotondR->Fill(minDR);
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
    hMuPt->Fill(recoMu.pt());
    reco::TrackRef muTrack = recoMu.track();
    if(!muTrack) continue;
    reco::TransientTrack muonTT = reco::TransientTrack(muTrack, &field);

    const ParticleMass muon_mass(0.105658);
    float muon_sigma = 1E-6;

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
  cout << "Fitting" << endl;
  RefCountedKinematicTree vertexFitTree = fitter.fit(allParticles);
  if (!vertexFitTree->isValid()) return;

  // get the fitted particle (i.e. dimuon) and vertex
  vertexFitTree->movePointerToTheTop();
  RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
  RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
  if (!fitVertex->vertexIsValid()) return;

  GlobalPoint fittedGlobalPoint = fitVertex->position();
  math::XYZPoint fittedGlobalPoint_math (fittedGlobalPoint.x(),fittedGlobalPoint.y(),fittedGlobalPoint.z());

  hDimuonVertexXResidual->Fill(fittedGlobalPoint.x() - genSV.x());
  hDimuonVertexYResidual->Fill(fittedGlobalPoint.y() - genSV.y());
  hDimuonVertexZResidual->Fill(fittedGlobalPoint.z() - genSV.z());

  GlobalVector dimuonMomentum = fitParticle->currentState().kinematicParameters().momentum();
  math::XYZVector dimuonMomentum_math(dimuonMomentum.x(),dimuonMomentum.y(),dimuonMomentum.z());
  math::XYZTLorentzVector dimuonP4(dimuonMomentum.x(), dimuonMomentum.y(), dimuonMomentum.z(), fitParticle->currentState().kinematicParameters().energy());

  hDimuonMassResidualNoFit->Fill((recoMatchedMuons.at(0)->p4() + recoMatchedMuons.at(1)->p4()).mass() - (genMatchedMuons.at(0)->p4() + genMatchedMuons.at(1)->p4()).mass());
  hDimuonMassResidualWithFit->Fill(dimuonP4.mass() - (genMatchedMuons.at(0)->p4() + genMatchedMuons.at(1)->p4()).mass());

  // find the PCA to PVs using recoMuons (fitDimuon) and genPhoton
  math::XYZVector fitDimuonGenPhotonMomentum = dimuonMomentum_math + genMatchedPhotons.at(0)->momentum();

  double minDistPcaPV = 1000.0;
  reco::Vertex bestPrimVertex;

  for(long unsigned int i=0; i<primaryVertices.size(); i++)
  {
    math::XYZPoint pca = Tools::pca(primaryVertices.at(i).position(), fittedGlobalPoint_math, fitDimuonGenPhotonMomentum);
    math::XYZVector tempDiffPcaPV = pca - primaryVertices.at(i).position();
    double tempDistPcaPV = TMath::Sqrt(tempDiffPcaPV.mag2());

    hPcaPV->Fill(tempDistPcaPV);

    if (tempDistPcaPV < minDistPcaPV)
    {
      minDistPcaPV = tempDistPcaPV;
      bestPrimVertex = primaryVertices.at(i);
    }
  }
  hBestPcaPV->Fill(minDistPcaPV);

  if(minDistPcaPV > 0.01) return;


  // photon enters  
  if(recoMatchedPhotons.size() != 1) return;  //redundant

  hMuonFitGenGammaResidual->Fill((dimuonP4 + genMatchedPhotons.at(0)->p4()).mass() - (genMatchedMuons.at(0)->p4() + genMatchedMuons.at(1)->p4() + genMatchedPhotons.at(0)->p4()).mass());

  reco::Photon recoPhoton = *recoMatchedPhotons.at(0);

  recoPhoton.setVertex(reco::Candidate::Point(fittedGlobalPoint.x(), fittedGlobalPoint.y(), fittedGlobalPoint.z()));
  math::XYZTLorentzVector photonP4 = recoPhoton.p4();
  GlobalVector photonMomentum(photonP4.x(), photonP4.y(), photonP4.z());

  double BsMass = (dimuonP4 + photonP4).mass();
  hBsMass->Fill(BsMass);
  hBsMassResidual->Fill((BsMass - 5.366));

  // spatial vectors
  GlobalPoint pvGlobalPoint(bestPrimVertex.position().x(), bestPrimVertex.position().y(), bestPrimVertex.position().z());
  GlobalVector PVToSV = fittedGlobalPoint - pvGlobalPoint;

  GlobalPoint caloPosition = GlobalPoint(recoPhoton.superCluster()->position().x(),
                                                  recoPhoton.superCluster()->position().y(),
                                                  recoPhoton.superCluster()->position().z());

  GlobalVector SVToCalo = caloPosition - fittedGlobalPoint;

  GlobalVector w = PVToSV.unit();
  GlobalVector u = SVToCalo.unit();
  GlobalVector v = dimuonMomentum;

  hDimuonGammaCosInBsFrame->Fill(w.cross(u).unit().dot(w.cross(v).unit())); //plane transverse to PVToSV

  //std::cout << "p4 cross pt to sv " << (dimuonMomentum + photonMomentum).unit().cross(w).mag() << std::endl; 

  // scale the photon momentum to the dimuon momentum, in the Bs transverse plane
  GlobalVector dimuonTransverse = w.cross(v);
  GlobalVector photonTransverse = w.cross(photonMomentum);
  double scaleFactor = dimuonTransverse.mag() / photonTransverse.mag();

  hScale->Fill(scaleFactor);

  math::XYZTLorentzVector scaledPhotonP4 = photonP4 * scaleFactor;
  double scaledBsMass = (dimuonP4 + scaledPhotonP4).mass();
  
  hScaledBsMassResidual->Fill((scaledBsMass - 5.366));

  // gen vs reco photon energy before and after scaling
  math::XYZTLorentzVector genPhotonP4 = genMatchedPhotons.at(0)->p4();
  hUnscaledRecoVsGenPhotonEnergy->Fill(photonP4.energy() - genPhotonP4.energy());
  hScaledRecoVsGenPhotonEnergy->Fill(scaledPhotonP4.energy() - genPhotonP4.energy());

  // scale the corrected photon momentum
  double correctedEnergy = (photonP4.energy() - 0.599366) / 1.02408;
  GlobalVector correctedPhotonMomentum = photonMomentum.unit() * correctedEnergy;
  math::XYZTLorentzVector correctedPhotonP4 = math::XYZTLorentzVector(correctedPhotonMomentum.x(),
                                              correctedPhotonMomentum.y(),
                                              correctedPhotonMomentum.z(),
                                              correctedEnergy);

  GlobalVector correctedPhotonTransverse = w.cross(correctedPhotonMomentum);
  double correctedScaleFactor = dimuonTransverse.mag() / correctedPhotonTransverse.mag();
  math::XYZTLorentzVector scaledCorrectedPhotonP4 = correctedPhotonP4 * correctedScaleFactor;

  hCorrScale->Fill(correctedScaleFactor);

  hUnscaledCorrRecoVsGenPhotonEnergy->Fill(correctedPhotonP4.energy() - genPhotonP4.energy());
  hScaledCorrRecoVsGenPhotonEnergy->Fill(scaledCorrectedPhotonP4.energy() - genPhotonP4.energy());


  // angle btwn genBs momentum and w - PV to SV vector
  GlobalVector genBsMomentum = GlobalVector(genB0s.momentum().x(),genB0s.momentum().y(),genB0s.momentum().z());
  GlobalVector genBsDirection = genBsMomentum.unit();
  hCos_GenB_PvToSv->Fill(genBsDirection.dot(w));  

  /*
  // angles between reco and gen dimuon and photon in Bs frame
  GlobalVector genDimuonMomentum = GlobalVector(genMatchedMuons.at(0)->momentum().x() + genMatchedMuons.at(1)->momentum().x(), 
                                    genMatchedMuons.at(0)->momentum().y() + genMatchedMuons.at(1)->momentum().y(), 
                                    genMatchedMuons.at(0)->momentum().z() + genMatchedMuons.at(1)->momentum().z());
  GlobalVector genPhotonMomentum = GlobalVector(genMatchedPhotons.at(0)->momentum().x(), 
                                    genMatchedPhotons.at(0)->momentum().y(), 
                                    genMatchedPhotons.at(0)->momentum().z());
  GlobalVector genBsMomentum = genDimuonMomentum + genPhotonMomentum;
  GlobalVector genBsDirection = genBsMomentum.unit();

  GlobalVector genDimuonMomentumInBsFrame = genBsDirection.cross(genDimuonMomentum);
  GlobalVector genPhotonMomentumInBsFrame = genBsDirection.cross(genPhotonMomentum);

  GlobalVector dimuonMomentumInBsFrame = genBsDirection.cross(dimuonMomentum);
  GlobalVector photonMomentumInBsFrame = genBsDirection.cross(photonMomentum);

  hDimuonAngleInBsFrame->Fill(acos(genBsDirection.cross(dimuonMomentum).unit().dot(genBsDirection.cross(genDimuonMomentum).unit()))*180 / TMath::Pi());
  hPhotonAngleInBsFrame->Fill(acos(genBsDirection.cross(photonMomentum).unit().dot(genBsDirection.cross(genPhotonMomentum).unit()))*180 / TMath::Pi());

  hRecoVsGenPtBsFrameMuonProfile->Fill(genDimuonMomentumInBsFrame.mag(), dimuonMomentumInBsFrame.mag());
  hRecoVsGenPtBsFramePhotonProfile->Fill(genPhotonMomentumInBsFrame.mag(), photonMomentumInBsFrame.mag());

  double correctedEnergy = (photonP4.energy() - 0.599366) / 1.02408;
  GlobalVector correctedPhotonMomentum = photonMomentum.unit() * correctedEnergy;
  GlobalVector correctedPhotonMomentumInBsFrame = genBsDirection.cross(correctedPhotonMomentum);
  hRecoVsGenPtBsFramePhotonProfileCorrected->Fill(genPhotonMomentumInBsFrame.mag(), correctedPhotonMomentumInBsFrame.mag());
  */

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(BsPtBalance);
