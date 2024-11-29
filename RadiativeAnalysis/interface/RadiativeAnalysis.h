#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_RadiativeAnalysis_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_RadiativeAnalysis_h


#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "MuonAnalysis/MuonAssociators/interface/L1MuonMatcherAlgo.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingVertexContainer.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelTopologyBuilder.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "Geometry/CommonDetUnit/interface/GeomDetType.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include <TFile.h>
#include <TH1F.h>
#include "DataFormats/PatCandidates/interface/GenericParticle.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/Candidate/interface/CompositeCandidateFwd.h"
#include "DataFormats/BTauReco/interface/SecondaryVertexTagInfo.h"
#include "DataFormats/PatCandidates/interface/IsolatedTrack.h"
#include "TLorentzVector.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"

//#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"


class RadiativeAnalysis : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit RadiativeAnalysis(const edm::ParameterSet&);
  ~RadiativeAnalysis() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&)override ;
  void endJob() override;
  void fillMCInfo( edm::Handle<edm::View<reco::GenParticle>>& genParticles);
  void setFitParGamma(RefCountedKinematicTree& myTree);
  void setFitParHyp1(RefCountedKinematicTree& myTree);
  void setFitParHyp2(RefCountedKinematicTree& myTree);
private:
  bool MCmatching(const reco::Candidate & particle,  edm::Handle<edm::View<reco::GenParticle>>& genParticles,
                  int &particlemcId, int &particlemomId, int &particlegmomId,
                  int condMom, int condGMom);
  bool MCmatchingBsMuMu(const reco::Candidate & muParticle, edm::Handle<edm::View<reco::GenParticle>>& genParticles,
                        int &particlemcId, int &particlemomId,
                        int condMom);
  bool MCmatchingJpsi(const reco::Candidate& particle,  edm::Handle<edm::View<reco::GenParticle>>& genParticles, int &particlemcId, int &particlemomId,	int condMom);
  bool MCmatchingPsi(const reco::Candidate& particle,  edm::Handle<edm::View<reco::GenParticle>>& genParticles, int &particlemcId, int &particlemomId, int condMom);
  reco::Vertex reVertex(const edm::Event& theEvent,  const edm::EventSetup& iSetup, edm::Handle<reco::BeamSpot>& vertexBeamSpot, reco::Vertex RecVtx, pat::Muon mu1, pat::Muon mu2, pat::Photon photon);
  reco::Vertex reVertex2(const edm::EventSetup& iSetup, edm::Handle<reco::BeamSpot>& vertexBeamSpot, reco::Vertex RecVtx, pat::Muon mu1, pat::Muon mu2, pat::Photon photon, double &vtxCL);
  double CalculateCtErrvertex(const edm::EventSetup& iSetup, reco::Vertex PV, TransientVertex SV, TMatrix SVcovariance2D ,GlobalVector Ptvec, double Bmass);
  double CalculateCtErrbeamspot(const edm::EventSetup& iSetup, edm::Handle<reco::BeamSpot>& vertexBeamSpot, TransientVertex SV, TMatrix SVcovariance2D ,GlobalVector Ptvec, double Bmass);
  void RecursivelyPrintMother( const reco::Candidate & genp );
  GlobalVector flightDirection(const reco::Vertex &pv, reco::Vertex &sv);
  bool selGlobalMuon(const pat::Muon theMuon, const math::XYZPoint RefVtx);
  bool selTrackerMuon(const pat::Muon theMuon, const math::XYZPoint RefVtx);
  bool selTightMuon(const pat::Muon theMuon, const reco::Vertex RecVtx);
  double MuonChargeCone(const edm::Event& theEvent, reco::TrackRef muTrackRef, const double delR, const double KExp, bool IncludingMuon);
  double MuonChargeConeWrtPV(const edm::Event& theEvent, reco::TrackRef muTrackRef, reco::Vertex PVtx, const double delR, const double KExp, bool IncludingMuon);
  double LeptonChargeCone(const reco::PFCandidateCollection & PFCand, const reco::PFCandidate theLept, const double Dr, const double KExp, bool IncludingLepton);
  short int FindMuonMCCode(const pat::Muon theMu, edm::Handle<edm::View<reco::GenParticle>>& genParticles);
  short int FindMuonMCSimpleCode(const pat::Muon theMu, edm::Handle<edm::View<reco::GenParticle>>& genParticles);
  int FindMuonAncestor(const pat::Muon theMu, edm::Handle<edm::View<reco::GenParticle>>& genParticles);
  int LookForMotherStringId(reco::GenParticle theGenP);
  short int LookForMotherString(reco::GenParticle theGenP);
  const TrackerGeometry* m_tracker;
  bool isMCstudy_;
  std::string outputFile_; // output file
  RadiativeRootTree *bmmgRootTree_;

  edm::ParameterSet theConfig_;
  edm::InputTag genParticlesLabel;
  edm::EDGetTokenT<edm::View<reco::GenParticle>> genParticlesTok;
  edm::InputTag MuonTag;
  edm::EDGetTokenT<edm::View<pat::Muon>> MuonTagTok;
  edm::InputTag JetTag;
  edm::EDGetTokenT<edm::View<pat::Jet>> JetTagTok;
  edm::InputTag PhotonTag;
  edm::EDGetTokenT<edm::View<reco::Photon>> PhotonTagTok;
  edm::InputTag OOTPhotonTag;
  edm::EDGetTokenT<edm::View<pat::Photon>> OOTPhotonTagTok;
  edm::InputTag ElectronTag;
  edm::EDGetTokenT<edm::View<pat::Electron>> ElectronTagTok;
  edm::InputTag SuperClusterTag;
  edm::EDGetTokenT<edm::View<reco::SuperCluster>> SuperClusterTagTok;
  edm::InputTag OOTSuperClusterTag;
  edm::EDGetTokenT<edm::View<reco::SuperCluster>> OOTSuperClusterTagTok;
  edm::InputTag PUInfo;
  edm::EDGetTokenT<edm::View<PileupSummaryInfo>> PUInfoTok;
  edm::InputTag vertexBeamSpot;
  edm::EDGetTokenT<reco::BeamSpot> vertexBeamSpotTok;
  edm::InputTag primaryvertex;
  edm::EDGetTokenT<edm::View<reco::Vertex>> primaryvertexTok;
  edm::InputTag triggerbits;
  edm::EDGetTokenT<edm::TriggerResults> triggerbitsTok;
  edm::InputTag prescale;
  edm::EDGetTokenT<pat::PackedTriggerPrescales> prescaleTok;
  edm::InputTag triggerobj;
  edm::EDGetTokenT<edm::View<pat::TriggerObjectStandAlone>> triggerobjTok;
  edm::InputTag pfCandTag;
  edm::EDGetTokenT<edm::View<pat::PackedCandidate>> pfCandTagTok;
  edm::InputTag IsoTrackTag;
  edm::EDGetTokenT<edm::View<pat::IsolatedTrack>> IsoTrackTagTok;
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> trackBuilderTok;
  bool StoreDeDxInfo_;
  bool verbose_;
  bool TestVerbose_;
  
  
  
  const double nominalJpsiMass;
  const double nominalPsiMass;
  const double nominalPhiMass;
  const double nominalElectronMass;
  const double nominalMuonMass;
  const double nominalPiZeroMass;
  const double nominalEtaMesonMass;
  const double nominalEtaPrimeMass;
  const double nominalKaonMass;
  double PionZeroMassWindowNoFit_;
  double EtaMesonMassWindowNoFit_;
  double EtaPrimeMassWindowNoFit_;
  double JpsiMassWindowBeforeFit_;
  double JpsiMassWindowAfterFit_;
  double PhiMassWindowAfterFit_;
  double PhiMassWindowBeforeFit_;
  double JpsiPtCut_;
  double KaonTrackPtCut_;
  double PsiMassWindowAfterFit_;
  double PsiMassWindowBeforeFit_;
  double BsLowerMassCutBeforeFit_;
  double BsUpperMassCutBeforeFit_;
  double BsLowerMassCutAfterFit_ ;
  double BsUpperMassCutAfterFit_ ;

  double BsPDGMass_;
  double BdPDGMass_;
  double BpPDGMass_;
  double JpsiPDGMass_;
  double PsiPDGMass_;
  double PionZeroPDGMass_;
  double EtaMesonPDGMass_;
  double EtaPrimePDGMass_;

  unsigned int tagmucounter_;
  unsigned int event_counter_;
  unsigned int elecounter_;
  unsigned int muoncounter_;
  unsigned int jetcounter_;
  unsigned int photoncounter_;

  int    isCowboy              = 0;
  double MuonsDCA              = -9999999;
  double kaonmass              = 0.493677;
  double pionmass              = 0.139570; 
  double DeltaRPhotonJpsi      = -9999999;
  double DeltaRPhotonPsi       = -9999999;
  double JpsiPhotonDCA         = -9999999;
  double PsiPhotonDCA          = -9999999;
  double minVtxP               = -9999999;
  double KKDCA                = -9999999; 
  double MinPtVertex = 0.0;
  int    NSelectedVertices;
  double PtSumVertex = 0.0;
  std::set<size_t> excludedPhotons;
};
#endif
