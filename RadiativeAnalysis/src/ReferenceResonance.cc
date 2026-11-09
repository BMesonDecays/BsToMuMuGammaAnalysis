#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"



ReferenceResonance::ReferenceResonance(){}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
template <typename patOrRecoMuon>
ReferenceResonance::ResonanceDetails ReferenceResonance::findResonances(const patOrRecoMuon& mu1, const patOrRecoMuon& mu2, bool verbose) 
{
    ResonanceDetails result;
    double dimuonMass = ReferenceResonance::calculateProperty(mu1, mu2, [](const auto& p4) { return p4.M(); });

    std::vector<std::pair<ResonanceFlag, std::pair<double, double>>> resonanceList = {
        {Jpsi,  {Constants::JpsiMass_PDG,  Constants::JpsiWindow_PDG}},
        {Psi2S, {Constants::Psi2SMass_PDG, Constants::Psi2SWindow_PDG}},
        {Phi,   {Constants::PhiMass_PDG,   Constants::PhiWindow_PDG}}
    };

    result.resonanceFlag = ResonanceFlag::None;  // Default: no resonance found

    for (const auto& [flag, massInfo] : resonanceList) {
        double nominalMass = massInfo.first;
        double massWindow = massInfo.second;
        
        // classification by mass only; the muon pT cut is applied in MuonSelector
        if (fabs(dimuonMass - nominalMass) < massWindow)
        {
            result.resonanceFlag = flag;  // Assign flag directly from enum
            result.nominalMass = nominalMass;
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


// Explicit instantiations (the template is defined in this .cc file, so used types must be listed here)
template ReferenceResonance::ResonanceDetails ReferenceResonance::findResonances<reco::Muon>(
    const reco::Muon&, const reco::Muon&, bool);


