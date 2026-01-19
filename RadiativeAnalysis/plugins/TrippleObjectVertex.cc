//Author : Muhammad Alibordi, University of Warsaw
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TrippleObjectVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BaselineMatching.h"


TrippleObjectVertex::TrippleObjectVertex(){}
DecayChainVariables TrippleObjectVertex::TrippleObjectVertexObservables(
    //removing the qualifier TrippleObjectVertex fron the defintion of DecayChainVariables since it is not anymore a member of such class
    const std::vector<reco::Muon>& muons,
    const std::vector<reco::Photon>& photons,
     const std::vector<reco::Vertex>& PVs,
    const EcalClusterLazyTools& lazyTools,
    const pat::CompositeCandidateCollection& conversions,
    const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
    const MagneticField& bField,
    const double nominalMuonMass,
    const double nominalElectronMass,
    const TransientTrackBuilder& transientTrackBuilder) {
        
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
            int flag = static_cast<int>(resonanceResult.resonanceFlag);
            if (flag == 1) {
                dcv.dimuonMass_Jpsi = resonanceResult.mass;dcv.dimuonEta_Jpsi = resonanceResult.eta;
                dcv.dimuonPhi_Jpsi = resonanceResult.phi;dcv.dimuonPt_Jpsi = resonanceResult.pt;// Jpsi
            }
            else if (flag == 2) {  
                dcv.dimuonMass_Phi = resonanceResult.mass;dcv.dimuonEta_Phi = resonanceResult.eta;
                dcv.dimuonPhi_Phi = resonanceResult.phi;dcv.dimuonPt_Phi = resonanceResult.pt;// Phi
            }
            else if (flag == 3) {  
                dcv.dimuonMass_Kstar0 = resonanceResult.mass;dcv.dimuonEta_Kstar0 = resonanceResult.eta;
                dcv.dimuonPhi_Kstar0 = resonanceResult.phi;dcv.dimuonPt_Kstar0 = resonanceResult.pt;// KStar
            }
            else if (flag == 4) {  
                dcv.dimuonMass_NoBound = resonanceResult.mass;dcv.dimuonEta_NoBound = resonanceResult.eta;
                dcv.dimuonPhi_NoBound = resonanceResult.phi;dcv.dimuonPt_NoBound = resonanceResult.pt;// NonResonant
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
           if (mu1TS.isValid() && mu2TS.isValid()) {
            auto par1 = mu1TS.perigeeParameters();
            auto par2 = mu2TS.perigeeParameters();
            AlgebraicVector5 p1 = par1.vector();
            AlgebraicVector5 p2 = par2.vector();
            AlgebraicSymMatrix55 C1 = mu1TS.perigeeError().covarianceMatrix();
            AlgebraicSymMatrix55 C2 = mu2TS.perigeeError().covarianceMatrix();

            AlgebraicSymMatrix55 Csum = C1 + C2;
            AlgebraicVector5 diff = p1 - p2;
            bool invertible = Csum.Invert();
            if (invertible) {
                double md2 = ROOT::Math::Similarity(diff, Csum);
                if (md2 >= 0.0)dcv.mahalanobis = std::sqrt(md2);
                //std::cout<< " Mahalanobis distance ------------------------------------------------------------------- : "<< dcv.mahalanobis << "\n";
            }
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

        // Just check the consistency of the assigment and sorting the charge - Alibordi
        
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






// loops for converted photon and reco supposed to be mutually exclusive , 
// and we need proper flags so that the kinematics must not get overwritten one by another - Alibordi
// Tetra object requires same treatement , then the additional variable for the soft muoin MVA ID in the ntuple 
// Trigger filter matching delta and flag to have a surface level handle of estimating trigger efficiency - 8th Dec 2025 
//



      
        for (const auto& conv : conversions) {
            dcv.vertexFitFlag_mmconvg = 1;
            //std::cout<<" The vertex fit flag is set to 1 for the converted photons : "<<dcv.vertexFitFlag<<"\n";
                const reco::Track eletk0 = *conv.userData<reco::Track>("track0");
                const reco::Track eletk1 = *conv.userData<reco::Track>("track1");
                std::vector<reco::TransientTrack> tttrk_electrons = {
                    reco::TransientTrack(eletk0, &bField),
                    reco::TransientTrack(eletk1, &bField)
                };
                TLorentzVector BCand, eleTrack1, eleTrack2, muonTrack1, muonTrack2, photon4vec;
                eleTrack1.SetPtEtaPhiM(eletk0.pt(), eletk0.eta(), eletk0.phi(), nominalElectronMass);
                eleTrack2.SetPtEtaPhiM(eletk1.pt(), eletk1.eta(), eletk1.phi(), nominalElectronMass);
                photon4vec = eleTrack1 + eleTrack2;
                muonTrack1.SetPtEtaPhiM(mu1.pt(), mu1.eta(), mu1.phi(), nominalMuonMass);
                muonTrack2.SetPtEtaPhiM(mu2.pt(), mu2.eta(), mu2.phi(), nominalMuonMass);
                BCand = eleTrack1 + eleTrack2 + muonTrack1 + muonTrack2;
                MassLimits m_lim;
                dcv.fourvectorconvgammapt_mmconvg  = photon4vec.Pt();
                dcv.fourvectorconvgammaeta_mmconvg = photon4vec.Eta();
                dcv.fourvectorconvgammaphi_mmconvg = photon4vec.Phi();
                if (BCand.M() < m_lim.BsMassCutLower || BCand.M() > m_lim.BsMassCutUpper) continue;
                //std::cout<<"mass B Converted Case-------------------------------------------------------------: "<<BCand.M()<<"\n";
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
                
                reco::Vertex vertexbskalman = kvfbsvertex;
                if (!kvfbsvertex.isValid()) continue;
                GlobalError gigibs=kvfbsvertex.positionError();
                double vtxprob_Bs = TMath::Prob(vertexbskalman.chi2(),(int)vertexbskalman.ndof());
                if (vtxprob_Bs < 1e-5) continue;
                dcv.bsvtxprob_mmconvg = vtxprob_Bs;
                KinematicConstrainedFit BCandFitter;
                bool fitSuccess = BCandFitter.TrippleObjectVertexFitConvertedPhoton(ttrk_muons, nominalMuonMass, tttrk_electrons, nominalElectronMass, verbose);
                if (!fitSuccess) continue;
		        //std::cout<<"print the fit sucess with converted photons  : "<< fitSuccess<< "\n";
                dcv.fittedBmassConvertedPhoton = BCandFitter.getBhadronMass();
                //std::cout<<"fitted B mass from vertex -------------------------------------------------: "<<dcv.fittedBmassConvertedPhoton<<"\n";
                dcv.fourvectorbsmass_mmconvg = BCand.M();
                dcv.fourvectorbspt_mmconvg   = BCand.Pt();
                dcv.fourvectorbseta_mmconvg  = BCand.Eta();
                dcv.fourvectorbsphi_mmconvg  = BCand.Phi();
                RefCountedKinematicParticle bs = BCandFitter.getBhardon();
	  		    RefCountedKinematicVertex bVertex = BCandFitter.getVertex();
	  		    AlgebraicVector7 b_par = bs->currentState().kinematicParameters().vector();
                GlobalVector Bsvec(b_par[3], b_par[4], b_par[5]);
                reco::Vertex PVvtxHightestPt = PVs[bsAndVtxInfo.VtxIndex];
	  		    // AlgebraicSymMatrix77 bs_er = bs->currentState().kinematicParametersError().matrix(); // unused variable
                AlgebraicMatrix33 BVError(bVertex->error().matrix());
                dcv.vertexfitbsmass_mmconvg = b_par[6];
                dcv.vertexfitbspt_mmconvg = Bsvec.perp();
                dcv.vertexfitbseta_mmconvg = Bsvec.eta();
                dcv.vertexfitbsphi_mmconvg = Bsvec.phi();
                dcv.vertexfitbspz_mmconvg = Bsvec.z();
                TMatrix cova(2,2);
                cova.IsSymmetric();
                cova(0,0)=gigibs.cxx();
                cova(1,1)=gigibs.cyy();
                cova(0,1)=gigibs.cyx();
                cova(1,0)=gigibs.cyx();


                

                


                //  Primary vertex methods from old Giacomos code - Alibordi 8th Dec 2025


                dcv.bsct3d_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y()+
                (kvfbsvertex.position().z()-PVvtxHightestPt.z())*Bsvec.z())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());
                
                dcv.bsct2d_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
                std::cout << " the decay time 2D -------------------------------------------------------------------------: " << dcv.bsct2d_mmconvg << "\n";
                dcv.bsct2dbs_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-bsAndVtxInfo.bs_x)*Bsvec.x()+
                (kvfbsvertex.position().y()-bsAndVtxInfo.bs_y)*Bsvec.y())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
                
                Int_t PVCosThetaIndex = -1;
                Int_t PVClosestZIndex = -1;
                double MinDistance  = std::numeric_limits<double>::max();
	            double MinDistanceZ = std::numeric_limits<double>::max();
                const double bsZ = kvfbsvertex.position().z();
                for (std::size_t i = 0; i < PVs.size(); ++i) {
                    const auto& vtx = PVs[i];
                    if (!vtx.isValid()) continue;
                    const double dz = std::abs(bsZ - vtx.z());
                    if (dz < MinDistanceZ) {
                        MinDistanceZ = dz;
                        PVClosestZIndex = static_cast<int>(i);
                    }
                    Double_t PVSVvecDotBsPvec=(kvfbsvertex.position().x()-vtx.x())*Bsvec.x()+(kvfbsvertex.position().y()-vtx.y())*Bsvec.y()+(kvfbsvertex.position().z()-vtx.z())*Bsvec.z();
                    Double_t PVSVlength = TMath::Sqrt( pow((kvfbsvertex.position().x()- vtx.x()), 2.0) + pow((kvfbsvertex.position().y()-vtx.y()), 2.0) + pow((kvfbsvertex.position().z()- vtx.z()), 2.0) );
                    Double_t BsPlength = TMath::Sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y() + Bsvec.z()*Bsvec.z());
                    Double_t BsCosTheta = PVSVvecDotBsPvec / (BsPlength * PVSVlength);
                    Double_t distance = 1-BsCosTheta;
                    if(distance < MinDistance){
                        MinDistance = distance;
                        PVCosThetaIndex = static_cast<int>(i);
                        std::cout << " The index of the primary vertex with cos theta method : " << PVCosThetaIndex << "\n";
                    }

                }
                if (PVCosThetaIndex==-1) continue;
                if (PVClosestZIndex==-1) continue;
                BsPVVtxInd = PVCosThetaIndex;
                reco::Vertex PVvtxCosTheta = PVs[PVCosThetaIndex];
                reco::Vertex PVvtxClosestZ = PVs[PVClosestZIndex]; 

                std::cout << " The index of the primary vertex with cos theta method : " << BsPVVtxInd << "\n";
                if (BsPVVtxInd >= 0 && BsPVVtxInd < static_cast<int>(PVs.size())) {
                    const reco::Vertex& bsPV = PVs[BsPVVtxInd];
                    std::cout<< " number of tracks :               " << bsPV.nTracks() << "\n";
                    PVvtxCosTheta  = bsPV;
                }
                


                /*std::vector<reco::TransientTrack> pvRefitTracks;
                for (auto trkRef : PVvtxCosTheta.tracks()) {
                    if (!trkRef.isNonnull()) continue;
                    if (trkRef.key() == muTrack1.key() ||
                    trkRef.key() == muTrack2.key() ||
                    trkRef.key() == tttrk_electrons[0].track().key() ||
                    trkRef.key() == tttrk_electrons[1].track().key())
                    {
                        continue;
                    }
                    reco::TransientTrack tTrk = transientTrackBuilder.build(trkRef);
                    if (tTrk.isValid()) pvRefitTracks.push_back(tTrk);
                    }
                    reco::BeamSpot vertexBeamSpot(
                        GlobalPoint(bsAndVtxInfo.bs_x, bsAndVtxInfo.bs_y, bsAndVtxInfo.bs_z),
                        bsAndVtxInfo.bs_sigmaZ,
                        bsAndVtxInfo.bs_dxdz,
                        bsAndVtxInfo.bs_dydz
                    );

                    AdaptiveVertexFitter avf;
                    TransientVertex newPV = avf.vertex(pvRefitTracks, vertexBeamSpot);
                    if (newPV.isValid()) PVvtxCosTheta = reco::Vertex(newPV);

                    
                    std::vector<reco::TransientTrack> pvClosestZRefitTracks;
                    for (auto trkRef : PVvtxClosestZ.tracks()) {
                        if (!trkRef.isNonnull()) continue;
                        if (trkRef.key() == muTrack1.key() ||
                        trkRef.key() == muTrack2.key() ||
                        trkRef.key() == tttrk_electrons[0].track().key() ||
                        trkRef.key() == tttrk_electrons[1].track().key()) continue;
                        TransientTrack tTrk = transientTrackBuilder.build(trkRef);
                        if (tTrk.isValid()) pvClosestZRefitTracks.push_back(tTrk);
                    }
                    TransientVertex newClosestZ = avf.vertex(pvClosestZRefitTracks, vertexBeamSpot);
                    if(newClosestZ.isValid()) PVvtxClosestZ = reco::Vertex(newClosestZ);*/
                    dcv.vertexfitPVxrefitcosTheta_mmconvg = PVvtxCosTheta.x();
                    dcv.vertexfitPVyrefitcosTheta_mmconvg = PVvtxCosTheta.y();
                    dcv.vertexfitPVzrefitcosTheta_mmconvg = PVvtxCosTheta.z();
                    dcv.vertexfitPVxrefitclosestZ_mmconvg = PVvtxClosestZ.x();
                    dcv.vertexfitPVyrefitclosestZ_mmconvg = PVvtxClosestZ.y();
                    dcv.vertexfitPVzrefitclosestZ_mmconvg = PVvtxClosestZ.z();

                    dcv.vertexfitBsCt3DPVClosestZ_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxClosestZ.x())*Bsvec.x() + 
                    (kvfbsvertex.position().y()-PVvtxClosestZ.y())*Bsvec.y() + 
                    (kvfbsvertex.position().z()-PVvtxClosestZ.z())*Bsvec.z() )/( Bsvec.x()*Bsvec.x() + Bsvec.y()*Bsvec.y() +
                     Bsvec.z()*Bsvec.z() );

                    dcv.vertexfitBsCt2DPVClosestZ_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxClosestZ.x())*Bsvec.x() + 
                    (kvfbsvertex.position().y()-PVvtxClosestZ.y())*Bsvec.y()  )/( Bsvec.x()*Bsvec.x() + Bsvec.y()*Bsvec.y()  ); 

                    dcv.vertexfitBsCt3DPVCosTheta_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxCosTheta.x())*Bsvec.x() + 
                    (kvfbsvertex.position().y()-PVvtxCosTheta.y())*Bsvec.y() + 
                    (kvfbsvertex.position().z()-PVvtxCosTheta.z())*Bsvec.z() )/( Bsvec.x()*Bsvec.x() + Bsvec.y()*Bsvec.y() + Bsvec.z()*Bsvec.z() );
                    dcv.vertexfitBsCt2DPVCosTheta_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxCosTheta.x())*Bsvec.x() + 
                    (kvfbsvertex.position().y()-PVvtxCosTheta.y())*Bsvec.y() )/( Bsvec.x()*Bsvec.x() + Bsvec.y()*Bsvec.y() );
                    dcv.vertexfitBsCt2DPVCosThetaOld_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxCosTheta.x())*BCand.Px() + 
                    (kvfbsvertex.position().y()-PVvtxCosTheta.y())*BCand.Py() )/( BCand.Px()*BCand.Px() + BCand.Py()*BCand.Py() );
                    dcv.vertexfitBsCt2DPVClosestZOld_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxClosestZ.x())*BCand.Px() + 
                    (kvfbsvertex.position().y()-PVvtxClosestZ.y())*BCand.Py() )/( BCand.Px()*BCand.Px() + BCand.Py()*BCand.Py() );
                    dcv.vertexfitBsCt2DBSOld_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-bsAndVtxInfo.bs_x)*BCand.Px() + 
                    (kvfbsvertex.position().y()-bsAndVtxInfo.bs_y)*BCand.Py() )/( BCand.Px()*BCand.Px() + BCand.Py()*BCand.Py() );  
                    dcv.vertexfitBsCt2DOld_mmconvg = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*BCand.Px() + 
                    (kvfbsvertex.position().y()-PVvtxHightestPt.y())*BCand.Py() )/( BCand.Px()*BCand.Px() + BCand.Py()*BCand.Py() );

                    // Smart lambda for ct error calculation - keeps everything inline
                    auto calculateCtError = [&](const GlobalPoint& refVertex, 
                                               const GlobalError& refVertexErr,
                                               const GlobalVector& momentumVec,
                                               const char* outputVarName) -> double {
                        VertexDistanceXY d;
                        const GlobalPoint mySV(kvfbsvertex.position().x(), kvfbsvertex.position().y(), kvfbsvertex.position().z());
                        const GlobalError mySVErr = kvfbsvertex.positionError();
                        
                        Measurement1D VtxDist = d.distance(VertexState(mySV, mySVErr), VertexState(refVertex, refVertexErr));
                        double VtxDistErr = VtxDist.error();
                        double transmom = std::sqrt(momentumVec.y()*momentumVec.y() + momentumVec.x()*momentumVec.x());
                        
                        if(transmom == 0 || VtxDist.value() == 0) return -1.0; // sentinel value
                        
                        double dx = kvfbsvertex.position().x() - refVertex.x();
                        double dy = kvfbsvertex.position().y() - refVertex.y();
                        double cos = (dx*momentumVec.x() + dy*momentumVec.y()) / (transmom * VtxDist.value());
                        
                        TVector LengthVector(2);
                        LengthVector(0) = dx;
                        LengthVector(1) = dy;
                        
                        double firstTerm2 = std::pow(m_lim.BsPDGMass * VtxDistErr * std::abs(cos) / transmom, 2.);
                        double secondTerm2 = std::pow(m_lim.BsPDGMass * std::abs(cos) / (transmom*transmom), 2.) * cova.Similarity(LengthVector);
                        
                        return std::sqrt(firstTerm2 + secondTerm2);
                    };

                    // Now use it - clean and compact!
                    // Beam spot with fitted momentum

                    // commenting out, because it doesn't work
                    // const GlobalPoint myBeamSpot(bsAndVtxInfo.bs_x, bsAndVtxInfo.bs_y, bsAndVtxInfo.bs_z);
                    // dcv.vertexfitBsCtErr2DBS_mmconvg = calculateCtError(myBeamSpot, bsAndVtxInfo.BeamSpot_cov2d, Bsvec, "BS_fitted");

                    // PV variants with fitted momentum
                    dcv.vertexfitBsCtErr2D_mmconvg = calculateCtError(
                        GlobalPoint(PVvtxHightestPt.x(), PVvtxHightestPt.y(), PVvtxHightestPt.z()),
                        PVvtxHightestPt.covariance(), Bsvec, "PV_highestPt");

                    dcv.vertexfitBsCtErr2DClosestZ_mmconvg = calculateCtError(
                        GlobalPoint(PVvtxClosestZ.x(), PVvtxClosestZ.y(), PVvtxClosestZ.z()),
                        PVvtxClosestZ.covariance(), Bsvec, "PV_closestZ");

                    dcv.vertexfitBsCtErr2DCostheta_mmconvg = calculateCtError(
                        GlobalPoint(PVvtxCosTheta.x(), PVvtxCosTheta.y(), PVvtxCosTheta.z()),
                        PVvtxCosTheta.covariance(), Bsvec, "PV_costheta");

                    // Same vertices with non-fitted momentum
                    GlobalVector BsVecNonFitted(BCand.Px(), BCand.Py(), BCand.Pz());

                    dcv.vertexfitBsCtErr2DOld_mmconvg = calculateCtError(
                        GlobalPoint(PVvtxHightestPt.x(), PVvtxHightestPt.y(), PVvtxHightestPt.z()),
                        PVvtxHightestPt.covariance(), BsVecNonFitted, "PV_highestPt_old");

                    dcv.vertexfitBsCtErr2DClosestZOld_mmconvg = calculateCtError(
                        GlobalPoint(PVvtxClosestZ.x(), PVvtxClosestZ.y(), PVvtxClosestZ.z()),
                        PVvtxClosestZ.covariance(), BsVecNonFitted, "PV_closestZ_old");

                    dcv.vertexfitBsCtErr2DOld_mmconvg = calculateCtError(
                        GlobalPoint(PVvtxCosTheta.x(), PVvtxCosTheta.y(), PVvtxCosTheta.z()),
                        PVvtxCosTheta.covariance(), BsVecNonFitted, "PV_costheta_old");

                    // commenting out, because it doesn't work
                    // dcv.vertexfitBsCtErr2DBSOld_mmconvg = calculateCtError(
                    //     myBeamSpot, bsAndVtxInfo.BeamSpot_cov2d, BsVecNonFitted, "BS_old");

                    
                    AlgebraicMatrix31 pB;
                    pB(0,0) = bs->currentState().globalMomentum().x();
                    pB(1,0) = bs->currentState().globalMomentum().y();
                    pB(2,0) = bs->currentState().globalMomentum().z();

                    AlgebraicMatrix13 pBT;
                    pBT(0,0) = bs->currentState().globalMomentum().x();
                    pBT(0,1) = bs->currentState().globalMomentum().y();
                    pBT(0,2) = bs->currentState().globalMomentum().z();

                    AlgebraicMatrix31 PV;
                    PV(0,0) = PVvtxClosestZ.x();
                    PV(0,1) = PVvtxClosestZ.y();
                    PV(0,2) = PVvtxClosestZ.z(); // not sure if this is what was meant
                    
                    AlgebraicMatrix31 BV;
                    BV(0,0) = bVertex->position().x();
                    BV(0,1) = bVertex->position().y();
                    BV(0,2) = bVertex->position().z();
                    AlgebraicMatrix31 lxyz = BV-PV;
                    AlgebraicMatrix33 PVError(PVvtxHightestPt.error());
                    AlgebraicMatrix33 lxyzError = PVError + BVError;
                    lxyzError.Invert();

                    AlgebraicMatrix11 a = pBT * lxyzError * pB ;
                    AlgebraicMatrix11 b = pBT * lxyzError * lxyz;
                    double num(b(0,0));
                    double deno(a(0,0));
                    dcv.vertexfitBsCtMPVrefit_mmconvg = (num*bs->currentState().mass())/(deno);
                    dcv.vertexfitBsCtErrMPV_mmconvg = m_lim.BsPDGMass/sqrt(deno);



                    GlobalPoint SVpos( bVertex->position().x(), bVertex->position().y(), bVertex->position().z());
                    GlobalPoint PVpos( PVvtxHightestPt.x(), PVvtxHightestPt.y(), PVvtxHightestPt.z());
                    GlobalError SVerr( bVertex->error() );
                    GlobalError PVerr( PVvtxHightestPt.error() );
                    VertexDistance3D dist3D;
                    Measurement1D measurement3D = dist3D.distance(VertexState(SVpos,SVerr),VertexState(PVpos,PVerr));
                    double error3D = measurement3D.error();
                    double scale1 = ((bVertex->position().x() - PVvtxHightestPt.x())*Bsvec.x()+
                    (bVertex->position().y() - PVvtxHightestPt.y())*Bsvec.y()+
                    (bVertex->position().z() - PVvtxHightestPt.z())*Bsvec.z())/
                    (sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z())*
                    sqrt((bVertex->position().x() - PVvtxHightestPt.x())*(bVertex->position().x() - PVvtxHightestPt.x())+
                    (bVertex->position().y() - PVvtxHightestPt.y())*(bVertex->position().y() - PVvtxHightestPt.y())+
                    (bVertex->position().z() - PVvtxHightestPt.z())*(bVertex->position().z() - PVvtxHightestPt.z())));
                    dcv.vertexfitBsCtErr3D_mmconvg = m_lim.BsPDGMass*(error3D*abs(scale1))/sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());

                    dcv.vertexfitBsCt3Drefit_mmconvg = m_lim.BsPDGMass*((bVertex->position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                    +(bVertex->position().y()-PVvtxHightestPt.y())*Bsvec.y()+
                    +(bVertex->position().z()-PVvtxHightestPt.z())*Bsvec.z())/
                    (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());

                    dcv.vertexfitBsCt2Drefit_mmconvg = m_lim.BsPDGMass*((bVertex->position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                    +(bVertex->position().y()-PVvtxHightestPt.y())*Bsvec.y())/
                    (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());


                    AlgebraicMatrix31 pB2;
                    pB2(0,0) = bs->currentState().globalMomentum().x();
                    pB2(1,0) = bs->currentState().globalMomentum().y();
                    pB2(2,0) = bs->currentState().globalMomentum().z();

                    AlgebraicMatrix13 pBT2;
                    pBT2(0,0) = bs->currentState().globalMomentum().x();
                    pBT2(0,1) = bs->currentState().globalMomentum().y();
                    pBT2(0,2) = bs->currentState().globalMomentum().z();

                    AlgebraicMatrix31 PV2;
                    PV2(0,0) = PVvtxHightestPt.x();
                    PV2(0,1) = PVvtxHightestPt.y();
                    PV2(0,2) = PVvtxHightestPt.z();
                    
                    AlgebraicMatrix31 BV2;
                    BV2(0,0) = bVertex->position().x();
                    BV2(0,1) = bVertex->position().y();
                    BV2(0,2) = bVertex->position().z();
                    
                    AlgebraicMatrix31 lxyz2 = BV2 - PV2;
                    AlgebraicMatrix33 PVError2(PVvtxHightestPt.error());
                    AlgebraicMatrix33 BVError2(bVertex->error().matrix());
                    AlgebraicMatrix33 lxyzError2 = PVError2 + BVError2;
                    lxyzError2.Invert();

                    AlgebraicMatrix11 a2 = pBT2 * lxyzError2 * pB2;
                    AlgebraicMatrix11 b2 = pBT2 * lxyzError2 * lxyz2;
                    double num2(b2(0,0));
                    double deno2(a2(0,0));
                    dcv.vertexfitBsCtMPVrefit_mmconvg = (num2 * bs->currentState().mass()) / (deno2);



                    //Error on ctau 3D
                    GlobalPoint SVpos2( bVertex->position().x(), bVertex->position().y(), bVertex->position().z());
                    GlobalPoint PVpos2( PVvtxHightestPt.x(), PVvtxHightestPt.y(), PVvtxHightestPt.z());
                    GlobalError SVerr2( bVertex->error() );
                    GlobalError PVerr2( PVvtxHightestPt.error() );
                    VertexDistance3D dist12;
                    Measurement1D measurement12 = dist12.distance(VertexState(SVpos2,SVerr2),VertexState(PVpos2,PVerr2));
                    double error3D2 = measurement12.error();
                    double scale12 = ((bVertex->position().x() - PVvtxHightestPt.x())*Bsvec.x()+
                             (bVertex->position().y() - PVvtxHightestPt.y())*Bsvec.y()+
                             (bVertex->position().z() - PVvtxHightestPt.z())*Bsvec.z())/(sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z())*
                             sqrt((bVertex->position().x() - PVvtxHightestPt.x())*(bVertex->position().x() - PVvtxHightestPt.x())+
                             (bVertex->position().y() - PVvtxHightestPt.y())*(bVertex->position().y() - PVvtxHightestPt.y())+
                             (bVertex->position().z() - PVvtxHightestPt.z())*(bVertex->position().z() - PVvtxHightestPt.z())));
                    dcv.vertexfitBsCtErr3Drefit_mmconvg = m_lim.BsPDGMass*(error3D2*abs(scale12))/sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());
                    //Error on catu 2D
                    VertexDistanceXY dist22;
                    Measurement1D measurement22 = dist22.distance(VertexState(SVpos2,SVerr2),VertexState(PVpos2,PVerr2));
                    double error2D2 = measurement22.error();
                    // commenting out, because it doesn't work
                    // double scale22 = ((bVertex->position().x() - reFitVertex.x())*Bsvec.x()+
                    // (bVertex->position().y() - reFitVertex.y())*Bsvec.y())/(sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y())*
                    // sqrt((bVertex->position().x() - reFitVertex.x())*(bVertex->position().x() - reFitVertex.x())+
                    // (bVertex->position().y() - reFitVertex.y())*(bVertex->position().y() - reFitVertex.y())));
                    // dcv.vertexfitBsCtErr2Drefit_mmconvg= mlim.BsPDGMass*(error2D2*abs(scale22))/sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
                    if(Bsvec.perp()!=0) {
                        dcv.vertexfitBsLxy_mmconvg = dist22.distance(VertexState(SVpos2,SVerr2),VertexState(PVpos2,PVerr2)).value();
                        dcv.vertexfitBsLxyErr_mmconvg = error2D2;
                    }












                    
            


                   




                    


                    







                    





                //Helicity and acoplanarity calculation for converted photons - in the rest frame of Bs
                TLorentzVector pBs = BCand;
                TVector3 boostVec = -pBs.BoostVector();
                TLorentzVector gammaRF = photon4vec;
                TLorentzVector mu1RF = muonTrack1;
                TLorentzVector mu2RF = muonTrack2;
                gammaRF.Boost(boostVec);
                mu1RF.Boost(boostVec);
                mu2RF.Boost(boostVec);
                
                TVector3 gammaDir = gammaRF.Vect().Unit();
                TVector3 dimuDir  = (mu1RF + mu2RF).Vect().Unit();
                float helicity_conv = gammaDir.Dot(dimuDir);   // = cos(theta*)
                dcv.bshelicity_mmconvg = helicity_conv;
               
                TVector3 n1 = mu1RF.Vect().Cross(mu2RF.Vect()).Unit();
                TVector3 n2 = gammaRF.Vect().Cross(dimuDir).Unit();
                double cosPhi = n1.Dot(n2);
                cosPhi = std::clamp(cosPhi, -1.0, 1.0);
                float acoplanarity_conv = acos(cosPhi);
                dcv.bscoplanarity_mmconvg = acoplanarity_conv;






                
                }//end of conversion loop
            
        

//////////////////////////////////////////////////////////////////
//////////////////// Reco photon loop starts here - Alibordi//////
//////////////////////////////////////////////////////////////////

            for (size_t i = 0; i < photons.size(); ++i) {
            dcv.vertexFitFlag_mmrecog = 2;
            std::cout<<" The vertex fit flag is set to 2 for the reco photons : "<<dcv.vertexFitFlag_mmrecog<<"\n";
            const reco::Photon& photon = photons[i];
            if (photon.superCluster().isNull()) continue;
            if (photon.superCluster()->energy() < 1.0) continue; // Minimum energy cut for photons
            if (photon.isEB() && photon.superCluster()->eta() < -2.5) continue; // Exclude barrel photons with eta < -2.5
            //this barrel cut supposed to be exact - upper line , lower line 
            if (photon.isEE() && photon.superCluster()->eta() > 2.5) continue; // Exclude endcap photons with eta > 2.5
            TLorentzVector BCand, photonvec, muonTrack1, muonTrack2;
            photonvec.SetPtEtaPhiE(photon.pt(), photon.eta(), photon.phi(), photon.energy());
            muonTrack1.SetPtEtaPhiE(mu1.pt(), mu1.eta(), mu1.phi(), mu1.energy());
            muonTrack2.SetPtEtaPhiE(mu2.pt(), mu2.eta(), mu2.phi(), mu2.energy());
            BCand = photonvec + muonTrack1 + muonTrack2;
            MassLimits m_lim;
            if (BCand.M() < m_lim.BsMassCutLower || BCand.M() > m_lim.BsMassCutUpper) continue;
            std::cout<<"mass B Reco Photon Case : "<<BCand.M()<<"\n";
            //std::vector<reco::TransientTrack> ttrk_photons = {reco::TransientTrack(photon.superCluster()->seed().trackRef(), &bField)};
            GlobalPoint vertexPostion(bsAndVtxInfo.pv_x, bsAndVtxInfo.pv_y, bsAndVtxInfo.pv_z);
            GlobalVector vertexDirection(photon.px(), photon.py(), photon.pz());
            TrackCharge photon_charge = 0; 
            FreeTrajectoryState photonFTS = FreeTrajectoryState(vertexPostion, vertexDirection, photon_charge, &bField);
            //std::cout<<"Photon FTS: "<<photonFTS.position().x()<<"\t"<<photonFTS.position().y()<<"\t"<<photonFTS.position().z()<<"\n";
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
            dcv.bsvtxprob_mmrecog = vtxprob_Bs;
            KinematicConstrainedFit BCandFitter;
            bool fitSuccess = BCandFitter.TrippleObjectVertexFitRecoPhoton(ttrk_muons, photonTT, dcv.dimuonMass, 0.001, photons, *covPtr);
            if (!fitSuccess) continue;
            //std::cout<<"print the fit sucess : "<< fitSuccess<< "\n";
            dcv.fittedBmassRecoPhoton = BCandFitter.getBhadronMass();
            std::cout<<"fitted B mass reco photon case: "<<dcv.fittedBmassRecoPhoton<<"\n";
            dcv.fourvectorbsmass_mmrecog = BCand.M();
            dcv.fourvectorbspt_mmrecog   = BCand.Pt();
            dcv.fourvectorbseta_mmrecog  = BCand.Eta();
            dcv.fourvectorbsphi_mmrecog  = BCand.Phi();
            RefCountedKinematicParticle bs = BCandFitter.getBhardon();
            RefCountedKinematicVertex bVertex = BCandFitter.getVertex();
            AlgebraicVector7 b_par = bs->currentState().kinematicParameters().vector();
            GlobalVector Bsvec(b_par[3], b_par[4], b_par[5]);
            reco::Vertex recVtxs;
            reco::Vertex PVvtxHightestPt = PVs[bsAndVtxInfo.VtxIndex];
            dcv.bsct3d_mmrecog = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
            (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y()+
            (kvfbsvertex.position().z()-PVvtxHightestPt.z())*Bsvec.z())/
            (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());
            dcv.bsct2d_mmrecog = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-PVvtxHightestPt.x())*Bsvec.x()+
            (kvfbsvertex.position().y()-PVvtxHightestPt.y())*Bsvec.y())/
            (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
            dcv.bsct2dbs_mmrecog = m_lim.BsPDGMass*( (kvfbsvertex.position().x()-bsAndVtxInfo.bs_x)*Bsvec.x()+
            (kvfbsvertex.position().y()-bsAndVtxInfo.bs_y)*Bsvec.y())/
            (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());  
            }//end of reco photon loop 
            std::cout << " vertex fit flag :"<< dcv.vertexFitFlag_mmrecog << "\n";
        
	}
    }
    return dcv;


} //Here is the brace of the main funtion 
