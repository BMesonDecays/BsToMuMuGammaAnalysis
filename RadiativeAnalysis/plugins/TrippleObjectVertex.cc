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
            if (!mu1.isGlobalMuon() && !mu1.isTrackerMuon()) continue;
            if (!mu2.isGlobalMuon() && !mu2.isTrackerMuon()) continue;
            

           ReferenceResonance::ResonanceDetails resonanceResult = ReferenceResonance::findResonances(mu1, mu2, verbose);
           if (resonanceResult.isValid) {
            dcv.dimuonMass = resonanceResult.mass;
            dcv.dimuonEta = resonanceResult.eta;
            dcv.dimuonPhi = resonanceResult.phi;
            dcv.dimuonPt = resonanceResult.pt;
            dcv.resonanceFlag = static_cast<int>(resonanceResult.resonanceFlag); 
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
             dcv.dimuonchi2 = vertex.chi2();             
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
           if (resonanceResult.isValid) {
           dcv.dimuonlxyOverPt = (displacementFromBeamspot.x() * resonanceResult.px+ 
           displacementFromBeamspot.y() * resonanceResult.py) / (resonanceResult.pt * resonanceResult.pt);
                //std::cout<< "LXYOverPt: "<<dcv.dimuonlxyOverPt<<"\n";
           }
            //std::cout<<"LXYerror: "<<dcv.dimuonlxyerr<<"\t"<<"LXY: "<<dcv.dimuonlxy<<"\n";
		if (mu1.charge() == -1) {
			dcv.mu1pt = mu1.pt();dcv.mu1pz = mu1.pz();dcv.mu1eta = mu1.eta();dcv.mu1phi = mu1.phi();	
			dcv.mu2phi = mu2.phi();dcv.mu2pt = mu2.pt();dcv.mu2pz = mu2.pz();dcv.mu2eta = mu2.eta();
		}else {
			dcv.mu2pt = mu1.pt();dcv.mu2pz = mu1.pz();dcv.mu2eta = mu1.eta();dcv.mu2phi = mu1.phi();
			dcv.mu1phi = mu2.phi();dcv.mu1pt = mu2.pt();dcv.mu1pz = mu2.pz();dcv.mu1eta = mu2.eta();
		}
		//std::cout << "mu1pt: " << dcv.mu1pt << ", "<< "mu1pz: " << dcv.mu1pz << ", "<< "mu1eta: " << dcv.mu1eta << ", "<< "mu1phi: " << dcv.mu1phi <<"\n";
        if (mu1.isTrackerMuon() && !mu1.isGlobalMuon())       dcv.diMuon_mu1Cat = 1;
        else if (!mu1.isTrackerMuon() && mu1.isGlobalMuon())  dcv.diMuon_mu1Cat = 2;
        else if (mu1.isTrackerMuon() && mu1.isGlobalMuon())   dcv.diMuon_mu1Cat = 3;
        else if (!mu1.isTrackerMuon() && !mu1.isGlobalMuon()) dcv.diMuon_mu1Cat = 4;
        if (mu1.isPFMuon())       dcv.diMuon_mu1Cat = 1;
        if (mu2.isTrackerMuon() && !mu2.isGlobalMuon())       dcv.diMuon_mu2Cat = 1;
        else if (!mu2.isTrackerMuon() && mu2.isGlobalMuon())  dcv.diMuon_mu2Cat = 2;
        else if (mu2.isTrackerMuon() && mu2.isGlobalMuon())   dcv.diMuon_mu2Cat = 3;
        else if (!mu2.isTrackerMuon() && !mu2.isGlobalMuon()) dcv.diMuon_mu2Cat = 4;
        if (mu2.isPFMuon())       dcv.diMuon_mu2Cat = 1;
        
        int pixhits1 = 0;
        const reco::HitPattern& pp1 = muTrack1.get()->hitPattern();
        for (int iter=0; iter<pp1.numberOfAllHits(reco::HitPattern::TRACK_HITS); iter++) {
			uint32_t hit = pp1.getHitPattern(reco::HitPattern::TRACK_HITS,iter);
            //std::cout<<"hits  for first muon  : "<<hit<<"\n";
			if (pp1.validHitFilter(hit) && pp1.pixelBarrelHitFilter(hit)) pixhits1++;
			if (pp1.validHitFilter(hit) && pp1.pixelEndcapHitFilter(hit)) pixhits1++;
            }
      dcv.diMuon_mu1PixelHits   = pixhits1;
      int pixhits2 = 0;
      const reco::HitPattern& pp2 = muTrack1.get()->hitPattern();
      for (int iter=0; iter<pp2.numberOfAllHits(reco::HitPattern::TRACK_HITS); iter++) {
			uint32_t hit = pp2.getHitPattern(reco::HitPattern::TRACK_HITS,iter);
			if (pp2.validHitFilter(hit) && pp2.pixelBarrelHitFilter(hit)) pixhits2++;
			if (pp2.validHitFilter(hit) && pp2.pixelEndcapHitFilter(hit)) pixhits2++;
      }
     
      dcv.diMuon_mu2PixelHits = pixhits2;
            for (const auto& conv : conversions) {
                const reco::Track eletk0 = *conv.userData<reco::Track>("track0");
                const reco::Track eletk1 = *conv.userData<reco::Track>("track1");
                std::vector<reco::TransientTrack> tttrk_electrons = {
                    reco::TransientTrack(eletk0, &bField),
                    reco::TransientTrack(eletk1, &bField)
                };
                TLorentzVector BCand, eleTrack1, eleTrack2, muonTrack1, muonTrack2;
                eleTrack1.SetPtEtaPhiM(eletk0.pt(), eletk0.eta(), eletk0.phi(), nominalElectronMass);
                eleTrack2.SetPtEtaPhiM(eletk1.pt(), eletk1.eta(), eletk1.phi(), nominalElectronMass);
                muonTrack1.SetPtEtaPhiM(mu1.pt(), mu1.eta(), mu1.phi(), nominalMuonMass);
                muonTrack2.SetPtEtaPhiM(mu2.pt(), mu2.eta(), mu2.phi(), nominalMuonMass);
                BCand = eleTrack1 + eleTrack2 + muonTrack1 + muonTrack2;
                MassLimits m_lim;
                if (BCand.M() < m_lim.BsMassCutLower || BCand.M() > m_lim.BsMassCutUpper) continue;
                std::cout<<"mass B: "<<BCand.M()<<"\n";
       	  		/*BCand.addDaughter(mu1);
       	  		BCand.addDaughter(mu2);
       	  	    BCand.addDaughter(eletk0);
       	  		BCand.addDaughter(eletk1);
       	  		AddFourMomenta add4mom;
       	  		add4mom.set(BCand);*/
               
       	  		
                
                KinematicConstrainedFit BCandFitter;
                bool fitSuccess = BCandFitter.TrippleObjectVertexFit(ttrk_muons, nominalMuonMass, tttrk_electrons, nominalElectronMass);
                if (!fitSuccess) continue;
		        std::cout<<"print the fit sucess : "<< fitSuccess<< "\n";
                dcv.fittedBmass = BCandFitter.getBhadronMass();
                dcv.BsMass = BCand.M();
                dcv.BsPt   = BCand.Pt();
                dcv.BsEta  = BCand.Eta();
                dcv.BsPhi  = BCand.Phi();
	    }
	}
    }
    return dcv;
}
