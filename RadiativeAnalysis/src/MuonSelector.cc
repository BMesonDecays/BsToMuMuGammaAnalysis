#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonSelector.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexPrimitives/interface/VertexException.h"
#include "TMath.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

MuonSelector::MuonSelector() 
     {}

MuonSelector::~MuonSelector() {}

template <typename MuonCollection>
std::tuple < std::vector<size_t>, std::vector<size_t>, std::vector<double> > MuonSelector::selectMuonPairs(
        const MuonCollection& muons, const TransientTrackBuilder& transientTrackBuilder)
{
    std::vector<size_t> firstIdx, secondIdx;
    std::vector<double> commonVtxProb;

    for (size_t i = 0; i < muons.size(); ++i) {
        for (size_t j = i + 1; j < muons.size(); ++j) {
            const auto& mu1 = muons[i];
            const auto& mu2 = muons[j];

            // Skip muons that do not pass selection criteria
            if (!passMuonSelection(mu1, mu2)) continue;
            
            // Create transient tracks
            auto tt1 = transientTrackBuilder.build(mu1.muonBestTrack());
            auto tt2 = transientTrackBuilder.build(mu2.muonBestTrack());
            
            std::vector<reco::TransientTrack> tts = {tt1, tt2};
            
            // Fit vertex
            KalmanVertexFitter fitter;
            TransientVertex vertex;     // invalid by default
            try {
                vertex = fitter.vertex(tts);
            } catch (const VertexException&) {
                continue;   // numerically singular fit: treat the pair as not compatible
            }

            if (!vertex.isValid())   continue;
            double chi2 = vertex.totalChiSquared();
            double ndf = vertex.degreesOfFreedom();
            double prob = TMath::Prob(chi2, ndf);

            // Skip uncompatible muons
            if (prob < 0.1) continue;

            firstIdx.push_back(i);
            secondIdx.push_back(j);
            commonVtxProb.push_back(prob);

        } // second muon
    } // first muon

    return {firstIdx, secondIdx, commonVtxProb};
}


template <typename patOrReco>
bool MuonSelector::passMuonSelection(const patOrReco& mu1, const patOrReco& mu2) const {
    
    if(mu1.charge() * mu2.charge() >= 0) return false; // Require opposite charge
    if (mu1.pt() <= muonPtCut || mu2.pt() <= muonPtCut) return false;
    if (mu1.innerTrack().isNull()) return false;
    if (mu2.innerTrack().isNull()) return false;

    reco::TrackRef muTrack1 = mu1.track();
    if (!muTrack1) return false;
    reco::TrackRef muTrack2 = mu2.track();
    if (!muTrack2) return false;

    if (!mu1.isGlobalMuon() && !mu1.isTrackerMuon()) return false;
    if (!mu2.isGlobalMuon() && !mu2.isTrackerMuon()) return false;

    if ( muon::overlap(mu1,mu2,1,1,true) ) return false;

    return true;
}

// Explicit instantiations (the templates are defined in this .cc file, so used types must be listed here)
template std::tuple < std::vector<size_t>, std::vector<size_t>, std::vector<double> > MuonSelector::selectMuonPairs<edm::View<reco::Muon>>(
    const edm::View<reco::Muon>&, const TransientTrackBuilder&);