//Author : Muhammad Alibordi, University of Warsaw
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ParticleFlowCandidate.h"

#include <vector>
#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include <iostream>
#include <algorithm>

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include "TMath.h"



ParticleFlowCandidate::ParticleFlowCandidate(){}
std::vector<ParticleFlowCandidate::PFCandidateVariables> ParticleFlowCandidate::PFCandObservables(const std::vector<reco::PFCandidate>& pfcandidate){
	std::vector<PFCandidateVariables> pfcandVars;
    std::vector<reco::PFCandidate> sortedPFCandidates = pfcandidate;
	std::sort(sortedPFCandidates.begin(), sortedPFCandidates.end(), [](const reco::PFCandidate& a, const reco::PFCandidate& b) { return a.pt() > b.pt();});    
    // Loop through the first two highest momentum photons (if available)

    size_t maxPFPhotons = std::min(sortedPFCandidates.size(), size_t(2));  // Limit to 2 particle candidate photons
    for (size_t i = 0; i < maxPFPhotons; ++i) {
        const reco::PFCandidate& irecoPFCandidate = sortedPFCandidates[i];
        PFCandidateVariables pfcandVar;
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
        bool hasPhotonRef = !photonRef.isNull();
        std::cout<<"Photon Reference: " << (hasPhotonRef ? "Exists" : "Does not exist") << std::endl;
        std::cout<<" ========================================================================="<<"\n";
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
            pfcandVar.refphotonscBrem = photonRef->superCluster()->phiWidth() / photonRef->superCluster()->etaWidth();
            pfcandVar.refphotonr9 = photonRef->r9();
            pfcandVar.refphotonhadTowOverEm = photonRef->hadTowOverEm();
            pfcandVar.refphotonhcalOverEcal.clear();
            pfcandVar.refphotonhcalOverEcal.assign(iShowerShape.hcalOverEcal.begin(), iShowerShape.hcalOverEcal.end());
            pfcandVar.refphotonhcalOverEcalBc.clear();
            pfcandVar.refphotonhcalOverEcalBc.assign(iShowerShape.hcalOverEcalBc.begin(), iShowerShape.hcalOverEcalBc.end());
            //pfcandVar.refphotonhcalOverEcal = iShowerShape.hcalOverEcal;
            //pfcandVar.refphotonhcalOverEcalBc = iShowerShape.hcalOverEcalBc;


            } else {
                std::cout << "No valid photonRef for this PFCandidate.\n";
            }
        std::cout<<" ========================================================================="<<"\n";
        if (!irecoPFCandidate.photonRef().isNull())
            std::cout << "Photon pt = " << irecoPFCandidate.photonRef()->pt() << "\n";
        if (!irecoPFCandidate.gsfElectronRef().isNull())
            std::cout << "GSF Electron eta = " << irecoPFCandidate.gsfElectronRef()->eta() << "\n";
        if (!irecoPFCandidate.conversionRef().isNull())
            std::cout << "Conversion z = " << irecoPFCandidate.conversionRef()->conversionVertex().z() << "\n";
        if (!irecoPFCandidate.superClusterRef().isNull())
            std::cout << "SC energy = " << irecoPFCandidate.superClusterRef()->energy() << "\n";

        std::cout << "---------------------------------------------check other refrences --------------------------\n";
        pfcandVars.push_back(pfcandVar);
    }       
    return pfcandVars;
}
