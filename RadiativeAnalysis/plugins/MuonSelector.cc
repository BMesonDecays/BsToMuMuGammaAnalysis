#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonSelector.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "TMath.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

MuonSelector::MuonSelector() 
     {}

MuonSelector::~MuonSelector() {}

std::pair<std::vector<reco::Muon>, std::vector<float>> MuonSelector::selectMuonPair(
    const std::vector<reco::Muon>& muons,
    const TransientTrackBuilder& transientTrackBuilder,
    const reco::BeamSpot& beamSpot,
    std::vector<float> mvaScores,
    double mvaCut) {
    
    
    // Form pairs from selected muons
    float bestVertexProb = -1.0;
    std::vector<reco::Muon> bestPair;
    std::vector<float> bestPairScores;
    for (size_t i = 0; i < muons.size(); ++i) {
        for (size_t j = i + 1; j < muons.size(); ++j) {
            const reco::Muon& mu1 = muons[i];
            const reco::Muon& mu2 = muons[j];

            // Skip muons that do not pass selection criteria
            if (!passMuonSelection(mu1, mu2)) continue;
            float mvaScore1 = mvaScores.at(i);
            float mvaScore2 = mvaScores.at(j);
            if (mvaScore1 < mvaCut || mvaScore2 < mvaCut) continue;
            
            // Create transient tracks
            auto tt1 = transientTrackBuilder.build(mu1.muonBestTrack());
            auto tt2 = transientTrackBuilder.build(mu2.muonBestTrack());
            
            std::vector<reco::TransientTrack> tts = {tt1, tt2};
            
            // Fit vertex
            KalmanVertexFitter fitter;
            TransientVertex vertex = fitter.vertex(tts);
            
            if (vertex.isValid()) {
                double chi2 = vertex.totalChiSquared();
                double ndf = vertex.degreesOfFreedom();
                double prob = TMath::Prob(chi2, ndf);

                // calculate displacement from beamspot
                double dx = vertex.position().x() - beamSpot.x0();
                double dy = vertex.position().y() - beamSpot.y0();
                double disp = sqrt(dx*dx + dy*dy);
                if (disp < 0.01) continue;
                
                // Update best pair based on vertex probability
                if (prob > bestVertexProb) {
                    bestVertexProb = prob;
                    bestPair = {mu1, mu2};
                    bestPairScores = {mvaScore1, mvaScore2};
                }
            }
        }
    }
    
    return std::make_pair(bestPair, bestPairScores);
}

bool MuonSelector::passMuonSelection(const reco::Muon& mu1, const reco::Muon& mu2) const {
    
    if(mu1.charge() * mu2.charge() >= 0) return false; // Require opposite charge
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