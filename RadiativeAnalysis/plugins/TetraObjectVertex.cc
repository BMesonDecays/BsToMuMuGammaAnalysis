//Author : Muhammad Alibordi, University of Warsaw
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TetraObjectVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/VariableDefinitions.h"



TetraObjectVertex::TetraObjectVertex(){}
DecayChainVariables TetraObjectVertex::TetraObjectVertexObservables(
                    const std::vector<reco::Muon>& muons, 
		    const std::vector<reco::Photon>& photons,
		    const std::vector<reco::Vertex>& PVs,
                    const EcalClusterLazyTools& lazyTools,
			const pat::CompositeCandidateCollection& conversions,
			const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
			const MagneticField& bField,
			const double nominalMuonMass,
			const double nominalElectronMass, 
		 	const TransientTrackBuilder& transientTrackBuilder,
      RadiativeRootTree* rrt){
                bool verbose = true;
                DecayChainVariables dcv;

    for(size_t i=0; i < muons.size(); ++i){
	    const pat::Muon & mu1 = muons[i];
                if (mu1.innerTrack().isNull())continue;
                reco::TrackRef muTrack1 = mu1.track();
                if(!muTrack1) continue;
                
                for (size_t j=i+1; j < muons.size(); ++j){
                        const pat::Muon & mu2 = muons[j];
                        if (mu2.innerTrack().isNull())continue;
                        if(mu1.charge()*mu2.charge() ==1 ) continue;
                        reco::TrackRef muTrack2 = mu2.track();
                        if(!muTrack2) continue;
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
                        rrt->DiMuonMass_Jpsi_ = resonanceResult.mass;
                        rrt->DiMuonEta_Jpsi_ = resonanceResult.eta;
                        rrt->DiMuonPhi_Jpsi_ = resonanceResult.phi;
                        rrt->DiMuonPt_Jpsi_ = resonanceResult.pt;
                        }
                        reco::TransientTrack muonTT1 = reco::TransientTrack(muTrack1, &bField);
                        reco::TransientTrack muonTT2 = reco::TransientTrack(muTrack2, &bField);
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
      // start of the conversion loop
      //
			for (const auto& conv1 : conversions) {
                dcv.vertexFitFlag_mmconvgg = 3;
                TLorentzVector BCand, eleTrack1, eleTrack2, eleTrack3, eleTrack4, muonTrack1, muonTrack2, diphoton4vec;
                const reco::Track eletk0 = *conv1.userData<reco::Track>("track0");
                const reco::Track eletk1 = *conv1.userData<reco::Track>("track1");

                eleTrack1.SetPtEtaPhiM(eletk0.pt(), eletk0.eta(), eletk0.phi(), nominalElectronMass);
                eleTrack2.SetPtEtaPhiM(eletk1.pt(), eletk1.eta(), eletk1.phi(), nominalElectronMass);
                
				std::vector<reco::TransientTrack> tttrk_electrons = {
					reco::TransientTrack(*conv1.userData<reco::Track>("track0"), &bField),
					reco::TransientTrack(*conv1.userData<reco::Track>("track1"), &bField)
				};
        
				for (const auto& conv2 : conversions) {
                    const reco::Track eletk0 = *conv2.userData<reco::Track>("track0");
                    const reco::Track eletk1 = *conv2.userData<reco::Track>("track1");
                    eleTrack3.SetPtEtaPhiM(eletk0.pt(), eletk0.eta(), eletk0.phi(), nominalElectronMass);
                    eleTrack4.SetPtEtaPhiM(eletk1.pt(), eletk1.eta(), eletk1.phi(), nominalElectronMass);
					if (&conv1 == &conv2) continue;  // Ensure conv1 and conv2 are distinct
					std::vector<reco::TransientTrack> tttrk_electrons_pair = tttrk_electrons;  // Copy base tracks
					tttrk_electrons_pair.emplace_back(*conv2.userData<reco::Track>("track0"), &bField);
					tttrk_electrons_pair.emplace_back(*conv2.userData<reco::Track>("track1"), &bField);

                diphoton4vec = eleTrack1 + eleTrack2 + eleTrack3 + eleTrack4;
                rrt->FourvectorConvGammaPt_mmconvgg_  = diphoton4vec.Pt();
                rrt->FourvectorConvGammaEta_mmconvgg_ = diphoton4vec.Eta();
                rrt->FourvectorConvGammaPhi_mmconvgg_ = diphoton4vec.Phi();
                muonTrack1.SetPtEtaPhiM(mu1.pt(), mu1.eta(), mu1.phi(), nominalMuonMass);
                muonTrack2.SetPtEtaPhiM(mu2.pt(), mu2.eta(), mu2.phi(), nominalMuonMass);
                BCand = eleTrack1 + eleTrack2 + eleTrack3 + eleTrack4 + muonTrack1 + muonTrack2;
                MassLimits m_lim;
                if (BCand.M() < m_lim.BsMassCutLower || BCand.M() > m_lim.BsMassCutUpper) continue;
                rrt->FourvectorBsMass_mmconvgg_ = BCand.M();
                rrt->FourvectorBsPt_mmconvgg_   = BCand.Pt();
                rrt->FourvectorBsEta_mmconvgg_  = BCand.Eta();
                rrt->FourvectorBsPhi_mmconvgg_  = BCand.Phi();
                
					KinematicConstrainedFit BCandFitter;
					bool fitSuccess = BCandFitter.TetraObjectVertexFitConvertedPhoton(ttrk_muons, nominalMuonMass, tttrk_electrons_pair, nominalElectronMass);
					if (!fitSuccess) continue;
					rrt->VertexfitBsMass_mmconvgg_ = BCandFitter.getBhadronMass();
                    RefCountedKinematicParticle bs = BCandFitter.getBhardon();
	  	        	RefCountedKinematicVertex bVertex = BCandFitter.getVertex();
	  		        AlgebraicVector7 b_par = bs->currentState().kinematicParameters().vector();
                    GlobalVector Bsvec(b_par[3], b_par[4], b_par[5]);

                    GlobalError gigibs=bVertex->error();
                    double vtxprob_Bs = TMath::Prob(bVertex->chiSquared(),(int)bVertex->degreesOfFreedom());
                    if (vtxprob_Bs < 1e-5) continue;
                    rrt->BsVtxProb_mmconvgg_ = vtxprob_Bs;

                //std::cout<<"Vertex position after the fit  "<< Bsvec.x() << "\t"<< Bsvec.y() << "\t"<< Bsvec.z() << "\n";
                reco::Vertex PVvtxHightestPt = PVs[bsAndVtxInfo.VtxIndex];
		        //std::cout<<" Tetra Object : Primary vertex HightestPt"<<PVvtxHightestPt.x()<< "\t"<<PVvtxHightestPt.y()<< "\t"<<PVvtxHightestPt.z() <<"\n";
             
                rrt->BsCt3D_mmconvgg_ = m_lim.BsPDGMass*( (bVertex->position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                (bVertex->position().y()-PVvtxHightestPt.y())*Bsvec.y()+
                (bVertex->position().z()-PVvtxHightestPt.z())*Bsvec.z())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y()+Bsvec.z()*Bsvec.z());

                rrt->BsCt2D_mmconvgg_ = m_lim.BsPDGMass*( (bVertex->position().x()-PVvtxHightestPt.x())*Bsvec.x()+
                (bVertex->position().y()-PVvtxHightestPt.y())*Bsvec.y())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());

                rrt->BsCt2DBS_mmconvgg_ = m_lim.BsPDGMass*( (bVertex->position().x()-bsAndVtxInfo.bs_x)*Bsvec.x()+
                (bVertex->position().y()-bsAndVtxInfo.bs_y)*Bsvec.y())/
                (Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y());
				}//converted photon loop 1 end 
			}// converted photon loop 2 end
     

// Double photon loop
for (size_t i = 0; i < photons.size(); ++i) {
    const reco::Photon& photon1 = photons[i];
    if (photon1.superCluster().isNull()) continue;
    if (photon1.superCluster()->energy() < 1.0) continue;
    if (photon1.isEB() && photon1.superCluster()->eta() < -2.5) continue;
    if (photon1.isEE() && photon1.superCluster()->eta() > 2.5) continue;
    //std::cout << "Processing photon pair with photon1 pt: " << photon1.pt() << "\n" << ", eta: " << photon1.eta() <<"\n"<< ", phi: " << photon1.phi() << "\n";

    for (size_t j = i + 1; j < photons.size(); ++j) {
        const reco::Photon& photon2 = photons[j];
        if (photon2.superCluster().isNull()) continue;
        if (photon2.superCluster()->energy() < 1.0) continue;
        if (photon2.isEB() && photon2.superCluster()->eta() < -2.5) continue;
        if (photon2.isEE() && photon2.superCluster()->eta() > 2.5) continue;

        rrt->FourvectorGamma1Pt_mmrecogg_ = photon1.pt();
        rrt->FourvectorGamma1Eta_mmrecogg_ = photon1.eta();
        rrt->FourvectorGamma1Phi_mmrecogg_ = photon1.phi();

        rrt->FourvectorGamma2Pt_mmrecogg_ = photon2.pt();
        rrt->FourvectorGamma2Eta_mmrecogg_ = photon2.eta();
        rrt->FourvectorGamma2Phi_mmrecogg_ = photon2.phi();

        dcv.vertexFitFlag_mmrecogg = 4;  // flag for 2-photon case

        TLorentzVector photonvec1, photonvec2, muonTrack1, muonTrack2, BCand;
        photonvec1.SetPtEtaPhiE(photon1.pt(), photon1.eta(), photon1.phi(), photon1.energy());
        photonvec2.SetPtEtaPhiE(photon2.pt(), photon2.eta(), photon2.phi(), photon2.energy());
        muonTrack1.SetPtEtaPhiE(mu1.pt(), mu1.eta(), mu1.phi(), mu1.energy());
        muonTrack2.SetPtEtaPhiE(mu2.pt(), mu2.eta(), mu2.phi(), mu2.energy());

        BCand = photonvec1 + photonvec2 + muonTrack1 + muonTrack2;
        //std::cout << "Bs candidate mass: " << BCand.M() << "\n";
        MassLimits m_lim;
        if (BCand.M() < m_lim.BsMassCutLower || BCand.M() > m_lim.BsMassCutUpper) continue;
        //std::cout << "Bs candidate mass after cuts: " << BCand.M() << "\n";
        // ΔR between each photon and the dimuon system
        TLorentzVector dimuon = muonTrack1 + muonTrack2;
        rrt->DeltaRPhoton1DiMuon_mmrecogg_ = deltaR(dimuon.Eta(), dimuon.Phi(), photon1.eta(), photon1.phi());
        rrt->DeltaRPhoton2DiMuon_mmrecogg_ = deltaR(dimuon.Eta(), dimuon.Phi(), photon2.eta(), photon2.phi());
        rrt->DeltaRPhoton1Photon2_mmrecogg_ = deltaR(photon1.eta(), photon1.phi(), photon2.eta(), photon2.phi());
        //std::cout << "ΔR between photon1 and dimuon: " << rrt->DeltaRPhoton1DiMuon_mmrecogg_ << "\n";
        //std::cout << "ΔR between photon2 and dimuon: " << rrt->DeltaRPhoton2DiMuon_mmrecogg_ << "\n";
        //std::cout << "ΔR between photon1 and photon2: " << rrt->DeltaRPhoton1Photon2_mmrecogg_ << "\n";
        std::vector<reco::TransientTrack> ttrk_photons;
        TMatrixD* covPtr;
        //std::vector<std::unique_ptr<TMatrixD>> covPtrs;
        std::vector<TMatrixD*> covPtrs;

        for (const auto& photon : {photon1, photon2}) {
            GlobalPoint vertexPostion(bsAndVtxInfo.pv_x, bsAndVtxInfo.pv_y, bsAndVtxInfo.pv_z);
            GlobalVector vertexDirection(photon.px(), photon.py(), photon.pz());
            TrackCharge photon_charge = 0; 
            FreeTrajectoryState photonFTS(vertexPostion, vertexDirection, photon_charge, &bField);

            // Optional debugging
            // std::cout << "Photon FTS: " 
            // << photonFTS.position().x() 
            // << "\t"<< photonFTS.position().y() 
            // << "\t" << photonFTS.position().z() <<"\n";

            // Photon covariance and error
            TMatrixD cov(lazyTools.covariancesXYZ(*photon.superCluster()));
            //covPtrs.push_back(std::make_unique<TMatrixD>(cov));
            covPtr = new TMatrixD(cov);
            covPtrs.push_back(covPtr);
            AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
            AlgebraicVector6 diagonal(1e6, 1e6, 1e6, 1e6, 1e6, 1e6);
            photonCov.SetDiagonal(diagonal);
            CartesianTrajectoryError photonErr(photonCov);
            photonFTS.setCartesianError(photonErr);

            reco::TransientTrack transientrackforPhoton = transientTrackBuilder.build(photonFTS);

            ttrk_photons.push_back(transientrackforPhoton);
        }

        GlobalPoint SVKalman(tv.position().x(), tv.position().y(), tv.position().z());
        GlobalVector Bsp3sum = GlobalVector(BCand.Px(), BCand.Py(), BCand.Pz());
        int lowestDCAIndex = BeamSpotAndVertex::LowestDCAIndex(PVs, SVKalman, Bsp3sum);
        reco::Vertex PVlowestDCA = PVs[lowestDCAIndex];

        KinematicConstrainedFit BCandFitter;
        bool fitSuccess = BCandFitter.TetraObjectVertexFitRecoPhoton(ttrk_muons, ttrk_photons, resonanceResult, {photon1, photon2}, covPtrs, PVlowestDCA, bField, transientTrackBuilder);
        if (!fitSuccess) continue;
        std::cout << " the the Bhdrom mass after the fit  " << BCandFitter.getBhadronMass() << "       fit success: " << fitSuccess << "\n";
        rrt->VertexfitBsMass_mmrecogg_ = BCandFitter.getBhadronMass();
        rrt->FourvectorBsMass_mmrecogg_ = BCand.M();
        rrt->FourvectorBsPt_mmrecogg_   = BCand.Pt();
        rrt->FourvectorBsEta_mmrecogg_  = BCand.Eta();
        rrt->FourvectorBsPhi_mmrecogg_  = BCand.Phi();

        RefCountedKinematicParticle bs = BCandFitter.getBhardon();
        RefCountedKinematicVertex bVertex = BCandFitter.getVertex();
        AlgebraicVector7 b_par = bs->currentState().kinematicParameters().vector();
        GlobalVector Bsvec(b_par[3], b_par[4], b_par[5]);

        double vtxprob_Bs = TMath::Prob(bVertex->chiSquared(), (int)bVertex->degreesOfFreedom());
        if (vtxprob_Bs < 1e-5) continue;
        rrt->BsVtxProb_mmrecogg_ = vtxprob_Bs;

        // 3D and 2D decay length using PV and BS
        reco::Vertex PVvtxHightestPt = PVs[bsAndVtxInfo.VtxIndex];


        rrt->BsCt3D_mmrecogg_ = VariableDefinitions::calculateCt3D(PVlowestDCA, bVertex, bs);

        rrt->BsCt2D_mmrecogg_ = VariableDefinitions::calculateCt2D(PVlowestDCA, bVertex, bs);

        rrt->VertexfitBsCtErr2D_mmrecogg_ = VariableDefinitions::calculateCt2DError(PVlowestDCA, bVertex, bs);

        rrt->VertexfitBsCtErr3D_mmrecogg_ = VariableDefinitions::calculateCt3DError(PVlowestDCA, bVertex, bs);

        rrt->BsCt2DBS_mmrecogg_ = m_lim.BsPDGMass * ((bVertex->position().x() - bsAndVtxInfo.bs_x) * Bsvec.x() +
                                          (bVertex->position().y() - bsAndVtxInfo.bs_y) * Bsvec.y()) /
                                          (Bsvec.x()*Bsvec.x() + Bsvec.y()*Bsvec.y());

    }//reco gamma 2
}//reco gamma 1




}//end of muon loop 1
}//end of muon loop 2
    return dcv;
}