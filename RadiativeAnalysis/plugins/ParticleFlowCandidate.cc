//Author : Muhammad Alibordi, University of Warsaw
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ParticleFlowCandidate.h"

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



ParticleFlowCandidate::ParticleFlowCandidate(){}
std::vector<ParticleFlowCandidate::PFCandidateVariables> ParticleFlowCandidate::PFCandObservables(const std::vector<reco::PFCandidate>& pfcandidate){
	std::vector<PFCandidateVariables> pfcandVars;
    // the (up to) two highest-pt PF candidates of any type (sorting indices, not copying the collection)
    const size_t maxPFCands = std::min(pfcandidate.size(), size_t(2));
    std::vector<size_t> order(pfcandidate.size());
    std::iota(order.begin(), order.end(), 0);
    std::partial_sort(order.begin(), order.begin() + maxPFCands, order.end(),
        [&pfcandidate](size_t a, size_t b) { return pfcandidate[a].pt() > pfcandidate[b].pt(); });

    for (size_t i = 0; i < maxPFCands; ++i) {
        const reco::PFCandidate& irecoPFCandidate = pfcandidate[order[i]];
        PFCandidateVariables pfcandVar;
        pfcandVar.index = order[i];
        pfcandVar.pt = irecoPFCandidate.pt();
        pfcandVar.eta = irecoPFCandidate.eta();
        pfcandVar.phi = irecoPFCandidate.phi();
        pfcandVar.energy = irecoPFCandidate.energy();
        pfcandVar.time = irecoPFCandidate.time();
        pfcandVar.mass = irecoPFCandidate.mass();
        pfcandVar.charge = irecoPFCandidate.charge();
        pfcandVar.ecalEnergy = irecoPFCandidate.ecalEnergy();
        pfcandVar.rawEcalEnergy = irecoPFCandidate.rawEcalEnergy();
        pfcandVar.hcalEnergy = irecoPFCandidate.hcalEnergy();
        pfcandVar.rawHcalEnergy = irecoPFCandidate.rawHcalEnergy();
        pfcandVar.hoEnergy = irecoPFCandidate.hoEnergy();
        pfcandVar.rawHoEnergy = irecoPFCandidate.rawHoEnergy();
        reco::PhotonRef photonRef = irecoPFCandidate.photonRef();
        //std::cout<<"Photon Reference: " << (hasPhotonRef ? "Exists" : "Does not exist") << std::endl;
        //std::cout<<" ========================================================================="<<"\n";
        if (!photonRef.isNull()) {
            pfcandVar.refphotonpt = photonRef->pt();
            pfcandVar.refphotoneta = photonRef->eta();
            pfcandVar.refphotonphi = photonRef->phi();
            pfcandVar.refphotonenergy = photonRef->energy();
            pfcandVar.refphotonet = photonRef->et();
            const reco::Photon::ShowerShape& iShowerShape = photonRef->full5x5_showerShapeVariables();
            pfcandVar.refphotonsigmaIEtaIEta = iShowerShape.sigmaIetaIeta;
            pfcandVar.refphotonsigmaIEtaIPhi = iShowerShape.sigmaIetaIphi;
            pfcandVar.refphotonsigmaIPhiIPhi = iShowerShape.sigmaIphiIphi;
            pfcandVar.refphotonsigmaEtaEta = iShowerShape.sigmaEtaEta;
            pfcandVar.refphotone1x5 = iShowerShape.e1x5;
            pfcandVar.refphotone2x5 = iShowerShape.e2x5;
            pfcandVar.refphotone3x3 = iShowerShape.e3x3;
            pfcandVar.refphotone5x5 = iShowerShape.e5x5;
            pfcandVar.refphotonhcalDepth1OverEcal = iShowerShape.hcalDepth1OverEcal;
            pfcandVar.refphotonhcalDepth2OverEcal = iShowerShape.hcalDepth2OverEcal;
            pfcandVar.refphotonhcalDepth1OverEcalBc = iShowerShape.hcalDepth1OverEcalBc;
            pfcandVar.refphotonhcalDepth2OverEcalBc = iShowerShape.hcalDepth2OverEcalBc;
            pfcandVar.refphotonmaxEnergyXtal = iShowerShape.maxEnergyXtal;
            pfcandVar.refphotoneffSigmaRR = iShowerShape.effSigmaRR;
            pfcandVar.refphotonscEnergy = photonRef->superCluster()->energy();
            pfcandVar.refphotonscRawEnergy = photonRef->superCluster()->rawEnergy();
            pfcandVar.refphotonscEta = photonRef->superCluster()->eta();
            pfcandVar.refphotonscPhi = photonRef->superCluster()->phi();
            pfcandVar.refphotonscEtaWidth = photonRef->superCluster()->etaWidth();
            pfcandVar.refphotonscPhiWidth = photonRef->superCluster()->phiWidth();
            if (photonRef->superCluster()->etaWidth() > 0)
                pfcandVar.refphotonscBrem = photonRef->superCluster()->phiWidth() / photonRef->superCluster()->etaWidth();
            pfcandVar.refphotonr9 = photonRef->r9();
            pfcandVar.refphotonhadTowOverEm = photonRef->hadTowOverEm();
            pfcandVar.refphotonhcalOverEcal.clear();
            pfcandVar.refphotonhcalOverEcal.assign(iShowerShape.hcalOverEcal.begin(), iShowerShape.hcalOverEcal.end());
            pfcandVar.refphotonhcalOverEcalBc.clear();
            pfcandVar.refphotonhcalOverEcalBc.assign(iShowerShape.hcalOverEcalBc.begin(), iShowerShape.hcalOverEcalBc.end());
            //pfcandVar.refphotonhcalOverEcal = iShowerShape.hcalOverEcal;
            //pfcandVar.refphotonhcalOverEcalBc = iShowerShape.hcalOverEcalBc;


            }
        pfcandVars.push_back(pfcandVar);
    }       
    return pfcandVars;
}
