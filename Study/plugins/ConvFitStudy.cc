// Scratch: why does the e+e- zero-mass refit fail? One TTree entry per conversion.
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/ColinearityKinematicConstraint.h"
#include "RecoVertex/VertexPrimitives/interface/VertexException.h"
#include "RecoVertex/KinematicFit/interface/InputSort.h"
#include "RecoVertex/LinearizationPointFinders/interface/DefaultLinearizationPointFinder.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexUpdator.h"
#include "RecoVertex/VertexTools/interface/VertexTrackFactory.h"
#include "RecoVertex/VertexPrimitives/interface/CachingVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/TrackKinematicStatePropagator.h"
#include "TTree.h"
#include "TMath.h"
#include <cmath>

namespace {
  constexpr double me = 0.00051099895;
  struct Out { int status = -9; float mass = -1, massErr = -1, prob = -1, rho = -1, pt = -1; };
  // status: 0 ok, 1 vertex fit invalid, 2 vertex fit exception, 3 constraint invalid, 4 constraint exception, 5 particle build exception
  float probOf(const RefCountedKinematicParticle& p) {
    const float n = p->degreesOfFreedom();
    return n > 0 ? TMath::Prob(p->chiSquared(), std::lround(n)) : -1.f;
  }
  void fillTop(Out& o, RefCountedKinematicTree& tree) {
    tree->movePointerToTheTop();
    const auto p = tree->currentParticle();
    const auto& s = p->currentState();
    o.mass = s.mass();
    o.massErr = std::sqrt(std::max(0., double(s.kinematicParametersError().matrix()(6, 6))));
    o.prob = probOf(p);
    o.rho = tree->currentDecayVertex()->position().perp();
    o.pt = s.globalMomentum().perp();
  }
}

class ConvFitStudy : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit ConvFitStudy(const edm::ParameterSet& cfg)
      : miniAOD_(cfg.getParameter<bool>("miniAOD")),
        ttToken_(esConsumes(edm::ESInputTag("", "TransientTrackBuilder"))) {
    if (miniAOD_) onia_ = consumes(cfg.getParameter<edm::InputTag>("conversions"));
    else conv_ = consumes(cfg.getParameter<edm::InputTag>("conversions"));
    gen_ = consumes(cfg.getParameter<edm::InputTag>("genParticles"));
    usesResource("TFileService");
    edm::Service<TFileService> fs;
    t_ = fs->make<TTree>("t", "conversions");
    t_->Branch("fromB", &fromB_);
    t_->Branch("genPt", &genPt_);
    t_->Branch("recoRho", &recoRho_);
    t_->Branch("recoProb", &recoProb_);
    t_->Branch("pairPt", &pairPt_);
    t_->Branch("dCotTheta", &dCot_);
    t_->Branch("dPhi", &dPhi_);
    t_->Branch("vtxOnlyMass", &vtxMass_);
    t_->Branch("dg_linRho", &dgLinRho_);
    t_->Branch("dg_linDist", &dgLinDist_);
    t_->Branch("dg_willProp", &dgWillProp_);
    t_->Branch("dg_reason", &dgReason_);
    t_->Branch("dg_steps", &dgSteps_);
    t_->Branch("dg_lastShift", &dgLastShift_);
    t_->Branch("dg_lastRho", &dgLastRho_);
    t_->Branch("dg_minShift", &dgMinShift_);
    t_->Branch("vtxOnlyMassErr", &vtxMassErr_);
    for (int i = 0; i < kN; ++i) {
      const std::string n = names_[i];
      t_->Branch((n + "_status").c_str(), &out_[i].status);
      t_->Branch((n + "_mass").c_str(), &out_[i].mass);
      t_->Branch((n + "_massErr").c_str(), &out_[i].massErr);
      t_->Branch((n + "_prob").c_str(), &out_[i].prob);
      t_->Branch((n + "_rho").c_str(), &out_[i].rho);
      t_->Branch((n + "_pt").c_str(), &out_[i].pt);
    }
  }

  void analyze(const edm::Event& ev, const edm::EventSetup& es) override {
    const auto& ttb = es.getData(ttToken_);
    std::vector<const reco::GenParticle*> gph;
    if (auto g = ev.getHandle(gen_); g.isValid())
      for (const auto& p : *g)
        if (p.pdgId() == 22 && p.numberOfMothers() && (std::abs(p.mother(0)->pdgId()) == 511 || std::abs(p.mother(0)->pdgId()) == 531))
          gph.push_back(&p);
    std::vector<std::tuple<const reco::Track*, const reco::Track*, GlobalPoint, float, reco::Candidate::LorentzVector>> in;
    if (miniAOD_) {
      auto h = ev.getHandle(onia_);
      if (!h.isValid()) return;
      for (const auto& c : *h)
        in.emplace_back(c.userData<reco::Track>("track0"), c.userData<reco::Track>("track1"),
                        GlobalPoint(c.vx(), c.vy(), c.vz()), -1.f, c.p4());
    } else {
      for (const auto& c : ev.get(conv_)) {
        if (c.nTracks() != 2 || !c.conversionVertex().isValid()) continue;
        const auto& v = c.conversionVertex();
        const auto& p = c.pairMomentum();
        reco::Candidate::LorentzVector p4(p.x(), p.y(), p.z(), std::sqrt(p.mag2()));
        in.emplace_back(c.tracks()[0].get(), c.tracks()[1].get(), GlobalPoint(v.x(), v.y(), v.z()),
                        TMath::Prob(v.chi2(), std::lround(v.ndof())), p4);
      }
    }
    for (auto& [t0, t1, vtx, prob, p4] : in) {
      if (!t0 || !t1 || t0->charge() * t1->charge() >= 0 || p4.pt() < 0.5) continue;
      fromB_ = 0; genPt_ = -1;
      for (auto* g : gph)
        if (reco::deltaR(*g, p4) < 0.1 && std::abs(p4.pt() - g->pt()) < 0.5 * g->pt()) { fromB_ = 1; genPt_ = g->pt(); }
      recoRho_ = vtx.perp();
      recoProb_ = prob;
      pairPt_ = p4.pt();
      dCot_ = 1. / std::tan(t0->theta()) - 1. / std::tan(t1->theta());
      dPhi_ = reco::deltaPhi(t0->phi(), t1->phi());
      const reco::TransientTrack tt0 = ttb.build(*t0), tt1 = ttb.build(*t1);
      vtxMass_ = vtxMassErr_ = -1;
      for (int i = 0; i < kN; ++i) out_[i] = Out();
      for (int i = 0; i < kN; ++i) run(i, tt0, tt1, vtx);
      t_->Fill();
    }
  }

private:
  enum { kCur, kCurAtConv, kCurSig1e3, kTTMass, kTTMassAtConv, kColin, kColinAtConv, kVtxOnly, kColinMass0, kSergey, kN };
  const char* names_[kN] = {"cur", "curAtConv", "curSig1e3", "ttMass", "ttMassAtConv", "colin", "colinAtConv", "vtxOnly", "colinMass0", "sergey"};

  void run(int i, const reco::TransientTrack& a, const reco::TransientTrack& b, const GlobalPoint& vtx) {
    Out& o = out_[i];
    const bool atConv = (i == kCurAtConv || i == kTTMassAtConv || i == kColinAtConv);
    KinematicParticleFactoryFromTransientTrack f;
    float chi = 0, ndf = 0, sigma = 1e-6;
    std::vector<RefCountedKinematicParticle> parts;
    try {
      for (const auto* t : {&a, &b}) {
        if (atConv) {
          const FreeTrajectoryState fts = t->trajectoryStateClosestToPoint(vtx).theState();
          parts.push_back(f.particle(*t, me, chi, ndf, sigma, fts));
        } else
          parts.push_back(f.particle(*t, me, chi, ndf, sigma));
      }
    } catch (const std::exception&) { o.status = 5; return; }
    if (i == kColinMass0) {
      try {
        KinematicConstrainedVertexFitter kcv;
        ColinearityKinematicConstraint c(ColinearityKinematicConstraint::PhiTheta);
        RefCountedKinematicTree tree = kcv.fit(parts, &c);
        if (!tree->isValid()) { o.status = 1; return; }
        tree->movePointerToTheTop();
        MassKinematicConstraint zero(ParticleMass(0.), float(1e-4));
        KinematicParticleFitter cf;
        tree = cf.fit(&zero, tree);
        if (!tree->isValid()) { o.status = 3; return; }
        fillTop(o, tree);
        o.status = 0;
      } catch (const std::exception&) { o.status = 4; }
      return;
    }
    if (i == kTTMass || i == kTTMassAtConv || i == kColin || i == kColinAtConv) {
      try {
        KinematicConstrainedVertexFitter kcv;
        RefCountedKinematicTree tree;
        if (i == kColin || i == kColinAtConv) {
          ColinearityKinematicConstraint c(ColinearityKinematicConstraint::PhiTheta);
          tree = kcv.fit(parts, &c);
        } else {
          ParticleMass zm = 0.;
          TwoTrackMassKinematicConstraint c(zm);
          tree = kcv.fit(parts, &c);
        }
        if (!tree->isValid()) { o.status = 1; return; }
        fillTop(o, tree);
        o.status = 0;
      } catch (const std::exception&) { o.status = 2; }
      return;
    }
    RefCountedKinematicTree tree;
    try {
      KinematicParticleVertexFitter vf;
      tree = vf.fit(parts);
      if (!tree->isValid()) { o.status = 1; return; }
    } catch (const std::exception&) { o.status = 2; return; }
    if (i == kCur) {
      tree->movePointerToTheTop();
      const auto& s = tree->currentParticle()->currentState();
      vtxMass_ = s.mass();
      vtxMassErr_ = std::sqrt(std::max(0., double(s.kinematicParametersError().matrix()(6, 6))));
    }
    if (i == kVtxOnly) { fillTop(o, tree); o.status = 0; return; }
    try {
      MassKinematicConstraint zero(ParticleMass(0.), float(i == kCurSig1e3 ? 1e-3 : 1e-4));
      KinematicParticleFitter cf;
      tree = cf.fit(&zero, tree);
      if (!tree->isValid()) { o.status = 3; return; }
      fillTop(o, tree);
      o.status = 0;
    } catch (const std::exception&) { o.status = 4; }
    if (i == kSergey) {
      try {
        const ParticleMass zero_mass(0);
        float zero_sigma = 1E-6;
        KinematicParticleVertexFitter photonfitter;
        RefCountedKinematicTree photonVertexFitTree = photonfitter.fit(parts);
        if (!photonVertexFitTree->isValid())
        {
          edm::ParameterSet pSet;
          pSet.addParameter<double>("maxDistance", 3);
          pSet.addParameter<int>("maxNbrOfIterations", 10000);
          KinematicParticleVertexFitter photonfitter2(pSet);
          photonVertexFitTree = photonfitter2.fit(parts);
        }
        if (!photonVertexFitTree->isValid()) {o.status = 1; return;}
        KinematicParticleFitter csFitterPhoton;
        KinematicConstraint * pho_c = new MassKinematicConstraint(zero_mass, zero_sigma);
        // add mass constraint to the photon fit to do a constrained fit:
        photonVertexFitTree->movePointerToTheTop();
        photonVertexFitTree = csFitterPhoton.fit(pho_c, photonVertexFitTree);
        if (!photonVertexFitTree->isValid()) {o.status = 3; return;}
        fillTop(o, photonVertexFitTree);
        o.status = 0;
      } catch (const std::exception&) { o.status = 4; }
    }
  }

  // replica of KinematicParticleVertexFitter + SequentialVertexFitter<6>::fit with diagnostics
  // reason: 0 converged, 1 propagation check failed, 2 Kalman update invalid (last step), 3 NaN, 4 out of bounds, 5 max steps
  void diagnose(const reco::TransientTrack& a, const reco::TransientTrack& b, const GlobalPoint& recoVtx) {
    dgLinRho_ = dgLinDist_ = dgLastShift_ = dgLastRho_ = dgMinShift_ = -1; dgWillProp_ = -1; dgReason_ = -1; dgSteps_ = 0;
    KinematicParticleFactoryFromTransientTrack f;
    float chi = 0, ndf = 0, sigma = 1e-6;
    std::vector<RefCountedKinematicParticle> parts{f.particle(a, me, chi, ndf, sigma), f.particle(b, me, chi, ndf, sigma)};
    InputSort iSort;
    auto input = iSort.sort(parts);
    DefaultLinearizationPointFinder lpf;
    const GlobalPoint linP = lpf.getLinearizationPoint(input.second);
    dgLinRho_ = linP.perp();
    dgLinDist_ = (linP - recoVtx).mag();
    AlgebraicSymMatrix33 we; we(0, 0) = we(1, 1) = we(2, 2) = 10000.;
    VertexState state(linP, GlobalError(we));
    TrackKinematicStatePropagator prop;
    VertexTrackFactory<6> vf;
    std::vector<RefCountedVertexTrack6> tracks;
    dgWillProp_ = 1;
    for (auto& p : input.first) {
      if (!p->currentState().isValid() || !prop.willPropagateToTheTransversePCA(p->currentState(), linP)) { dgWillProp_ = 0; dgReason_ = 1; return; }
      tracks.push_back(vf.vertexTrack(p->particleLinearizedTrackState(linP), state, 1.));
    }
    KalmanVertexUpdator<6> upd;
    const GlobalPoint initPos = tracks[0]->linearizedTrack()->linearizationPoint();
    VertexState initState(initPos, GlobalError(we));
    CachingVertex<6> initial(initPos, GlobalError(we), std::vector<RefCountedVertexTrack6>(), 0);
    CachingVertex<6> ret = initial;
    GlobalPoint newPos = initPos, prevPos;
    bool valid = true; int step = 0; int reason = 0; float minShift = 1e9;
    std::vector<RefCountedVertexTrack6> gl = tracks;
    do {
      CachingVertex<6> fv = initial;
      if (step != 0) {
        gl.clear();
        for (auto& t : tracks)
          gl.push_back(vf.vertexTrack(t->linearizedTrack()->stateWithNewLinearizationPoint(ret.vertexState().position()), ret.vertexState(), t->weight()));
      }
      for (auto& t : gl) { fv = upd.add(fv, t); if (!fv.isValid()) break; }
      valid = fv.isValid(); reason = valid ? 0 : 2;
      if (valid && (std::isnan(fv.position().x()) || std::isnan(fv.position().y()) || std::isnan(fv.position().z()))) { valid = false; reason = 3; }
      if (valid && !(fv.position().perp() < 112. && std::abs(fv.position().z()) < 273.5)) { valid = false; reason = 4; }
      if (!valid) fv = CachingVertex<6>(GlobalPoint(0, 0, 0), GlobalError(we), std::vector<RefCountedVertexTrack6>(), 0);
      prevPos = newPos; newPos = fv.position(); ret = fv; ++step;
      dgLastShift_ = (prevPos - newPos).transverse();
      if (step > 1) minShift = std::min(minShift, dgLastShift_);
    } while (step != 100 && (dgLastShift_ > 0.01 || !valid));
    dgSteps_ = step; dgLastRho_ = newPos.perp(); dgMinShift_ = minShift;
    dgReason_ = !valid ? reason : (step >= 100 ? 5 : 0);
  }
  typedef ReferenceCountingPointer<VertexTrack<6>> RefCountedVertexTrack6;
  float dgLinRho_, dgLinDist_, dgLastShift_, dgLastRho_, dgMinShift_;
  int dgWillProp_, dgReason_, dgSteps_;

  const bool miniAOD_;
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> ttToken_;
  edm::EDGetTokenT<std::vector<reco::Conversion>> conv_;
  edm::EDGetTokenT<std::vector<pat::CompositeCandidate>> onia_;
  edm::EDGetTokenT<std::vector<reco::GenParticle>> gen_;
  TTree* t_;
  int fromB_;
  float genPt_;
  float recoRho_, recoProb_, pairPt_, dCot_, dPhi_, vtxMass_, vtxMassErr_;
  Out out_[kN];
};
DEFINE_FWK_MODULE(ConvFitStudy);
