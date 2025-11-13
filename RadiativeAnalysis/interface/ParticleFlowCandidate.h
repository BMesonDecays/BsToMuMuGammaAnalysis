#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_ParticleFlowCandidate_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_ParticleFlowCandidate_h

#include <iostream>
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
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateElectronExtra.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
class ParticleFlowCandidate{
    public: 
        ParticleFlowCandidate();
        ~ParticleFlowCandidate(){}
	struct PFCandidateVariables {
		double pt = -9999.0, eta = -9999.0, phi = -9999.0, energy = -9999.0;
        double time = -9999.0, mass = -9999.0, charge = -9999.0;
        double ecalEnergy = -9999.0, rawEcalEnergy = -9999.0;
        double hcalEnergy = -9999.0, rawHcalEnergy = -9999.0;
        double hoEnergy = -9999.0, rawHoEnergy = -9999.0;	

        double refphotonpt = -9999.0, refphotoneta = -9999.0, refphotonphi = -9999.0;
        double refphotonenergy = -9999.0, refphotonet = -9999.0;
        double refphotonsigmaIEtaIEta = -9999.0, refphotonsigmaIEtaIPhi = -9999.0, refphotonsigmaIPhiIPhi = -9999.0, refphotonsigmaEtaEta = -9999.0;
        double refphotone1x5 = -9999.0, refphotone2x5 = -9999.0, refphotone3x3 = -9999.0, refphotone5x5 = -9999.0;
        double refphotonhcalDepth1OverEcal = -9999.0, refphotonhcalDepth2OverEcal = -9999.0;        
        double refphotonhcalDepth1OverEcalBc = -9999.0, refphotonhcalDepth2OverEcalBc = -9999.0;
        double refphotonscEnergy = -9999.0, refphotonscRawEnergy = -9999.0, refphotonscEta = -9999.0, refphotonscPhi = -9999.0;
        double refphotonscEtaWidth = -9999.0, refphotonscPhiWidth = -9999.0, refphotonscBrem = -9999.0;
        double refphotonr9 = -9999.0, refphotonhadTowOverEm = -9999.0;
        double refphotonmaxEnergyXtal = -9999.0, refphotoneffSigmaRR = -9999.0; 
        std::vector<float> refphotonhcalOverEcal;
        std::vector<float> refphotonhcalOverEcalBc;





         
	};
	std::vector<PFCandidateVariables> PFCandObservables(const std::vector<reco::PFCandidate>& pfcandidate);
      
};

#endif // BsToMuMuGammaAnalysis_RadiativeAnalysis_ParticleFlowCandidate_h