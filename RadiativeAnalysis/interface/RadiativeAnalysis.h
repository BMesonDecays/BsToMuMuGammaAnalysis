#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_RadiativeAnalysis_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_RadiativeAnalysis_h


#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GlobalIncludes.h"

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
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "TLorentzVector.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/OutputTree.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include <TLorentzVector.h>
#include "BsToMuMuGammaAnalysis/run3mvaid/interface/MuonMVAID.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenFilterInfo.h"


//#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"


class RadiativeAnalysis : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns, edm::one::WatchLuminosityBlocks> {
public:
  explicit RadiativeAnalysis(const edm::ParameterSet&);
  ~RadiativeAnalysis() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&)override ;
  void endJob() override;
  void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) override;
  void endLuminosityBlock(const edm::LuminosityBlock& iLumi, const edm::EventSetup&  iSetup) override;
  void beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
  void endRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
private:
  void processHLTBits(const edm::Event& iEvent);

  bool isMCstudy_;
  bool isMINIAOD_;
  std::string outputFile_; // output file
  OutputTree *outputTree_;
  MuonMVAID* muonMVAIDProducer_;

  edm::ParameterSet theConfig_;
  edm::EDGetTokenT<GenRunInfoProduct> generatorInfoTok;
  edm::EDGetTokenT<GenFilterInfo> genFilterTok;

  edm::InputTag genParticlesLabel;
  edm::EDGetTokenT<std::vector<reco::GenParticle>> genParticlesTok;
  edm::InputTag MuonTag;
  edm::EDGetTokenT<edm::View<reco::Muon>> MuonTagTok;
  edm::InputTag PhotonTag;
  edm::EDGetTokenT<edm::View<reco::Photon>> PhotonTagTok;
  edm::InputTag PUInfo;
  edm::EDGetTokenT<edm::View<PileupSummaryInfo>> PUInfoTok;
  edm::InputTag vertexBeamSpot;
  edm::EDGetTokenT<reco::BeamSpot> vertexBeamSpotTok;
  edm::InputTag primaryvertex;
  edm::EDGetTokenT<std::vector<reco::Vertex>> primaryvertexTok;
  edm::InputTag triggerbits;
  edm::EDGetTokenT<edm::TriggerResults> triggerbitsTok;
  edm::InputTag ecalrechitEB;
  edm::EDGetTokenT<EcalRecHitCollection> ecalrechitEBTok;
  edm::InputTag ecalrechitEE;
  edm::EDGetTokenT<EcalRecHitCollection> ecalrechitEETok;
  edm::InputTag valMapTag;
  edm::EDGetTokenT<edm::ValueMap<float>> valMapTok;

  // AOD: reco::Track collection; MINIAOD: packedPFCandidates + lostTracks
  edm::InputTag trackTag;
  edm::EDGetTokenT<std::vector<reco::Track>> trackTagTok;
  edm::EDGetTokenT<edm::View<pat::PackedCandidate>> packedTrackTagTok;
  edm::InputTag lostTrackTag;
  edm::EDGetTokenT<edm::View<pat::PackedCandidate>> lostTrackTagTok;
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> trackBuilderTok;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> theBFieldTok;
  unique_ptr<EcalClusterLazyToolsBase::ESGetTokens> iSetupGetTok;

  // generator summary
  double genXsec_ = -1.0;
  double genXsecErr_ = 0.0;
  double filterEff_ = 1.0;
  double filterEffErr_ = 0.0;
  GenFilterInfo totalGenFilter_;      // filter counts merged over all luminosity blocks of the job
  bool hasGenFilter_ = false;
  double lumiEquivalent_ = -1.0;
  double lumiEquivalentErr_ = 0.0;

  unsigned int event_counter_;
};
#endif
