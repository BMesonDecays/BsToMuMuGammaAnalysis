#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TetraObjectVertex.h"


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

TetraObjectVertex::TetraObjectVertex(){}
TetraObjectVertex::DecayChainVariables TetraObjectVertex::TetraObjectVertexObservables(
    const std::vector<reco::Muon>& muons,
    const pat::CompositeCandidateCollection& conversions,
    const MagneticField& bField,
    const double nominalMuonMass,
    const double nominalElectronMass){
    DecayChainVariables dcv;
    for(size_t i=0; i < muons.size(); ++i){
	    const pat::Muon & mu1 = muons[i];
                if (mu1.innerTrack().isNull())continue;
                reco::TrackRef muTrack1 = mu1.track();
                if(!muTrack1) continue;
                reco::TransientTrack muonTT1 = reco::TransientTrack(muTrack1, &bField);
                for (size_t j=i+1; j < muons.size(); ++j){
                        const pat::Muon & mu2 = muons[j];
                        if (mu2.innerTrack().isNull())continue;
                        if(mu1.charge()*mu2.charge() ==1 ) continue;
                        reco::TrackRef muTrack2 = mu2.track();
                        if(!muTrack2) continue;
                        reco::TransientTrack muonTT2 = reco::TransientTrack(muTrack2, &bField);
                        std::vector<TransientTrack> tttrk_muons;
                        tttrk_muons.push_back(muonTT1);
                        tttrk_muons.push_back(muonTT2);
			for (const auto& conv1 : conversions) {
				std::vector<reco::TransientTrack> tttrk_electrons = {
					reco::TransientTrack(*conv1.userData<reco::Track>("track0"), &bField),
					reco::TransientTrack(*conv1.userData<reco::Track>("track1"), &bField)
				};
				for (const auto& conv2 : conversions) {
					if (&conv1 == &conv2) continue;  // Ensure conv1 and conv2 are distinct
					std::vector<reco::TransientTrack> tttrk_electrons_pair = tttrk_electrons;  // Copy base tracks
					tttrk_electrons_pair.emplace_back(*conv2.userData<reco::Track>("track0"), &bField);
					tttrk_electrons_pair.emplace_back(*conv2.userData<reco::Track>("track1"), &bField);
					KinematicConstrainedFit BCandFitter;
					bool fitSuccess = BCandFitter.TetraObjectVertexFit(tttrk_muons, nominalMuonMass, tttrk_electrons_pair, nominalElectronMass);
					if (!fitSuccess) continue;
					dcv.mass = BCandFitter.getBhadronMass();
				}
			}
		}
    }
/*			for (pat::CompositeCandidateCollection::const_iterator conv1 = conversions->begin(); conv1!= conversions->end(); ++conv1){

                                std::vector<reco::TransientTrack> tttrk_electrons;
                                const reco::Track eletk0=*conv1->userData<reco::Track>("track0");
                                const reco::Track eletk1=*conv1->userData<reco::Track>("track1");
                                reco::TrackCollection convTracks1;
                                convTracks1.push_back(eletk0);
                                convTracks1.push_back(eletk1);
                                reco::TransientTrack electronTT1(convTracks1[0], &bField );
                                reco::TransientTrack electronTT2(convTracks1[1], &bField );
                                tttrk_electrons.push_back(electronTT1);
                                tttrk_electrons.push_back(electronTT2);
                        for (pat::CompositeCandidateCollection::const_iterator conv2 = conv1 + 1; conv2 != conversions->end(); ++conv2) {
                                const reco::Track eletk2=*conv2->userData<reco::Track>("track0");
                                const reco::Track eletk3=*conv2->userData<reco::Track>("track1");
                                //std::cout<< " the track output 2 and 3 : "<< eletk2.pt() << "\t"<< eletk3.pt() << "\t" <<eletk2.eta() << "\t"<< eletk3.eta()<< "\n";
                                reco::TrackCollection convTracks2;
                                convTracks2.push_back(eletk2);
                                convTracks2.push_back(eletk3);
                                reco::TransientTrack electronTT3(convTracks2[0], &bField );
                                reco::TransientTrack electronTT4(convTracks2[1], &bField );
                                tttrk_electrons.push_back(electronTT3);
                                tttrk_electrons.push_back(electronTT4);
                                KinematicConstrainedFit BCandFitter;
                                bool fitSuccess = BCandFitter.TetraObjectVertexFit(tttrk_muons,nominalMuonMass,tttrk_electrons,nominalElectronMass);
                                if(fitSuccess != 1) continue;
                                dcv.mass = BCandFitter.getBhadronMass();
			}
			}
		}
    }*/
    return dcv;
}
