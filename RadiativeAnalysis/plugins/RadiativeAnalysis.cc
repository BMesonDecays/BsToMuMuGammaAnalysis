// -*- C++ -*-
// Package:    BsToMuMuGammaAnalysis/RadiativeAnalysis
// Class:      RadiativeAnalysis
//
// Ntuplizer for B -> mu mu gamma and related channels. The work is done by the components of the
// package library (namespace bmmg):
//   object selection -> dimuons -> B candidates (MMGVertexFit) -> NtupleWriter
// Channels (each can be switched off): mu mu gamma (ECAL photon), mu mu gamma (conversion),
// mu mu gamma gamma, mu mu K K (Bs -> J/psi phi normalisation).
// All selected objects and all candidates are stored (vectors); the best-candidate selection
// is left to the offline analysis. The ntuple is documented in NTUPLE.md.

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DimuonBuilder.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GenChain.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MMGVertexFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateBuilders.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/NtupleWriter.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ObjectSelection.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TriggerMatcher.h"
#include "BsToMuMuGammaAnalysis/run3mvaid/interface/MuonMVAID.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "SimDataFormats/GeneratorProducts/interface/GenFilterInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include <algorithm>
#include <cmath>

#include "TMath.h"
#include <memory>

class RadiativeAnalysis
    : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns, edm::one::WatchLuminosityBlocks> {
public:
  explicit RadiativeAnalysis(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;
  void beginRun(const edm::Run&, const edm::EventSetup&) override;
  void endRun(const edm::Run&, const edm::EventSetup&) override;
  void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) override {}
  void endLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) override;

  const bool isMC_;
  const bool verbose_;

  // collections
  const bool isMiniAOD_;
  const edm::EDGetTokenT<edm::View<reco::Muon>> muonsToken_;
  const edm::EDGetTokenT<edm::View<reco::Photon>> photonsToken_;
  const edm::EDGetTokenT<edm::ValueMap<float>> photonMvaToken_;
  const edm::EDGetTokenT<edm::ValueMap<bool>> photonTightIdToken_;
  const edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
  const edm::EDGetTokenT<std::vector<reco::Vertex>> verticesToken_;
  const edm::EDGetTokenT<EcalRecHitCollection> ebRecHitsToken_;
  const edm::EDGetTokenT<EcalRecHitCollection> eeRecHitsToken_;
  const edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken_;
  edm::EDGetTokenT<trigger::TriggerEvent> triggerEventToken_;                     // AOD
  edm::EDGetTokenT<std::vector<pat::TriggerObjectStandAlone>> triggerObjectsToken_;  // MiniAOD
  edm::EDGetTokenT<std::vector<pat::PackedCandidate>> packedToken_, lostTracksToken_;  // MiniAOD
  edm::EDGetTokenT<std::vector<reco::Conversion>> conversionsToken_;
  edm::EDGetTokenT<std::vector<reco::Track>> tracksToken_;
  edm::EDGetTokenT<std::vector<reco::GenParticle>> genParticlesToken_;
  edm::EDGetTokenT<std::vector<pat::PackedGenParticle>> packedGenParticlesToken_;   // MiniAOD
  edm::EDGetTokenT<std::vector<PileupSummaryInfo>> pileupToken_;
  edm::EDGetTokenT<GenRunInfoProduct> genRunInfoToken_;
  edm::EDGetTokenT<GenFilterInfo> genFilterInfoToken_;
  const edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> ttBuilderToken_;
  const EcalClusterLazyToolsBase::ESGetTokens lazyToolsTokens_;

  // components
  bmmg::MuonCuts muonCuts_;
  bmmg::PhotonCuts photonCuts_;
  bmmg::ConversionCuts conversionCuts_;
  bmmg::TrackCuts trackCuts_;
  bmmg::CandidateConfig candidateConfig_;
  bmmg::DiphotonConfig diphotonConfig_;
  bmmg::KaonConfig kaonConfig_;
  MMGVertexFit mmgVertexFit_;
  bmmg::TriggerMatcher triggerMatcher_;
  MuonMVAID muonMVA_;
  const bool doMuMuGamma_, doMuMuConvGamma_, doMuMuGammaGamma_, doMuMuKK_;
  // write only the events with at least one valid candidate: fit OK and vertex probability > minCandidateVtxProb_
  const bool onlyEventsWithCandidates_;
  const double minCandidateVtxProb_;
  bool isValidCandidate(const bmmg::BCandidate& c) const;
  std::unique_ptr<bmmg::NtupleWriter> writer_;

  // job-level bookkeeping
  unsigned long long nEvents_ = 0;         // processed events
  unsigned long long nEventsStored_ = 0;   // events written to the tree
  double xsecSumW_ = 0., xsecSumWX_ = 0., xsec_ = -1., xsecErr_ = -1.;
  std::unique_ptr<GenFilterInfo> genFilterInfo_;
};

RadiativeAnalysis::RadiativeAnalysis(const edm::ParameterSet& cfg)
    : isMC_(cfg.getParameter<bool>("isMC")),
      verbose_(cfg.getUntrackedParameter<bool>("verbose")),
      isMiniAOD_(cfg.getParameter<std::string>("dataFormat") == "MINIAOD"),
      muonsToken_(consumes(cfg.getParameter<edm::InputTag>("muons"))),
      photonsToken_(consumes(cfg.getParameter<edm::InputTag>("photons"))),
      photonMvaToken_(consumes(cfg.getParameter<edm::InputTag>("photonMVA"))),
      photonTightIdToken_(consumes(cfg.getParameter<edm::InputTag>("photonTightId"))),
      beamSpotToken_(consumes(cfg.getParameter<edm::InputTag>("beamSpot"))),
      verticesToken_(consumes(cfg.getParameter<edm::InputTag>("primaryVertices"))),
      ebRecHitsToken_(consumes(cfg.getParameter<edm::InputTag>("ebRecHits"))),
      eeRecHitsToken_(consumes(cfg.getParameter<edm::InputTag>("eeRecHits"))),
      triggerResultsToken_(consumes(cfg.getParameter<edm::InputTag>("triggerResults"))),
      ttBuilderToken_(esConsumes(edm::ESInputTag("", "TransientTrackBuilder"))),
      lazyToolsTokens_(consumesCollector()),
      mmgVertexFit_({cfg.getParameter<double>("clusterCovScalePhiEB"), cfg.getParameter<double>("clusterCovScaleThetaEB"),
                     cfg.getParameter<double>("energyErrorScaleEB")},
                    {cfg.getParameter<double>("clusterCovScalePhiEE"), cfg.getParameter<double>("clusterCovScaleThetaEE"),
                     cfg.getParameter<double>("energyErrorScaleEE")}),
      triggerMatcher_(cfg.getParameter<std::vector<std::string>>("triggerPaths"),
                      cfg.getParameter<std::string>("hltProcess"),
                      cfg.getParameter<double>("triggerMatchDeltaR")),
      muonMVA_(cfg),
      doMuMuGamma_(cfg.getParameter<bool>("doMuMuGamma")),
      doMuMuConvGamma_(cfg.getParameter<bool>("doMuMuConvGamma")),
      doMuMuGammaGamma_(cfg.getParameter<bool>("doMuMuGammaGamma")),
      doMuMuKK_(cfg.getParameter<bool>("doMuMuKK")),
      onlyEventsWithCandidates_(cfg.getParameter<bool>("onlyEventsWithCandidates")),
      minCandidateVtxProb_(cfg.getParameter<double>("minCandidateVtxProb")) {
    if (doMuMuConvGamma_) conversionsToken_ = consumes(cfg.getParameter<edm::InputTag>("conversions"));
    if (isMiniAOD_) {
      triggerObjectsToken_ = consumes(cfg.getParameter<edm::InputTag>("triggerObjects"));
      packedToken_ = consumes(cfg.getParameter<edm::InputTag>("packedCandidates"));
      lostTracksToken_ = consumes(cfg.getParameter<edm::InputTag>("lostTracks"));
    } else {
      triggerEventToken_ = consumes(cfg.getParameter<edm::InputTag>("triggerEvent"));
      if (doMuMuKK_) tracksToken_ = consumes(cfg.getParameter<edm::InputTag>("tracks"));
    }
    if (isMC_) {
      genParticlesToken_ = consumes(cfg.getParameter<edm::InputTag>("genParticles"));
      if (isMiniAOD_) packedGenParticlesToken_ = consumes(cfg.getParameter<edm::InputTag>("packedGenParticles"));
      pileupToken_ = consumes(cfg.getParameter<edm::InputTag>("pileup"));
      genRunInfoToken_ = consumes<GenRunInfoProduct, edm::InRun>(cfg.getParameter<edm::InputTag>("genRunInfo"));
      genFilterInfoToken_ = consumes<GenFilterInfo, edm::InLumi>(cfg.getParameter<edm::InputTag>("genFilterInfo"));
    }
    muonCuts_.minPt = cfg.getParameter<double>("muonMinPt");
    muonCuts_.maxAbsEta = cfg.getParameter<double>("muonMaxAbsEta");
    muonCuts_.minMVA = cfg.getParameter<double>("muonMinMVA");
    photonCuts_.minSCEnergy = cfg.getParameter<double>("photonMinSCEnergy");
    photonCuts_.maxAbsSCEta = cfg.getParameter<double>("photonMaxAbsSCEta");
    photonCuts_.minMVA = cfg.getParameter<double>("photonMinMVA");
    conversionCuts_.minPt = cfg.getParameter<double>("conversionMinPt");
    conversionCuts_.minVertexProb = cfg.getParameter<double>("conversionMinVertexProb");
    trackCuts_.minPt = cfg.getParameter<double>("kaonMinPt");
    candidateConfig_.minPrefitMass = cfg.getParameter<double>("minPrefitMass");
    candidateConfig_.maxPrefitMass = cfg.getParameter<double>("maxPrefitMass");
    candidateConfig_.computeTrackCompatibility = cfg.getParameter<bool>("computeTrackCompatibility");
    diphotonConfig_.minMass = cfg.getParameter<double>("diphotonMinMass");
    diphotonConfig_.maxMass = cfg.getParameter<double>("diphotonMaxMass");
    kaonConfig_.minPt = trackCuts_.minPt;
    kaonConfig_.minKKMass = cfg.getParameter<double>("kkMinMass");
    kaonConfig_.maxKKMass = cfg.getParameter<double>("kkMaxMass");
    kaonConfig_.jpsiOnly = cfg.getParameter<bool>("kkJpsiOnly");
    kaonConfig_.minBMass = cfg.getParameter<double>("kkMinBMass");
    kaonConfig_.maxBMass = cfg.getParameter<double>("kkMaxBMass");
    usesResource(TFileService::kSharedResource);
  }

void RadiativeAnalysis::beginJob() {
  edm::Service<TFileService> fs;
  std::vector<bmmg::NtupleWriter::Channel> channels;
  if (doMuMuGamma_) channels.push_back(bmmg::NtupleWriter::kMuMuGamma);
  if (doMuMuConvGamma_) channels.push_back(bmmg::NtupleWriter::kMuMuConvGamma);
  if (doMuMuGammaGamma_) channels.push_back(bmmg::NtupleWriter::kMuMuGammaGamma);
  if (doMuMuKK_) channels.push_back(bmmg::NtupleWriter::kMuMuKK);
  writer_ = std::make_unique<bmmg::NtupleWriter>(*fs, triggerMatcher_.paths(), isMC_, channels);
}

void RadiativeAnalysis::beginRun(const edm::Run& run, const edm::EventSetup& setup) {
  triggerMatcher_.beginRun(run, setup, verbose_);
}

void RadiativeAnalysis::endRun(const edm::Run& run, const edm::EventSetup&) {
  if (!isMC_) return;
  edm::Handle<GenRunInfoProduct> info;
  run.getByToken(genRunInfoToken_, info);
  if (!info.isValid()) {
    edm::LogWarning("RadiativeAnalysis") << "GenRunInfoProduct not found";
    return;
  }
  // inverse-variance weighted mean over the runs of the job
  const auto& xs = info->internalXSec();
  if (xs.value() > 0 && xs.error() > 0) {
    const double w = 1. / (xs.error() * xs.error());
    xsecSumW_ += w;
    xsecSumWX_ += w * xs.value();
    xsec_ = xsecSumWX_ / xsecSumW_;
    xsecErr_ = 1. / std::sqrt(xsecSumW_);
  } else if (xsecSumW_ == 0) {
    xsec_ = xs.value();
    xsecErr_ = xs.error();
  }
}

void RadiativeAnalysis::endLuminosityBlock(const edm::LuminosityBlock& lumi, const edm::EventSetup&) {
  if (!isMC_) return;
  edm::Handle<GenFilterInfo> info;
  lumi.getByToken(genFilterInfoToken_, info);
  if (!info.isValid()) return;
  if (!genFilterInfo_)
    genFilterInfo_ = std::make_unique<GenFilterInfo>(*info);
  else
    genFilterInfo_->mergeProduct(*info);
}

void RadiativeAnalysis::analyze(const edm::Event& event, const edm::EventSetup& setup) {
  ++nEvents_;
  writer_->beginEvent(event.id(), event.luminosityBlock());

  const auto& ttBuilder = setup.getData(ttBuilderToken_);
  const auto& beamSpot = event.get(beamSpotToken_);
  const auto& vertices = event.get(verticesToken_);
  const auto& muons = event.get(muonsToken_);
  const auto photonsHandle = event.getHandle(photonsToken_);

  // charged tracks associated to the primary vertices (isolation, track compatibility)
  std::vector<bmmg::PVTrack> pvTracks;
  std::vector<reco::Track> packedTracks;   // MiniAOD: pseudo-tracks (kaon candidates)
  if (isMiniAOD_) {
    const auto& packed = event.get(packedToken_);
    const auto& lost = event.get(lostTracksToken_);
    pvTracks = bmmg::pvTracksFromPacked(packed, lost);
    if (doMuMuKK_) packedTracks = bmmg::tracksFromPacked(packed, lost);
  } else {
    pvTracks = bmmg::pvTracksFromVertices(vertices);
  }
  EcalClusterLazyTools lazyTools(event, lazyToolsTokens_.get(setup), ebRecHitsToken_, eeRecHitsToken_);

  writer_->fillVertices(beamSpot, vertices);

  // generator truth (MC)
  std::unique_ptr<GenDecayChain> gen;
  if (isMC_) {
    writer_->fillPileup(event.get(pileupToken_));
    gen = std::make_unique<GenDecayChain>(event.get(genParticlesToken_),
                                          isMiniAOD_ ? &event.get(packedGenParticlesToken_) : nullptr);
    writer_->fillGen(*gen);
  }

  // objects
  std::vector<const reco::Muon*> allMuons;
  for (const auto& m : muons) allMuons.push_back(&m);
  const std::vector<float> muonMVA = muonMVA_.produce(allMuons);
  const auto photonMVA = event.getHandle(photonMvaToken_);
  const auto photonTightId = event.getHandle(photonTightIdToken_);
  const std::vector<bmmg::SelectedMuon> selMuons = bmmg::selectMuons(muons, muonMVA, ttBuilder, muonCuts_);
  const std::vector<bmmg::SelectedPhoton> selPhotons =
      bmmg::selectPhotons(photonsHandle, photonMVA, photonTightId, photonCuts_);

  // trigger
  std::vector<const reco::Muon*> muonPtrs;
  for (const auto& m : selMuons) muonPtrs.push_back(m.muon);
  const auto& triggerResults = event.get(triggerResultsToken_);
  const edm::TriggerNames& triggerNames = event.triggerNames(triggerResults);
  bmmg::TriggerMatcher::Result trigger;
  if (isMiniAOD_) {
    trigger = triggerMatcher_.evaluate(triggerResults, triggerNames, event.get(triggerObjectsToken_), muonPtrs);
  } else {
    const auto triggerEvent = event.getHandle(triggerEventToken_);
    trigger = triggerMatcher_.evaluate(triggerResults, triggerNames,
                                       triggerEvent.isValid() ? triggerEvent.product() : nullptr, muonPtrs);
  }
  writer_->fillTrigger(trigger);

  // candidates
  const std::vector<bmmg::DimuonCandidate> dimuons = bmmg::buildDimuons(selMuons, beamSpot);
  // every candidate needs a dimuon with a valid vertex: nothing to store without one
  if (onlyEventsWithCandidates_ &&
      std::none_of(dimuons.begin(), dimuons.end(), [](const bmmg::DimuonCandidate& d) { return d.vtxValid; }))
    return;
  bool hasValidCandidate = false;
  auto check = [this, &hasValidCandidate](const std::vector<bmmg::BCandidate>& cands) -> const std::vector<bmmg::BCandidate>& {
    for (const auto& c : cands)
      if (!hasValidCandidate && isValidCandidate(c)) hasValidCandidate = true;
    return cands;
  };
  // the object collections are filled first: the candidate truth flags use their gen matches
  writer_->fillMuons(selMuons, beamSpot, trigger, gen.get());
  writer_->fillPhotons(selPhotons, lazyTools, mmgVertexFit_, gen.get());
  writer_->fillDimuons(dimuons);

  if (doMuMuGamma_)
    writer_->fillCandidates(bmmg::NtupleWriter::kMuMuGamma,
                            check(bmmg::buildMuMuGamma(selMuons, dimuons, selPhotons, vertices, pvTracks, lazyTools,
                                                       ttBuilder, mmgVertexFit_, candidateConfig_)),
                            vertices);
  if (doMuMuGammaGamma_)
    writer_->fillCandidates(bmmg::NtupleWriter::kMuMuGammaGamma,
                            check(bmmg::buildMuMuGammaGamma(selMuons, dimuons, selPhotons, vertices, pvTracks, lazyTools,
                                                            ttBuilder, mmgVertexFit_, candidateConfig_, diphotonConfig_)),
                            vertices);
  if (doMuMuConvGamma_) {
    const std::vector<bmmg::SelectedConversion> conversions =
        bmmg::selectConversions(event.get(conversionsToken_), ttBuilder, conversionCuts_);
    writer_->fillConversions(conversions, gen.get());
    writer_->fillCandidates(
        bmmg::NtupleWriter::kMuMuConvGamma,
        check(bmmg::buildMuMuConvGamma(selMuons, dimuons, conversions, vertices, pvTracks, ttBuilder, candidateConfig_)),
        vertices);
  }
  if (doMuMuKK_) {
    const std::vector<bmmg::SelectedTrack> tracks =
        bmmg::selectKaonTracks(isMiniAOD_ ? packedTracks : event.get(tracksToken_), selMuons, ttBuilder, trackCuts_);
    std::vector<bmmg::SelectedTrack> usedTracks;
    bmmg::CandidateConfig kkConfig = candidateConfig_;
    kkConfig.minPrefitMass = kaonConfig_.minBMass;
    kkConfig.maxPrefitMass = kaonConfig_.maxBMass;
    const std::vector<bmmg::BCandidate> mmkk =
        bmmg::buildMuMuKK(selMuons, dimuons, tracks, vertices, pvTracks, ttBuilder, kkConfig, kaonConfig_, usedTracks);
    writer_->fillTracks(usedTracks, beamSpot, gen.get());   // before the candidates (truth flags)
    writer_->fillCandidates(bmmg::NtupleWriter::kMuMuKK, check(mmkk), vertices);
  }
  if (onlyEventsWithCandidates_ && !hasValidCandidate) return;
  writer_->fill();
  ++nEventsStored_;
}

bool RadiativeAnalysis::isValidCandidate(const bmmg::BCandidate& c) const {
  if (!c.fit) return false;
  const auto& v = c.fit->vertex;
  if (!v || !v->vertexIsValid()) return false;
  if (minCandidateVtxProb_ < 0.) return true;
  return TMath::Prob(v->chiSquared(), static_cast<int>(std::lround(v->degreesOfFreedom()))) > minCandidateVtxProb_;
}

void RadiativeAnalysis::endJob() {
  bmmg::NtupleWriter::RunInfo info;
  info.nEvents = nEvents_;
  info.nEventsStored = nEventsStored_;
  if (isMC_) {
    info.xsec = xsec_;
    info.xsecErr = xsecErr_;
    if (genFilterInfo_) {
      info.filterEff = genFilterInfo_->filterEfficiency(-1);
      info.filterEffErr = genFilterInfo_->filterEfficiencyError(-1);
    }
    // integrated luminosity equivalent of the processed events
    if (info.xsec > 0 && info.filterEff > 0 && nEvents_ > 0) {
      info.lumiEquivalent = nEvents_ / (info.xsec * info.filterEff);
      const double rel = std::hypot(info.xsecErr / info.xsec, info.filterEffErr / info.filterEff);
      info.lumiEquivalentErr = info.lumiEquivalent * rel;
    }
  }
  writer_->fillRunInfo(info);
}

void RadiativeAnalysis::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<bool>("isMC", true);
  desc.addUntracked<bool>("verbose", false);
  // input format: "AOD" or "MINIAOD"; the collection names below must match (see runNtuplizer_cfg.py)
  desc.add<std::string>("dataFormat", "AOD");
  // collections (AOD defaults)
  desc.add<edm::InputTag>("muons", edm::InputTag("muons"));
  desc.add<edm::InputTag>("photons", edm::InputTag("gedPhotons"));
  desc.add<edm::InputTag>("photonMVA",
                          edm::InputTag("photonMVAValueMapProducer", "PhotonMVAEstimatorRunIIIWinter22v1Values"));
  desc.add<edm::InputTag>("photonTightId", edm::InputTag("egmPhotonIDs", "mvaPhoID-RunIIIWinter22-v1-wp90"));
  desc.add<edm::InputTag>("beamSpot", edm::InputTag("offlineBeamSpot"));
  desc.add<edm::InputTag>("primaryVertices", edm::InputTag("offlinePrimaryVertices"));
  desc.add<edm::InputTag>("ebRecHits", edm::InputTag("reducedEcalRecHitsEB"));
  desc.add<edm::InputTag>("eeRecHits", edm::InputTag("reducedEcalRecHitsEE"));
  desc.add<edm::InputTag>("triggerResults", edm::InputTag("TriggerResults", "", "HLT"));
  desc.add<edm::InputTag>("triggerEvent", edm::InputTag("hltTriggerSummaryAOD", "", "HLT"));   // AOD
  desc.add<edm::InputTag>("triggerObjects", edm::InputTag("slimmedPatTrigger"));                // MiniAOD
  desc.add<edm::InputTag>("packedCandidates", edm::InputTag("packedPFCandidates"));              // MiniAOD
  desc.add<edm::InputTag>("lostTracks", edm::InputTag("lostTracks"));                            // MiniAOD
  desc.add<edm::InputTag>("genParticles", edm::InputTag("genParticles"));                         // MiniAOD: prunedGenParticles
  desc.add<edm::InputTag>("packedGenParticles", edm::InputTag("packedGenParticles"));            // MiniAOD
  desc.add<edm::InputTag>("pileup", edm::InputTag("addPileupInfo"));
  desc.add<edm::InputTag>("genRunInfo", edm::InputTag("generator"));
  desc.add<edm::InputTag>("genFilterInfo", edm::InputTag("genFilterEfficiencyProducer"));
  // trigger
  desc.add<std::vector<std::string>>("triggerPaths",
                                     {"HLT_DoubleMu4_3_Bs", "HLT_DoubleMu4_3_Jpsi", "HLT_DoubleMu4_3_LowMass",
                                      "HLT_DoubleMu4_LowMass_Displaced", "HLT_DoubleMu4_3_Photon4_BsToMMG",
                                      "HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG", "HLT_DoubleMu4_JpsiTrkTrk_Displaced",
                                      "HLT_DoubleMu4_Jpsi_NoVertexing", "HLT_DoubleMu4_Jpsi_Displaced"});
  desc.add<std::string>("hltProcess", "HLT");
  desc.add<double>("triggerMatchDeltaR", 0.1);
  // muon MVA (run3mvaid)
  desc.add<std::vector<std::string>>("xgboost_models", {"Run2022-20231030-1731-Event0"});
  desc.add<std::vector<std::string>>("xgboost_variable_names", {"run3muonmva"});
  // preselection
  desc.add<double>("muonMinPt", 2.0);
  desc.add<double>("muonMaxAbsEta", 2.4);
  desc.add<double>("muonMinMVA", -99.);
  desc.add<double>("photonMinSCEnergy", 1.0);
  desc.add<double>("photonMaxAbsSCEta", 2.5);
  desc.add<double>("photonMinMVA", -99.);
  // channels
  desc.add<bool>("doMuMuGamma", true);
  desc.add<bool>("doMuMuConvGamma", true);
  desc.add<bool>("doMuMuGammaGamma", true);
  desc.add<bool>("doMuMuKK", true);
  // skim: write only the events with >= 1 candidate (any enabled channel) whose fit succeeded and whose vertex
  // probability is > minCandidateVtxProb (< 0: any successful fit). RunInfo.nEvents still counts all events.
  desc.add<bool>("onlyEventsWithCandidates", false);
  desc.add<double>("minCandidateVtxProb", -1.);
  desc.add<edm::InputTag>("conversions", edm::InputTag("allConversions"));
  desc.add<edm::InputTag>("tracks", edm::InputTag("generalTracks"));
  desc.add<double>("minPrefitMass", 4.0);   // pre-fit B mass window (photon channels)
  desc.add<double>("maxPrefitMass", 7.0);
  desc.add<bool>("computeTrackCompatibility", true);
  desc.add<double>("conversionMinPt", 0.5);
  desc.add<double>("conversionMinVertexProb", 1e-6);
  desc.add<double>("diphotonMinMass", 0.05);
  desc.add<double>("diphotonMaxMass", 1.1);
  desc.add<double>("kaonMinPt", 0.7);
  desc.add<double>("kkMinMass", 0.99);
  desc.add<double>("kkMaxMass", 1.05);
  desc.add<bool>("kkJpsiOnly", true);
  desc.add<double>("kkMinBMass", 4.9);   // pre-fit B mass window of mu mu K K
  desc.add<double>("kkMaxBMass", 5.9);
  // MMGVertexFit photon error scale factors (sigma), tuned on gedPhotons (MMGVertexFitExample.py)
  desc.add<double>("clusterCovScalePhiEB", 0.16);
  desc.add<double>("clusterCovScaleThetaEB", 0.165);
  desc.add<double>("energyErrorScaleEB", 1.2);
  desc.add<double>("clusterCovScalePhiEE", 0.20);
  desc.add<double>("clusterCovScaleThetaEE", 0.24);
  desc.add<double>("energyErrorScaleEE", 1.3);
  descriptions.add("radiativeAnalysis", desc);
}

DEFINE_FWK_MODULE(RadiativeAnalysis);
