#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_TrippleObjectVertex_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_TrippleObjectVertex_h

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DecayChainVariables.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"



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
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "DataFormats/MuonReco/interface/MuonChamberMatch.h"
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
	int BsPVVtxInd=0;
	
	typedef reco::Candidate::LorentzVector LorentzVector;
    typedef std::pair<const reco::MuonChamberMatch*, const reco::MuonSegmentMatch*> MatchPair;


	DecayChainVariables TrippleObjectVertexObservables(const std::vector<reco::Muon>& muons, 
		    const std::vector<reco::Photon>& photons,
		     const std::vector<reco::Vertex>& PVs,
            const EcalClusterLazyTools& lazyTools,
			const pat::CompositeCandidateCollection& conversions,
			const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
			const MagneticField& bField,
			const double nominalMuonMass,
			const double nominalElectronMass, 
		 	const TransientTrackBuilder& transientTrackBuilder,
			RadiativeRootTree* rrt);
      
};


#endif
