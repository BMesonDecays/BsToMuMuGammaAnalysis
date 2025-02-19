#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_TetraObjectVertex_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_TetraObjectVertex_h



#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"



#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include <iostream>
#include <TMath.h>

#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CompositeCandidateFwd.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonTrackLinks.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"


class TetraObjectVertex{
    public: 
        TetraObjectVertex();
        ~TetraObjectVertex(){}
        struct DecayChainVariables{
            double fittedBmass = -9999;
            int mu1softid = -9999;
            int mu2softid = -9999;
            int mu1tightid = -9999;
            int mu2tightid = -9999;
            int mu1globalMuon = -9999;
            int mu2globalMuon = -9999;
            int mu1trkMuon = -9999;
            int mu2trkMuon = -9999;
            double max_Dr1 = -9999;
            double max_Dr2 = -9999;
            
    
            double dimuonMass = -9999;
            double dimuonEta = -9999;
            double dimuonPhi = -9999;
            double dimuonPt = -9999;
            int resonanceFlag = -9999;
            double opening_angle = -9999;
            double dimuonvtxprob = -9999;
            double mumudca = -9999;
            double dimuonlxy = -9999;
            double dimuonlxyerr = -9999;
            double dimuonlxyOverPt = -9999;
            double dimuonchi2 = -9999;
            double mu1pz = -9999.0, mu1pt = -9999.0, mu1eta = -9999.0, mu1phi = -9999.0;
            double mu2pz = -9999.0, mu2pt = -9999.0, mu2eta = -9999.0, mu2phi = -9999.0;
            int isCowboy = -9999;
            int diMuon_mu1Cat = -9999;
            int diMuon_mu2Cat = -9999;
            int diMuon_mu1PixelHits = -9999;
            int diMuon_mu2PixelHits = -9999;
            double BsMass = -9999;
            double BsPt = -9999;
            double BsEta = -9999;
            double BsPhi = -9999;
            
            
        
        };
        struct MassLimits {
        
            static constexpr double BsMassCutLower   = 4.5; 
            static constexpr double BsMassCutUpper   = 6.0;
            static constexpr double BdMassCutLower   = 4.5;
            static constexpr double BdMassCutUpper   = 6.0; 
              
                   
        };

        DecayChainVariables TetraObjectVertexObservables(const std::vector<reco::Muon>& muons, 
			const pat::CompositeCandidateCollection& conversions,
			const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
			const MagneticField& bField,
			const double nominalMuonMass,
			const double nominalElectronMass);

};



#endif
