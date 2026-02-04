#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_MuonSelector_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_MuonSelector_h

#include <vector>
#include "DataFormats/MuonReco/interface/Muon.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"


class MuonSelector {
public:
    MuonSelector();
    ~MuonSelector();

    /// Select the muon pair from a vector of reco::Muon objects
    std::vector<reco::Muon> selectMuonPair(
        const std::vector<reco::Muon>& muons,
        const TransientTrackBuilder& transientTrackBuilder);

private:
    /// Check if a muon passes selection criteria
    bool passMuonSelection(const reco::Muon& muon) const;
};

#endif