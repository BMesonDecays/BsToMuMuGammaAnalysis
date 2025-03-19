#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_ReferenceModeratorVertex_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_ReferenceModeratorVertex_h


#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"




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
            double muonpairdr = -9999;
            double mu1trkbsxy = -9999;
            double mu1trkbsz = -9999;
            double mu2trkbsxy = -9999;
            double mu2trkbsz = -9999;
            int mu1pixelhits = -9999;
            int mu2pixelhits = -9999;
            int mu1trackerhits = -9999;
            int mu2trackerhits = -9999;
            int mu1isgood = -9999;
            int mu2isgood = -9999;
            int mu1innertrkhq = -9999;
            int mu2innertrkhq = -9999;
    
    
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
            double BsVtxProb = -9999;
            double BsCt3D = -9999;
            double BsCt2D = -9999;
            double BsCt2DBS = -9999;
            
            
        
        };
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
                const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
                const MagneticField& bField,
                const double nominalMuonMass,
                const double nominalKaonMass);


};

#endif
