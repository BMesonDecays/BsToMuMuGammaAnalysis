#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_Tools_H
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_Tools_H

#include <vector>
#include <set>
#include "TMath.h"
#include "Math/Vector4D.h"

namespace Tools{

    // Klara's method
    inline math::XYZPoint pca(math::XYZPoint pv, math::XYZPoint sv, math::XYZVector mom){
        //s = (PV - SV) * (mom) / |mom|^2 
        double s = ((pv - sv).Dot(mom)) / mom.Mag2();
        //pca = sv + s*mom
        math::XYZPoint PCA = sv + s*mom;

        return PCA;
    }
}


#endif