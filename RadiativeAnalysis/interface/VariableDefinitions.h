#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_VariableDefinitions_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_VariableDefinitions_h

#include <cmath>

#include "GlobalIncludes.h"

namespace VariableDefinitions {

struct MassLimits {
        
        static constexpr double BsMassCutLower   = 4.5; 
		static constexpr double BsMassCutUpper   = 6.0;
		static constexpr double BdMassCutLower   = 4.5;
		static constexpr double BdMassCutUpper   = 6.0; 
        static constexpr double BsPDGMass = 5.36677;
    };

double calculateCt2D(const reco::Vertex& PV,
                     const RefCountedKinematicVertex& bVertex,
                     const RefCountedKinematicParticle& bCand);

double calculateCt3D(const reco::Vertex& PV,
                     const RefCountedKinematicVertex& bVertex,
                     const RefCountedKinematicParticle& bCand);

double calculateCt2DError(const reco::Vertex& PV,
                          const RefCountedKinematicVertex& bVertex,
                          const RefCountedKinematicParticle& bCand);

double calculateCt3DError(const reco::Vertex& PV,
                          const RefCountedKinematicVertex& bVertex,
                          const RefCountedKinematicParticle& bCand);

double isolation(const reco::Candidate::LorentzVector& candidateP4,
                 const reco::Muon& mu1,
                 const reco::Muon& mu2,
                 reco::Vertex PV,
                 double = 0.7, double ptMin = 0.9);

double maxCompatibility(const pat::Muon& mu1,
                        const pat::Muon& mu2,
                        reco::Vertex PV,
                        const TransientTrackBuilder& transientTrackBuilder);


} // namespace VariableDefinitions

#endif