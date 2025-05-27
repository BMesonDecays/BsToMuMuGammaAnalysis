//Author : Muhammad Alibordi, University of Warsaw
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TrippleObjectVertex.h"



TrippleObjectVertex::TrippleObjectVertex(){}
DecayChainVariables TrippleObjectVertex::TrippleObjectVertexObservables(
    //removing the qualifier TrippleObjectVertex fron the defintion of DecayChainVariables since it is not anymore a member of such class
    const std::vector<reco::Muon>& muons,
    const std::vector<reco::Photon>& photons,
    const EcalClusterLazyTools& lazyTools,
    const pat::CompositeCandidateCollection& conversions,
    const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
    const MagneticField& bField,
    const double nominalMuonMass,
    const double nominalElectronMass,
    const TransientTrackBuilder& transientTrackBuilder) {
        
        bool verbose = false;
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
			dcv.mu1pt = mu1.pt();dcv.mu1pz = mu1.pz();dcv.mu1eta = mu1.eta();dcv.mu1phi = mu1.phi(); dcv.mu1energy = mu1.energy();	
			dcv.mu2phi = mu2.phi();dcv.mu2pt = mu2.pt();dcv.mu2pz = mu2.pz();dcv.mu2eta = mu2.eta(); dcv.mu2energy = mu2.energy();
		}else {
			dcv.mu2pt = mu1.pt();dcv.mu2pz = mu1.pz();dcv.mu2eta = mu1.eta();dcv.mu2phi = mu1.phi(); dcv.mu2energy = mu1.energy();
			dcv.mu1phi = mu2.phi();dcv.mu1pt = mu2.pt();dcv.mu1pz = mu2.pz();dcv.mu1eta = mu2.eta(); dcv.mu1energy = mu2.energy();
		}
        //std::cout<< " muon1 enenrgy : "<< dcv.mu1energy << "\n";
        //std::cout<< " muon2 enenrgy : "<< dcv.mu2energy << "\n";
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
      Mu1.SetPtEtaPhiM( mu1.pt(), mu1.eta(), mu1.phi(), MuMass);
      Mu2.SetPtEtaPhiM( mu2.pt(), mu2.eta(), mu2.phi(), MuMass);
      dcv.muonpairdr = Mu1.DeltaR(Mu2) ;
      dcv.mu1trkbsxy  = mu1.innerTrack()->dxy(bsAndVtxInfo.position);
      dcv.mu1trkbsz   = mu1.innerTrack()->dz(bsAndVtxInfo.position);
      dcv.mu2trkbsxy  = mu2.innerTrack()->dxy(bsAndVtxInfo.position);
      dcv.mu2trkbsz   = mu2.innerTrack()->dz(bsAndVtxInfo.position);
      dcv.mu1pixelhits = mu1.innerTrack()->hitPattern().pixelLayersWithMeasurement() ;
      dcv.mu2pixelhits = mu2.innerTrack()->hitPattern().pixelLayersWithMeasurement() ;
      if( mu1.innerTrack()->quality(reco::TrackBase::highPurity) )	{dcv.mu1innertrkhq = 1 ;}
      if( muon::isGoodMuon(mu1, muon::TMOneStationTight) )	{ dcv.mu1isgood= 1 ;}
      if( mu2.innerTrack()->quality(reco::TrackBase::highPurity) )	{dcv.mu2innertrkhq = 1 ;}
      if( muon::isGoodMuon(mu2, muon::TMOneStationTight) )	{ dcv.mu2isgood= 1 ;}
      
        for (const auto& conv : conversions) {
		dcv.vertexFitFlag = 1;
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
                //std::cout<<"mass B: "<<BCand.M()<<"\n";
       	  		/*BCand.addDaughter(mu1);
       	  		BCand.addDaughter(mu2);
       	  	    BCand.addDaughter(eletk0);
       	  		BCand.addDaughter(eletk1);
       	  		AddFourMomenta add4mom;
       	  		add4mom.set(BCand);*/
               
       	  		std::vector<reco::TransientTrack> t_tracks;
                t_tracks.push_back(muonTT1);
                t_tracks.push_back(muonTT2);
                t_tracks.push_back(tttrk_electrons[0]);
                t_tracks.push_back(tttrk_electrons[1]);
                KalmanVertexFitter kvfbs(true);
                TransientVertex kvfbsvertex = kvfbs.vertex(t_tracks);
                //std::cout<< " the vertex position : "<< kvfbsvertex.position().x() << "\t"<< kvfbsvertex.position().y() << "\t"<< kvfbsvertex.position().z() << "\n";
                reco::Vertex vertexbskalman = kvfbsvertex;
                if (!kvfbsvertex.isValid()) continue;
                GlobalError gigibs=kvfbsvertex.positionError();
                double vtxprob_Bs = TMath::Prob(vertexbskalman.chi2(),(int)vertexbskalman.ndof());
                if (vtxprob_Bs < 1e-5) continue;
                dcv.BsVtxProb = vtxprob_Bs;
                //std::cout<<"vtxprob_Bs: "<<vtxprob_Bs<<"\n";
                KinematicConstrainedFit BCandFitter;
                bool fitSuccess = BCandFitter.TrippleObjectVertexFitConvertedPhoton(ttrk_muons, nominalMuonMass, tttrk_electrons, nominalElectronMass);
                if (!fitSuccess) continue;
		        //std::cout<<"print the fit sucess : "<< fitSuccess<< "\n";
                dcv.fittedBmass = BCandFitter.getBhadronMass();
                dcv.BsMass = BCand.M();
                dcv.BsPt   = BCand.Pt();
                dcv.BsEta  = BCand.Eta();
                dcv.BsPhi  = BCand.Phi();
                RefCountedKinematicParticle bs = BCandFitter.getBhardon();
	  		    RefCountedKinematicVertex bVertex = BCandFitter.getVertex();
	  		    AlgebraicVector7 b_par = bs->currentState().kinematicParameters().vector();
                GlobalVector Bsvec(b_par[3], b_par[4], b_par[5]);
                //std::cout<<"Vertex position after the fit  "<< Bsvec.x() << "\t"<< Bsvec.y() << "\t"<< Bsvec.z() << "\n";
                reco::Vertex recVtxs;
                //std::cout << " the PV multiplicity returen in the TBV class : " << bsAndVtxInfo.VtxIndex<< "\n";
                reco::Vertex PVvtxHightestPt;//:wq = recVtxs[bsAndVtxInfo.VtxIndex];
                /*Need input to solve the problem of multiple primary vertex*/
                //std::cout<<"Primary vertex HightestPt"<<PVvtxHightestPt.x()<< "\t"<<PVvtxHightestPt.y()<< "\t"<<PVvtxHightestPt.z() <<"\n";
                
                dcv.BsCt3D = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y()+
                (kvfbsvertex.position().z()-PVvtxHightestPt.z())*Bsvec.z())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());
                //std::cout << " the decay time 3D : " << dcv.BsCt3D << "\n";
                dcv.BsCt2D = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
                //std::cout << " the decay time 2D : " << dcv.BsCt3D << "\n";
                dcv.BsCt2DBS = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-bsAndVtxInfo.bs_x)*Bsvec.x()+
                (kvfbsvertex.position().y()-bsAndVtxInfo.bs_y)*Bsvec.y())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
                //std::cout << " the decay time 2D BS : " << dcv.BsCt3D << "\n";
                }//end of conversion loop
            
        
            for (size_t i = 0; i < photons.size(); ++i) {
	    dcv.vertexFitFlag = 2;
            const reco::Photon& photon = photons[i];
            if (photon.superCluster().isNull()) continue;
            if (photon.superCluster()->energy() < 1.0) continue; // Minimum energy cut for photons
            if (photon.isEB() && photon.superCluster()->eta() < -2.5) continue; // Exclude barrel photons with eta < -2.5
            if (photon.isEE() && photon.superCluster()->eta() > 2.5) continue; // Exclude endcap photons with eta > 2.5
            TLorentzVector BCand, photonvec, muonTrack1, muonTrack2;
            photonvec.SetPtEtaPhiE(photon.pt(), photon.eta(), photon.phi(), photon.energy());
            muonTrack1.SetPtEtaPhiE(mu1.pt(), mu1.eta(), mu1.phi(), mu1.energy());
            muonTrack2.SetPtEtaPhiE(mu2.pt(), mu2.eta(), mu2.phi(), mu2.energy());
            BCand = photonvec + muonTrack1 + muonTrack2;
            MassLimits m_lim;
            if (BCand.M() < m_lim.BsMassCutLower || BCand.M() > m_lim.BsMassCutUpper) continue;
            //std::cout<<"mass B: "<<BCand.M()<<"\n";
            //std::vector<reco::TransientTrack> ttrk_photons = {reco::TransientTrack(photon.superCluster()->seed().trackRef(), &bField)};
            GlobalPoint vertexPostion(bsAndVtxInfo.pv_x, bsAndVtxInfo.pv_y, bsAndVtxInfo.pv_z);
            GlobalVector vertexDirection(photon.px(), photon.py(), photon.pz());
            TrackCharge photon_charge = 0; 
            FreeTrajectoryState photonFTS = FreeTrajectoryState(vertexPostion, vertexDirection, photon_charge, &bField);
            std::cout<<"Photon FTS: "<<photonFTS.position().x()<<"\t"<<photonFTS.position().y()<<"\t"<<photonFTS.position().z()<<"\n";
            reco::TransientTrack  transientrackforPhotn = transientTrackBuilder.build(photonFTS);
            std::vector<reco::TransientTrack>  photonTT = {transientrackforPhotn};
            //if (!photonTT[0].isValid()) continue;
            TMatrixD cov(lazyTools.covariancesXYZ(*photon.superCluster()));
            TMatrixD* covPtr(new TMatrixD(cov));
            AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
            AlgebraicVector6 diagonal(1., 1., 1., 1., 1., 1.);
            photonCov.SetDiagonal(diagonal);
            CartesianTrajectoryError photonErr(photonCov);
            photonFTS.setCartesianError(photonErr);
            std::vector<reco::TransientTrack> t_tracks_RecPhoton;
            t_tracks_RecPhoton.push_back(transientrackforPhotn);
            t_tracks_RecPhoton.push_back(muonTT1);
            t_tracks_RecPhoton.push_back(muonTT2);
            KalmanVertexFitter kvfbs(true);
            TransientVertex kvfbsvertex = kvfbs.vertex(t_tracks_RecPhoton);
            if (!kvfbsvertex.isValid()) continue;
            reco::Vertex vertexbskalman = kvfbsvertex;
            GlobalError gigibs=kvfbsvertex.positionError();
            double vtxprob_Bs = TMath::Prob(vertexbskalman.chi2(),(int)vertexbskalman.ndof());
            if (vtxprob_Bs < 1e-5) continue;
            dcv.BsVtxProb = vtxprob_Bs;
            std::cout<<"RecoPhoton : vtxprob_Bs: "<<vtxprob_Bs<<"\n";
            KinematicConstrainedFit BCandFitter;
            bool fitSuccess = BCandFitter.TrippleObjectVertexFitRecoPhoton(ttrk_muons, photonTT, dcv.dimuonMass, 0.001, photons, *covPtr);
            if (!fitSuccess) continue;
            //std::cout<<"print the fit sucess : "<< fitSuccess<< "\n";
            dcv.fittedBmass = BCandFitter.getBhadronMass();
            dcv.BsMass = BCand.M();
            dcv.BsPt   = BCand.Pt();
            dcv.BsEta  = BCand.Eta();
            dcv.BsPhi  = BCand.Phi();
            RefCountedKinematicParticle bs = BCandFitter.getBhardon();
            RefCountedKinematicVertex bVertex = BCandFitter.getVertex();
            AlgebraicVector7 b_par = bs->currentState().kinematicParameters().vector();
            GlobalVector Bsvec(b_par[3], b_par[4], b_par[5]);
            std::cout<<"Vertex position after the fit  "<< Bsvec.x() << "\t"<< Bsvec.y() << "\t"<< Bsvec.z() << "\n";
            reco::Vertex recVtxs;
            //std::cout << " the PV multiplicity returen in the TBV class : " << bsAndVtxInfo.VtxIndex<< "\n";
            reco::Vertex PVvtxHightestPt;//:wq = recVtxs[bsAndVtxInfo.VtxIndex];
            //Need input to solve the problem of multiple primary vertex
            //std::cout<<"Primary vertex HightestPt"<<PVvtxHightestPt.x()<< "\t"<<PVvtxHightestPt.y()<< "\t"<<PVvtxHightestPt.z() <<"\n";
            dcv.BsCt3D = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
            (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y()+
            (kvfbsvertex.position().z()-PVvtxHightestPt.z())*Bsvec.z())/
            (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());
            //std::cout << " the decay time 3D : " << dcv.BsCt3D << "\n";
            dcv.BsCt2D = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
            (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y())/
            (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
            //std::cout << " the decay time 2D : " << dcv.BsCt3D << "\n";
            dcv.BsCt2DBS = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-bsAndVtxInfo.bs_x)*Bsvec.x()+
            (kvfbsvertex.position().y()-bsAndVtxInfo.bs_y)*Bsvec.y())/
            (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());  
            std::cout << " the decay time 2D BS : " << dcv.BsCt3D << "\n";
            }//end of reco photon loop 
        

        
	}
    }
    return dcv;


} //Here is the brace of the main funtion 
