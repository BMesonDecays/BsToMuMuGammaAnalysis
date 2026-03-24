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
    jpsi_(nullptr),
    neutralMeson_(nullptr),
    vectorMeson_(nullptr)
{
    analyze();
}
void GenDecayChain::analyze() {
    findBMeson();
   
    if (!BMeson_) {
        info_.isValid = false;
        return;
    }
    walkDecayChain();
    extractKinematics();
    computeLifetimeVariables();
    classifyDecayMode();
    computeAngularVariables();
    fillDileptonVariables();
    sortPhotonsByPt();  
    info_.isValid = true;
    info_.isHardProcess = isFromHardProcess(BMeson_);
}


void GenDecayChain::findBMeson() {
    // Priority: Look for Bs (531), Bd (511), then pi0 (111)
    // Take the first hard-process B meson found
   
    for (const auto& p : genParticles_) {
        int absId = abs(p.pdgId());
        if (absId == 531 || absId == 511) {
            // look for muons in the decay chain
            int nMuons = countMuonsInDecayChain(&p);

            if (nMuons == 2) {
                // now find the oldest ancestor B meson in the chain (handle oscillations)
                const reco::GenParticle* current = &p;
                while (abs(current->mother()->pdgId()) == absId)
                {
                    const reco::GenParticle* mother = dynamic_cast<const reco::GenParticle*>(current->mother());
                    if (!mother) break;
                    current = mother;
                }
                BMeson_ = current;
                info_.motherPdgId = current->pdgId();
                return;
            }
        }
    }
    for (const auto& p : genParticles_) {
        int absId = abs(p.pdgId());
        if (absId == 111) {
            BMeson_ = &p;
            info_.motherPdgId = p.pdgId();
            return;
        }
    }
}


void GenDecayChain::walkDecayChain() {
    if (!BMeson_) return;
    processDaughters(BMeson_);
    identifyResonances();
}

// Process daughters of a given particle
void GenDecayChain::processDaughters(const reco::GenParticle* mother) {
    if (!mother) return;
   
    for (size_t i = 0; i < mother->numberOfDaughters(); ++i) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(mother->daughter(i));
        if (!dau) continue;
       
        int dauId = dau->pdgId();
        int absDauId = abs(dauId);
       
        // Muons
        if (absDauId == 13) {
            if (dauId == -13) {  // mu+
                muPlus_ = dau;
            } else if (dauId == 13) {  // mu-
                muMinus_ = dau;
            }
        }
        // Photons
        else if (absDauId == 22) {
            photons_.push_back(dau);
        }
        // J/psi
        else if (absDauId == 443) {
            jpsi_ = dau;
            processCascade(dau);  // Process J/psi daughters
        }
        // K*0, K*+, K*-
        else if (absDauId == 313 || absDauId == 323) {
            vectorMeson_ = dau;
            processCascade(dau);
        }
        // phi
        else if (absDauId == 333) {
            vectorMeson_ = dau;
            processCascade(dau);
        }
        // Neutral mesons: pi0, eta, etap
        else if (absDauId == 111 || absDauId == 221 || absDauId == 331) {
            neutralMeson_ = dau;
            processCascade(dau);
        }
        // If daughter is another B meson (rare oscillation), skip
        else if (absDauId == 531 || absDauId == 511) {
            // Recursive call for oscillated B
            processDaughters(dau);
        }
        // Other intermediate states
        else {
            // Recursively process cascade decays
            processCascade(dau);
        }
    }
}


void GenDecayChain::processCascade(const reco::GenParticle* intermediate) {
    if (!intermediate) return;
   
    for (size_t i = 0; i < intermediate->numberOfDaughters(); ++i) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(intermediate->daughter(i));
        if (!dau) continue;
       
        int dauId = dau->pdgId();
        int absDauId = abs(dauId);
       
        // Collect muons from resonance decays
        if (absDauId == 13) {
            if (dauId == -13 && !muPlus_) {
                muPlus_ = dau;
            } else if (dauId == 13 && !muMinus_) {
                muMinus_ = dau;
            }
        }
        // Collect photons from neutral meson decays
        else if (absDauId == 22) {
            photons_.push_back(dau);
        }
        // Further cascades (rare, but handle them)
        else {
            processCascade(dau);
        }
    }
}


void GenDecayChain::identifyResonances() {
    if (jpsi_) {
        info_.jpsiPdgId = jpsi_->pdgId();
        info_.jpsiP4.SetPxPyPzE(jpsi_->px(), jpsi_->py(), jpsi_->pz(), jpsi_->energy());
        info_.hasJpsi = true;
    }
   
    if (neutralMeson_) {
        info_.neutralMesonId = neutralMeson_->pdgId();
        info_.neutralMesonP4.SetPxPyPzE(neutralMeson_->px(), neutralMeson_->py(),
                                       neutralMeson_->pz(), neutralMeson_->energy());
        info_.hasNeutralMeson = true;
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

//This is wrong : I have to it with proper fitting of vertex
void GenDecayChain::computeLifetimeVariables() {
    if (!BMeson_) return;
    extractVertexInfo();
    float dx = info_.Bdecayvtx.X() - info_.Bvtx.X();
    float dy = info_.Bdecayvtx.Y() - info_.Bvtx.Y();
    float dz = info_.Bdecayvtx.Z() - info_.Bvtx.Z();
    info_.BLxy = sqrt(dx*dx + dy*dy);
    float BL = sqrt(dx*dx + dy*dy + dz*dz);
    float mB = info_.Bp4.M();
    float ptB = info_.Bp4.Pt();
   
    if (ptB > 0 && mB > 0) {
        info_.Bct2D = info_.BLxy * mB / ptB;
        info_.Bct3D = BL * mB / info_.Bp4.P();
    }
}


void GenDecayChain::extractVertexInfo() {
    if (!BMeson_) return;
    info_.Bvtx.SetXYZ(BMeson_->vx(), BMeson_->vy(), BMeson_->vz());
    if (BMeson_->numberOfDaughters() > 0) {
        const reco::GenParticle* firstDau =
            dynamic_cast<const reco::GenParticle*>(BMeson_->daughter(0));
        // find the first daughter after oscillations
        while (firstDau && abs(firstDau->pdgId()) == abs(info_.motherPdgId) &&
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

void GenDecayChain::classifyDecayMode() {
    int absBId = abs(info_.motherPdgId);
    int nMu = info_.nMuons;
    int nGam = info_.nPhotons;
   
   
    // Pi0 gun sample

    if (absBId == 111) {
        info_.decayMode = kPi0_Gun;
        info_.is3BodyDecay = false;
        info_.is4BodyDecay = false;
        return;
    }
   
    
    // Three-body decays: B → μμγ
   
    if (nMu == 2 && nGam == 1 && !info_.hasJpsi && !info_.hasNeutralMeson) {
        info_.is3BodyDecay = true;
       
        if (absBId == 531) {
            // Check for vector meson intermediate states
            if (info_.hasVectorMeson) {
                int vecId = abs(info_.vectorMesonId);
                if (vecId == 313) info_.decayMode = kBs_Kstar0_Gamma;
                else if (vecId == 333) info_.decayMode = kBs_Phi_Gamma;
                else info_.decayMode = kBs_MuMu_Gamma;  // Default
            } else {
                info_.decayMode = kBs_MuMu_Gamma;  // Direct Bs → μμγ
            }
        } else if (absBId == 511) {
            if (info_.hasVectorMeson) {
                info_.decayMode = kBd_Kstar_Gamma;
            } else {
                info_.decayMode = kBd_MuMu_Gamma;  // Direct Bd → μμγ
            }
        } else {
            info_.decayMode = kUnknown;
        }
        return;
    }
   
    
    // J/psi radiative: B → J/ψ(→μμ) + γ
 
    if (nMu == 2 && nGam == 1 && info_.hasJpsi && !info_.hasNeutralMeson) {
        info_.is3BodyDecay = true;
       
        if (absBId == 531) {
            info_.decayMode = kBs_Jpsi_Gamma;
        } else if (absBId == 511) {
            info_.decayMode = kBd_Jpsi_Gamma;
        } else {
            info_.decayMode = kUnknown;
        }
        return;
    }
   
    // ========================================================================
    // Four-body decays: B → J/ψ(→μμ) + X(→γγ)
    // ========================================================================
    if (nMu == 2 && nGam == 2 && info_.hasJpsi && info_.hasNeutralMeson) {
        info_.is4BodyDecay = true;
       
        int neutralId = abs(info_.neutralMesonId);
        if (neutralId == 111) {
            info_.decayMode = kBs_Jpsi_Pi0;
        } else if (neutralId == 221) {
            info_.decayMode = kBs_Jpsi_Eta;
        } else if (neutralId == 331) {
            info_.decayMode = kBs_Jpsi_Etap;
        } else {
            info_.decayMode = kUnknown;
        }
        return;
    }
    info_.decayMode = kUnknown;
}


void GenDecayChain::computeAngularVariables() {
   
    if (info_.is3BodyDecay && info_.nPhotons >= 1) {
        TLorentzVector photon = info_.photons[0].p4;
        info_.helicity3 = computeHelicity(photon, info_.Bp4);
       
        if (info_.hasMuonPlus && info_.hasMuonMinus) {
            info_.coplanarity3 = computeCoplanarity(info_.muPlusP4, info_.muMinusP4,
                                                   photon, info_.Bp4);
            info_.cosTheta_l = computeLeptonAngle(info_.muPlusP4, info_.muMinusP4, info_.Bp4);
            info_.triBodyMass = (info_.muPlusP4 + info_.muMinusP4 + photon).M();
        }
    }
   
   
    if (info_.is4BodyDecay && info_.nPhotons >= 2) {
        TLorentzVector photon1 = info_.photons[0].p4;
        TLorentzVector photon2 = info_.photons[1].p4;
        TLorentzVector diphoton = photon1 + photon2;
       
        info_.helicity4 = computeHelicity(diphoton, info_.Bp4);
       
        if (info_.hasMuonPlus && info_.hasMuonMinus) {
            info_.coplanarity4 = computeCoplanarity(info_.muPlusP4, info_.muMinusP4,
                                                   diphoton, info_.Bp4);
            info_.fourBodyMass = (info_.muPlusP4 + info_.muMinusP4 + diphoton).M();
        }
       
        info_.diphotonMass = diphoton.M();
        info_.diphotonPt = diphoton.Pt();
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
int GenDecayChain::classifyPhotonOrigin(const reco::GenParticle* photon) const {
    if (!photon) return PhotonOrigin::pUnknown;
   
    const reco::GenParticle* mother = findMother(photon);
   
    // Orphan photon (no mother)
    if (!mother) return PhotonOrigin::pOrphan;
   
    int absMomId = abs(mother->pdgId());
   
    // FSR from muon
    if (absMomId == 13) return PhotonOrigin::pFSR;
   
    // Direct from B hadron
    if (absMomId == 531 || absMomId == 511 || absMomId == 521 || absMomId == 541) {
        return PhotonOrigin::pFromB;
    }
   
    // From J/psi, phi, K*
    if (absMomId == 443 || absMomId == 333 || absMomId == 313 || absMomId == 323) {
        return PhotonOrigin::pFromResonance;
    }
   
    // From neutral mesons
    if (absMomId == 111 || absMomId == 221 || absMomId == 331) {
        return PhotonOrigin::pFromNeutral;
    }
   
    // Check grandmother for cascades
    const reco::GenParticle* grandmother = findMother(mother);
    if (grandmother) {
        int absGmId = abs(grandmother->pdgId());
        if (absGmId == 531 || absGmId == 511) {
            if (absMomId == 111 || absMomId == 221 || absMomId == 331) {
                return PhotonOrigin::pFromNeutral;
            }
            return PhotonOrigin::pFromResonance;
        }
    }
   
    return PhotonOrigin::pUnknown;
}

bool GenDecayChain::isFromHardProcess(const reco::GenParticle* particle) const {
    if (!particle) return false;
    return (particle->isHardProcess() || particle->fromHardProcessFinalState());
}


const reco::GenParticle* GenDecayChain::findMother(const reco::GenParticle* particle) const {
    if (!particle || particle->numberOfMothers() == 0) return nullptr;
    return dynamic_cast<const reco::GenParticle*>(particle->mother(0));
}


bool GenDecayChain::isAncestor(const reco::GenParticle* ancestor,
                              const reco::GenParticle* particle) const {
    if (!ancestor || !particle) return false;
   
    const reco::GenParticle* current = particle;
    while (current) {
        if (current == ancestor) return true;
        current = findMother(current);
    }
    return false;
}


void GenDecayChain::sortPhotonsByPt() {
    std::sort(info_.photons.begin(), info_.photons.end(),
             [](const PhotonInfo& a, const PhotonInfo& b) {
                 return a.p4.Pt() > b.p4.Pt();
             });
}

// ============================================================================
// Truth matching: Match reco particle to gen particle
// ============================================================================
GenDecayChain::TruthMatchInfo GenDecayChain::matchToGen(
    const reco::Candidate& recoTrack,
    int expectedPdgId,
    int expectedMotherPdgId,
    int expectedGMotherPdgId,
    float deltaRCut,
    float deltaPtCut
) const {
   
    TruthMatchInfo matchInfo;
   
    double minDeltaR = 999.;
    int bestMatchIdx = -1;
   
    // Use stored genParticles_ reference
    for (size_t i = 0; i < genParticles_.size(); ++i) {
        const reco::GenParticle& genPart = genParticles_[i];
       
        if (expectedPdgId != 0 && abs(genPart.pdgId()) != abs(expectedPdgId)) {
            continue;
        }
       
        double dR = deltaR(genPart.eta(), genPart.phi(),
                          recoTrack.eta(), recoTrack.phi());
       
        if (dR < minDeltaR && dR < deltaRCut) {
            float genPt = genPart.pt();
            float recoPt = recoTrack.pt();
            float ptDiff = std::abs(genPt - recoPt) / genPt;
           
            if (ptDiff < deltaPtCut) {
                minDeltaR = dR;
                bestMatchIdx = i;
            }
        }
    }
   
    if (bestMatchIdx >= 0) {
        const reco::GenParticle& genPart = genParticles_[bestMatchIdx];
       
        matchInfo.genPdgId = genPart.pdgId();
        matchInfo.deltaR = minDeltaR;
        matchInfo.genIndex = bestMatchIdx;
        matchInfo.deltaPt = std::abs(genPart.pt() - recoTrack.pt()) / genPart.pt();
       
        if (genPart.numberOfMothers() > 0) {
            const reco::GenParticle* mother =
                dynamic_cast<const reco::GenParticle*>(genPart.mother(0));
            if (mother) {
                matchInfo.genMotherPdgId = mother->pdgId();
               
                if (mother->numberOfMothers() > 0) {
                    const reco::GenParticle* gmother =
                        dynamic_cast<const reco::GenParticle*>(mother->mother(0));
                    if (gmother) {
                        matchInfo.genGMotherPdgId = gmother->pdgId();
                    }
                }
            }
        }
       
        bool motherOK = (expectedMotherPdgId == 0) ||
                       (abs(matchInfo.genMotherPdgId) == abs(expectedMotherPdgId));
        bool gmotherOK = (expectedGMotherPdgId == 0) ||
                        (abs(matchInfo.genGMotherPdgId) == abs(expectedGMotherPdgId));
       
        matchInfo.isMatched = motherOK && gmotherOK;
    }
   
    return matchInfo;
}


// ============================================================================
// Match complete topology
// ============================================================================
GenDecayChain::TopologyMatch GenDecayChain::matchTopology(
    const reco::Candidate& recoMu1,
    const reco::Candidate& recoMu2,
    const std::vector<const reco::Photon*>& recoPhotons
) const {
   
    TopologyMatch topoMatch;
   
    // Match muons (no need to pass genParticles!)
    topoMatch.mu1Info = matchToGen(recoMu1, 13, 0, 531, 0.05, 0.5);
    topoMatch.mu2Info = matchToGen(recoMu2, 13, 0, 531, 0.05, 0.5);
   
    topoMatch.mu1Matched = topoMatch.mu1Info.isMatched;
    topoMatch.mu2Matched = topoMatch.mu2Info.isMatched;
   
    // Match photons
    for (size_t i = 0; i < recoPhotons.size(); ++i) {
        if (!recoPhotons[i]) continue;
       
        TruthMatchInfo photonInfo = matchToGen(*recoPhotons[i], 22, 0, 0, 0.1, 0.5);
        topoMatch.photonInfo.push_back(photonInfo);
       
        if (i == 0 && photonInfo.isMatched) topoMatch.photon1Matched = true;
        if (i == 1 && photonInfo.isMatched) topoMatch.photon2Matched = true;
    }
   
    bool dimuonMatched = topoMatch.mu1Matched && topoMatch.mu2Matched;
    bool photonsMatched = (recoPhotons.size() == 0) ||
                         (recoPhotons.size() == 1 && topoMatch.photon1Matched) ||
                         (recoPhotons.size() >= 2 && topoMatch.photon1Matched && topoMatch.photon2Matched);
   
    topoMatch.isMatched = dimuonMatched && photonsMatched;
   
    if (topoMatch.isMatched && info_.isValid) {
        if (info_.decayMode == kBs_MuMu_Gamma && recoPhotons.size() == 1) {
            topoMatch.decayModeMatched = true;
        }
        else if ((info_.decayMode == kBs_Jpsi_Pi0 ||
                  info_.decayMode == kBs_Jpsi_Eta ||
                  info_.decayMode == kBs_Jpsi_Etap) &&
                 recoPhotons.size() == 2) {
            topoMatch.decayModeMatched = true;
        }
    }
   
    return topoMatch;
}

int GenDecayChain::countMuonsInDecayChain(const reco::GenParticle* particle) const {
    if (!particle) return 0;
   
    int count = 0;
    for (size_t i = 0; i < particle->numberOfDaughters(); ++i) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(particle->daughter(i));
        if (!dau) continue;
       
        int absId = abs(dau->pdgId());
        if (absId == 13) {
            count++;
        } else {
            count += countMuonsInDecayChain(dau);
        }
    }
    return count;
}