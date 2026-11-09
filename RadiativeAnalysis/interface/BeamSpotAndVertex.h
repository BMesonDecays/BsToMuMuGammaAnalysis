#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_BeamSpotAndVertex_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_BeamSpotAndVertex_h


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

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "DataFormats/BTauReco/interface/SecondaryVertexTagInfo.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicVertex.h"



class BeamSpotAndVertex{
    public: 
        BeamSpotAndVertex(bool isMINIAOD);
        ~BeamSpotAndVertex(){}
		struct BSAndVtxVariables {
			// Beam Spot Variables
			int VtxIndex = -99;
			double bs_x = -9999.0;
			double bs_y = -9999.0;
			double bs_z = -9999.0;
			double bs_dx = -9999.0;
			double bs_dy = -9999.0;
			double bs_dz = -9999.0;
			double bs_dxdz = -9999.0;
			double bs_dydz = -9999.0;
			double bs_sigmaZ = -9999.0;
			double bs_dsigmaZ = -9999.0;
			reco::BeamSpot::CovarianceMatrix BeamSpot_cov2d;	// full 7x7 beam-spot covariance
			reco::BeamSpot::Point position;
			// Vertex Variables
			double pv_x = -9999.0;		// highest sum-pT vertex (index VtxIndex)
			double pv_y = -9999.0;
			double pv_z = -9999.0;
			int pv_multiplicity = -9999;	// all vertices
			int pv_cutmultiplicity = -9999;	// vertices passing the quality cuts
		};

	BSAndVtxVariables BSAndVtxObservables(const reco::BeamSpot& beamSpot, 
			 const std::vector<reco::Vertex>& vertex);

	static int ClosestZindex(const std::vector<reco::Vertex>& vertex,
			RefCountedKinematicVertex bVertex);

	static int CosThetaindex(const std::vector<reco::Vertex>& vertex,
			RefCountedKinematicVertex bVertex,
			GlobalVector Bsvec);
	
	static int LowestDCAIndex(const std::vector<reco::Vertex>& vertex,
			GlobalPoint bVertex,
			GlobalVector Bsvec);

	private:
	static math::XYZPoint pca(math::XYZPoint pv, math::XYZPoint sv, math::XYZVectorD pMuMu);

	bool isMINIAOD_;
      
};

#endif
