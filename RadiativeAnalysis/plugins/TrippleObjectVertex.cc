//Author : Muhammad Alibordi, University of Warsaw
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TrippleObjectVertex.h"



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
        dcv.dimuonMasses.clear();  
        dcv.resonanceTypes.clear();
	dcv.dimuonEta.clear();
	dcv.dimuonPhi.clear();
	dcv.dimuonPt.clear();
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
            std::string bestResonanceType;
            bool isAnyValidResonance = false;
            std::vector<std::string> validResonanceTypes;
            std::vector<double> validMasses;
	    std::vector<double> validEta;
	    std::vector<double> validPhi;
	    std::vector<double> validPt;


            for (const std::string& resonanceType : resonances) {
                ReferenceResonance::ResonanceDetails resonanceResult = ReferenceResonance::applyResonanceMassCut(mu1, mu2, resonanceType, verbose);
                if (resonanceResult.isValid) {
                    validResonanceTypes.push_back(resonanceType);
                    validMasses.push_back(resonanceResult.mass);
		    validEta.push_back(resonanceResult.eta);
		    validPhi.push_back(resonanceResult.phi);
		    validPt.push_back(resonanceResult.pt);
                    isAnyValidResonance = true;
                    //std::cout << "Found dimuon resonance: " << resonanceType << " with mass: " << resonanceResult.mass << std::endl;
                    //goto resonance_found;
                }
               // break;
            }
            //resonance_found:
            if (isAnyValidResonance && !validResonanceTypes.empty()) {
                for (size_t i = 0; i < validMasses.size(); ++i) {
                    dcv.dimuonMasses.push_back(validMasses[i]);
                    dcv.dimuonEta.push_back(validEta[i]);
                    dcv.dimuonPhi.push_back(validPhi[i]);
                    dcv.dimuonPt.push_back(validPt[i]);
                    dcv.resonanceTypes.push_back(validResonanceTypes[i]);
                    std::cout << "Selected dimuon resonance: " << dcv.resonanceTypes[i] << " with mass: " << dcv.dimuonMasses[i] << std::endl;
		    std::cout << "Eta , Phi and Pt:  "<< dcv.dimuonEta[i] << "\t"<< dcv.dimuonPhi[i]<<"\t"<< dcv.dimuonPt[i]<<"\n";
                    }
            }
            
            reco::TransientTrack muonTT1(muTrack1, &bField);
            reco::TransientTrack muonTT2(muTrack2, &bField);
            KalmanVertexFitter kvf(true);
            std::vector<reco::TransientTrack> ttrk_muons = {muonTT1, muonTT2};
            TransientVertex tv = kvf.vertex(ttrk_muons);
            
             if (mu1.charge()==1) {
             float mupPhi = atan(mu1.py()/mu1.px());
             if ( mu1.px() < 0 && mu1.py() < 0 ) mupPhi -= TMath::Pi();
             if ( mu1.px() < 0 && mu1.py() > 0 ) mupPhi += TMath::Pi();
             float mumPhi = atan(mu2.py()/mu2.px());
             if ( mu2.px() < 0 && mu2.py() < 0 ) mumPhi -= TMath::Pi();
             if ( mu2.px() < 0 && mu2.py() > 0 ) mumPhi += TMath::Pi();
             if ( (mupPhi - mumPhi)>0 ) dcv.isCowboy=1; } 
             else {
                   float mupPhi = atan(mu2.py()/mu2.px());
                   if ( mu2.px() < 0 && mu2.py() < 0 ) mupPhi -= TMath::Pi();
                   if ( mu2.px() < 0 && mu2.py() > 0 ) mupPhi += TMath::Pi();
                   float mumPhi = atan(mu1.py()/mu1.px());
                   if ( mu1.px() < 0 && mu1.py() < 0 ) mumPhi -= TMath::Pi();
                   if ( mu1.px() < 0 && mu1.py() > 0 ) mumPhi += TMath::Pi();
                   if ( (mupPhi - mumPhi)>0 ) dcv.isCowboy=1;
                  }
             
	     reco::Vertex vertex = tv;
             double  vtxProb_dimuon = TMath::Prob(vertex.chi2(),(int)vertex.ndof());
             dcv.dimuonvtxprob = vtxProb_dimuon;
             math::XYZVector      pperp(mu1.px() + mu2.px(), mu1.py() + mu2.py(), 0.);
             reco::Vertex::Point  vpoint=vertex.position();
             GlobalPoint secondaryVertex (vpoint.x(), vpoint.y(), vpoint.z());
             GlobalPoint displacementFromBeamspot( -1*((bsAndVtxInfo.bs_x -  secondaryVertex.x()) +  
             (secondaryVertex.z() - bsAndVtxInfo.bs_z) * bsAndVtxInfo.bs_dxdz),-1*((bsAndVtxInfo.bs_y - secondaryVertex.y())+
               (secondaryVertex.z() - bsAndVtxInfo.bs_z) * bsAndVtxInfo.bs_dydz), 0);
	     reco::Vertex::Point vperp(displacementFromBeamspot.x(),displacementFromBeamspot.y(),0.);
	     double CosAlpha = vperp.Dot(pperp)/(vperp.R()*pperp.R());
	     dcv.opening_angle = CosAlpha;
               TrajectoryStateClosestToPoint mu1TS = muonTT1.impactPointTSCP();
               TrajectoryStateClosestToPoint mu2TS = muonTT2.impactPointTSCP();
               if (mu1TS.isValid() && mu2TS.isValid()) {
		       MuonClosestApproachCalculator dcaCalculator;
		       double distanceOfClosestApproach = dcaCalculator.calculateDCA(mu1TS, mu2TS);
		       if (distanceOfClosestApproach >= 0.0)dcv.mumudca = distanceOfClosestApproach;
	       }
	       dcv.max_Dr1 = fabs( (- (mu1.vx()-bsAndVtxInfo.bs_x) * mu1.py() + (mu1.vy()-bsAndVtxInfo.bs_y) * mu1.px() ) / mu1.pt() );
	       dcv.max_Dr2 = fabs( (- (mu2.vx()-bsAndVtxInfo.bs_x) * mu2.py() + (mu2.vy()-bsAndVtxInfo.bs_y) * mu2.px() ) / mu2.pt() );
	       //std::cout<<"maxDR1: "<<dcv.max_Dr1<<"\t maxDR2 : "<<dcv.max_Dr2<<"\n";
	       if ( muon::overlap(mu1,mu2,1,1,true) ) continue; /// Skip the mu-mu combination if the two muons overlap
	       reco::Vertex::Error verr = vertex.error();
	       GlobalError err(verr.At(0,0), verr.At(1,0), verr.At(1,1), verr.At(2,0), verr.At(2,1), verr.At(2,2) );
	       dcv.dimuonlxy = displacementFromBeamspot.perp();
	       dcv.dimuonlxyerr = sqrt(err.rerr(displacementFromBeamspot));
	       //std::cout<<"LXYerror: "<<dcv.dimuonlxyerr<<"\t"<<"LXY: "<<dcv.dimuonlxy<<"\n";
		if (mu1.charge() == -1) {
			dcv.mu1pt = mu1.pt();dcv.mu1pz = mu1.pz();dcv.mu1eta = mu1.eta();dcv.mu1phi = mu1.phi();	
			dcv.mu2phi = mu2.phi();dcv.mu2pt = mu2.pt();dcv.mu2pz = mu2.pz();dcv.mu2eta = mu2.eta();
		}else {
			dcv.mu2pt = mu1.pt();dcv.mu2pz = mu1.pz();dcv.mu2eta = mu1.eta();dcv.mu2phi = mu1.phi();
			dcv.mu1phi = mu2.phi();dcv.mu1pt = mu2.pt();dcv.mu1pz = mu2.pz();dcv.mu1eta = mu2.eta();
		}
		//std::cout << "mu1pt: " << dcv.mu1pt << ", "<< "mu1pz: " << dcv.mu1pz << ", "<< "mu1eta: " << dcv.mu1eta << ", "<< "mu1phi: " << dcv.mu1phi <<"\n";

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
