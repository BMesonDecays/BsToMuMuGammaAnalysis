#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_ObjectSelection_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_ObjectSelection_h

// Loose preselection of muons and photons. All preselected objects are stored in the ntuple;
// tighter cuts (MVA, ID) are applied offline. minMVA cuts default to "no cut".

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
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
  // (edm::View: reco::Muon in AOD, pat::Muon in MiniAOD)
  std::vector<SelectedMuon> selectMuons(const edm::View<reco::Muon>& muons, const std::vector<float>& mva,
                                        const TransientTrackBuilder& ttBuilder, const MuonCuts& cuts);

  // photons with a supercluster in the ECAL acceptance; the value maps may be invalid (then -99 / -1)
  std::vector<SelectedPhoton> selectPhotons(const edm::Handle<edm::View<reco::Photon>>& photons,
                                            const edm::Handle<edm::ValueMap<float>>& mva,
                                            const edm::Handle<edm::ValueMap<bool>>& tightId,
                                            const PhotonCuts& cuts);

  // conversions with two opposite-charge tracks and a valid vertex; the e+ e- pair is refitted with a
  // zero-mass constraint (SelectedConversion::photon, empty if that fit fails)
  std::vector<SelectedConversion> selectConversions(const std::vector<reco::Conversion>& conversions,
                                                    const TransientTrackBuilder& ttBuilder, const ConversionCuts& cuts);

  // high-purity tracks for the kaon legs, excluding the inner tracks of the selected muons;
  // "tracks" must outlive the result (SelectedTrack points to its elements)
  std::vector<SelectedTrack> selectKaonTracks(const std::vector<reco::Track>& tracks,
                                              const std::vector<SelectedMuon>& muons,
                                              const TransientTrackBuilder& ttBuilder, const TrackCuts& cuts);

  // ---- format-dependent track lists ----
  // AOD: the tracks of each primary vertex fit
  std::vector<PVTrack> pvTracksFromVertices(const std::vector<reco::Vertex>& pvs);
  // MiniAOD: packed candidates and lost tracks with track details that were used in the fit of their PV
  std::vector<PVTrack> pvTracksFromPacked(const std::vector<pat::PackedCandidate>& packed,
                                          const std::vector<pat::PackedCandidate>& lost);
  // MiniAOD: pseudo-tracks of all charged packed candidates and lost tracks with track details
  std::vector<reco::Track> tracksFromPacked(const std::vector<pat::PackedCandidate>& packed,
                                            const std::vector<pat::PackedCandidate>& lost);

}  // namespace bmmg

#endif
