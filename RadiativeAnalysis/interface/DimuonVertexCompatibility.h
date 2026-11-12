#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_DimuonVertexCompatibility_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_DimuonVertexCompatibility_h

#include <vector>
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/GlobalError.h"


// Dimuon "isolation" based on vertex compatibility:
// check whether any other track of the event forms a good common vertex with the two muons
class DimuonVertexCompatibility {
public:
    // a track is treated as one of the muons (and skipped) if it has the same charge,
    // deltaR < sameTrackMaxDeltaR and |pT - pT_mu| / pT_mu < sameTrackMaxRelDPt
    // (packed-candidate pseudo-tracks in MINIAOD have a reduced eta/phi precision, ~1e-4)
    static constexpr double sameTrackMaxDeltaR  = 0.001;
    static constexpr double sameTrackMaxRelDPt  = 0.01;

    // speed-up: tracks whose impact-parameter significance w.r.t. the dimuon vertex exceeds this value are not fitted
    // (they would add chi2 >~ 25 to the 3-track vertex, i.e. a probability below ~1e-5 for ndf = 3)
    static constexpr double maxIPSignificance   = 5.0;

    // maximal Kalman vertex probability of (mu1, mu2, track) over all tracks that are not one of the muons;
    // 0 if no such track gives a valid vertex (or if the dimuon vertex fit fails)
    static double maxThirdTrackVtxProb(
        const reco::TransientTrack& muonTT1,
        const reco::TransientTrack& muonTT2,
        const std::vector<reco::TransientTrack>& tracks);

private:
    static bool isSameTrack(const reco::Track& track, const reco::Track& muonTrack);
    // squared impact-parameter significance of the track w.r.t. the vertex (linear approximation, track + vertex errors)
    static double ipSignificance2(const reco::Track& track, const GlobalPoint& vtx, const GlobalError& vtxErr);
};

#endif
