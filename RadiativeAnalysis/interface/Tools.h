#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_Tools_H
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_Tools_H

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

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

    // find first produced B0s in MC sample
    const reco::Candidate* findFirstB0s(const reco::Candidate* partB0s)
    {
        if (partB0s->numberOfMothers() > 1)   return partB0s; //first B0s/B0s_bar in the B0s-B0s_bar state

        return findFirstB0s(partB0s->mother(0));
    }
}


#endif