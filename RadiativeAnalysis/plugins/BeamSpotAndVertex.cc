#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include <limits>



BeamSpotAndVertex::BeamSpotAndVertex(){}
BeamSpotAndVertex::BSAndVtxVariables BeamSpotAndVertex::BSAndVtxObservables(const reco::BeamSpot& beamSpot,
                        const std::vector<reco::Vertex>& vertex){
    BSAndVtxVariables bsvtxv;
    
    for(size_t i=0; i<9;++i){bsvtxv.pv_covariance[i] = -9999;} 
    bsvtxv.pv_multiplicity = vertex.size();
    NSelectedVertices = 0;

    // event-level primary vertex: the one with the highest scalar sum of track pt
    // (all pv_* variables below are taken from this same vertex)
	int VtxIndex_maxPt = -99;
	double maxSumPt = -1.0;
    for (size_t iVtx = 0; iVtx < vertex.size(); ++iVtx) {
		const reco::Vertex& vtx = vertex[iVtx];
		double sumpT = 0.0;
		for (const auto& track : vtx.tracks()) {
			sumpT += track->pt();
		}
		if (sumpT > maxSumPt) {
			maxSumPt = sumpT;
			VtxIndex_maxPt = iVtx;
		}
		// count good vertices
		if(!vtx.isValid())continue;
	    if(vtx.isFake())continue;
	    if(vtx.ndof() < 4)continue;
	    if(fabs(vtx.z()) >= 24.0)continue;
	    if(vtx.position().Rho() >= 2)continue;
	 	NSelectedVertices++;
    }
    bsvtxv.VtxIndex = VtxIndex_maxPt;
    bsvtxv.pv_cutmultiplicity = NSelectedVertices;
    if(bsvtxv.VtxIndex >= 0) {
		const reco::Vertex &RecVtx = vertex[bsvtxv.VtxIndex];
	    bsvtxv.ispv     = 1;
		bsvtxv.pv_x     = RecVtx.x();
		bsvtxv.pv_y     = RecVtx.y();
		bsvtxv.pv_z     = RecVtx.z();
		bsvtxv.pv_errx  = RecVtx.xError();
		bsvtxv.pv_erry  = RecVtx.yError();
		bsvtxv.pv_errz  = RecVtx.zError();
		bsvtxv.pv_ntracks = RecVtx.nTracks();
		bsvtxv.pv_ndof  = RecVtx.ndof();
		bsvtxv.pv_rho   = RecVtx.position().Rho();
		int iteratorCov = 0;
		for(int i = 0; i < 3 ; ++i)
			for(int j = 0; j < 3; ++j)
				bsvtxv.pv_covariance[iteratorCov++] = RecVtx.covariance(i, j);
	}
	else {
		bsvtxv.isbs     = 1;
                bsvtxv.pv_x     = beamSpot.x0();
                bsvtxv.pv_y     = beamSpot.y0();
                bsvtxv.pv_z     = beamSpot.z0();
                bsvtxv.pv_errx  = beamSpot.x0Error();
                bsvtxv.pv_erry  = beamSpot.y0Error();
                bsvtxv.pv_errz  = beamSpot.z0Error();
	}


	    bsvtxv.bs_x = beamSpot.x0();
	    bsvtxv.bs_y = beamSpot.y0();
	    bsvtxv.bs_z = beamSpot.z0();
	    bsvtxv.bs_dx = beamSpot.x0Error();
	    bsvtxv.bs_dy = beamSpot.y0Error();
	    bsvtxv.bs_dz = beamSpot.z0Error();
	    bsvtxv.bs_dxdz = beamSpot.dxdz();
	    bsvtxv.bs_dydz = beamSpot.dydz();
	    bsvtxv.bs_sigmaZ = beamSpot.sigmaZ();
	    bsvtxv.bs_dsigmaZ = beamSpot.sigmaZ0Error();
		bsvtxv.position = beamSpot.position();
		bsvtxv.BeamSpot_cov2d = beamSpot.covariance(); // not sure if this is meant, this is 7 dimensional covariance
		bsvtxv.BeamSpot_cov3d = beamSpot.covariance3D();
    
    
    return bsvtxv;
}

int BeamSpotAndVertex::ClosestZindex(const std::vector<reco::Vertex>& vertex,
			RefCountedKinematicVertex bVertex)
{
	int PVClosestZIndex = -1;
	double MinDistanceZ = std::numeric_limits<double>::max();
	const double bsZ = bVertex->position().z();
	for (std::size_t i = 0; i < vertex.size(); ++i) {
		const auto& vtx = vertex[i];
		if (!vtx.isValid()) continue;
		const double dz = std::abs(bsZ - vtx.z());
		if (dz < MinDistanceZ) {
			MinDistanceZ = dz;
			PVClosestZIndex = static_cast<int>(i);
		}
	}
	return PVClosestZIndex;
}

int BeamSpotAndVertex::CosThetaindex(const std::vector<reco::Vertex>& vertex,
			RefCountedKinematicVertex bVertex,
			GlobalVector Bsvec)
{
	int PVCosThetaIndex = -1;
	double MinDistance  = std::numeric_limits<double>::max();
	for (std::size_t i = 0; i < vertex.size(); ++i) {
		const auto& vtx = vertex[i];
		if (!vtx.isValid()) continue;

		Double_t PVSVvecDotBsPvec=(bVertex->position().x()-vtx.x())*Bsvec.x()+(bVertex->position().y()-vtx.y())*Bsvec.y()+(bVertex->position().z()-vtx.z())*Bsvec.z();
		Double_t PVSVlength = TMath::Sqrt( pow((bVertex->position().x()- vtx.x()), 2.0) + pow((bVertex->position().y()-vtx.y()), 2.0) + pow((bVertex->position().z()- vtx.z()), 2.0) );
		Double_t BsPlength = TMath::Sqrt(Bsvec.x()*Bsvec.x()+Bsvec.y()*Bsvec.y() + Bsvec.z()*Bsvec.z());
		Double_t BsCosTheta = PVSVvecDotBsPvec / (BsPlength * PVSVlength);
		Double_t distance = 1-BsCosTheta;
		if(distance < MinDistance){
			MinDistance = distance;
			PVCosThetaIndex = static_cast<int>(i);
		}
	}
	return PVCosThetaIndex;
}

int BeamSpotAndVertex::LowestDCAIndex(const std::vector<reco::Vertex>& vertex,
			GlobalPoint bVertex,
			GlobalVector Bsvec)
{
	if(vertex.empty()) return -1;

	math::XYZPoint SV(bVertex.x(), bVertex.y(), bVertex.z());
	math::XYZVectorD p3sum(Bsvec.x(), Bsvec.y(), Bsvec.z());

	// primary vertex closest to the line through the secondary vertex along the B momentum
	int best = -1;
	double bestDist = std::numeric_limits<double>::max();
	for (size_t i = 0; i < vertex.size(); ++i)
	{
		const math::XYZPoint pcaPoint = BeamSpotAndVertex::pca(vertex[i].position(), SV, p3sum);
		const double dist = (pcaPoint - vertex[i].position()).R();
		if (dist < bestDist) { bestDist = dist; best = static_cast<int>(i); }
	}
	return best;
}

math::XYZPoint BeamSpotAndVertex::pca(math::XYZPoint pv, math::XYZPoint sv, math::XYZVectorD pMuMu){
  //s = (PV - SV) * (pMuMu) / |pMuMu|^2 
  double s = ((pv - sv).Dot(pMuMu)) / pMuMu.Mag2();
  //pca = sv + s*pMuMu
  math::XYZPoint PCA = sv + s*pMuMu;

  return PCA;
}