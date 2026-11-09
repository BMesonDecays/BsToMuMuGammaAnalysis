#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GenChain.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>

using namespace std;


GenDecayChain::GenDecayChain(const std::vector<reco::GenParticle>& genParticles_in) :
    genParticles_(genParticles_in),
    BMeson_(nullptr),
    muPlus_(nullptr),
    muMinus_(nullptr),
    signalPhoton_(nullptr),
    jpsi_(nullptr),
    vectorMeson_(nullptr)
{
    analyze();
}


void GenDecayChain::analyze() {
    findSignalDecay();
   
    if (!BMeson_) {
        info_.isValid = false;
        return;
    }
    identifyResonances();
    extractKinematics();
    extractVertexInfo();
    computeAngularVariables();
    fillDileptonVariables();
    info_.isValid = true;
    info_.isHardProcess = isFromHardProcess(BMeson_);
}


void GenDecayChain::findSignalDecay() {
    // Look for a Bs (531) or Bd (511) that decays through one of the signal modes
    // (see matchSignalDecay). Take the first such B meson found.

    for (const auto& p : genParticles_) {
        int absId = abs(p.pdgId());
        if (absId != 531 && absId != 511) continue;

        // the decay products are attached to the last copy of the B (after oscillations)
        DecayMode mode = matchSignalDecay(lastCopy(&p));
        if (mode == kUnknown) continue;

        // now find the oldest ancestor B meson in the chain (handle oscillations)
        const reco::GenParticle* current = &p;
        while (current->mother() && abs(current->mother()->pdgId()) == absId)
        {
            const reco::GenParticle* mother = dynamic_cast<const reco::GenParticle*>(current->mother());
            if (!mother) break;
            current = mother;
        }
        BMeson_ = current;
        info_.bMesonPdgId = current->pdgId();
        info_.decayMode = mode;
        info_.is3BodyDecay = true;
        return;
    }
}


// Follow single-daughter copies with the same |pdgId| (e.g. Bs -> anti-Bs oscillation,
// generator copies) down to the copy that actually decays
const reco::GenParticle* GenDecayChain::lastCopy(const reco::GenParticle* particle) const {
    while (particle && particle->numberOfDaughters() == 1) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(particle->daughter(0));
        if (!dau || abs(dau->pdgId()) != abs(particle->pdgId())) break;
        particle = dau;
    }
    return particle;
}


// Check whether the (decaying) B meson decays through one of the signal modes:
//   B -> mu+ mu- gamma
//   B -> R gamma, R -> mu+ mu-,  R = J/psi, psi(2S) (Bs and Bd) or phi (Bs only)
// Any number of additional (FSR) photons is allowed among the B and R daughters;
// any other daughter rejects the B. The highest-pT photon among the B daughters
// is taken as the signal photon.
// On success the chain particles are stored in the data members and the mode is returned;
// otherwise nothing is stored and kUnknown is returned.
DecayMode GenDecayChain::matchSignalDecay(const reco::GenParticle* decayingB) {
    if (!decayingB) return kUnknown;
    const bool isBs = (abs(decayingB->pdgId()) == 531);

    const reco::GenParticle* muPlus    = nullptr;
    const reco::GenParticle* muMinus   = nullptr;
    const reco::GenParticle* resonance = nullptr;
    std::vector<const reco::GenParticle*> photons;

    // B daughters: mu+, mu-, at most one resonance, photons
    for (size_t i = 0; i < decayingB->numberOfDaughters(); ++i) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(decayingB->daughter(i));
        if (!dau) return kUnknown;
        int id = dau->pdgId();
        int absId = abs(id);

        if      (id == -13 && !muPlus)  muPlus = dau;
        else if (id ==  13 && !muMinus) muMinus = dau;
        else if (absId == 22)           photons.push_back(dau);
        else if (!resonance && (absId == 443 || absId == 100443 || (isBs && absId == 333))) resonance = dau;
        else return kUnknown;   // any other or duplicated daughter
    }
    if (photons.empty()) return kUnknown;

    // signal photon: the highest-pT photon among the B daughters (the others are FSR)
    const reco::GenParticle* signalPhoton = *std::max_element(photons.begin(), photons.end(),
        [](const reco::GenParticle* a, const reco::GenParticle* b) { return a->pt() < b->pt(); });

    DecayMode mode = kUnknown;
    if (!resonance) {
        // B -> mu+ mu- gamma
        if (!muPlus || !muMinus) return kUnknown;
        mode = isBs ? kBs_MuMu_Gamma : kBd_MuMu_Gamma;
    } else {
        // B -> R gamma: no muons directly from the B; R -> mu+ mu- (+ FSR photons)
        if (muPlus || muMinus) return kUnknown;
        const reco::GenParticle* decayingR = lastCopy(resonance);
        for (size_t i = 0; i < decayingR->numberOfDaughters(); ++i) {
            const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(decayingR->daughter(i));
            if (!dau) return kUnknown;
            int id = dau->pdgId();

            if      (id == -13 && !muPlus)  muPlus = dau;
            else if (id ==  13 && !muMinus) muMinus = dau;
            else if (abs(id) == 22)         photons.push_back(dau);
            else return kUnknown;
        }
        if (!muPlus || !muMinus) return kUnknown;

        int absResId = abs(resonance->pdgId());
        if      (absResId == 443)    mode = isBs ? kBs_Jpsi_Gamma  : kBd_Jpsi_Gamma;
        else if (absResId == 100443) mode = isBs ? kBs_Psi2S_Gamma : kBd_Psi2S_Gamma;
        else                         mode = kBs_Phi_Gamma;
    }

    // store the chain
    muPlus_       = muPlus;
    muMinus_      = muMinus;
    photons_      = photons;
    signalPhoton_ = signalPhoton;
    if (resonance) {
        if (abs(resonance->pdgId()) == 333) vectorMeson_ = resonance;
        else                                jpsi_        = resonance;
    }
    return mode;
}


void GenDecayChain::identifyResonances() {
    if (jpsi_) {
        info_.jpsiPdgId = jpsi_->pdgId();
        info_.jpsiP4.SetPxPyPzE(jpsi_->px(), jpsi_->py(), jpsi_->pz(), jpsi_->energy());
        info_.hasJpsi = true;
    }
   
    if (vectorMeson_) {
        info_.vectorMesonId = vectorMeson_->pdgId();
        info_.vectorMesonP4.SetPxPyPzE(vectorMeson_->px(), vectorMeson_->py(),
                                      vectorMeson_->pz(), vectorMeson_->energy());
        info_.hasVectorMeson = true;
    }
}


void GenDecayChain::extractKinematics() {
    // B meson
    if (BMeson_) {
        info_.Bp4.SetPxPyPzE(BMeson_->px(), BMeson_->py(), BMeson_->pz(), BMeson_->energy());
    }
   
    // Muons
    if (muPlus_) {
        info_.hasMuonPlus = true;
        info_.muPlusP4.SetPxPyPzE(muPlus_->px(), muPlus_->py(), muPlus_->pz(), muPlus_->energy());
        const reco::GenParticle* muPlusMom = findMother(muPlus_);
        info_.muPlusMotherId = muPlusMom ? muPlusMom->pdgId() : 0;
        info_.nMuons++;
    }
   
    if (muMinus_) {
        info_.hasMuonMinus = true;
        info_.muMinusP4.SetPxPyPzE(muMinus_->px(), muMinus_->py(), muMinus_->pz(), muMinus_->energy());
        const reco::GenParticle* muMinusMom = findMother(muMinus_);
        info_.muMinusMotherId = muMinusMom ? muMinusMom->pdgId() : 0;
        info_.nMuons++;
    }
   
    // Photons
    info_.nPhotons = photons_.size();
    if (signalPhoton_) {
        info_.signalPhotonP4.SetPxPyPzE(signalPhoton_->px(), signalPhoton_->py(), signalPhoton_->pz(), signalPhoton_->energy());
    }
    for (auto* photon : photons_) {
        PhotonInfo pInfo;
        pInfo.p4.SetPxPyPzE(photon->px(), photon->py(), photon->pz(), photon->energy());
        pInfo.origin = classifyPhotonOrigin(photon);
       
        const reco::GenParticle* mother = findMother(photon);
        pInfo.motherPdgId = mother ? mother->pdgId() : 0;
        pInfo.isHardProcess = isFromHardProcess(photon);
       
        // Compute deltaR to muons
        if (muPlus_) {
            pInfo.deltaR_mu1 = deltaR(photon->eta(), photon->phi(),
                                     muPlus_->eta(), muPlus_->phi());
        }
        if (muMinus_) {
            pInfo.deltaR_mu2 = deltaR(photon->eta(), photon->phi(),
                                     muMinus_->eta(), muMinus_->phi());
        }
       
        info_.photons.push_back(pInfo);
    }
}

void GenDecayChain::extractVertexInfo() {
    if (!BMeson_) return;
    info_.Bvtx.SetXYZ(BMeson_->vx(), BMeson_->vy(), BMeson_->vz());
    if (BMeson_->numberOfDaughters() > 0) {
        const reco::GenParticle* firstDau =
            dynamic_cast<const reco::GenParticle*>(BMeson_->daughter(0));
        // find the first daughter after oscillations
        while (firstDau && abs(firstDau->pdgId()) == abs(info_.bMesonPdgId) &&
               firstDau->numberOfDaughters() > 0) {
            firstDau = dynamic_cast<const reco::GenParticle*>(firstDau->daughter(0));
        }
        if (firstDau) {
            info_.Bdecayvtx.SetXYZ(firstDau->vx(), firstDau->vy(), firstDau->vz());
        }
    } else {
        info_.Bdecayvtx = info_.Bvtx;
    }
}

void GenDecayChain::computeAngularVariables() {
   
    if (info_.is3BodyDecay && signalPhoton_) {
        TLorentzVector photon(signalPhoton_->px(), signalPhoton_->py(), signalPhoton_->pz(), signalPhoton_->energy());
        info_.helicity3 = computeHelicity(photon, info_.Bp4);
       
        if (info_.hasMuonPlus && info_.hasMuonMinus) {
            info_.coplanarity3 = computeCoplanarity(info_.muPlusP4, info_.muMinusP4,
                                                   photon, info_.Bp4);
            info_.cosTheta_l = computeLeptonAngle(info_.muPlusP4, info_.muMinusP4, info_.Bp4);
            info_.triBodyMass = (info_.muPlusP4 + info_.muMinusP4 + photon).M();
        }
    }
}


void GenDecayChain::fillDileptonVariables() {
    if (info_.hasMuonPlus && info_.hasMuonMinus) {
        TLorentzVector dimuon = info_.muPlusP4 + info_.muMinusP4;
        info_.dimuonMass = dimuon.M();
        info_.dimuonPt = dimuon.Pt();
    }
}


float GenDecayChain::computeHelicity(const TLorentzVector& daughter,
                                    const TLorentzVector& parent) const {
    // Boost daughter to parent rest frame
    TLorentzVector dau_boosted = daughter;
    TVector3 boost = -parent.BoostVector();
    dau_boosted.Boost(boost);
   
    // Helicity = cos(theta) in parent rest frame
    TVector3 parentDir = parent.Vect().Unit();
    TVector3 dauDir = dau_boosted.Vect().Unit();
   
    return parentDir.Dot(dauDir);
}


float GenDecayChain::computeCoplanarity(const TLorentzVector& p1, const TLorentzVector& p2,
                                       const TLorentzVector& p3, const TLorentzVector& p4) const {
    // Plane 1: defined by p1 and p2
    TVector3 normal1 = p1.Vect().Cross(p2.Vect());
   
    // Plane 2: defined by p3 and p4
    TVector3 normal2 = p3.Vect().Cross(p4.Vect());
   
    // Angle between planes
    if (normal1.Mag() < 1e-10 || normal2.Mag() < 1e-10) return -999;
   
    float cosPhi = normal1.Dot(normal2) / (normal1.Mag() * normal2.Mag());
    cosPhi = std::max(-1.0f, std::min(1.0f, cosPhi));  // Clamp to [-1,1]
   
    return acos(cosPhi);
}

// lepton angle in dilepton rest frame

float GenDecayChain::computeLeptonAngle(const TLorentzVector& lPlus,
                                       const TLorentzVector& lMinus,
                                       const TLorentzVector& parent) const {
    TLorentzVector dilepton = lPlus + lMinus;
   
    // Boost l+ to dilepton rest frame
    TLorentzVector lPlus_boosted = lPlus;
    TVector3 boost = -dilepton.BoostVector();
    lPlus_boosted.Boost(boost);
   
    // cos(theta_l) = direction of l+ relative to dilepton direction
    TVector3 dileptonDir = dilepton.Vect().Unit();
    TVector3 lPlusDir = lPlus_boosted.Vect().Unit();
   
    return dileptonDir.Dot(lPlusDir);
}

// ============================================================================
// Classify photon origin
// ============================================================================
// The signal photon comes from the B decay; every other photon of the chain
// (extra B daughters or resonance daughters) is FSR
int GenDecayChain::classifyPhotonOrigin(const reco::GenParticle* photon) const {
    if (!photon) return PhotonOrigin::pUnknown;
    if (photon == signalPhoton_) return PhotonOrigin::pFromB;
    return PhotonOrigin::pFSR;
}

bool GenDecayChain::isFromHardProcess(const reco::GenParticle* particle) const {
    if (!particle) return false;
    return (particle->isHardProcess() || particle->fromHardProcessFinalState());
}


const reco::GenParticle* GenDecayChain::findMother(const reco::GenParticle* particle) const {
    if (!particle || particle->numberOfMothers() == 0) return nullptr;
    return dynamic_cast<const reco::GenParticle*>(particle->mother(0));
}


// ============================================================================
// Truth matching restricted to the particles of the found decay chain
// ============================================================================
GenDecayChain::TruthMatchInfo GenDecayChain::matchMuonToChain(
    const reco::Candidate& recoMuon, float deltaRCut, float deltaPtCut) const {

    // mu+ has pdgId -13
    const reco::GenParticle* genMuon = (recoMuon.charge() > 0) ? muPlus_ : muMinus_;
    if (!genMuon) return TruthMatchInfo();
    return matchToParticles(recoMuon, {genMuon}, deltaRCut, deltaPtCut);
}

GenDecayChain::TruthMatchInfo GenDecayChain::matchPhotonToChain(
    const reco::Candidate& recoPhoton, float deltaRCut, float deltaPtCut) const {

    // match only to the signal photon (not to the FSR photons)
    if (!signalPhoton_) return TruthMatchInfo();
    return matchToParticles(recoPhoton, {signalPhoton_}, deltaRCut, deltaPtCut);
}

GenDecayChain::TruthMatchInfo GenDecayChain::matchToParticles(
    const reco::Candidate& recoCand,
    const std::vector<const reco::GenParticle*>& genCands,
    float deltaRCut,
    float deltaPtCut
) const {

    TruthMatchInfo matchInfo;
    const reco::GenParticle* bestMatch = nullptr;
    double minDeltaR = deltaRCut;

    for (const auto* genPart : genCands) {
        if (!genPart || genPart->pt() <= 0) continue;

        double dR = deltaR(genPart->eta(), genPart->phi(), recoCand.eta(), recoCand.phi());
        float ptDiff = std::abs(genPart->pt() - recoCand.pt()) / genPart->pt();

        if (dR < minDeltaR && ptDiff < deltaPtCut) {
            minDeltaR = dR;
            bestMatch = genPart;
        }
    }

    if (!bestMatch) return matchInfo;

    matchInfo.isMatched = true;
    matchInfo.genPdgId  = bestMatch->pdgId();
    matchInfo.deltaR    = minDeltaR;
    matchInfo.deltaPt   = std::abs(bestMatch->pt() - recoCand.pt()) / bestMatch->pt();
    matchInfo.genIndex  = bestMatch - genParticles_.data();

    const reco::GenParticle* mother = findMother(bestMatch);
    if (mother) {
        matchInfo.genMotherPdgId = mother->pdgId();
        const reco::GenParticle* gmother = findMother(mother);
        if (gmother) matchInfo.genGMotherPdgId = gmother->pdgId();
    }

    return matchInfo;
}
