#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_MuonSelector_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_MuonSelector_h

#include <vector>
#include <utility>
#include <tuple>

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"


class MuonSelector {
public:
    MuonSelector();
    ~MuonSelector();

    // Return all muon pairs passing the selection and common vertex criteria
    // return a tuple with 3 vectors: firstMuonIndex, secondMuonIndex, commonVertexProbability
    // MuonCollection can be e.g. std::vector<reco::Muon> or edm::View<reco::Muon>
    template <typename MuonCollection>
    std::tuple < std::vector<size_t>, std::vector<size_t>, std::vector<double> > selectMuonPairs(
        const MuonCollection& muons,
        const TransientTrackBuilder& transientTrackBuilder);

    /// Minimum transverse momentum required for each muon of the pair [GeV]
    static constexpr double muonPtCut = 3.0;

private:
    /// Check if a muon passes selection criteria
    template <typename patOrReco>
    bool passMuonSelection(const patOrReco& mu1, const patOrReco& mu2) const;
};

#endif