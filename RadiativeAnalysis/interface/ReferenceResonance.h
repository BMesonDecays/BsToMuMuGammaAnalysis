#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_ReferenceResonance_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_ReferenceResonance_h
// if(abs(offlineChain.mass - ParticleMass_PDG) >  Particle_Window_PDG)continue



#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include <iostream>

//Root 
#include <TMath.h>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include "Math/Vector4D.h"
#include <vector>
#include <functional>
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Math/interface/LorentzVector.h"

class ReferenceResonance{
    public:
        ReferenceResonance();
        ~ReferenceResonance(){}
	public:
    struct ResonanceDetails {
        double mass = -9999;
	double eta  = -9999;
	double phi  = -9999;
	double pt   = -9999;
        bool isValid = false;
    };


    struct Constants {
        // Mass, mass window and momentum in GeV
        static constexpr double JpsiMass_PDG           = 3.0969;    
        static constexpr double PhiMass_PDG            = 1.019;      
        static constexpr double KStar0Mass_PDG         = 0.89555; 
        static constexpr double nonResonantMass_PDG    = 0.0;   
        static constexpr double EtaMass_PDG            = 0.547862;      
        static constexpr double EtaPrimeMass_PDG       = 0.957780; 
        static constexpr double Pi0Mass_PDG            = 0.1349768;
        static constexpr double BsMass_PDG             = 5.3699;
        static constexpr double BdMass_PDG             = 5.2794;
        static constexpr double BpMass_PDG             = 5.2790;
        static constexpr double JpsiWindow_PDG         = 0.31;
        static constexpr double PhiWindow_PDG          = 0.03;
        static constexpr double nonResonantWindow_PDG  = 0.0;
        static constexpr double KStar0Window_PDG        = 0.02;
        static constexpr double EtaWindow_PDG          = 0.017;
        static constexpr double EtaPrimeWindow_PDG     = 0.230;
        static constexpr double Pi0Window_PDG          = 0.0005;
        static constexpr double BsWindow_Upper         = 6.5;
        static constexpr double PtCut                  = 3.0;          
    };;

    


    static ResonanceDetails applyResonanceMassCut(const reco::Muon& mu1, const reco::Muon& mu2, const std::string& resonanceType, bool verbose = false);
    static ResonanceDetails applyResonanceMassCut(const reco::Photon& photon1, const reco::Photon& photon2, const std::string& resonanceType, bool verbose = false);
    static ResonanceDetails applyResonanceMassCut(const pat::CompositeCandidateCollection& conversions1,const pat::CompositeCandidateCollection& conversions2, const std::string& resonanceType, bool verbose = false);
 

private:
     template <typename T1, typename T2>
    static auto CombinedpFour(const T1& p1, const T2& p2) {
        return (p1.p4() + p2.p4());
    }

    template <typename T1, typename T2, typename Func>
    static auto calculateProperty(const T1& p1, const T2& p2, Func propertyExtractor) {
        auto combinedP4 = CombinedpFour(p1, p2);
        return propertyExtractor(combinedP4);
    }

    //template <typename T1, typename T2>
    //static double calculateMassFromComponents(const T1& p1, const T2& p2);

    static double calculateMassFromComponents(const pat::CompositeCandidateCollection& conv1, const pat::CompositeCandidateCollection& conv2);
};

#endif
