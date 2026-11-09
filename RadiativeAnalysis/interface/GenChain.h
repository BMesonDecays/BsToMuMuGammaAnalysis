#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_GenChain_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_GenChain_h

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
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
    kBs_Phi_Gamma      = 5,     // Bs -> phi(-> mu mu) gamma
    kBd_MuMu_Gamma     = 10,
    kBs_Psi2S_Gamma    = 12,    // Bs -> psi(2S)(-> mu mu) gamma
    kBd_Psi2S_Gamma    = 13     // Bd -> psi(2S)(-> mu mu) gamma
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

        int bMesonPdgId;
        TLorentzVector Bp4;
        TVector3 Bvtx;
        TVector3 Bdecayvtx;
       
        bool hasMuonPlus;
        bool hasMuonMinus;
        TLorentzVector muPlusP4;
        TLorentzVector muMinusP4;
        int muPlusMotherId;
        int muMinusMotherId;
       
        std::vector<PhotonInfo> photons;        // signal photon + FSR photons
        TLorentzVector signalPhotonP4;          // the photon from the B decay vertex
       
        int jpsiPdgId;
        TLorentzVector jpsiP4;
        int vectorMesonId;
        TLorentzVector vectorMesonP4;
       
        float helicity3;
        float coplanarity3;
        float cosTheta_l;

        float dimuonMass;
        float dimuonPt;
        float triBodyMass;

        DecayMode decayMode;
        bool is3BodyDecay;
        bool hasJpsi;
        bool hasVectorMeson;
       
        bool isValid;
        bool isHardProcess;
        int nMuons;
        int nPhotons;
       
        GenInfo() :
            bMesonPdgId(0),
            hasMuonPlus(false), hasMuonMinus(false),
            muPlusMotherId(0), muMinusMotherId(0),
            jpsiPdgId(0), vectorMesonId(0),
            helicity3(-999), coplanarity3(-999), cosTheta_l(-999),
            dimuonMass(-999), dimuonPt(-999),
            triBodyMass(-999),
            decayMode(kUnknown),
            is3BodyDecay(false),
            hasJpsi(false), hasVectorMeson(false),
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

    // ------------------------------------------------------------------------
    // Constructor: Store reference to gen particles
    // ------------------------------------------------------------------------
    explicit GenDecayChain(const std::vector<reco::GenParticle>& genParticles);
   
    ~GenDecayChain() = default;
    const GenInfo& info() const { return info_; }

    // Match only to the particles of the found decay chain (not to any gen particle in the event)
    // muons are matched to the gen muon of the same charge, photons only to the signal photon (not FSR)
    TruthMatchInfo matchMuonToChain(const reco::Candidate& recoMuon, float deltaRCut = 0.05, float deltaPtCut = 0.5) const;
    TruthMatchInfo matchPhotonToChain(const reco::Candidate& recoPhoton, float deltaRCut = 0.1, float deltaPtCut = 0.5) const;

private:
    GenInfo info_;
    const std::vector<reco::GenParticle>& genParticles_;  
   
  
    const reco::GenParticle* BMeson_;
    const reco::GenParticle* muPlus_;
    const reco::GenParticle* muMinus_;
    std::vector<const reco::GenParticle*> photons_;     // all photons of the chain (signal + FSR)
    const reco::GenParticle* signalPhoton_;              // the photon from the B decay vertex
    const reco::GenParticle* jpsi_;                      // J/psi or psi(2S)
    const reco::GenParticle* vectorMeson_;               // phi


    void analyze();
    void findSignalDecay();
    void extractKinematics();
    void computeAngularVariables();
    void fillDileptonVariables();


    const reco::GenParticle* lastCopy(const reco::GenParticle* particle) const;
    DecayMode matchSignalDecay(const reco::GenParticle* decayingB);
    void identifyResonances();
    int classifyPhotonOrigin(const reco::GenParticle* photon) const;
    bool isFromHardProcess(const reco::GenParticle* particle) const;
   
  
    float computeHelicity(const TLorentzVector& daughter, const TLorentzVector& parent) const;
    float computeCoplanarity(const TLorentzVector& p1, const TLorentzVector& p2,
                            const TLorentzVector& p3, const TLorentzVector& p4) const;
    float computeLeptonAngle(const TLorentzVector& lPlus, const TLorentzVector& lMinus,
                            const TLorentzVector& parent) const;
   
    void extractVertexInfo();

    const reco::GenParticle* findMother(const reco::GenParticle* particle) const;

    TruthMatchInfo matchToParticles(const reco::Candidate& recoCand,
                                    const std::vector<const reco::GenParticle*>& genCands,
                                    float deltaRCut, float deltaPtCut) const;
};

#endif