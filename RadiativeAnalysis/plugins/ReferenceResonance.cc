#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"



ReferenceResonance::ReferenceResonance(){}
double ReferenceResonance::calculateMassFromComponents(const pat::CompositeCandidateCollection& conv1, const pat::CompositeCandidateCollection& conv2) {
    constexpr double electronMass = 0.000511;
    TLorentzVector totalVec;
    for (const auto& p1 : conv1) {
    const reco::Track& eletk0 = *p1.userData<reco::Track>("track0");
    const reco::Track& eletk1 = *p1.userData<reco::Track>("track1");
    for (const auto& p2 : conv2){
    const reco::Track& eletk2 = *p2.userData<reco::Track>("track0");
    const reco::Track& eletk3 = *p2.userData<reco::Track>("track1");
     if (eletk0.charge() * eletk1.charge() >= 0 || eletk2.charge() * eletk3.charge() >= 0) {
	      return -1;
     }
     std::set<const reco::Track*> trackSet = {&eletk0, &eletk1};
     if (trackSet.count(&eletk2) > 0 || trackSet.count(&eletk3) > 0) {
	     return -1;
     }
    TLorentzVector vec_e1, vec_e2, vec_e3, vec_e4;
    vec_e1.SetPtEtaPhiM(eletk0.pt(), eletk0.eta(), eletk0.phi(), electronMass);
    vec_e2.SetPtEtaPhiM(eletk1.pt(), eletk1.eta(), eletk1.phi(), electronMass);
    vec_e3.SetPtEtaPhiM(eletk2.pt(), eletk2.eta(), eletk2.phi(), electronMass);
    vec_e4.SetPtEtaPhiM(eletk3.pt(), eletk3.eta(), eletk3.phi(), electronMass);
    TLorentzVector totalVec = vec_e1 + vec_e2 + vec_e3 + vec_e4;
    }
    }
    return totalVec.M();

}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
ReferenceResonance::ResonanceDetails ReferenceResonance::findResonances(const reco::Muon& mu1, const reco::Muon& mu2, bool verbose) 
{
    ResonanceDetails result;
    double dimuonMass = ReferenceResonance::calculateProperty(mu1, mu2, [](const auto& p4) { return p4.M(); });

    std::vector<std::pair<ResonanceFlag, std::pair<double, double>>> resonanceList = {
        {Jpsi, {Constants::JpsiMass_PDG, Constants::JpsiWindow_PDG}},
        {Phi, {Constants::PhiMass_PDG, Constants::PhiWindow_PDG}},
        {KStar, {Constants::KStar0Mass_PDG, Constants::KStar0Window_PDG}},
        {NonResonant, {Constants::nonResonantMass_PDG, Constants::nonResonantWindow_PDG}}
    };

    result.resonanceFlag = ResonanceFlag::None;  // Default: no resonance found

    for (const auto& [flag, massInfo] : resonanceList) {
        double nominalMass = massInfo.first;
        double massWindow = massInfo.second;
        
        if (fabs(dimuonMass - nominalMass) < massWindow &&
            mu1.pt() > Constants::PtCut && mu2.pt() > Constants::PtCut) 
        {
            result.resonanceFlag = flag;  // Assign flag directly from enum
            result.isValid = true;
            break;
        }
    }

    result.mass = dimuonMass;
    if (result.isValid) {
        result.eta = ReferenceResonance::calculateProperty(mu1, mu2, [](const auto& p4) { return p4.eta(); });
        result.phi = ReferenceResonance::calculateProperty(mu1, mu2, [](const auto& p4) { return p4.phi(); });
        result.pt  = ReferenceResonance::calculateProperty(mu1, mu2, [](const auto& p4) { return p4.pt(); });
        result.px  = ReferenceResonance::calculateProperty(mu1, mu2, [](const auto& p4) { return p4.px(); });
        result.py  = ReferenceResonance::calculateProperty(mu1, mu2, [](const auto& p4) { return p4.py(); });
    }

   

    return result;
}


ReferenceResonance::ResonanceDetails ReferenceResonance::findResonances(const reco::Photon& photon1, const reco::Photon& photon2, const std::string& resonanceType, bool verbose) {
    ResonanceDetails result;

    double diPhotonMass = CombinedpFour(photon1, photon2).M();
    double nominalMass = 0.0;
    double massWindow = 0.0;

    if (resonanceType == "Eta") {
        nominalMass = Constants::EtaMass_PDG;
        massWindow = Constants::EtaWindow_PDG;
    } else if (resonanceType == "Etaprime") {
        nominalMass = Constants::EtaPrimeMass_PDG;
        massWindow = Constants::EtaPrimeWindow_PDG;
    } else if (resonanceType == "Pi0") {
        nominalMass = Constants::Pi0Mass_PDG;
        massWindow = Constants::Pi0Window_PDG;
    }

    bool isValid = (fabs(diPhotonMass - nominalMass) < massWindow);

    result.mass = diPhotonMass;
    result.isValid = isValid;

    if (verbose) {
        std::cout << "Calculated DiPhoton Mass: " << diPhotonMass << " (Nominal: " << nominalMass << ", Window: " << massWindow << ")\n";
    }

    return result;
}
ReferenceResonance::ResonanceDetails ReferenceResonance::findResonances(const pat::CompositeCandidateCollection& conversion1, const pat::CompositeCandidateCollection& conversion2, const std::string& resonanceType, bool verbose) {
    ResonanceDetails result;
    double diConversionMass = calculateMassFromComponents(conversion1, conversion2);
    double nominalMass = 0.0;
    double massWindow = 0.0;

    if (resonanceType == "Eta") {
        nominalMass = Constants::EtaMass_PDG;
        massWindow = Constants::EtaWindow_PDG;
    } else if (resonanceType == "Etaprime") {
        nominalMass = Constants::EtaPrimeMass_PDG;
        massWindow = Constants::EtaPrimeWindow_PDG;
    } else if (resonanceType == "Pi0") {
        nominalMass = Constants::Pi0Mass_PDG;
        massWindow = Constants::Pi0Window_PDG;
    }

    bool isValid = (fabs(diConversionMass - nominalMass) < massWindow);

    result.mass = diConversionMass;
    result.isValid = isValid;

    if (verbose) {
        std::cout << "Calculated DiConversion Mass: " << diConversionMass
                  << " (Nominal: " << nominalMass << ", Window: " << massWindow << ")\n";
    }

    return result;
}


