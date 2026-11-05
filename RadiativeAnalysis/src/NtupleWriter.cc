#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/NtupleWriter.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicVertex.h"
#include "TMath.h"

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

  NtupleWriter::NtupleWriter(TFileService& fs, const std::vector<std::string>& triggerPaths, bool isMC)
      : events_(fs.make<TTree>("Events", "B -> mu mu gamma ntuple")),
        runInfo_(fs.make<TTree>("RunInfo", "job-level information")),
        b_(events_),
        r_(runInfo_),
        triggerPaths_(triggerPaths),
        isMC_(isMC) {
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

    // ---- mu mu gamma candidates ----
    bookInts(b_, "mmg_", {"dimu", "mu1", "mu2", "pho", "pv", "fitOK"});
    bookFloats(b_, "mmg_", {"prefitMass", "prefitPt", "prefitEta", "prefitPhi",
                            "mass", "massErr", "pt", "eta", "phi",
                            "vtxProb", "vtxChi2", "vtxNdof", "svX", "svY", "svZ", "svXErr", "svYErr", "svZErr",
                            "pvX", "pvY", "pvZ",
                            "l3d", "l3dErr", "lxy", "lxyErr", "cosAlpha3D", "cosAlpha2D",
                            "ct3D", "ct3DErr", "ct2D", "ct2DErr",
                            "mu1FitPt", "mu1FitEta", "mu1FitPhi", "mu2FitPt", "mu2FitEta", "mu2FitPhi",
                            "phoFitPt", "phoFitEta", "phoFitPhi",
                            "helicity", "coplanarity", "cosThetaL", "isolation", "maxTrackCompatibility"});
    if (isMC_) bookInts(b_, "mmg_", {"genMatched"});

    // ---- generator ----
    if (isMC_) {
      b_.bookScalar<int>("gen_valid", 0);
      b_.bookScalar<int>("gen_decayMode", -1);
      b_.bookScalar<int>("gen_B_pdgId", 0);
      b_.bookScalar<int>("gen_isSignal", 0);
      b_.bookScalar<int>("gen_is3Body", 0);
      b_.bookScalar<int>("gen_is4Body", 0);
      b_.bookScalar<int>("gen_hasJpsi", 0);
      for (const char* n : {"gen_B_pt", "gen_B_eta", "gen_B_phi", "gen_B_mass", "gen_B_ct2D", "gen_B_ct3D", "gen_B_lxy",
                            "gen_pvX", "gen_pvY", "gen_pvZ", "gen_svX", "gen_svY", "gen_svZ",
                            "gen_muPlus_pt", "gen_muPlus_eta", "gen_muPlus_phi",
                            "gen_muMinus_pt", "gen_muMinus_eta", "gen_muMinus_phi",
                            "gen_dimuonMass", "gen_helicity", "gen_coplanarity", "gen_cosThetaL", "gen_triBodyMass"})
        b_.bookScalar<float>(n, kNaN);
      bookInts(b_, "gen_pho_", {"origin", "motherPdgId"});
      bookFloats(b_, "gen_pho_", {"pt", "eta", "phi"});
    }

    // ---- run info ----
    for (const char* n : {"xsec", "xsecErr", "filterEff", "filterEffErr", "lumiEquivalent", "lumiEquivalentErr"})
      r_.bookScalar<double>(n, -1.);
    r_.bookScalar<unsigned long long>("nEvents", 0);
  }

  // =====================================================================================
  // filling
  // =====================================================================================
  void NtupleWriter::beginEvent(const edm::EventID& id, unsigned int lumi) {
    b_.reset();
    muonFromB_.clear();
    photonFromB_.clear();
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

  void NtupleWriter::fillMuMuGamma(const std::vector<MMGCandidate>& candidates, const std::vector<reco::Vertex>& pvs) {
    auto F = [this](const char* n) -> std::vector<float>& { return b_.vec<float>(std::string("mmg_") + n); };
    for (const auto& c : candidates) {
      b_.vec<int>("mmg_dimu").push_back(c.dimuon);
      b_.vec<int>("mmg_mu1").push_back(c.mu1);
      b_.vec<int>("mmg_mu2").push_back(c.mu2);
      b_.vec<int>("mmg_pho").push_back(c.photon);
      b_.vec<int>("mmg_pv").push_back(c.pv);
      b_.vec<int>("mmg_fitOK").push_back(c.fit.has_value());
      if (isMC_) {
        const bool matched = c.mu1 < (int)muonFromB_.size() && c.mu2 < (int)muonFromB_.size() &&
                             c.photon < (int)photonFromB_.size() && muonFromB_[c.mu1] && muonFromB_[c.mu2] &&
                             photonFromB_[c.photon];
        b_.vec<int>("mmg_genMatched").push_back(matched);
      }
      F("prefitMass").push_back(c.prefitP4.mass());
      F("prefitPt").push_back(c.prefitP4.pt());
      F("prefitEta").push_back(c.prefitP4.eta());
      F("prefitPhi").push_back(c.prefitP4.phi());
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

      if (c.fit) {
        const KinematicState& s = c.fit->particle->currentState();
        const RefCountedKinematicVertex& v = c.fit->vertex;
        const GlobalVector p = s.globalMomentum();
        F("mass").push_back(s.mass());
        F("pt").push_back(p.perp());
        F("eta").push_back(p.eta());
        F("phi").push_back(p.phi());
        F("vtxChi2").push_back(v->chiSquared());
        F("vtxNdof").push_back(v->degreesOfFreedom());
        F("vtxProb").push_back(TMath::Prob(v->chiSquared(), static_cast<int>(std::lround(v->degreesOfFreedom()))));
        F("svX").push_back(v->position().x());
        F("svY").push_back(v->position().y());
        F("svZ").push_back(v->position().z());
        F("svXErr").push_back(std::sqrt(v->error().cxx()));
        F("svYErr").push_back(std::sqrt(v->error().cyy()));
        F("svZErr").push_back(std::sqrt(v->error().czz()));
        const char* names[3][3] = {{"mu1FitPt", "mu1FitEta", "mu1FitPhi"},
                                   {"mu2FitPt", "mu2FitEta", "mu2FitPhi"},
                                   {"phoFitPt", "phoFitEta", "phoFitPhi"}};
        for (int k = 0; k < 3; ++k) {
          const GlobalVector dp = c.fit->daughters.at(k)->currentState().globalMomentum();
          F(names[k][0]).push_back(dp.perp());
          F(names[k][1]).push_back(dp.eta());
          F(names[k][2]).push_back(dp.phi());
        }
      } else {
        for (const char* n : {"mass", "pt", "eta", "phi", "vtxChi2", "vtxNdof", "vtxProb", "svX", "svY", "svZ",
                              "svXErr", "svYErr", "svZErr", "mu1FitPt", "mu1FitEta", "mu1FitPhi", "mu2FitPt",
                              "mu2FitEta", "mu2FitPhi", "phoFitPt", "phoFitEta", "phoFitPhi"})
          F(n).push_back(kNaN);
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
