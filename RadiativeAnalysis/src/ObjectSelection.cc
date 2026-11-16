#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ObjectSelection.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/Ref.h"

#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "TMath.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include <algorithm>

#include <cmath>

namespace bmmg {

  std::vector<SelectedMuon> selectMuons(const edm::View<reco::Muon>& muons, const std::vector<float>& mva,
                                        const TransientTrackBuilder& ttBuilder, const MuonCuts& cuts) {
    std::vector<SelectedMuon> selected;
    for (size_t i = 0; i < muons.size(); ++i) {
      const reco::Muon& mu = muons[i];
      if (!mu.isGlobalMuon() && !mu.isTrackerMuon()) continue;
      if (mu.innerTrack().isNull()) continue;
      if (mu.pt() < cuts.minPt || std::abs(mu.eta()) > cuts.maxAbsEta) continue;
      const float score = (i < mva.size()) ? mva[i] : -99.f;
      if (score < cuts.minMVA) continue;
      SelectedMuon sm;
      sm.index = i;
      sm.muon = &mu;
      sm.mva = score;
      sm.track = ttBuilder.build(mu.innerTrack());
      selected.push_back(std::move(sm));
    }
    // the kinematic particles point to the TransientTracks: built only once the vector is final
    // (returning the vector moves its buffer, the elements keep their addresses)
    for (auto& sm : selected) sm.kinParticle = MMGVertexFit::muonParticle(sm.track);
    return selected;
  }

  std::vector<SelectedPhoton> selectPhotons(const edm::Handle<edm::View<reco::Photon>>& photons,
                                            const edm::Handle<edm::ValueMap<float>>& mva,
                                            const edm::Handle<edm::ValueMap<bool>>& tightId,
                                            const PhotonCuts& cuts) {
    std::vector<SelectedPhoton> selected;
    for (size_t i = 0; i < photons->size(); ++i) {
      const reco::Photon& pho = (*photons)[i];
      if (pho.superCluster().isNull()) continue;
      if (pho.superCluster()->energy() < cuts.minSCEnergy) continue;
      if (std::abs(pho.superCluster()->eta()) > cuts.maxAbsSCEta) continue;
      SelectedPhoton sp;
      sp.index = i;
      sp.photon = &pho;
      const edm::Ptr<reco::Photon> ptr = photons->ptrAt(i);
      if (mva.isValid() && mva->contains(ptr.id())) sp.mva = (*mva)[ptr];
      if (tightId.isValid() && tightId->contains(ptr.id())) sp.tightId = (*tightId)[ptr] ? 1 : 0;
      if (sp.mva < cuts.minMVA) continue;
      selected.push_back(sp);
    }
    return selected;
  }

  std::vector<SelectedConversion> selectConversions(const std::vector<reco::Conversion>& conversions,
                                                    const TransientTrackBuilder& ttBuilder, const ConversionCuts& cuts) {
    std::vector<SelectedConversion> selected;
    for (size_t i = 0; i < conversions.size(); ++i) {
      const reco::Conversion& conv = conversions[i];
      if (conv.nTracks() != 2) continue;
      const auto& trks = conv.tracks();
      // the tracks must be available (in MiniAOD the conversion track references point to tracks not stored)
      if (!trks[0].isAvailable() || !trks[1].isAvailable()) continue;
      if (trks[0]->charge() * trks[1]->charge() >= 0) continue;
      if (!conv.conversionVertex().isValid()) continue;
      if (conv.pairMomentum().perp2() < cuts.minPt * cuts.minPt) continue;
      if (TMath::Prob(conv.conversionVertex().chi2(), static_cast<int>(conv.conversionVertex().ndof())) < cuts.minVertexProb)
        continue;
      SelectedConversion sc;
      sc.index = i;
      sc.conversion = &conv;
      const bool firstPositive = trks[0]->charge() > 0;
      sc.track1 = ttBuilder.build(*trks[firstPositive ? 0 : 1]);
      sc.track2 = ttBuilder.build(*trks[firstPositive ? 1 : 0]);
      selected.push_back(std::move(sc));
    }
    // e+ e- refit once the vector is final: the particles point to the TransientTracks
    for (auto& sc : selected) {
      sc.photon = MMGVertexFit::conversionParticle(sc.track1, sc.track2);
      if (sc.photon) {
        const KinematicState& s = (*sc.photon)->currentState();
        const GlobalVector p = s.globalMomentum();
        sc.p4.SetXYZT(p.x(), p.y(), p.z(), p.mag());
      } else {
        const auto& p = sc.conversion->pairMomentum();
        sc.p4.SetXYZT(p.x(), p.y(), p.z(), std::sqrt(p.mag2()));
      }
    }
    return selected;
  }

  std::vector<SelectedTrack> selectKaonTracks(const std::vector<reco::Track>& tracks,
                                              const std::vector<SelectedMuon>& muons,
                                              const TransientTrackBuilder& ttBuilder, const TrackCuts& cuts) {
    std::vector<SelectedTrack> selected;
    for (size_t i = 0; i < tracks.size(); ++i) {
      const reco::Track& trk = tracks[i];
      if (!trk.quality(reco::TrackBase::highPurity)) continue;
      if (trk.pt() < cuts.minPt || std::abs(trk.eta()) > cuts.maxAbsEta) continue;
      bool isMuon = false;   // same track as a muon inner track (works for AOD tracks and MiniAOD pseudo-tracks)
      for (const auto& m : muons) {
        const reco::Track& inner = *m.muon->innerTrack();
        // tolerances allow for the reduced precision of the MiniAOD pseudo-tracks
        if (reco::deltaR(inner, trk) < 5e-3 && std::abs(inner.pt() - trk.pt()) < 1e-2 * trk.pt() &&
            inner.charge() == trk.charge()) {
          isMuon = true;
          break;
        }
      }
      if (isMuon) continue;
      SelectedTrack st;
      st.index = i;
      st.track = &trk;
      st.ttrack = ttBuilder.build(trk);
      selected.push_back(std::move(st));
    }
    return selected;
  }

  std::vector<PVTrack> pvTracksFromVertices(const std::vector<reco::Vertex>& pvs) {
    std::vector<PVTrack> result;
    // tracks with weight > 0.5 in the fit of the PV: the same definition as PackedCandidate::UsedInFitTight
    for (size_t iv = 0; iv < pvs.size(); ++iv)
      for (auto it = pvs[iv].tracks_begin(); it != pvs[iv].tracks_end(); ++it)
        if (pvs[iv].trackWeight(*it) > 0.5f) result.push_back({**it, static_cast<int>(iv)});
    return result;
  }

  namespace {
    void addPacked(const std::vector<pat::PackedCandidate>& cands, std::vector<PVTrack>& out) {
      for (const auto& c : cands) {
        if (c.charge() == 0 || !c.hasTrackDetails() || c.vertexRef().isNull()) continue;
        // used in the fit of its associated PV with weight > 0.5. Note: fromPV() without argument tests PV 0
        // only, which would drop the tracks of all other PVs.
        if (c.pvAssociationQuality() != pat::PackedCandidate::UsedInFitTight) continue;
        out.push_back({c.pseudoTrack(), static_cast<int>(c.vertexRef().key())});
      }
    }
    void addPackedTracks(const std::vector<pat::PackedCandidate>& cands, std::vector<reco::Track>& out) {
      for (const auto& c : cands)
        if (c.charge() != 0 && c.hasTrackDetails()) out.push_back(c.pseudoTrack());
    }
  }  // namespace

  std::vector<PVTrack> pvTracksFromPacked(const std::vector<pat::PackedCandidate>& packed,
                                          const std::vector<pat::PackedCandidate>& lost) {
    std::vector<PVTrack> result;
    addPacked(packed, result);
    addPacked(lost, result);
    return result;
  }

  std::vector<reco::Track> tracksFromPacked(const std::vector<pat::PackedCandidate>& packed,
                                            const std::vector<pat::PackedCandidate>& lost) {
    std::vector<reco::Track> result;
    addPackedTracks(packed, result);
    addPackedTracks(lost, result);
    return result;
  }

  // ---------------------------------------------------------------------------- isolation inputs
  namespace {
    bool isNeutralForIsolation(int pdgId) { return pdgId == 22 || pdgId == 130; }   // not the HF candidates (1, 2)
  }

  std::vector<NeutralParticle> neutralsFromPF(const std::vector<reco::PFCandidate>& pf, double minPt) {
    std::vector<NeutralParticle> out;
    for (size_t i = 0; i < pf.size(); ++i)
      if (pf[i].charge() == 0 && isNeutralForIsolation(std::abs(pf[i].pdgId())) && pf[i].pt() > minPt)
        out.push_back({float(pf[i].pt()), float(pf[i].eta()), float(pf[i].phi()), static_cast<int>(i)});
    return out;
  }

  std::vector<NeutralParticle> neutralsFromPacked(const std::vector<pat::PackedCandidate>& packed, double minPt) {
    std::vector<NeutralParticle> out;
    for (size_t i = 0; i < packed.size(); ++i)
      if (packed[i].charge() == 0 && isNeutralForIsolation(std::abs(packed[i].pdgId())) && packed[i].pt() > minPt)
        out.push_back({float(packed[i].pt()), float(packed[i].eta()), float(packed[i].phi()), static_cast<int>(i)});
    return out;
  }

  void setPhotonFootprints(std::vector<SelectedPhoton>& photons, const edm::Handle<edm::View<reco::Photon>>& handle,
                           const edm::ValueMap<std::vector<reco::PFCandidateRef>>& footprints, edm::ProductID pfId) {
    for (auto& sp : photons) {
      sp.footprint.clear();
      const edm::Ptr<reco::Photon> ptr = handle->ptrAt(sp.index);
      if (!footprints.contains(ptr.id())) continue;
      for (const auto& ref : footprints[ptr])
        if (ref.id() == pfId) sp.footprint.push_back(static_cast<int>(ref.key()));
      std::sort(sp.footprint.begin(), sp.footprint.end());
    }
  }

  void setPhotonFootprints(std::vector<SelectedPhoton>& photons, edm::ProductID packedId) {
    for (auto& sp : photons) {
      sp.footprint.clear();
      const auto* pat = dynamic_cast<const pat::Photon*>(sp.photon);
      if (!pat) continue;
      for (const auto& ref : pat->associatedPackedPFCandidates())
        if (ref.id() == packedId) sp.footprint.push_back(static_cast<int>(ref.key()));
      std::sort(sp.footprint.begin(), sp.footprint.end());
    }
  }

}  // namespace bmmg
