#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_TrippleObjectVertex_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_TrippleObjectVertex_h

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"



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
        struct DecayChainVariables{
		double mass = -9999;
		int mu1softid = -9999;
		int mu2softid = -9999;
		int mu1tightid = -9999;
		int mu2tightid = -9999;
		int mu1globalMuon = -9999;
		int mu2globalMuon = -9999;
		int mu1trkMuon = -9999;
		int mu2trkMuon = -9999;

		std::vector<double> dimuonMasses;
		std::vector<std::string> resonanceTypes; 
		double opening_angle = -9999;
		double dimuonvtxprob = -9999;
		double mumudca = -9999;	
		int isCowboy = -9999;
	
	};

	DecayChainVariables TrippleObjectVertexObservables(const std::vector<reco::Muon>& muons, 
			const pat::CompositeCandidateCollection& conversions,
			const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
			const MagneticField& bField,
			const double nominalMuonMass,
			const double nominalElectronMass);
      
};


#endif
