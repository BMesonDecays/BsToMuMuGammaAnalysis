#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_MuonClosestApproachCalculator_H
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_MuonClosestApproachCalculator_H



#include "TrackingTools/TrajectoryState/interface/TrajectoryStateClosestToPoint.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include <iostream>

class MuonClosestApproachCalculator {
public:
    MuonClosestApproachCalculator();

    ~MuonClosestApproachCalculator();

    double calculateDCA(const TrajectoryStateClosestToPoint& mu1TS, const TrajectoryStateClosestToPoint& mu2TS);

private:
    ClosestApproachInRPhi cApp_;
};

#endif // MUON_CLOSEST_APPROACH_CALCULATOR_H

