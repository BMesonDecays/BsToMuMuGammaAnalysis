#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_ReferenceModeratorVertex_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_ReferenceModeratorVertex_h


#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DecayChainVariables.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"



#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/IsolatedTrack.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"
#include "DataFormats/Candidate/interface/Candidate.h"



#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include <iostream>
#include "TMath.h"



class ReferenceModeratorVertex{
    public:
        ReferenceModeratorVertex();
        ~ReferenceModeratorVertex(){}
        
        struct MassLimits {
            
            static constexpr double BsMassCutLower   = 4.5; 
            static constexpr double BsMassCutUpper   = 6.0;
            static constexpr double BdMassCutLower   = 4.5;
            static constexpr double BdMassCutUpper   = 6.0; 
            static constexpr double BsPDGMass = 5.36677;
              
                   
        };
        double MuMass =  0.1056583715;
        TLorentzVector Mu1, Mu2 ;
    
        DecayChainVariables ReferenceModeratorVertexObservables(const std::vector<reco::Muon>& muons, 
                const std::vector<reco::Track>& tracks,
                const std::vector<reco::Vertex>& PVs,
                const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
                const MagneticField& bField,
                const double nominalMuonMass,
                const double nominalKaonMass,
                RadiativeRootTree* rrt);


};

#endif
