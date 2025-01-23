#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"



BeamSpotAndVertex::BeamSpotAndVertex(){}
BeamSpotAndVertex::BSAndVtxVariables BeamSpotAndVertex::BSAndVtxObservables(const reco::BeamSpot& beamSpot,
                        const std::vector<reco::Vertex>& vertex){
    BSAndVtxVariables bsvtxv;
    int VtxIndex = -99;
    for(size_t i=0; i<9;++i){bsvtxv.pv_covariance[i] = -9999;} 
    const reco::Vertex &recVtxs_track = vertex.front();
    bsvtxv.pv_ntracks = recVtxs_track.nTracks();
    bsvtxv.pv_multiplicity = vertex.size();
    NSelectedVertices = 0;
    for (size_t iVtx = 0; iVtx < vertex.size(); ++iVtx) {
		VtxIndex = iVtx;
		const reco::Vertex& vtx = vertex[iVtx];
		int iteratorCov =0;
		for(int i = 0; i < 3 ; ++i){
			for(int j =0; j< 3; ++j){
				bsvtxv.pv_covariance[iteratorCov] = vtx.covariance(i, j);

			}
		}
		bsvtxv.pv_ndof = vtx.ndof();
		bsvtxv.pv_rho = vtx.position().Rho();
		if(!vtx.isValid())continue;
	        if(vtx.isFake())continue;
	        if(vtx.ndof() < 4)continue;
	        if(fabs(vtx.z()) >= 24.0)continue;
	        if(vtx.position().Rho() >= 2)continue;
	 	NSelectedVertices++;
		/*for(reco::Vertex::trackRef_iterator trackRef = vtx.tracks_begin(); trackRef !=vtx.tracks_end(); ++trackRef){
				 const reco::TrackBaseRef& vtx_trackRef = *trackRef;
				 if (vtx_trackRef.isNonnull() && vtx_trackRef.isAvailable()) {
					 const reco::Track& VtxTrack = *vtx_trackRef.castTo<reco::TrackRef>();
					 PtSumVertex += std::abs(VtxTrack.pt());
				 }
				 else {
					 std::cout << "Invalid track reference : "<<"\n";
				 }
			 }
			 if (PtSumVertex >  MinPtVertex) {
				 VtxIndex = iVtx;
				 MinPtVertex = PtSumVertex;
				 std::cout<<" min/max pt sum vertex : "<< MinPtVertex <<"\t"<< " & veretx index : " << VtxIndex<< "\n";
			 }*/
    }
    bsvtxv.pv_cutmultiplicity = NSelectedVertices;
    const reco::Vertex &RecVtx = vertex[VtxIndex];
    if(VtxIndex !=-99) {
	        bsvtxv.ispv     = 1;
		bsvtxv.pv_x     = RecVtx.x();
		bsvtxv.pv_y     = RecVtx.y();
		bsvtxv.pv_z     = RecVtx.z();
		bsvtxv.pv_errx  = RecVtx.xError();
		bsvtxv.pv_erry  = RecVtx.yError();
		bsvtxv.pv_errz  = RecVtx.zError();

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
	    //bsvtxv.bs_dx = beamSpot.x0Error();
	    ////bsvtxv.bs_dy = beamSpot.y0Error();
	    ////bsvtxv.bs_dz = beamSpot.z0Error();
	    bsvtxv.bs_dxdz = beamSpot.dxdz();
	    bsvtxv.bs_dydz = beamSpot.dydz();
	    bsvtxv.bs_sigmaZ = beamSpot.sigmaZ();
	    bsvtxv.bs_dsigmaZ = beamSpot.sigmaZ0Error();
    
    
    return bsvtxv;
}
