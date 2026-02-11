#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonSelector_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonSelector_h

#include <vector>
#include "DataFormats/MuonReco/interface/Muon.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"


class PhotonSelector {
public:
    PhotonSelector();
    ~PhotonSelector();

    /// Select the photon from a vector of photons or conversions, using the given muon pair
    std::vector<reco::Photon> selectPhoton(
        const std::vector<reco::Photon>& photons,
        const std::vector<reco::Muon>& muons,
        const TransientTrackBuilder& transientTrackBuilder);

    std::vector<pat::CompositeCandidate> selectConvertedPhoton(
        const std::vector<pat::CompositeCandidate>& conversions,
        const std::vector<reco::Muon>& muons,
        const TransientTrackBuilder& transientTrackBuilder);

private:
    /// Check if a photon passes selection criteria
    bool passPhotonSelection(const reco::Photon& photon) const;
};

#endif