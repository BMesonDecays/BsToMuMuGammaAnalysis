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




class BeamSpotAndVertex{
    public: 
        BeamSpotAndVertex();
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
			// Vertex Variables
			double pv_x = -9999.0;
			double pv_y = -9999.0;
			double pv_z = -9999.0;
			double pv_errx = -9999.0;
			double pv_erry = -9999.0;
			double pv_errz = -9999.0;
			int pv_ntracks = -9999;
			int pv_multiplicity = -9999;
			double pv_ndof = -9999.0;
			double pv_rho = -9999.0;
			double pv_covariance[9];
			int pv_cutmultiplicity = -9999.0;
			bool isValidPV = false;
			bool ispv = false;
			bool isbs = false;
			reco::BeamSpot::Point position;
		};;

	BSAndVtxVariables BSAndVtxObservables(const reco::BeamSpot& beamSpot, 
			 const std::vector<reco::Vertex>& vertex);
	int    NSelectedVertices;
      
};

#endif
