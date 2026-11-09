#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_GenChain_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_GenChain_h

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/Candidate/interface/CompositeCandidateFwd.h"


#include <iterator>
#include <algorithm>
#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include <vector>
#include <unordered_map>

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include "TMath.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>



enum DecayMode {
    kUnknown           = 0,
    kBs_MuMu_Gamma     = 1,
    kBs_Jpsi_Gamma     = 2,
    kBd_Jpsi_Gamma     = 3,
    kBs_Kstar0_Gamma   = 4,
    kBs_Phi_Gamma      = 5,
    kBd_Kstar_Gamma    = 6,
    kBs_Jpsi_Eta       = 7,
    kBs_Jpsi_Etap      = 8,
    kBs_Jpsi_Pi0       = 9,
    kBd_MuMu_Gamma     = 10,
    kPi0_Gun           = 11,
    kBd_Jpsi_Pi0       = 12,
    kBd_Jpsi_Eta       = 13,
    kBd_Jpsi_Etap      = 14
};

namespace PhotonOrigin {
    enum Type {
        pUnknown       = 0,
        pFromB         = 1,
        pFromResonance = 2,
        pFromNeutral   = 3,
        pFSR           = 4,
        pISR           = 5,
        pOrphan        = 6,
        pPileup        = 7
    };
}


class GenDecayChain {
public:
   
  
    struct PhotonInfo {
        TLorentzVector p4;
        int origin;
        int motherPdgId;
        float deltaR_mu1;
        float deltaR_mu2;
        bool isHardProcess;
       
        PhotonInfo() : origin(PhotonOrigin::pUnknown), motherPdgId(0),
                      deltaR_mu1(-1), deltaR_mu2(-1), isHardProcess(false) {}
    };
   
    
    struct GenInfo {
        
        int motherPdgId;
        TLorentzVector Bp4;
        TVector3 Bvtx;
        TVector3 Bdecayvtx;
        float BLxy;
        float Bct2D;
        float Bct3D;
       
        bool hasMuonPlus;
        bool hasMuonMinus;
        TLorentzVector muPlusP4;
        TLorentzVector muMinusP4;
        int muPlusMotherId;
        int muMinusMotherId;
       
        std::vector<PhotonInfo> photons;
       
        int jpsiPdgId;
        TLorentzVector jpsiP4;
        int neutralMesonId;
        TLorentzVector neutralMesonP4;
        int vectorMesonId;
        TLorentzVector vectorMesonP4;
       
        float helicity3;
        float coplanarity3;
        float cosTheta_l;
       
        float helicity4;
        float coplanarity4;
       
        float dimuonMass;
        float dimuonPt;
        float diphotonMass;
        float diphotonPt;
        float triBodyMass;
        float fourBodyMass;
       
        DecayMode decayMode;
        bool is3BodyDecay;
        bool is4BodyDecay;
        bool hasJpsi;
        bool hasNeutralMeson;
        bool hasVectorMeson;
       
        bool isValid;
        bool isHardProcess;
        int nMuons;
        int nPhotons;
       
        GenInfo() :
            motherPdgId(0),
            BLxy(-999), Bct2D(-999), Bct3D(-999),
            hasMuonPlus(false), hasMuonMinus(false),
            muPlusMotherId(0), muMinusMotherId(0),
            jpsiPdgId(0), neutralMesonId(0), vectorMesonId(0),
            helicity3(-999), coplanarity3(-999), cosTheta_l(-999),
            helicity4(-999), coplanarity4(-999),
            dimuonMass(-999), dimuonPt(-999),
            diphotonMass(-999), diphotonPt(-999),
            triBodyMass(-999), fourBodyMass(-999),
            decayMode(kUnknown),
            is3BodyDecay(false), is4BodyDecay(false),
            hasJpsi(false), hasNeutralMeson(false), hasVectorMeson(false),
            isValid(false), isHardProcess(false),
            nMuons(0), nPhotons(0)
        {}
    };
   
    // ------------------------------------------------------------------------
    // Truth matching structures
    // ------------------------------------------------------------------------
    struct TruthMatchInfo {
        bool isMatched;
        int genPdgId;
        int genMotherPdgId;
        int genGMotherPdgId;
        float deltaR;
        float deltaPt;
        int genIndex;
       
        TruthMatchInfo() :
            isMatched(false), genPdgId(0), genMotherPdgId(0),
            genGMotherPdgId(0), deltaR(999), deltaPt(999), genIndex(-1) {}
    };
   
    struct TopologyMatch {
        bool isMatched;
        bool mu1Matched;
        bool mu2Matched;
        bool photon1Matched;
        bool photon2Matched;
        bool decayModeMatched;
       
        TruthMatchInfo mu1Info;
        TruthMatchInfo mu2Info;
        std::vector<TruthMatchInfo> photonInfo;
       
        TopologyMatch() :
            isMatched(false), mu1Matched(false), mu2Matched(false),
            photon1Matched(false), photon2Matched(false),
            decayModeMatched(false) {}
    };
   
    // genParticles: genParticles (AOD) or prunedGenParticles (MiniAOD).
    // packed: packedGenParticles (MiniAOD), or nullptr. The pruned collection drops most final-state
    // photons (e.g. from pi0 / eta decays); with the packed collection they are attached to their
    // pruned mother when the decay is walked, and the truth matching uses the packed final state.
    explicit GenDecayChain(const std::vector<reco::GenParticle>& genParticles,
                           const std::vector<pat::PackedGenParticle>* packed = nullptr);
   
    ~GenDecayChain() = default;
    const GenInfo& info() const { return info_; }
    TruthMatchInfo matchToGen(
        const reco::Candidate& recoTrack,
        int expectedPdgId = 0,
        int expectedMotherPdgId = 0,
        int expectedGMotherPdgId = 0,
        float deltaRCut = 0.05,
        float deltaPtCut = 0.5
    ) const;
   
    // Convenience methods
    TruthMatchInfo matchMuonToGen(const reco::Candidate& recoMuon) const { return matchToGen(recoMuon, 13, 0, 0, 0.05, 0.5); }
    TruthMatchInfo matchPhotonToGen(const reco::Candidate& recoPhoton)const { return matchToGen(recoPhoton, 22, 0, 0, 0.1, 0.5); }
    TopologyMatch matchTopology(
        const reco::Candidate& recoMu1,
        const reco::Candidate& recoMu2,
        const std::vector<const reco::Photon*>& recoPhotons
    ) const;
   
    
    bool isSignalLike() const {
        return (info_.decayMode == kBs_MuMu_Gamma ||
                info_.decayMode == kBd_MuMu_Gamma);
    }
   
    bool isJpsiBackground() const { return info_.hasJpsi; }

    // true if the gen particle of a match descends from the signal B of the event
    bool fromSignalB(const TruthMatchInfo& match) const;
    // the gen particle of a match (nullptr if not matched)
    const reco::Candidate* genParticle(const TruthMatchInfo& match) const {
        return (match.genIndex >= 0) ? finalState_[match.genIndex] : nullptr;
    }
   
private:
    GenInfo info_;
    const std::vector<reco::GenParticle>& genParticles_;
    // final-state particles used for the truth matching: packed (MiniAOD) or status-1 genParticles (AOD)
    std::vector<const reco::Candidate*> finalState_;
    // packed final-state particles that are not in the pruned collection, keyed by their pruned mother
    std::unordered_multimap<const reco::Candidate*, const reco::Candidate*> packedChildren_;

    const reco::Candidate* BMeson_;
    const reco::Candidate* muPlus_;
    const reco::Candidate* muMinus_;
    std::vector<const reco::Candidate*> photons_;
    const reco::Candidate* jpsi_;
    const reco::Candidate* neutralMeson_;
    const reco::Candidate* vectorMeson_;

    // daughters of a particle, including the packed final-state children (MiniAOD)
    std::vector<const reco::Candidate*> daughters(const reco::Candidate* mother) const;
   
 
    void analyze();
    void findBMeson();
    void walkDecayChain();
    void extractKinematics();
    void computeLifetimeVariables();
    void classifyDecayMode();
    void computeAngularVariables();
    void fillDileptonVariables();
   
  
    void processDaughters(const reco::Candidate* mother);
    void processCascade(const reco::Candidate* intermediate);
    void identifyResonances();
    int classifyPhotonOrigin(const reco::Candidate* photon) const;
    bool isFromHardProcess(const reco::Candidate* particle) const;
   
  
    float computeHelicity(const TLorentzVector& daughter, const TLorentzVector& parent) const;
    float computeCoplanarity(const TLorentzVector& p1, const TLorentzVector& p2,
                            const TLorentzVector& p3, const TLorentzVector& p4) const;
    float computeLeptonAngle(const TLorentzVector& lPlus, const TLorentzVector& lMinus,
                            const TLorentzVector& parent) const;
   
    void extractVertexInfo();
    float computeProperTime(const TVector3& prod, const TVector3& decay,
                           const TLorentzVector& p4) const;
   
    const reco::Candidate* findMother(const reco::Candidate* particle) const;
    bool isAncestor(const reco::Candidate* ancestor, const reco::Candidate* particle) const;
    void sortPhotonsByPt();

    // muons from the decay of a particle; open-charm hadrons and taus are not followed
    // (semileptonic cascades of the other b hadron of the event)
    int countMuonsInDecayChain(const reco::Candidate* particle) const;
};

#endif