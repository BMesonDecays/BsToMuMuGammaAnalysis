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
    
    size_t maxPhotons = std::min(sortedPFCandidates.size(), size_t(2));  // Limit to 2 photons
    for (size_t i = 0; i < maxPhotons; ++i) {
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
            std::cout << "pt eta phi: "
              << photonRef->pt() << " "
              << photonRef->eta() << " "
              << photonRef->phi() << "\n";
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
