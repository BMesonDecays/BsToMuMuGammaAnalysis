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

  // global or tracker muons with an inner track; mva[i] is the MVA of muons[i]
  std::vector<SelectedMuon> selectMuons(const std::vector<reco::Muon>& muons, const std::vector<float>& mva,
                                        const TransientTrackBuilder& ttBuilder, const MuonCuts& cuts);

  // photons with a supercluster in the ECAL acceptance; the value maps may be invalid (then -99 / -1)
  std::vector<SelectedPhoton> selectPhotons(const edm::Handle<std::vector<reco::Photon>>& photons,
                                            const edm::Handle<edm::ValueMap<float>>& mva,
                                            const edm::Handle<edm::ValueMap<bool>>& tightId,
                                            const PhotonCuts& cuts);

}  // namespace bmmg

#endif
