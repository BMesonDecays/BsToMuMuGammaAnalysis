#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonSelector_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonSelector_h

#include <vector>
#include <utility>
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"


class PhotonSelector {
public:
    PhotonSelector();
    ~PhotonSelector();

    /// Select the photon from a vector of photons or conversions, using the given muon pair
    std::pair<std::vector<reco::Photon>, std::vector<double>> selectPhoton(
        const std::vector<reco::Photon>& photons,
        const std::vector<reco::Muon>& muons,
        const TransientTrackBuilder& transientTrackBuilder,
        std::vector<float> photonMVAIDs,
        double photonMVAcut);
    
    std::pair<std::vector<reco::Photon>, std::vector<double>> selectPhotons(
        const std::vector<reco::Photon>& photons,
        const std::vector<reco::Muon>& muons,
        const TransientTrackBuilder& transientTrackBuilder,
        std::vector<float> photonMVAIDs,
        double photonMVAcut);


    std::vector<pat::CompositeCandidate> selectConvertedPhoton(
        const std::vector<pat::CompositeCandidate>& conversions,
        const std::vector<reco::Muon>& muons,
        const TransientTrackBuilder& transientTrackBuilder);

    std::vector<pat::CompositeCandidate> selectConvertedPhotons(
        const std::vector<pat::CompositeCandidate>& conversions,
        const std::vector<reco::Muon>& muons,
        const TransientTrackBuilder& transientTrackBuilder);

    
    // sets the photon vertex, selects all photons that pass a selection and have deltaR to the dimuon less than max_dR
    // returns a vector with indices of photons that are selected and a vector with corresponding deltaRs
    // PhotonCollection can be e.g. std::vector<reco::Photon> or edm::View<reco::Photon>
    template <typename PhotonCollection>
    std::pair< std::vector<size_t>, std::vector<double> > selectRecoPhotons(
        const PhotonCollection& photons,
        const RefCountedKinematicParticle& dimuon,
        const math::XYZPoint dimuonVtxPosition,
        double max_dR = 0.5);


private:
    /// Check if a photon passes selection criteria
    template <typename Photon>
    bool passPhotonSelection(const Photon& photon) const;
};

#endif