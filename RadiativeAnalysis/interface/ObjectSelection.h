#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_ObjectSelection_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_ObjectSelection_h

// Loose preselection of muons and photons. All preselected objects are stored in the ntuple;
// tighter cuts (MVA, ID) are applied offline. minMVA cuts default to "no cut".

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include <vector>

namespace bmmg {

  struct MuonCuts {
    double minPt = 2.0;
    double maxAbsEta = 2.4;
    double minMVA = -99.;
  };

  struct PhotonCuts {
    double minSCEnergy = 1.0;
    double maxAbsSCEta = 2.5;
    double minMVA = -99.;
  };

  struct ConversionCuts {
    double minPt = 0.5;              // pair momentum
    double minVertexProb = 1e-6;
  };

  struct TrackCuts {
    double minPt = 0.7;
    double maxAbsEta = 2.5;
  };

  // global or tracker muons with an inner track; mva[i] is the MVA of muons[i]
  std::vector<SelectedMuon> selectMuons(const std::vector<reco::Muon>& muons, const std::vector<float>& mva,
                                        const TransientTrackBuilder& ttBuilder, const MuonCuts& cuts);

  // photons with a supercluster in the ECAL acceptance; the value maps may be invalid (then -99 / -1)
  std::vector<SelectedPhoton> selectPhotons(const edm::Handle<std::vector<reco::Photon>>& photons,
                                            const edm::Handle<edm::ValueMap<float>>& mva,
                                            const edm::Handle<edm::ValueMap<bool>>& tightId,
                                            const PhotonCuts& cuts);

  // conversions with two opposite-charge tracks and a valid vertex; the e+ e- pair is refitted with a
  // zero-mass constraint (SelectedConversion::photon, empty if that fit fails)
  std::vector<SelectedConversion> selectConversions(const std::vector<reco::Conversion>& conversions,
                                                    const TransientTrackBuilder& ttBuilder, const ConversionCuts& cuts);

  // high-purity tracks for the kaon legs, excluding the inner tracks of the selected muons
  std::vector<SelectedTrack> selectKaonTracks(const edm::Handle<std::vector<reco::Track>>& tracks,
                                              const std::vector<SelectedMuon>& muons,
                                              const TransientTrackBuilder& ttBuilder, const TrackCuts& cuts);

}  // namespace bmmg

#endif
