#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ObjectSelection.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/Ref.h"

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
      sm.kinParticle = MMGVertexFit::muonParticle(mu, &ttBuilder);
      selected.push_back(std::move(sm));
    }
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

}  // namespace bmmg
