#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TrippleObjectVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeAnalysis.h"


#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include <iostream>
#include <TMath.h>
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
using namespace reco;
using namespace edm;
using namespace std;
using namespace pat;
#include <TMath.h>


TrippleObjectVertex::TrippleObjectVertex(){}
TrippleObjectVertex::DecayChainVariables TrippleObjectVertex::TrippleObjectVertexObservables(
    const std::vector<reco::Muon>& muons,
    const pat::CompositeCandidateCollection& conversions,
    const MagneticField& bField,
    const double nominalMuonMass,
    const double nominalElectronMass){
    DecayChainVariables dcv;
    dcv.mass = 0.0;
    for (size_t i = 0; i < muons.size(); ++i) {
        const reco::Muon& mu1 = muons[i];
        if (mu1.innerTrack().isNull()) continue;
        reco::TrackRef muTrack1 = mu1.track();
        if (!muTrack1) continue;
        reco::TransientTrack muonTT1(muTrack1, &bField);

        for (size_t j = i + 1; j < muons.size(); ++j) {
            const reco::Muon& mu2 = muons[j];
            if (mu2.innerTrack().isNull()) continue;
            if (mu1.charge() * mu2.charge() == 1) continue;
            reco::TrackRef muTrack2 = mu2.track();
            if (!muTrack2) continue;
            reco::TransientTrack muonTT2(muTrack2, &bField);

            std::vector<reco::TransientTrack> tttrk_muons = {muonTT1, muonTT2};

            for (const auto& conv : conversions) {
                const reco::Track eletk0 = *conv.userData<reco::Track>("track0");
                const reco::Track eletk1 = *conv.userData<reco::Track>("track1");
                std::vector<reco::TransientTrack> tttrk_electrons = {
                    reco::TransientTrack(eletk0, &bField),
                    reco::TransientTrack(eletk1, &bField)
                };

                KinematicConstrainedFit BCandFitter;
                bool fitSuccess = BCandFitter.TrippleObjectVertexFit(tttrk_muons, nominalMuonMass, tttrk_electrons, nominalElectronMass);
                if (!fitSuccess) continue;
		std::cout<<"print the fit sucess : "<< fitSuccess<< "\n";
                dcv.mass = BCandFitter.getBhadronMass();
	    }
	}
    }
    return dcv;
}
