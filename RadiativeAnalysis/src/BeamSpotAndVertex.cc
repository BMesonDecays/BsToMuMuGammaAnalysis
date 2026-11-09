#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"

#include <numeric>      // std::iota


BeamSpotAndVertex::BeamSpotAndVertex(bool isMINIAOD) {isMINIAOD_ = isMINIAOD;}
BeamSpotAndVertex::BSAndVtxVariables BeamSpotAndVertex::BSAndVtxObservables(const reco::BeamSpot& beamSpot,
                        const std::vector<reco::Vertex>& vertices){
    BSAndVtxVariables bsvtxv;
    bsvtxv.pv_multiplicity = vertices.size();

    // highest sum-pT vertex and number of vertices passing the quality cuts
    int nSelectedVertices = 0;
	size_t VtxIndex_maxPt = 0;     // MINIAOD: stays 0 (vertices have no track references)
	double maxSumPt = -1.0;
    for (size_t iVtx = 0; iVtx < vertices.size(); ++iVtx) {
		const reco::Vertex& vtx = vertices[iVtx];

		if (!isMINIAOD_) {
			double sumpT = 0.0;
			for (const auto& track : vtx.tracks()) {
				sumpT += track->pt();
			}
			if (sumpT > maxSumPt) {
				maxSumPt = sumpT;
				VtxIndex_maxPt = iVtx;
			}
		}

		if(!vtx.isValid())continue;
	        if(vtx.isFake())continue;
	        if(vtx.ndof() < 4)continue;
	        if(fabs(vtx.z()) >= 24.0)continue;
	        if(vtx.position().Rho() >= 2)continue;
	 	nSelectedVertices++;
    }
    bsvtxv.VtxIndex = VtxIndex_maxPt;
    bsvtxv.pv_cutmultiplicity = nSelectedVertices;

    // position of the highest sum-pT vertex (used by SCRecHitAccumulator)
    if (!vertices.empty()) {
		const reco::Vertex &RecVtx = vertices[bsvtxv.VtxIndex];
		bsvtxv.pv_x = RecVtx.x();
		bsvtxv.pv_y = RecVtx.y();
		bsvtxv.pv_z = RecVtx.z();
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
	bsvtxv.BeamSpot_cov2d = beamSpot.covariance(); // the full 7x7 beam-spot covariance (used by TrippleObjectVertex)

    
    return bsvtxv;
}

int BeamSpotAndVertex::ClosestZindex(const std::vector<reco::Vertex>& vertices,
			RefCountedKinematicVertex bVertex)
{
	int PVClosestZIndex = -1;
	double MinDistanceZ = std::numeric_limits<double>::max();
	const double bsZ = bVertex->position().z();
	for (std::size_t i = 0; i < vertices.size(); ++i) {
		const auto& vtx = vertices[i];
		if (!vtx.isValid()) continue;
		const double dz = std::abs(bsZ - vtx.z());
		if (dz < MinDistanceZ) {
			MinDistanceZ = dz;
			PVClosestZIndex = static_cast<int>(i);
		}
	}
	return PVClosestZIndex;
}

int BeamSpotAndVertex::CosThetaindex(const std::vector<reco::Vertex>& vertices,
			RefCountedKinematicVertex bVertex,
			GlobalVector Bsvec)
{
	int PVCosThetaIndex = -1;
	double MinDistance  = std::numeric_limits<double>::max();
	for (std::size_t i = 0; i < vertices.size(); ++i) {
		const auto& vtx = vertices[i];
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

int BeamSpotAndVertex::LowestDCAIndex(const std::vector<reco::Vertex>& vertices,
			GlobalPoint bVertex,
			GlobalVector Bsvec)
{
	if(vertices.empty()) return -1;

	math::XYZPoint SV(bVertex.x(), bVertex.y(), bVertex.z());
	math::XYZVectorD p3sum(Bsvec.x(), Bsvec.y(), Bsvec.z());

	// compute pca distances for every primary vertex
	std::vector<float> pcaDist;
	for (auto pv : vertices)
	{
		math::XYZPoint pca = BeamSpotAndVertex::pca(pv.position(), SV, p3sum);
		pcaDist.push_back((pca - pv.position()).R());
	}

	// sort pv indeces by pca distance
	std::vector<size_t> pvIndicesSortedByPCADist(vertices.size());
	iota(pvIndicesSortedByPCADist.begin(), pvIndicesSortedByPCADist.end(), 0);
	sort(pvIndicesSortedByPCADist.begin(), pvIndicesSortedByPCADist.end(),
		[&pcaDist](size_t i1, size_t i2) { return pcaDist[i1] < pcaDist[i2]; });
	
	return pvIndicesSortedByPCADist[0];
}

math::XYZPoint BeamSpotAndVertex::pca(math::XYZPoint pv, math::XYZPoint sv, math::XYZVectorD pMuMu){
  //s = (PV - SV) * (pMuMu) / |pMuMu|^2 
  double s = ((pv - sv).Dot(pMuMu)) / pMuMu.Mag2();
  //pca = sv + s*pMuMu
  math::XYZPoint PCA = sv + s*pMuMu;

  return PCA;
}