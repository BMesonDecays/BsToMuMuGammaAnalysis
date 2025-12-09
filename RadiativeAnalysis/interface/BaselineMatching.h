#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_BaselineMatching_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_BaselineMatching_h

#include <utility>
#include <cmath>
#include "DataFormats/MuonReco/interface/MuonChamberMatch.h"
#include "DataFormats/MuonReco/interface/MuonSegmentMatch.h"
#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"

class BaselineMatching 
{
public:

    // --- Type aliases ---
    typedef std::pair<
        const reco::MuonChamberMatch*, 
        const reco::MuonSegmentMatch*
    > MatchPair;

    // --- Constructors ---
    BaselineMatching() = default;
    ~BaselineMatching() = default;

    // --- Core interface ---
    const MatchPair& getBetterMatch(const MatchPair& m1, const MatchPair& m2) const;

    float dX(const MatchPair& match) const;
    float pullX(const MatchPair& match) const;
    float pullDxDz(const MatchPair& match) const;

    float dY(const MatchPair& match) const;
    float pullY(const MatchPair& match) const;
    float pullDyDz(const MatchPair& match) const;

private:
    float safeDivide(float num, float den) const;
};

#endif
