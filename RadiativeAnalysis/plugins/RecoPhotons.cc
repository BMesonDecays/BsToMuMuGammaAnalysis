//Author : Muhammad Alibordi, University of Warsaw
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RecoPhotons.h"

#include <vector>
#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include "TMath.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"

RecoPhotons::RecoPhotons(){}
std::vector<RecoPhotons::PhotonVariables> RecoPhotons::PhotonObservables(const std::vector<reco::Photon>& photon){
	std::vector<PhotonVariables> phov;
	// indices of the (up to) two highest-pt photons; the index into the input collection is kept
	// so that the caller can look up the same photon (ID value maps, selection)
	const size_t maxPhotons = std::min(photon.size(), size_t(2));
	std::vector<size_t> order(photon.size());
	std::iota(order.begin(), order.end(), 0);
	std::partial_sort(order.begin(), order.begin() + maxPhotons, order.end(),
		[&photon](size_t a, size_t b) { return photon[a].pt() > photon[b].pt(); });

    for (size_t i = 0; i < maxPhotons; ++i) {
        const reco::Photon& irecoPhoton = photon[order[i]];
        const reco::Photon::ShowerShape& iShowerShape = irecoPhoton.full5x5_showerShapeVariables();

        PhotonVariables photonVar;
        photonVar.index = order[i];
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
        if (irecoPhoton.superCluster()->etaWidth() > 0)
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
