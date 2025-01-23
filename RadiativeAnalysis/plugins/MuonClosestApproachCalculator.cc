#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"

MuonClosestApproachCalculator::MuonClosestApproachCalculator() {}

MuonClosestApproachCalculator::~MuonClosestApproachCalculator() {}

double MuonClosestApproachCalculator::calculateDCA(const TrajectoryStateClosestToPoint& mu1TS, const TrajectoryStateClosestToPoint& mu2TS) {
    if (mu1TS.isValid() && mu2TS.isValid()) {
        cApp_.calculate(mu1TS.theState(), mu2TS.theState());
        if (cApp_.status()) {  // Check if calculation was successful
            double dca = cApp_.distance();
            std::cout << "Distance of closest approach between two muons: " << dca << "\n";
            return dca;
        } else {
            std::cerr << "Closest approach calculation failed.\n";
            return -1.0;  // Return a negative value to indicate failure
        }
    } else {
        std::cerr << "Invalid TrajectoryStateClosestToPoint provided.\n";
        return -1.0;  // Return a negative value to indicate failure
    }
}

