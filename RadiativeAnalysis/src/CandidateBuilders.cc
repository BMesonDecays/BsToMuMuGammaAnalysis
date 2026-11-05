#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateBuilders.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicVertex.h"

#include <cmath>
#include <map>
#include <utility>

namespace bmmg {

  namespace {
    TLorentzVector fittedP4(const RefCountedKinematicParticle& p) {
      const KinematicState& s = p->currentState();
      const GlobalVector m = s.globalMomentum();
      return TLorentzVector(m.x(), m.y(), m.z(), s.kinematicParameters().energy());
    }

    // Common part of all channels: PV choice, fit (by the channel-specific "fit" function),
    // observables. Returns false if the candidate has no PV.
    class Finisher {
    public:
      Finisher(const std::vector<SelectedMuon>& muons, const std::vector<reco::Vertex>& pvs,
               const TransientTrackBuilder& ttBuilder, const CandidateConfig& config)
          : muons_(muons), pvs_(pvs), ttBuilder_(ttBuilder), config_(config) {}

      // choose the PV for candidate c (prefitP4 must be set)
      bool choosePV(BCandidate& c, const DimuonCandidate& dimu) const {
        c.pv = lowestDCAVertex(pvs_, dimu.vtx, GlobalVector(c.prefitP4.px(), c.prefitP4.py(), c.prefitP4.pz()));
        return c.pv >= 0;
      }

      GlobalPoint pvPoint(const BCandidate& c) const {
        const reco::Vertex& pv = pvs_[c.pv];
        return GlobalPoint(pv.x(), pv.y(), pv.z());
      }

      // observables after the fit attempt; exclude: extra tracks of the candidate (kaons)
      void observables(BCandidate& c, const std::vector<const reco::Track*>& exclude) {
        const reco::Vertex& pv = pvs_[c.pv];
        const SelectedMuon& mu1 = muons_[c.mu1];
        const SelectedMuon& mu2 = muons_[c.mu2];
        TLorentzVector bP4, m1, m2, x;
        if (c.fit) {
          const KinematicState& s = c.fit->particle->currentState();
          const double massVar = s.kinematicParametersError().matrix()(6, 6);
          c.massErr = massVar > 0 ? std::sqrt(massVar) : -1.f;
          // the state of the top particle is at the decay vertex; its 7x7 covariance (x,y,z,px,py,pz,m)
          // gives the SV position, the momentum and their correlations
          const AlgebraicSymMatrix66 cov6 = s.kinematicParametersError().matrix().Sub<AlgebraicSymMatrix66>(0, 0);
          c.flight = flightInfo(pv, s.globalPosition(), s.globalMomentum(), cov6, config_.massHypothesis);
          bP4 = fittedP4(c.fit->particle);
          m1 = fittedP4(c.fit->daughters.at(0));
          m2 = fittedP4(c.fit->daughters.at(1));
          for (size_t k = 2; k < c.fit->daughters.size(); ++k) x += fittedP4(c.fit->daughters[k]);
        } else {
          bP4 = toTLV(c.prefitP4);
          m1 = toTLV(mu1.muon->p4());
          m2 = toTLV(mu2.muon->p4());
          x = toTLV(c.xP4);
        }
        c.helicity = helicityAngle(x, bP4);
        c.coplanarity = coplanarityAngle(m1, m2, x, bP4);
        c.cosThetaL = leptonAngle(m1, m2);   // m1 = mu+, m2 = mu-
        c.isolation = candidateIsolation(reco::Candidate::LorentzVector(bP4.Px(), bP4.Py(), bP4.Pz(), bP4.E()),
                                         *mu1.muon, *mu2.muon, pv, exclude);
        if (config_.computeTrackCompatibility) {
          if (exclude.empty()) {   // depends only on the dimuon and the PV: cached
            const auto key = std::make_pair(c.dimuon, c.pv);
            auto it = compatibilityCache_.find(key);
            if (it == compatibilityCache_.end())
              it = compatibilityCache_.emplace(key, maxTrackCompatibility(mu1.track, mu2.track, pv, ttBuilder_)).first;
            c.maxTrackCompatibility = it->second;
          } else {
            c.maxTrackCompatibility = maxTrackCompatibility(mu1.track, mu2.track, pv, ttBuilder_, exclude);
          }
        }
      }

      bool inWindow(const BCandidate& c) const {
        const double m = c.prefitP4.mass();
        return m >= config_.minPrefitMass && m <= config_.maxPrefitMass;
      }

    private:
      const std::vector<SelectedMuon>& muons_;
      const std::vector<reco::Vertex>& pvs_;
      const TransientTrackBuilder& ttBuilder_;
      const CandidateConfig& config_;
      std::map<std::pair<int, int>, float> compatibilityCache_;   // (dimuon, pv) -> max track compatibility
    };

    BCandidate startCandidate(int idimu, const DimuonCandidate& dimu) {
      BCandidate c;
      c.dimuon = idimu;
      c.mu1 = dimu.mu1;
      c.mu2 = dimu.mu2;
      return c;
    }

    bool muonParticles(const std::vector<SelectedMuon>& muons, const DimuonCandidate& d) {
      return muons[d.mu1].kinParticle && muons[d.mu2].kinParticle;
    }
  }  // namespace

  // ---------------------------------------------------------------------------- mu mu gamma
  std::vector<BCandidate> buildMuMuGamma(const std::vector<SelectedMuon>& muons,
                                         const std::vector<DimuonCandidate>& dimuons,
                                         const std::vector<SelectedPhoton>& photons,
                                         const std::vector<reco::Vertex>& pvs,
                                         const EcalClusterLazyTools& lazyTools,
                                         const TransientTrackBuilder& ttBuilder,
                                         const MMGVertexFit& fitter,
                                         const CandidateConfig& config) {
    std::vector<BCandidate> candidates;
    Finisher finisher(muons, pvs, ttBuilder, config);
    for (size_t id = 0; id < dimuons.size(); ++id) {
      const DimuonCandidate& dimu = dimuons[id];
      if (!dimu.vtxValid) continue;
      for (size_t ip = 0; ip < photons.size(); ++ip) {
        const reco::Photon& pho = *photons[ip].photon;
        BCandidate c = startCandidate(id, dimu);
        c.legs = {static_cast<int>(ip)};
        c.xP4 = pho.p4();
        c.prefitP4 = dimu.p4 + c.xP4;
        if (!finisher.inWindow(c) || !finisher.choosePV(c, dimu)) continue;
        if (muonParticles(muons, dimu))
          c.fit = fitter.fitCandidate(*muons[dimu.mu1].kinParticle, *muons[dimu.mu2].kinParticle, pho, lazyTools,
                                      &ttBuilder, finisher.pvPoint(c));
        finisher.observables(c, {});
        candidates.push_back(std::move(c));
      }
    }
    return candidates;
  }

  // ---------------------------------------------------------------------------- mu mu gamma(conversion)
  std::vector<BCandidate> buildMuMuConvGamma(const std::vector<SelectedMuon>& muons,
                                             const std::vector<DimuonCandidate>& dimuons,
                                             const std::vector<SelectedConversion>& conversions,
                                             const std::vector<reco::Vertex>& pvs,
                                             const TransientTrackBuilder& ttBuilder,
                                             const CandidateConfig& config) {
    std::vector<BCandidate> candidates;
    Finisher finisher(muons, pvs, ttBuilder, config);
    for (size_t id = 0; id < dimuons.size(); ++id) {
      const DimuonCandidate& dimu = dimuons[id];
      if (!dimu.vtxValid) continue;
      for (size_t ic = 0; ic < conversions.size(); ++ic) {
        const SelectedConversion& conv = conversions[ic];
        BCandidate c = startCandidate(id, dimu);
        c.legs = {static_cast<int>(ic)};
        c.xP4 = conv.p4;
        c.prefitP4 = dimu.p4 + c.xP4;
        if (!finisher.inWindow(c) || !finisher.choosePV(c, dimu)) continue;
        if (muonParticles(muons, dimu) && conv.photon)
          c.fit = MMGVertexFit::fitParticles(
              {*muons[dimu.mu1].kinParticle, *muons[dimu.mu2].kinParticle, *conv.photon}, finisher.pvPoint(c));
        finisher.observables(c, {&conv.track1.track(), &conv.track2.track()});
        candidates.push_back(std::move(c));
      }
    }
    return candidates;
  }

  // ---------------------------------------------------------------------------- mu mu gamma gamma
  std::vector<BCandidate> buildMuMuGammaGamma(const std::vector<SelectedMuon>& muons,
                                              const std::vector<DimuonCandidate>& dimuons,
                                              const std::vector<SelectedPhoton>& photons,
                                              const std::vector<reco::Vertex>& pvs,
                                              const EcalClusterLazyTools& lazyTools,
                                              const TransientTrackBuilder& ttBuilder,
                                              const MMGVertexFit& fitter,
                                              const CandidateConfig& config,
                                              const DiphotonConfig& diphoton) {
    std::vector<BCandidate> candidates;
    Finisher finisher(muons, pvs, ttBuilder, config);
    for (size_t id = 0; id < dimuons.size(); ++id) {
      const DimuonCandidate& dimu = dimuons[id];
      if (!dimu.vtxValid) continue;
      for (size_t i1 = 0; i1 < photons.size(); ++i1) {
        for (size_t i2 = i1 + 1; i2 < photons.size(); ++i2) {
          const reco::Photon& p1 = *photons[i1].photon;
          const reco::Photon& p2 = *photons[i2].photon;
          BCandidate c = startCandidate(id, dimu);
          c.legs = {static_cast<int>(i1), static_cast<int>(i2)};
          c.xP4 = p1.p4() + p2.p4();
          if (c.xP4.mass() < diphoton.minMass || c.xP4.mass() > diphoton.maxMass) continue;
          c.prefitP4 = dimu.p4 + c.xP4;
          if (!finisher.inWindow(c) || !finisher.choosePV(c, dimu)) continue;
          if (muonParticles(muons, dimu))
            c.fit = fitter.fitCandidate(*muons[dimu.mu1].kinParticle, *muons[dimu.mu2].kinParticle, {&p1, &p2},
                                        lazyTools, &ttBuilder, finisher.pvPoint(c));
          finisher.observables(c, {});
          candidates.push_back(std::move(c));
        }
      }
    }
    return candidates;
  }

  // ---------------------------------------------------------------------------- mu mu K K
  std::vector<BCandidate> buildMuMuKK(const std::vector<SelectedMuon>& muons,
                                      const std::vector<DimuonCandidate>& dimuons,
                                      const std::vector<SelectedTrack>& tracks,
                                      const std::vector<reco::Vertex>& pvs,
                                      const TransientTrackBuilder& ttBuilder,
                                      const CandidateConfig& config,
                                      const KaonConfig& kaons,
                                      std::vector<SelectedTrack>& usedTracks) {
    std::vector<BCandidate> candidates;
    usedTracks.clear();
    std::map<int, int> usedIndex;   // index in "tracks" -> index in usedTracks
    auto useTrack = [&](int i) {
      auto it = usedIndex.find(i);
      if (it != usedIndex.end()) return it->second;
      usedTracks.push_back(tracks[i]);
      return usedIndex[i] = static_cast<int>(usedTracks.size()) - 1;
    };

    Finisher finisher(muons, pvs, ttBuilder, config);
    const double mK = pdg::kaonMass;
    for (size_t id = 0; id < dimuons.size(); ++id) {
      const DimuonCandidate& dimu = dimuons[id];
      if (!dimu.vtxValid) continue;
      if (kaons.jpsiOnly && dimu.resonance != kJpsi) continue;
      const reco::Muon& mu1 = *muons[dimu.mu1].muon;
      const reco::Muon& mu2 = *muons[dimu.mu2].muon;

      // kaon candidates near this dimuon, excluding the muon tracks
      std::vector<int> near;
      for (size_t it = 0; it < tracks.size(); ++it) {
        const reco::Track& trk = *tracks[it].track;
        if (reco::deltaR(trk, dimu.p4) > kaons.maxDeltaR) continue;
        if (std::abs(trk.dz(math::XYZPoint(dimu.vtx.x(), dimu.vtx.y(), dimu.vtx.z()))) > kaons.maxDz) continue;
        if (reco::deltaR(trk, mu1) < 0.01 || reco::deltaR(trk, mu2) < 0.01) continue;
        near.push_back(it);
      }
      for (int iPlus : near) {
        const reco::Track& kPlus = *tracks[iPlus].track;
        if (kPlus.charge() <= 0) continue;
        for (int iMinus : near) {
          const reco::Track& kMinus = *tracks[iMinus].track;
          if (kMinus.charge() >= 0) continue;
          reco::Candidate::LorentzVector pk1, pk2;
          pk1.SetXYZT(kPlus.px(), kPlus.py(), kPlus.pz(), std::sqrt(kPlus.p() * kPlus.p() + mK * mK));
          pk2.SetXYZT(kMinus.px(), kMinus.py(), kMinus.pz(), std::sqrt(kMinus.p() * kMinus.p() + mK * mK));
          const reco::Candidate::LorentzVector kk = pk1 + pk2;
          if (kk.mass() < kaons.minKKMass || kk.mass() > kaons.maxKKMass) continue;

          BCandidate c = startCandidate(id, dimu);
          c.xP4 = kk;
          c.prefitP4 = dimu.p4 + kk;
          if (!finisher.inWindow(c) || !finisher.choosePV(c, dimu)) continue;
          c.legs = {useTrack(iPlus), useTrack(iMinus)};   // K+, K-
          if (muonParticles(muons, dimu))
            c.fit = MMGVertexFit::fitParticles({*muons[dimu.mu1].kinParticle, *muons[dimu.mu2].kinParticle,
                                                MMGVertexFit::trackParticle(tracks[iPlus].ttrack, mK),
                                                MMGVertexFit::trackParticle(tracks[iMinus].ttrack, mK)},
                                               finisher.pvPoint(c));
          finisher.observables(c, {&kPlus, &kMinus});
          candidates.push_back(std::move(c));
        }
      }
    }
    return candidates;
  }

}  // namespace bmmg
