#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_TrippleObjectVertex_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_TrippleObjectVertex_h

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DecayChainVariables.h"




#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"


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

class TrippleObjectVertex{
    public: 
        TrippleObjectVertex();
        ~TrippleObjectVertex(){}
       
	struct MassLimits {
        
        static constexpr double BsMassCutLower   = 4.5; 
		static constexpr double BsMassCutUpper   = 6.0;
		static constexpr double BdMassCutLower   = 4.5;
		static constexpr double BdMassCutUpper   = 6.0; 
		static constexpr double BsPDGMass = 5.36677;
		  
               
    };
	double MuMass =  0.1056583715;
	TLorentzVector Mu1, Mu2 ;

	DecayChainVariables TrippleObjectVertexObservables(const std::vector<reco::Muon>& muons, 
		    const std::vector<reco::Photon>& photons,
            const EcalClusterLazyTools& lazyTools,
			const pat::CompositeCandidateCollection& conversions,
			const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
			const MagneticField& bField,
			const double nominalMuonMass,
			const double nominalElectronMass);
      
};


#endif
