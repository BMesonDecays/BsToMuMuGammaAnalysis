#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_RecoPhotons_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_RecoPhotons_h




#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/Registry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "DataFormats/BTauReco/interface/SecondaryVertexTagInfo.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"




#include <iterator>  
#include <algorithm> 
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
using namespace std;
using namespace reco;
using namespace edm;
//using namespace pat;

class RecoPhotons{
    public: 
        RecoPhotons();
        ~RecoPhotons(){}
	struct PhotonVariables {
		double pt = -9999.0, eta = -9999.0, phi = -9999.0, energy = -9999.0, et = -9999.0;
		double sigmaIEtaIEta = -9999.0, sigmaIEtaIPhi = -9999.0, sigmaIPhiIPhi = -9999.0, sigmaEtaEta = -9999.0;
		double e1x5 = -9999.0, e2x5 = -9999.0, e3x3 = -9999.0, e5x5 = -9999.0;
		double hcalDepth1OverEcal = -9999.0, hcalDepth2OverEcal = -9999.0;
		double hcalDepth1OverEcalBc = -9999.0, hcalDepth2OverEcalBc = -9999.0;
		double maxEnergyXtal = -9999.0, effSigmaRR = -9999.0;
		double scEnergy = -9999.0, scRawEnergy = -9999.0, scEta = -9999.0, scPhi = -9999.0;
		double scEtaWidth = -9999.0, scPhiWidth = -9999.0, scBrem = -9999.0, r9 = -9999.0, hadTowOverEm = -9999.0;
		std::vector<float> hcalOverEcal;
		std::vector<float> hcalOverEcalBc;
	};


	std::vector<PhotonVariables> PhotonObservables(const std::vector<reco::Photon>& photon);

      
};

#endif
