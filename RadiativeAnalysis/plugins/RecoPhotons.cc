#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RecoPhotons.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeAnalysis.h"


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
#include <algorithm>
#include <vector>
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
using namespace reco;
using namespace edm;
using namespace std;
using namespace pat;

RecoPhotons::RecoPhotons(){}
std::vector<RecoPhotons::PhotonVariables> RecoPhotons::PhotonObservables(const std::vector<reco::Photon>& photon){
	std::vector<PhotonVariables> phov;
	std::vector<reco::Photon> sortedPhotons = photon;
	std::sort(sortedPhotons.begin(), sortedPhotons.end(), [](const reco::Photon& a, const reco::Photon& b) {
        return a.pt() > b.pt();
	});

    
    // Loop through the first two highest momentum photons (if available)
    size_t maxPhotons = std::min(sortedPhotons.size(), size_t(2));  // Limit to 2 photons
    for (size_t i = 0; i < maxPhotons; ++i) {
        const reco::Photon& irecoPhoton = sortedPhotons[i];
        const reco::Photon::ShowerShape& iShowerShape = irecoPhoton.full5x5_showerShapeVariables();

        PhotonVariables photonVar;
        photonVar.sigmaIEtaIEta = iShowerShape.sigmaIetaIeta;
        photonVar.sigmaIEtaIPhi = iShowerShape.sigmaIetaIphi;
        photonVar.sigmaIPhiIPhi = iShowerShape.sigmaIphiIphi;
        photonVar.sigmaEtaEta = iShowerShape.sigmaEtaEta;
        photonVar.e1x5 = iShowerShape.e1x5;
        photonVar.e2x5 = iShowerShape.e2x5;
        photonVar.e3x3 = iShowerShape.e3x3;
        photonVar.e5x5 = iShowerShape.e5x5;
        photonVar.hcalDepth1OverEcal = iShowerShape.hcalDepth1OverEcal;
        photonVar.hcalDepth2OverEcal = iShowerShape.hcalDepth2OverEcal;
        photonVar.hcalDepth1OverEcalBc = iShowerShape.hcalDepth1OverEcalBc;
        photonVar.hcalDepth2OverEcalBc = iShowerShape.hcalDepth2OverEcalBc;
        photonVar.maxEnergyXtal = iShowerShape.maxEnergyXtal;
        photonVar.effSigmaRR = iShowerShape.effSigmaRR;

        photonVar.scEnergy = irecoPhoton.superCluster()->energy();
        photonVar.scRawEnergy = irecoPhoton.superCluster()->rawEnergy();
        photonVar.scEta = irecoPhoton.superCluster()->eta();
        photonVar.scPhi = irecoPhoton.superCluster()->phi();
        photonVar.scEtaWidth = irecoPhoton.superCluster()->etaWidth();
        photonVar.scPhiWidth = irecoPhoton.superCluster()->phiWidth();
        photonVar.scBrem = irecoPhoton.superCluster()->phiWidth() / irecoPhoton.superCluster()->etaWidth();
        photonVar.r9 = irecoPhoton.r9();
        photonVar.hadTowOverEm = irecoPhoton.hadTowOverEm();
	photonVar.hcalOverEcal.clear();photonVar.hcalOverEcal.assign(iShowerShape.hcalOverEcal.begin(), iShowerShape.hcalOverEcal.end());
        photonVar.hcalOverEcalBc.clear();photonVar.hcalOverEcalBc.assign(iShowerShape.hcalOverEcalBc.begin(), iShowerShape.hcalOverEcalBc.end());

	//photonVar.hcalOverEcal = iShowerShape.hcalOverEcal;
	//photonVar.hcalOverEcalBc = iShowerShape.hcalOverEcalBc;
        photonVar.pt = irecoPhoton.pt();
        photonVar.eta = irecoPhoton.eta();
        photonVar.phi = irecoPhoton.phi();
        photonVar.energy = irecoPhoton.energy();
        photonVar.et = irecoPhoton.et();
	//photonVar.trackiso = ipatPhoton.trackIso();
        //photonVar.ecaliso  = ipatPhoton.ecalIso();
        //photonVar.hcaliso  = ipatPhoton.hcalIso();
        //photonVar.caloIso  = ipatPhoton.caloIso();
        phov.push_back(photonVar);
    }
    return phov;
}
