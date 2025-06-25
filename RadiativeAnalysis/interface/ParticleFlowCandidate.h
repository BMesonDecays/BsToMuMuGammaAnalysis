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
	};
	std::vector<PFCandidateVariables> PFCandObservables(const std::vector<reco::PFCandidate>& pfcandidate);
      
};

#endif // BsToMuMuGammaAnalysis_RadiativeAnalysis_ParticleFlowCandidate_h