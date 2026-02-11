#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonSelector.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "TMath.h"

MuonSelector::MuonSelector() 
     {}

MuonSelector::~MuonSelector() {}

std::vector<reco::Muon> MuonSelector::selectMuonPair(
    const std::vector<reco::Muon>& muons,
    const TransientTrackBuilder& transientTrackBuilder) {
    
    
    // Filter muons passing selection criteria
    std::vector<reco::Muon> selectedMuons;
    for (const auto& muon : muons) {
        if (passMuonSelection(muon)) {
            selectedMuons.push_back(muon);
        }
    }
    
    // Form pairs from selected muons
    float bestVertexProb = -1.0;
    std::vector<reco::Muon> bestPair;
    for (size_t i = 0; i < selectedMuons.size(); ++i) {
        for (size_t j = i + 1; j < selectedMuons.size(); ++j) {
            const reco::Muon& mu1 = selectedMuons[i];
            const reco::Muon& mu2 = selectedMuons[j];

            // Skip if same charge
            if (mu1.charge() == mu2.charge()) {
                continue;
            }
            
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
                
                // Update best pair based on vertex probability
                if (prob > bestVertexProb) {
                    bestVertexProb = prob;
                    bestPair = {mu1, mu2};
                }
            }
        }
    }
    
    return bestPair;
}

bool MuonSelector::passMuonSelection(const reco::Muon& muon) const {
    
    return true;
}