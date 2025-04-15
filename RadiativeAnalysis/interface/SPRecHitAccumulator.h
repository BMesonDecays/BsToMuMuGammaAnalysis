#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_SPRecHitAccumulator_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_SPRecHitAccumulator_h


#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"

#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include <iostream>
#include <vector>
#include <functional>

//Root 
#include <TMath.h>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include "Math/Vector4D.h"

#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Math/Vector3D.h"

class SPRecHitAccumulator{
    public:
    SPRecHitAccumulator();
        ~SPRecHitAccumulator(){}
	public:

    struct SCAndRecHitVariables {
        double sc_eta                     = -9999;
        double sc_phi                     = -9999;
        double sc_energy                  = -9999;
        double sc_eta_width               = -9999;
        double sc_phi_width               = -9999;
        double cluster_energies           = -9999;/ Per SuperCluster
        // RecHit variables (associated with SuperClusters)
        uint32_t rechit_rawids            = -9999; // Per SuperCluster
        int  rechit_subdet                = -9999;     // EcalBarrel or EcalEndcap
        double rechit_energies            = -9999;
        double rechit_times               = -9999;
        double rechit_x                   = -9999;
        double rechit_y                   = -9999;
        double rechit_z                   = -9999;
        double rechit_pv_distances        = -9999;
        double rechit_tofs                = -9999;
        // Flags for problematic RecHits
        bool rechit_is_weird              = false;
        bool rechit_is_problematic        = false;
        bool rechit_poor_reco             = false;
    };

    SCAndRecHitVariables SCAndRecHitObservables(const std::vector<reco::SuperCluster>& superclusters,
                                           const EcalRecHitCollection& recHits,
                                           const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
                                           const CaloGeometry& caloGeom);

private:
    const float c_light_ = 29.9792458; // cm/ns
};

#endif