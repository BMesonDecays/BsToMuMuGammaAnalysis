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
class MarcinsIdea : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit MarcinsIdea(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~MarcinsIdea();

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

  TProfile* hRecoVsGenEnergyProfile;
  TProfile* hRecoVsGenPtBsFrameMuonProfile;
  TProfile* hRecoVsGenPtBsFramePhotonProfile;
  TProfile* hRecoVsGenPtBsFramePhotonProfileCorrected;


  TH1D* hDimuonAngleInBsFrame;
  TH1D* hPhotonAngleInBsFrame;

  TH1D* hDimuonVertexXResidual;
  TH1D* hDimuonVertexYResidual;
  TH1D* hDimuonVertexZResidual;

  TTree* theTree;
  std::vector<float> vPVToSV;
  std::vector<float> vPhotonMomentum;
  std::vector<float> vDimuonMomentum;


  int nConvPhotons = 0;
  std::vector<int> MuMuG = {22, 13, -13};
};


MarcinsIdea::MarcinsIdea(const edm::ParameterSet& conf)
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

MarcinsIdea::~MarcinsIdea()
{
  cout <<" DTOR" << endl;
}

bool MarcinsIdea::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}


void MarcinsIdea::beginJob()
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

  hRecoVsGenEnergyProfile = new TProfile("hRecoVsGenEnergyProfile", "hRecoVsGenEnergyProfile; gen Energy; reco Energy", 35, 0, 35, "s");

  hRecoVsGenPtBsFrameMuonProfile = new TProfile("hRecoVsGenPtBsFrameMuonProfile", "hRecoVsGenPtBsFrameMuonProfile; gen Pt in Bs frame; reco Pt in Bs frame", 15, 0, 3, "s");
  hRecoVsGenPtBsFramePhotonProfile = new TProfile("hRecoVsGenPtBsFramePhotonProfile", "hRecoVsGenPtBsFramePhotonProfile; gen Pt in Bs frame; reco Pt in Bs frame", 15, 0, 3, "s");
  hRecoVsGenPtBsFramePhotonProfileCorrected = new TProfile("hRecoVsGenPtBsFramePhotonProfileCorrected", "hRecoVsGenPtBsFramePhotonProfileCorrected; gen Pt in Bs frame; reco Pt in Bs frame", 15, 0, 3, "s");

  hDimuonAngleInBsFrame = new TH1D("hDimuonAngleInBsFrame", "hDimuonAngleInBsFrame; dimuon in Bs frame, reco vs gen [degrees]", 100, 0, 10);
  hPhotonAngleInBsFrame = new TH1D("hPhotonAngleInBsFrame", "hPhotonAngleInBsFrame; gamma in Bs frame, reco vs gen [degrees]", 100, 0, 10);

  hDimuonVertexXResidual = new TH1D("hDimuonVertexXResidual", "hDimuonVertexXResidual; [cm]", 100, -0.05, 0.05);
  hDimuonVertexYResidual = new TH1D("hDimuonVertexYResidual", "hDimuonVertexYResidual; [cm]", 100, -0.05, 0.05);
  hDimuonVertexZResidual = new TH1D("hDimuonVertexZResidual", "hDimuonVertexZResidual; [cm]", 100, -0.05, 0.05);

  theTree = new TTree("theTree", "theTree");
  theTree->Branch("vPVToSV", &vPVToSV);
  theTree->Branch("vPhotonMomentum", &vPhotonMomentum);
  theTree->Branch("vDimuonMomentum", &vDimuonMomentum);

  cout << "HERE MarcinsIdea::beginJob()" << endl;
}

void MarcinsIdea::endJob()
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

  hRecoVsGenEnergyProfile->Write();

  hRecoVsGenPtBsFrameMuonProfile->Write();
  hRecoVsGenPtBsFramePhotonProfile->Write();
  hRecoVsGenPtBsFramePhotonProfileCorrected->Write();

  hDimuonAngleInBsFrame->Write();
  hPhotonAngleInBsFrame->Write();

  hDimuonVertexXResidual->Write();
  hDimuonVertexYResidual->Write();
  hDimuonVertexZResidual->Write();
  
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

  delete hRecoVsGenEnergyProfile;

  delete hRecoVsGenPtBsFrameMuonProfile;
  delete hRecoVsGenPtBsFramePhotonProfile;
  delete hRecoVsGenPtBsFramePhotonProfileCorrected;

  delete hDimuonAngleInBsFrame;
  delete hPhotonAngleInBsFrame;

  delete hDimuonVertexXResidual;
  delete hDimuonVertexYResidual;
  delete hDimuonVertexZResidual;

  delete theTree;

  cout << "HERE MarcinsIdea::endJob()" << endl;
}


void MarcinsIdea::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE MarcinsIdea::analyze "<< std::endl;

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
      }
    }
  }

  // reco muon matching
  for (const reco::Candidate* genMu : genMuons)
  {
    float minDR = 10;
    const reco::Muon* bestMatchedMuon;
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

  // reco photon matching
  for (const reco::Candidate* genPh : genPhotons)
  {
    float minDR = 10;
    const reco::Photon* bestMatchedPhoton;
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
    if (matched && minDR < 0.02)
    {
      recoMatchedPhotons.push_back(bestMatchedPhoton);
      genMatchedPhotons.push_back(genPh);
    }
  }
  if(recoMatchedPhotons.size() == 0) return;

  hRecoVsGenEnergyProfile->Fill(genMatchedPhotons[0]->energy(), recoMatchedPhotons[0]->energy());


  if(recoMatchedPhotons[0]->isEB() == 0) return; // only EB photons

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


  KinematicParticleVertexFitter fitter;
  cout << "Fitting" << endl;
  RefCountedKinematicTree vertexFitTree = fitter.fit(allParticles);

  if (!vertexFitTree->isValid()) return;
  // get the fitted particle and vertex
  vertexFitTree->movePointerToTheTop();
  RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
  RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
  if (!fitVertex->vertexIsValid()) return;

  GlobalPoint fittedGlobalPoint = fitVertex->position();

  hDimuonVertexXResidual->Fill(fittedGlobalPoint.x() - genSV.x());
  hDimuonVertexYResidual->Fill(fittedGlobalPoint.y() - genSV.y());
  hDimuonVertexZResidual->Fill(fittedGlobalPoint.z() - genSV.z());

  GlobalVector dimuonMomentum = fitParticle->currentState().kinematicParameters().momentum();
  math::XYZTLorentzVector dimuonP4(dimuonMomentum.x(), dimuonMomentum.y(), dimuonMomentum.z(), fitParticle->currentState().kinematicParameters().energy());

  hDimuonMassResidualNoFit->Fill((recoMatchedMuons.at(0)->p4() + recoMatchedMuons.at(1)->p4()).mass() - (genMatchedMuons.at(0)->p4() + genMatchedMuons.at(1)->p4()).mass());
  hDimuonMassResidualWithFit->Fill(dimuonP4.mass() - (genMatchedMuons.at(0)->p4() + genMatchedMuons.at(1)->p4()).mass());

  GlobalPoint pvGlobalPoint(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
  GlobalVector PVToSV = fittedGlobalPoint - pvGlobalPoint;

  if(recoMatchedPhotons.size() != 1) return;

  hMuonFitGenGammaResidual->Fill((dimuonP4 + genMatchedPhotons.at(0)->p4()).mass() - (genMatchedMuons.at(0)->p4() + genMatchedMuons.at(1)->p4() + genMatchedPhotons.at(0)->p4()).mass());

  reco::Photon recoPhoton = *recoMatchedPhotons.at(0);

  recoPhoton.setVertex(reco::Candidate::Point(fittedGlobalPoint.x(), fittedGlobalPoint.y(), fittedGlobalPoint.z()));
  math::XYZTLorentzVector photonP4 = recoPhoton.p4();
  GlobalVector photonMomentum(photonP4.x(), photonP4.y(), photonP4.z());

  double BsMass = (dimuonP4 + photonP4).mass();
  hBsMass->Fill(BsMass);
  hBsMassResidual->Fill((BsMass - 5.366));

  GlobalPoint caloPosition = GlobalPoint(recoPhoton.superCluster()->position().x(),
                                                  recoPhoton.superCluster()->position().y(),
                                                  recoPhoton.superCluster()->position().z());

  GlobalVector SVToCalo = caloPosition - fittedGlobalPoint;

  GlobalVector w = PVToSV.unit();
  GlobalVector u = SVToCalo.unit();
  GlobalVector v = dimuonMomentum;

  hDimuonGammaCosInBsFrame->Fill(w.cross(u).unit().dot(w.cross(v).unit()));

  std::cout << "p4 cross pt to sv " << (dimuonMomentum + photonMomentum).unit().cross(w).mag() << std::endl;

  // minimize the transverse component of the Bs
  double E = (w.dot(v)*w.dot(u) - v.dot(u))/(1 - w.dot(u)*w.dot(u));

  std::cout << "E: " << E << std::endl;

  math::XYZTLorentzVector correctedPhotonP4(u.x() * E, u.y() * E, u.z() * E, E);

  double correctedBsMass = (dimuonP4 + correctedPhotonP4).mass();
  std::cout << "Corrected Bs Mass: " << correctedBsMass << std::endl;

  hBsMassResidualCorrected->Fill((correctedBsMass - 5.366));


  vPVToSV.clear(); vPVToSV.push_back(PVToSV.x()); vPVToSV.push_back(PVToSV.y()); vPVToSV.push_back(PVToSV.z());
  vPhotonMomentum.clear(); vPhotonMomentum.push_back(photonMomentum.x()); vPhotonMomentum.push_back(photonMomentum.y()); vPhotonMomentum.push_back(photonMomentum.z());
  vDimuonMomentum.clear(); vDimuonMomentum.push_back(dimuonMomentum.x()); vDimuonMomentum.push_back(dimuonMomentum.y()); vDimuonMomentum.push_back(dimuonMomentum.z());

  theTree->Fill();

  // other idea, scale the photon momentum to the dimuon momentum, in the Bs transverse plane
  GlobalVector dimuonTransverse = w.cross(v);
  GlobalVector photonTransverse = w.cross(photonMomentum);
  double scaleFactor = dimuonTransverse.mag() / photonTransverse.mag();

  hScale->Fill(scaleFactor);

  math::XYZTLorentzVector scaledPhotonP4 = photonP4 * scaleFactor;
  double scaledBsMass = (dimuonP4 + scaledPhotonP4).mass();
  
  hScaledBsMassResidual->Fill((scaledBsMass - 5.366));


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


  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(MarcinsIdea);
