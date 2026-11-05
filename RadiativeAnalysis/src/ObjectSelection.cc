#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ObjectSelection.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/Ref.h"

#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "TMath.h"

#include <cmath>

namespace bmmg {

  std::vector<SelectedMuon> selectMuons(const std::vector<reco::Muon>& muons, const std::vector<float>& mva,
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

  std::vector<SelectedPhoton> selectPhotons(const edm::Handle<std::vector<reco::Photon>>& photons,
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
      const edm::Ptr<reco::Photon> ptr(photons, i);
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

  std::vector<SelectedTrack> selectKaonTracks(const edm::Handle<std::vector<reco::Track>>& tracks,
                                              const std::vector<SelectedMuon>& muons,
                                              const TransientTrackBuilder& ttBuilder, const TrackCuts& cuts) {
    std::vector<SelectedTrack> selected;
    for (size_t i = 0; i < tracks->size(); ++i) {
      const reco::Track& trk = (*tracks)[i];
      if (!trk.quality(reco::TrackBase::highPurity)) continue;
      if (trk.pt() < cuts.minPt || std::abs(trk.eta()) > cuts.maxAbsEta) continue;
      bool isMuon = false;
      for (const auto& m : muons) {
        const reco::TrackRef inner = m.muon->innerTrack();
        if (inner.id() == tracks.id() && inner.key() == i) {
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

}  // namespace bmmg
