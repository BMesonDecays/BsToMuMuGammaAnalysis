#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TrippleObjectVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"



#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
//#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
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
using namespace reco;
using namespace edm;
using namespace std;
using namespace pat;
#include <TMath.h>


TrippleObjectVertex::TrippleObjectVertex(){}
TrippleObjectVertex::DecayChainVariables TrippleObjectVertex::TrippleObjectVertexObservables(
    const std::vector<reco::Muon>& muons,
    const pat::CompositeCandidateCollection& conversions,
    const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
    const MagneticField& bField,
    const double nominalMuonMass,
    const double nominalElectronMass){
         std::string resonances[] = {"Jpsi", "Phi", "KStar", "nonResonant"};
         bool verbose = true;
        
        DecayChainVariables dcv;
    for (size_t i = 0; i < muons.size(); ++i) {
        const reco::Muon& mu1 = muons[i];
        if (mu1.innerTrack().isNull()) continue;
        reco::TrackRef muTrack1 = mu1.track();
        if (!muTrack1) continue;
        

        for (size_t j = i + 1; j < muons.size(); ++j) {
            const reco::Muon& mu2 = muons[j];
            if (mu2.innerTrack().isNull()) continue;
            if (mu1.charge() * mu2.charge() == 1) continue;
            reco::TrackRef muTrack2 = mu2.track();
            if (!muTrack2) continue;
            dcv.mu1softid = mu1.passed(reco::Muon::SoftCutBasedId);
            dcv.mu2softid = mu2.passed(reco::Muon::SoftCutBasedId);
            if(!mu1.isGlobalMuon() && !mu1.isTrackerMuon()) continue;
            if(!mu2.isGlobalMuon() && !mu2.isTrackerMuon()) continue;
            double bestMass = -99;
            std::string bestResonanceType;
            bool isAnyValidResonance = false;

            for (const std::string& resonanceType : resonances) {
                ReferenceResonance::ResonanceDetails resonanceResult = ReferenceResonance::applyResonanceMassCut(mu1, mu2, resonanceType, verbose);
                if (resonanceResult.isValid) {
                    isAnyValidResonance = true;
                    bestMass = resonanceResult.mass;
                    bestResonanceType = resonanceType;
                    goto resonance_found;
                }
            }
            resonance_found:
            if (isAnyValidResonance) {
                dcv.dimuonMass = bestMass;
                dcv.resonanceType = bestResonanceType;  // Save the identified resonance
                std::cout << "Selected dimuon resonance: " << bestResonanceType << " with mass: " << bestMass << std::endl;
                // Proceed with additional processing if necessary
            }
            
            reco::TransientTrack muonTT1(muTrack1, &bField);
            reco::TransientTrack muonTT2(muTrack2, &bField);
            KalmanVertexFitter kvf(true);
            std::vector<reco::TransientTrack> ttrk_muons = {muonTT1, muonTT2};
            TransientVertex tv = kvf.vertex(ttrk_muons);
            int isCowboy=0;
             if (mu1.charge()==1) {
             float mupPhi = atan(mu1.py()/mu1.px());
             if ( mu1.px() < 0 && mu1.py() < 0 ) mupPhi -= TMath::Pi();
             if ( mu1.px() < 0 && mu1.py() > 0 ) mupPhi += TMath::Pi();
             float mumPhi = atan(mu2.py()/mu2.px());
             if ( mu2.px() < 0 && mu2.py() < 0 ) mumPhi -= TMath::Pi();
             if ( mu2.px() < 0 && mu2.py() > 0 ) mumPhi += TMath::Pi();
             if ( (mupPhi - mumPhi)>0 ) isCowboy=1; } 
             else {
                   float mupPhi = atan(mu2.py()/mu2.px());
                   if ( mu2.px() < 0 && mu2.py() < 0 ) mupPhi -= TMath::Pi();
                   if ( mu2.px() < 0 && mu2.py() > 0 ) mupPhi += TMath::Pi();
                   float mumPhi = atan(mu1.py()/mu1.px());
                   if ( mu1.px() < 0 && mu1.py() < 0 ) mumPhi -= TMath::Pi();
                   if ( mu1.px() < 0 && mu1.py() > 0 ) mumPhi += TMath::Pi();
                   if ( (mupPhi - mumPhi)>0 ) isCowboy=1;
                  }
                  std::cout<< "isCowboy : "<< isCowboy<< "\n";
             Vertex vertex = tv;
             double  vtxProb_Jpsi = TMath::Prob(vertex.chi2(),(int)vertex.ndof());
             math::XYZVector      pperp(mu1.px() + mu2.px(), mu1.py() + mu2.py(), 0.);
             reco::Vertex::Point  vpoint=vertex.position();
             GlobalPoint secondaryVertex (vpoint.x(), vpoint.y(), vpoint.z());
             GlobalPoint displacementFromBeamspot( -1*((bsAndVtxInfo.bs_x -  secondaryVertex.x()) +  
             (secondaryVertex.z() - bsAndVtxInfo.bs_z) * bsAndVtxInfo.bs_dxdz),-1*((bsAndVtxInfo.bs_y - secondaryVertex.y())+
               (secondaryVertex.z() - bsAndVtxInfo.bs_z) * bsAndVtxInfo.bs_dydz), 0);
               reco::Vertex::Point vperp(displacementFromBeamspot.x(),displacementFromBeamspot.y(),0.);
               double CosAlpha = vperp.Dot(pperp)/(vperp.R()*pperp.R());
               std::cout<<"opening angle between two muons : "<<CosAlpha<<"\n";


            for (const auto& conv : conversions) {
                const reco::Track eletk0 = *conv.userData<reco::Track>("track0");
                const reco::Track eletk1 = *conv.userData<reco::Track>("track1");
                std::vector<reco::TransientTrack> tttrk_electrons = {
                    reco::TransientTrack(eletk0, &bField),
                    reco::TransientTrack(eletk1, &bField)
                };

                KinematicConstrainedFit BCandFitter;
                bool fitSuccess = BCandFitter.TrippleObjectVertexFit(ttrk_muons, nominalMuonMass, tttrk_electrons, nominalElectronMass);
                if (!fitSuccess) continue;
		        std::cout<<"print the fit sucess : "<< fitSuccess<< "\n";
                dcv.mass = BCandFitter.getBhadronMass();
	    }
	}
    }
    return dcv;
}
