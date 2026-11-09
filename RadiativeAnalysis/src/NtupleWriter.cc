#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/NtupleWriter.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicVertex.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "DataFormats/Candidate/interface/LeafCandidate.h"

#include <cmath>
#include <initializer_list>

namespace bmmg {

  namespace {
    constexpr float kNaN = -999.f;   // default of float scalars that are not filled

    void bookFloats(BranchSet& b, const std::string& prefix, std::initializer_list<const char*> names) {
      for (const char* n : names) b.bookVector<float>(prefix + n);
    }
    void bookInts(BranchSet& b, const std::string& prefix, std::initializer_list<const char*> names) {
      for (const char* n : names) b.bookVector<int>(prefix + n);
    }
    std::string triggerBranch(const std::string& path) {
      return path.rfind("HLT_", 0) == 0 ? path.substr(4) : path;   // HLT_DoubleMu4_3_Bs -> DoubleMu4_3_Bs
    }
  }  // namespace

  const NtupleWriter::Channel NtupleWriter::kMuMuGamma{"mmg_", {"pho"}, NtupleWriter::LegType::Photon};
  const NtupleWriter::Channel NtupleWriter::kMuMuConvGamma{"mmcg_", {"conv"}, NtupleWriter::LegType::Conversion};
  const NtupleWriter::Channel NtupleWriter::kMuMuGammaGamma{"mmgg_", {"pho1", "pho2"}, NtupleWriter::LegType::Photon};
  const NtupleWriter::Channel NtupleWriter::kMuMuKK{"mmkk_", {"trk1", "trk2"}, NtupleWriter::LegType::Track};

  NtupleWriter::NtupleWriter(TFileService& fs, const std::vector<std::string>& triggerPaths, bool isMC,
                             const std::vector<Channel>& channels)
      : events_(fs.make<TTree>("Events", "B -> mu mu gamma ntuple")),
        runInfo_(fs.make<TTree>("RunInfo", "job-level information")),
        b_(events_),
        r_(runInfo_),
        triggerPaths_(triggerPaths),
        isMC_(isMC),
        channels_(channels) {
    book();
  }

  // =====================================================================================
  // booking: one place per block, the fill functions below follow the same order
  // =====================================================================================
  void NtupleWriter::book() {
    // ---- event ----
    b_.bookScalar<unsigned int>("evt_run", 0);
    b_.bookScalar<unsigned int>("evt_lumi", 0);
    b_.bookScalar<unsigned long long>("evt_event", 0);
    b_.bookScalar<int>("evt_nPV", 0);
    b_.bookScalar<int>("evt_nGoodPV", 0);
    if (isMC_) {
      b_.bookScalar<int>("evt_nPU", -1);
      b_.bookScalar<float>("evt_nTruePU", -1.f);
    }

    // ---- beam spot and event primary vertex (first vertex of the collection) ----
    for (const char* n : {"bs_x", "bs_y", "bs_z", "bs_xErr", "bs_yErr", "bs_zErr", "bs_sigmaZ", "bs_dxdz", "bs_dydz",
                          "bs_widthX", "bs_widthY", "pv_x", "pv_y", "pv_z", "pv_xErr", "pv_yErr", "pv_zErr", "pv_chi2",
                          "pv_ndof"})
      b_.bookScalar<float>(n, kNaN);
    b_.bookScalar<int>("pv_nTracks", -1);

    // ---- trigger ----
    for (const auto& p : triggerPaths_) b_.bookScalar<int>("trg_" + triggerBranch(p), -1);

    // ---- muons ----
    bookInts(b_, "mu_", {"index", "charge", "isGlobal", "isTracker", "isPF", "isLoose", "isMedium", "isSoft",
                         "isGoodTMOneStationTight", "highPurity", "trkLayers", "pixLayers", "validHits"});
    bookFloats(b_, "mu_", {"pt", "eta", "phi", "mva", "dxyBS", "dxyBSErr", "dzBS", "trkNormChi2", "validFraction"});
    for (const auto& p : triggerPaths_) b_.bookVector<int>("mu_trg_" + triggerBranch(p));
    if (isMC_) {
      bookInts(b_, "mu_", {"genPdgId", "genMotherPdgId", "genFromSignalB"});
      bookFloats(b_, "mu_", {"genDR"});
    }

    // ---- photons ----
    bookInts(b_, "pho_", {"index", "isEB", "isEE", "tightId", "hasPixelSeed", "hasConversionTracks", "hasClusterHits"});
    bookFloats(b_, "pho_", {"pt", "eta", "phi", "energy", "energyErr", "mva", "scEnergy", "scRawEnergy", "scEta",
                            "scPhi", "scEtaWidth", "scPhiWidth", "r9", "sigmaIetaIeta", "sigmaIetaIphi",
                            "sigmaIphiIphi", "hOverE", "seedTime", "fitEnergyErr"});
    if (isMC_) {
      bookInts(b_, "pho_", {"genPdgId", "genMotherPdgId", "genFromSignalB"});
      bookFloats(b_, "pho_", {"genDR"});
    }

    // ---- dimuons ----
    bookInts(b_, "dimu_", {"mu1", "mu2", "vtxValid", "isCowboy", "resonance"});
    bookFloats(b_, "dimu_", {"mass", "pt", "eta", "phi", "deltaR", "vtxProb", "vtxChi2", "vtxNdof", "vtxX", "vtxY",
                             "vtxZ", "lxyBS", "lxyBSErr", "cosAlphaBS", "dca"});

    // ---- conversions (if the mu mu gamma(conv) channel is enabled) and kaon tracks (mu mu K K) ----
    for (const auto& ch : channels_) {
      if (ch.legType == LegType::Conversion) {
        bookInts(b_, "conv_", {"index", "fitOK"});
        bookFloats(b_, "conv_", {"pt", "eta", "phi", "vtxX", "vtxY", "vtxZ", "vtxRho", "vtxProb", "trkPlusPt",
                                 "trkMinusPt"});
        if (isMC_) {
          bookInts(b_, "conv_", {"genPdgId", "genMotherPdgId", "genFromSignalB"});
          bookFloats(b_, "conv_", {"genDR"});
        }
      }
      if (ch.legType == LegType::Track) {
        bookInts(b_, "trk_", {"index", "charge", "validHits", "pixLayers"});
        bookFloats(b_, "trk_", {"pt", "eta", "phi", "dxyBS", "dzBS", "normChi2"});
        if (isMC_) {
          bookInts(b_, "trk_", {"genPdgId", "genMotherPdgId", "genFromSignalB"});
          bookFloats(b_, "trk_", {"genDR"});
        }
      }
    }

    // ---- B candidates of each channel ----
    for (const auto& ch : channels_) bookCandidates(ch);

    // ---- generator ----
    if (isMC_) {
      b_.bookScalar<int>("gen_valid", 0);
      b_.bookScalar<int>("gen_decayMode", -1);
      b_.bookScalar<int>("gen_B_pdgId", 0);
      b_.bookScalar<int>("gen_isSignal", 0);
      b_.bookScalar<int>("gen_is3Body", 0);
      b_.bookScalar<int>("gen_is4Body", 0);
      b_.bookScalar<int>("gen_hasJpsi", 0);
      b_.bookScalar<int>("gen_neutralMesonPdgId", 0);
      b_.bookScalar<int>("gen_vectorMesonPdgId", 0);
      for (const char* n : {"gen_B_pt", "gen_B_eta", "gen_B_phi", "gen_B_mass", "gen_B_ct2D", "gen_B_ct3D", "gen_B_lxy",
                            "gen_pvX", "gen_pvY", "gen_pvZ", "gen_svX", "gen_svY", "gen_svZ",
                            "gen_muPlus_pt", "gen_muPlus_eta", "gen_muPlus_phi",
                            "gen_muMinus_pt", "gen_muMinus_eta", "gen_muMinus_phi",
                            "gen_dimuonMass", "gen_helicity", "gen_coplanarity", "gen_cosThetaL", "gen_triBodyMass",
                            "gen_diphotonMass", "gen_fourBodyMass", "gen_helicity4", "gen_coplanarity4"})
        b_.bookScalar<float>(n, kNaN);
      bookInts(b_, "gen_pho_", {"origin", "motherPdgId"});
      bookFloats(b_, "gen_pho_", {"pt", "eta", "phi"});
    }

    // ---- run info ----
    for (const char* n : {"xsec", "xsecErr", "filterEff", "filterEffErr", "lumiEquivalent", "lumiEquivalentErr"})
      r_.bookScalar<double>(n, -1.);
    r_.bookScalar<unsigned long long>("nEvents", 0);
  }

  void NtupleWriter::bookCandidates(const Channel& ch) {
    const std::string& p = ch.prefix;
    bookInts(b_, p, {"dimu", "mu1", "mu2", "pv", "fitOK"});
    for (const auto& leg : ch.legs) b_.bookVector<int>(p + leg);
    bookFloats(b_, p, {"prefitMass", "prefitPt", "prefitEta", "prefitPhi",
                       "mass", "massErr", "pt", "eta", "phi", "fitDimuonMass",
                       "vtxProb", "vtxChi2", "vtxNdof", "svX", "svY", "svZ", "svXErr", "svYErr", "svZErr",
                       "pvX", "pvY", "pvZ",
                       "l3d", "l3dErr", "lxy", "lxyErr", "cosAlpha3D", "cosAlpha2D",
                       "ct3D", "ct3DErr", "ct2D", "ct2DErr",
                       "mu1FitPt", "mu1FitEta", "mu1FitPhi", "mu2FitPt", "mu2FitEta", "mu2FitPhi",
                       "helicity", "coplanarity", "cosThetaL", "isolation", "maxTrackCompatibility"});
    if (ch.legs.size() > 1) b_.bookVector<float>(p + "xMass");   // diphoton / KK mass (pre-fit)
    for (const auto& leg : ch.legs)
      for (const char* v : {"FitPt", "FitEta", "FitPhi"}) b_.bookVector<float>(p + leg + v);
    if (isMC_) b_.bookVector<int>(p + "genMatched");
  }

  // =====================================================================================
  // filling
  // =====================================================================================
  void NtupleWriter::beginEvent(const edm::EventID& id, unsigned int lumi) {
    b_.reset();
    muonFromB_.clear();
    photonFromB_.clear();
    conversionFromB_.clear();
    trackFromB_.clear();
    b_.scalar<unsigned int>("evt_run") = id.run();
    b_.scalar<unsigned int>("evt_lumi") = lumi;
    b_.scalar<unsigned long long>("evt_event") = id.event();
  }

  void NtupleWriter::fillPileup(const std::vector<PileupSummaryInfo>& pu) {
    for (const auto& p : pu)
      if (p.getBunchCrossing() == 0) {
        b_.scalar<int>("evt_nPU") = p.getPU_NumInteractions();
        b_.scalar<float>("evt_nTruePU") = p.getTrueNumInteractions();
      }
  }

  void NtupleWriter::fillVertices(const reco::BeamSpot& bs, const std::vector<reco::Vertex>& pvs) {
    b_.scalar<float>("bs_x") = bs.x0();
    b_.scalar<float>("bs_y") = bs.y0();
    b_.scalar<float>("bs_z") = bs.z0();
    b_.scalar<float>("bs_xErr") = bs.x0Error();
    b_.scalar<float>("bs_yErr") = bs.y0Error();
    b_.scalar<float>("bs_zErr") = bs.z0Error();
    b_.scalar<float>("bs_sigmaZ") = bs.sigmaZ();
    b_.scalar<float>("bs_dxdz") = bs.dxdz();
    b_.scalar<float>("bs_dydz") = bs.dydz();
    b_.scalar<float>("bs_widthX") = bs.BeamWidthX();
    b_.scalar<float>("bs_widthY") = bs.BeamWidthY();

    b_.scalar<int>("evt_nPV") = pvs.size();
    int nGood = 0;
    for (const auto& pv : pvs)
      if (isGoodPrimaryVertex(pv)) ++nGood;
    b_.scalar<int>("evt_nGoodPV") = nGood;
    if (pvs.empty()) return;
    const reco::Vertex& pv = pvs.front();
    b_.scalar<float>("pv_x") = pv.x();
    b_.scalar<float>("pv_y") = pv.y();
    b_.scalar<float>("pv_z") = pv.z();
    b_.scalar<float>("pv_xErr") = pv.xError();
    b_.scalar<float>("pv_yErr") = pv.yError();
    b_.scalar<float>("pv_zErr") = pv.zError();
    b_.scalar<float>("pv_chi2") = pv.chi2();
    b_.scalar<float>("pv_ndof") = pv.ndof();
    b_.scalar<int>("pv_nTracks") = pv.nTracks();
  }

  void NtupleWriter::fillTrigger(const TriggerMatcher::Result& trigger) {
    for (size_t i = 0; i < triggerPaths_.size(); ++i)
      b_.scalar<int>("trg_" + triggerBranch(triggerPaths_[i])) = trigger.fired.at(i);
  }

  void NtupleWriter::fillMuons(const std::vector<SelectedMuon>& muons, const reco::BeamSpot& bs,
                               const TriggerMatcher::Result& trigger, const GenDecayChain* gen) {
    for (size_t i = 0; i < muons.size(); ++i) {
      const reco::Muon& mu = *muons[i].muon;
      const reco::Track& trk = *mu.innerTrack();
      b_.vec<int>("mu_index").push_back(muons[i].index);
      b_.vec<int>("mu_charge").push_back(mu.charge());
      b_.vec<int>("mu_isGlobal").push_back(mu.isGlobalMuon());
      b_.vec<int>("mu_isTracker").push_back(mu.isTrackerMuon());
      b_.vec<int>("mu_isPF").push_back(mu.isPFMuon());
      b_.vec<int>("mu_isLoose").push_back(muon::isLooseMuon(mu));
      b_.vec<int>("mu_isMedium").push_back(muon::isMediumMuon(mu));
      b_.vec<int>("mu_isSoft").push_back(mu.passed(reco::Muon::SoftCutBasedId));
      b_.vec<int>("mu_isGoodTMOneStationTight").push_back(muon::isGoodMuon(mu, muon::TMOneStationTight));
      b_.vec<int>("mu_highPurity").push_back(trk.quality(reco::TrackBase::highPurity));
      b_.vec<int>("mu_trkLayers").push_back(trk.hitPattern().trackerLayersWithMeasurement());
      b_.vec<int>("mu_pixLayers").push_back(trk.hitPattern().pixelLayersWithMeasurement());
      b_.vec<int>("mu_validHits").push_back(trk.numberOfValidHits());
      b_.vec<float>("mu_pt").push_back(mu.pt());
      b_.vec<float>("mu_eta").push_back(mu.eta());
      b_.vec<float>("mu_phi").push_back(mu.phi());
      b_.vec<float>("mu_mva").push_back(muons[i].mva);
      b_.vec<float>("mu_dxyBS").push_back(trk.dxy(bs.position()));
      b_.vec<float>("mu_dxyBSErr").push_back(trk.dxyError());
      b_.vec<float>("mu_dzBS").push_back(trk.dz(bs.position()));
      b_.vec<float>("mu_trkNormChi2").push_back(trk.normalizedChi2());
      b_.vec<float>("mu_validFraction").push_back(trk.validFraction());
      for (size_t p = 0; p < triggerPaths_.size(); ++p)
        b_.vec<int>("mu_trg_" + triggerBranch(triggerPaths_[p])).push_back(trigger.muonMatched.at(p).at(i));
      if (isMC_) {
        GenDecayChain::TruthMatchInfo m;
        if (gen) m = gen->matchMuonToGen(mu);
        const bool fromB = gen && gen->fromSignalB(m);
        muonFromB_.push_back(fromB);
        b_.vec<int>("mu_genPdgId").push_back(m.genPdgId);
        b_.vec<int>("mu_genMotherPdgId").push_back(m.genMotherPdgId);
        b_.vec<int>("mu_genFromSignalB").push_back(fromB);
        b_.vec<float>("mu_genDR").push_back(m.isMatched ? m.deltaR : -1.f);
      }
    }
  }

  void NtupleWriter::fillPhotons(const std::vector<SelectedPhoton>& photons, EcalClusterLazyTools& lazyTools,
                                 const MMGVertexFit& fitter, const GenDecayChain* gen) {
    for (const auto& sp : photons) {
      const reco::Photon& pho = *sp.photon;
      const reco::SuperCluster& sc = *pho.superCluster();
      const bool clusterHits = MMGVertexFit::hasClusterHits(pho);
      b_.vec<int>("pho_index").push_back(sp.index);
      b_.vec<int>("pho_isEB").push_back(pho.isEB());
      b_.vec<int>("pho_isEE").push_back(pho.isEE());
      b_.vec<int>("pho_tightId").push_back(sp.tightId);
      b_.vec<int>("pho_hasPixelSeed").push_back(pho.hasPixelSeed());
      b_.vec<int>("pho_hasConversionTracks").push_back(pho.hasConversionTracks());
      b_.vec<int>("pho_hasClusterHits").push_back(clusterHits);
      b_.vec<float>("pho_pt").push_back(pho.pt());
      b_.vec<float>("pho_eta").push_back(pho.eta());
      b_.vec<float>("pho_phi").push_back(pho.phi());
      b_.vec<float>("pho_energy").push_back(pho.energy());
      b_.vec<float>("pho_energyErr").push_back(pho.getCorrectedEnergyError(pho.getCandidateP4type()));
      b_.vec<float>("pho_mva").push_back(sp.mva);
      b_.vec<float>("pho_scEnergy").push_back(sc.energy());
      b_.vec<float>("pho_scRawEnergy").push_back(sc.rawEnergy());
      b_.vec<float>("pho_scEta").push_back(sc.eta());
      b_.vec<float>("pho_scPhi").push_back(sc.phi());
      b_.vec<float>("pho_scEtaWidth").push_back(sc.etaWidth());
      b_.vec<float>("pho_scPhiWidth").push_back(sc.phiWidth());
      b_.vec<float>("pho_r9").push_back(pho.full5x5_r9());
      b_.vec<float>("pho_sigmaIetaIeta").push_back(pho.full5x5_sigmaIetaIeta());
      b_.vec<float>("pho_sigmaIetaIphi").push_back(pho.full5x5_showerShapeVariables().sigmaIetaIphi);
      b_.vec<float>("pho_sigmaIphiIphi").push_back(pho.full5x5_showerShapeVariables().sigmaIphiIphi);
      b_.vec<float>("pho_hOverE").push_back(pho.hadronicOverEm());
      b_.vec<float>("pho_seedTime").push_back(clusterHits ? lazyTools.SuperClusterSeedTime(sc) : kNaN);
      b_.vec<float>("pho_fitEnergyErr").push_back(fitter.photonEnergyError(pho));
      if (isMC_) {
        GenDecayChain::TruthMatchInfo m;
        if (gen) m = gen->matchPhotonToGen(pho);
        const bool fromB = gen && gen->fromSignalB(m);
        photonFromB_.push_back(fromB);
        b_.vec<int>("pho_genPdgId").push_back(m.genPdgId);
        b_.vec<int>("pho_genMotherPdgId").push_back(m.genMotherPdgId);
        b_.vec<int>("pho_genFromSignalB").push_back(fromB);
        b_.vec<float>("pho_genDR").push_back(m.isMatched ? m.deltaR : -1.f);
      }
    }
  }

  void NtupleWriter::fillDimuons(const std::vector<DimuonCandidate>& dimuons) {
    for (const auto& d : dimuons) {
      b_.vec<int>("dimu_mu1").push_back(d.mu1);
      b_.vec<int>("dimu_mu2").push_back(d.mu2);
      b_.vec<int>("dimu_vtxValid").push_back(d.vtxValid);
      b_.vec<int>("dimu_isCowboy").push_back(d.isCowboy);
      b_.vec<int>("dimu_resonance").push_back(d.resonance);
      b_.vec<float>("dimu_mass").push_back(d.p4.mass());
      b_.vec<float>("dimu_pt").push_back(d.p4.pt());
      b_.vec<float>("dimu_eta").push_back(d.p4.eta());
      b_.vec<float>("dimu_phi").push_back(d.p4.phi());
      b_.vec<float>("dimu_deltaR").push_back(d.deltaR);
      b_.vec<float>("dimu_vtxProb").push_back(d.vtxProb);
      b_.vec<float>("dimu_vtxChi2").push_back(d.vtxChi2);
      b_.vec<float>("dimu_vtxNdof").push_back(d.vtxNdof);
      b_.vec<float>("dimu_vtxX").push_back(d.vtxValid ? d.vtx.x() : kNaN);
      b_.vec<float>("dimu_vtxY").push_back(d.vtxValid ? d.vtx.y() : kNaN);
      b_.vec<float>("dimu_vtxZ").push_back(d.vtxValid ? d.vtx.z() : kNaN);
      b_.vec<float>("dimu_lxyBS").push_back(d.lxyBS);
      b_.vec<float>("dimu_lxyBSErr").push_back(d.lxyBSErr);
      b_.vec<float>("dimu_cosAlphaBS").push_back(d.cosAlphaBS);
      b_.vec<float>("dimu_dca").push_back(d.dca);
    }
  }

  const std::vector<int>& NtupleWriter::legFromB(LegType type) const {
    switch (type) {
      case LegType::Photon:
        return photonFromB_;
      case LegType::Conversion:
        return conversionFromB_;
      default:
        return trackFromB_;
    }
  }

  void NtupleWriter::fillConversions(const std::vector<SelectedConversion>& conversions, const GenDecayChain* gen) {
    for (const auto& sc : conversions) {
      const reco::Conversion& conv = *sc.conversion;
      const auto& v = conv.conversionVertex();
      b_.vec<int>("conv_index").push_back(sc.index);
      b_.vec<int>("conv_fitOK").push_back(sc.photon.has_value());
      b_.vec<float>("conv_pt").push_back(sc.p4.pt());
      b_.vec<float>("conv_eta").push_back(sc.p4.eta());
      b_.vec<float>("conv_phi").push_back(sc.p4.phi());
      b_.vec<float>("conv_vtxX").push_back(v.x());
      b_.vec<float>("conv_vtxY").push_back(v.y());
      b_.vec<float>("conv_vtxZ").push_back(v.z());
      b_.vec<float>("conv_vtxRho").push_back(v.position().rho());
      b_.vec<float>("conv_vtxProb").push_back(TMath::Prob(v.chi2(), static_cast<int>(v.ndof())));
      b_.vec<float>("conv_trkPlusPt").push_back(sc.track1.track().pt());
      b_.vec<float>("conv_trkMinusPt").push_back(sc.track2.track().pt());
      if (isMC_) {
        GenDecayChain::TruthMatchInfo m;
        if (gen) m = gen->matchPhotonToGen(reco::LeafCandidate(0, sc.p4));
        const bool fromB = gen && gen->fromSignalB(m);
        conversionFromB_.push_back(fromB);
        b_.vec<int>("conv_genPdgId").push_back(m.genPdgId);
        b_.vec<int>("conv_genMotherPdgId").push_back(m.genMotherPdgId);
        b_.vec<int>("conv_genFromSignalB").push_back(fromB);
        b_.vec<float>("conv_genDR").push_back(m.isMatched ? m.deltaR : -1.f);
      }
    }
  }

  void NtupleWriter::fillTracks(const std::vector<SelectedTrack>& tracks, const reco::BeamSpot& bs,
                                const GenDecayChain* gen) {
    for (const auto& st : tracks) {
      const reco::Track& trk = *st.track;
      b_.vec<int>("trk_index").push_back(st.index);
      b_.vec<int>("trk_charge").push_back(trk.charge());
      b_.vec<int>("trk_validHits").push_back(trk.numberOfValidHits());
      b_.vec<int>("trk_pixLayers").push_back(trk.hitPattern().pixelLayersWithMeasurement());
      b_.vec<float>("trk_pt").push_back(trk.pt());
      b_.vec<float>("trk_eta").push_back(trk.eta());
      b_.vec<float>("trk_phi").push_back(trk.phi());
      b_.vec<float>("trk_dxyBS").push_back(trk.dxy(bs.position()));
      b_.vec<float>("trk_dzBS").push_back(trk.dz(bs.position()));
      b_.vec<float>("trk_normChi2").push_back(trk.normalizedChi2());
      if (isMC_) {
        GenDecayChain::TruthMatchInfo m;
        if (gen) {
          const double mK = pdg::kaonMass;
          const reco::Candidate::LorentzVector p4(trk.px(), trk.py(), trk.pz(), std::sqrt(trk.p() * trk.p() + mK * mK));
          m = gen->matchToGen(reco::LeafCandidate(trk.charge(), p4), 321, 0, 0, 0.05, 0.5);
        }
        const bool fromB = gen && gen->fromSignalB(m);
        trackFromB_.push_back(fromB);
        b_.vec<int>("trk_genPdgId").push_back(m.genPdgId);
        b_.vec<int>("trk_genMotherPdgId").push_back(m.genMotherPdgId);
        b_.vec<int>("trk_genFromSignalB").push_back(fromB);
        b_.vec<float>("trk_genDR").push_back(m.isMatched ? m.deltaR : -1.f);
      }
    }
  }

  void NtupleWriter::fillCandidates(const Channel& ch, const std::vector<BCandidate>& candidates,
                                    const std::vector<reco::Vertex>& pvs) {
    const std::string& p = ch.prefix;
    auto F = [this, &p](const std::string& n) -> std::vector<float>& { return b_.vec<float>(p + n); };
    auto I = [this, &p](const std::string& n) -> std::vector<int>& { return b_.vec<int>(p + n); };
    const std::vector<int>& legsFromB = legFromB(ch.legType);
    for (const auto& c : candidates) {
      I("dimu").push_back(c.dimuon);
      I("mu1").push_back(c.mu1);
      I("mu2").push_back(c.mu2);
      I("pv").push_back(c.pv);
      I("fitOK").push_back(c.fit.has_value());
      for (size_t k = 0; k < ch.legs.size(); ++k) I(ch.legs[k]).push_back(c.legs.at(k));
      if (isMC_) {
        auto fromB = [](const std::vector<int>& v, int i) { return i >= 0 && i < (int)v.size() && v[i]; };
        bool matched = fromB(muonFromB_, c.mu1) && fromB(muonFromB_, c.mu2);
        for (int leg : c.legs) matched = matched && fromB(legsFromB, leg);
        I("genMatched").push_back(matched);
      }
      F("prefitMass").push_back(c.prefitP4.mass());
      F("prefitPt").push_back(c.prefitP4.pt());
      F("prefitEta").push_back(c.prefitP4.eta());
      F("prefitPhi").push_back(c.prefitP4.phi());
      if (ch.legs.size() > 1) F("xMass").push_back(c.xP4.mass());
      const reco::Vertex& pv = pvs.at(c.pv);
      F("pvX").push_back(pv.x());
      F("pvY").push_back(pv.y());
      F("pvZ").push_back(pv.z());
      F("helicity").push_back(c.helicity);
      F("coplanarity").push_back(c.coplanarity);
      F("cosThetaL").push_back(c.cosThetaL);
      F("isolation").push_back(c.isolation);
      F("maxTrackCompatibility").push_back(c.maxTrackCompatibility);
      F("l3d").push_back(c.flight.l3d);
      F("l3dErr").push_back(c.flight.l3dErr);
      F("lxy").push_back(c.flight.lxy);
      F("lxyErr").push_back(c.flight.lxyErr);
      F("cosAlpha3D").push_back(c.flight.cosAlpha3D);
      F("cosAlpha2D").push_back(c.flight.cosAlpha2D);
      F("ct3D").push_back(c.flight.ct3D);
      F("ct3DErr").push_back(c.flight.ct3DErr);
      F("ct2D").push_back(c.flight.ct2D);
      F("ct2DErr").push_back(c.flight.ct2DErr);
      F("massErr").push_back(c.massErr);

      // daughter names in the order of the fit: mu1, mu2, legs...
      std::vector<std::string> daughterNames{"mu1", "mu2"};
      daughterNames.insert(daughterNames.end(), ch.legs.begin(), ch.legs.end());
      if (c.fit) {
        const KinematicState& s = c.fit->particle->currentState();
        const RefCountedKinematicVertex& v = c.fit->vertex;
        const GlobalVector mom = s.globalMomentum();
        F("mass").push_back(s.mass());
        F("pt").push_back(mom.perp());
        F("eta").push_back(mom.eta());
        F("phi").push_back(mom.phi());
        F("vtxChi2").push_back(v->chiSquared());
        F("vtxNdof").push_back(v->degreesOfFreedom());
        F("vtxProb").push_back(TMath::Prob(v->chiSquared(), static_cast<int>(std::lround(v->degreesOfFreedom()))));
        F("svX").push_back(v->position().x());
        F("svY").push_back(v->position().y());
        F("svZ").push_back(v->position().z());
        F("svXErr").push_back(std::sqrt(v->error().cxx()));
        F("svYErr").push_back(std::sqrt(v->error().cyy()));
        F("svZErr").push_back(std::sqrt(v->error().czz()));
        TLorentzVector dimu;
        for (size_t k = 0; k < daughterNames.size(); ++k) {
          const KinematicState& ds = c.fit->daughters.at(k)->currentState();
          const GlobalVector dp = ds.globalMomentum();
          if (k < 2) dimu += TLorentzVector(dp.x(), dp.y(), dp.z(), ds.kinematicParameters().energy());
          F(daughterNames[k] + "FitPt").push_back(dp.perp());
          F(daughterNames[k] + "FitEta").push_back(dp.eta());
          F(daughterNames[k] + "FitPhi").push_back(dp.phi());
        }
        F("fitDimuonMass").push_back(dimu.M());
      } else {
        for (const char* n : {"mass", "pt", "eta", "phi", "vtxChi2", "vtxNdof", "vtxProb", "svX", "svY", "svZ",
                              "svXErr", "svYErr", "svZErr", "fitDimuonMass"})
          F(n).push_back(kNaN);
        for (const auto& d : daughterNames)
          for (const char* v : {"FitPt", "FitEta", "FitPhi"}) F(d + v).push_back(kNaN);
      }
    }
  }

  void NtupleWriter::fillGen(const GenDecayChain& gen) {
    const GenDecayChain::GenInfo& g = gen.info();
    b_.scalar<int>("gen_valid") = g.isValid;
    if (!g.isValid) return;
    b_.scalar<int>("gen_decayMode") = g.decayMode;
    b_.scalar<int>("gen_B_pdgId") = g.motherPdgId;
    b_.scalar<int>("gen_isSignal") = gen.isSignalLike();
    b_.scalar<int>("gen_is3Body") = g.is3BodyDecay;
    b_.scalar<int>("gen_is4Body") = g.is4BodyDecay;
    b_.scalar<int>("gen_hasJpsi") = g.hasJpsi;
    b_.scalar<float>("gen_B_pt") = g.Bp4.Pt();
    b_.scalar<float>("gen_B_eta") = g.Bp4.Pt() > 0 ? g.Bp4.Eta() : kNaN;
    b_.scalar<float>("gen_B_phi") = g.Bp4.Phi();
    b_.scalar<float>("gen_B_mass") = g.Bp4.M();
    b_.scalar<float>("gen_B_ct2D") = g.Bct2D;
    b_.scalar<float>("gen_B_ct3D") = g.Bct3D;
    b_.scalar<float>("gen_B_lxy") = g.BLxy;
    b_.scalar<float>("gen_pvX") = g.Bvtx.X();
    b_.scalar<float>("gen_pvY") = g.Bvtx.Y();
    b_.scalar<float>("gen_pvZ") = g.Bvtx.Z();
    b_.scalar<float>("gen_svX") = g.Bdecayvtx.X();
    b_.scalar<float>("gen_svY") = g.Bdecayvtx.Y();
    b_.scalar<float>("gen_svZ") = g.Bdecayvtx.Z();
    if (g.hasMuonPlus) {
      b_.scalar<float>("gen_muPlus_pt") = g.muPlusP4.Pt();
      b_.scalar<float>("gen_muPlus_eta") = g.muPlusP4.Eta();
      b_.scalar<float>("gen_muPlus_phi") = g.muPlusP4.Phi();
    }
    if (g.hasMuonMinus) {
      b_.scalar<float>("gen_muMinus_pt") = g.muMinusP4.Pt();
      b_.scalar<float>("gen_muMinus_eta") = g.muMinusP4.Eta();
      b_.scalar<float>("gen_muMinus_phi") = g.muMinusP4.Phi();
    }
    b_.scalar<float>("gen_dimuonMass") = g.dimuonMass;
    b_.scalar<float>("gen_helicity") = g.helicity3;
    b_.scalar<float>("gen_coplanarity") = g.coplanarity3;
    b_.scalar<float>("gen_cosThetaL") = g.cosTheta_l;
    b_.scalar<float>("gen_triBodyMass") = g.triBodyMass;
    b_.scalar<int>("gen_neutralMesonPdgId") = g.neutralMesonId;
    b_.scalar<int>("gen_vectorMesonPdgId") = g.vectorMesonId;
    b_.scalar<float>("gen_diphotonMass") = g.diphotonMass;
    b_.scalar<float>("gen_fourBodyMass") = g.fourBodyMass;
    b_.scalar<float>("gen_helicity4") = g.helicity4;
    b_.scalar<float>("gen_coplanarity4") = g.coplanarity4;
    for (const auto& p : g.photons) {
      b_.vec<int>("gen_pho_origin").push_back(p.origin);
      b_.vec<int>("gen_pho_motherPdgId").push_back(p.motherPdgId);
      b_.vec<float>("gen_pho_pt").push_back(p.p4.Pt());
      b_.vec<float>("gen_pho_eta").push_back(p.p4.Pt() > 0 ? p.p4.Eta() : kNaN);
      b_.vec<float>("gen_pho_phi").push_back(p.p4.Phi());
    }
  }

  void NtupleWriter::fill() { events_->Fill(); }

  void NtupleWriter::fillRunInfo(const RunInfo& info) {
    r_.reset();
    r_.scalar<double>("xsec") = info.xsec;
    r_.scalar<double>("xsecErr") = info.xsecErr;
    r_.scalar<double>("filterEff") = info.filterEff;
    r_.scalar<double>("filterEffErr") = info.filterEffErr;
    r_.scalar<double>("lumiEquivalent") = info.lumiEquivalent;
    r_.scalar<double>("lumiEquivalentErr") = info.lumiEquivalentErr;
    r_.scalar<unsigned long long>("nEvents") = info.nEvents;
    runInfo_->Fill();
  }

}  // namespace bmmg
