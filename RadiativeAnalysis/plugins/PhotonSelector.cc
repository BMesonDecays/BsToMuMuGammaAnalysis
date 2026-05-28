#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/PhotonSelector.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MultiTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "TMath.h"

#include <utility>
#include <numeric>
#include <vector>
#include <algorithm>

PhotonSelector::PhotonSelector() 
     {}

PhotonSelector::~PhotonSelector() {}

std::vector<reco::Photon> PhotonSelector::selectPhoton(
    const std::vector<reco::Photon>& photons,
    const std::vector<reco::Muon>& muons,
    const TransientTrackBuilder& transientTrackBuilder,
    std::vector<float> photonMVAIDs,
    double photonMVAcut) {
    
    std::vector<reco::Photon> selectedPhotons;

    if(photons.size() == 0 || muons.size() < 2 || photonMVAIDs.size() != photons.size()) {
        return selectedPhotons; // Return empty if no photons or less than 2 muons
    }

    // reject photons with id score less than -0.07
    std::vector<reco::Photon> passingPhotons;
    for(long unsigned int i = 0; i < photons.size(); ++i) {
        if(photonMVAIDs.at(i) >= photonMVAcut) {
            passingPhotons.push_back(photons[i]);
        }
    }

    if (passingPhotons.empty()) {
        return selectedPhotons;
    }

    
    reco::Candidate::LorentzVector muonPairP4 = muons[0].p4() + muons[1].p4();

    std::vector<double> dimuonPhotonDeltaRs;
    for(auto pho : passingPhotons)
    {
        double dR = deltaR(muonPairP4.eta(), muonPairP4.phi(), pho.eta(), pho.phi());
        dimuonPhotonDeltaRs.push_back(dR);
    }

    std::vector<size_t> photonIndicesSortedByDeltaR(passingPhotons.size());
    std::iota(photonIndicesSortedByDeltaR.begin(), photonIndicesSortedByDeltaR.end(), 0);
    std::sort(photonIndicesSortedByDeltaR.begin(), photonIndicesSortedByDeltaR.end(),
       [&dimuonPhotonDeltaRs](size_t i1, size_t i2) { return dimuonPhotonDeltaRs[i1] < dimuonPhotonDeltaRs[i2]; });
    
    selectedPhotons.push_back(passingPhotons[photonIndicesSortedByDeltaR[0]]);

    return selectedPhotons;
}

std::vector<reco::Photon> PhotonSelector::selectPhotons(
    const std::vector<reco::Photon>& photons,
    const std::vector<reco::Muon>& muons,
    const TransientTrackBuilder& transientTrackBuilder,
    std::vector<float> photonMVAIDs,
    double photonMVAcut) {
    
    std::vector<reco::Photon> selectedPhotons;

    if(photons.size() < 2 || muons.size() < 2 || photonMVAIDs.size() != photons.size()) {
        return selectedPhotons; // Return empty if less than 2 photons or less than 2 muons
    }
    
    // reject photons with id score less than -0.07
    std::vector<reco::Photon> passingPhotons;
    for(long unsigned int i = 0; i < photons.size(); ++i) {
        if(photonMVAIDs.at(i) >= photonMVAcut) {
            passingPhotons.push_back(photons[i]);
        }
    }

    if (passingPhotons.size() < 2) {
        return selectedPhotons;
    }

    reco::Candidate::LorentzVector muonPairP4 = muons[0].p4() + muons[1].p4();

    // select a photon pair with the smallest deltaR to the dimuon system, and if it passes the selection criteria, add it to the selectedPhotons vector
    reco::Photon selectedPho1;
    reco::Photon selectedPho2;
    double minDeltaR = std::numeric_limits<double>::max();

    if(passingPhotons.size() > 1)
    {
        for(long unsigned int i = 0; i < passingPhotons.size(); i++)
        {
            for(long unsigned int j = i + 1; j < passingPhotons.size(); j++)
            {
                reco::Photon pho1 = passingPhotons[i];
                reco::Photon pho2 = passingPhotons[j];
                reco::Candidate::LorentzVector photonPairP4 = pho1.p4() + pho2.p4();
                double dR = deltaR(muonPairP4.eta(), muonPairP4.phi(), photonPairP4.eta(), photonPairP4.phi());
                if(dR < minDeltaR)
                {
                    minDeltaR = dR;
                    selectedPho1 = pho1;
                    selectedPho2 = pho2;
                }
            }
        }
        selectedPhotons.push_back(selectedPho1);
        selectedPhotons.push_back(selectedPho2);
    }

    return selectedPhotons;
}

std::vector<pat::CompositeCandidate> PhotonSelector::selectConvertedPhoton(
    const std::vector<pat::CompositeCandidate>& conversions,
    const std::vector<reco::Muon>& muons,
    const TransientTrackBuilder& transientTrackBuilder) {
    
    std::vector<pat::CompositeCandidate> selectedConversions;

    if(muons.size() < 2) {
        return selectedConversions;
    }
    
    reco::Candidate::LorentzVector muonPairP4 = muons[0].p4() + muons[1].p4();
    
    std::vector<RefCountedKinematicParticle> convPhotons;
    std::vector<int> convIndex;
    int convCount = 0;
    for (auto conv : conversions)
    {
        convIndex.push_back(convCount);
        convCount++;

        const ParticleMass zero_mass = 1e-4f; 
        float zero_sigma = 1e-4f;
        float eleSigma = 1e-4f;
        float muonSigma = 1e-4f;
        float chi = 1e-4f;
        float ndf = 1e-4f;
        float eleMass = 0.00051099893;
        KinematicParticleFactoryFromTransientTrack pFactory;
        const reco::Track eletk0 = *conv.userData<reco::Track>("track0");
        const reco::Track eletk1 = *conv.userData<reco::Track>("track1");
        std::vector<reco::TransientTrack> electronTT;
        electronTT.push_back(transientTrackBuilder.build(eletk0));
        electronTT.push_back(transientTrackBuilder.build(eletk1));

        std::vector<RefCountedKinematicParticle> photonParticles = {
        pFactory.particle(electronTT[0], eleMass, chi, ndf, eleSigma),
        pFactory.particle(electronTT[1], eleMass, chi, ndf, eleSigma)
    };
        KinematicParticleVertexFitter vertexFitter;
        RefCountedKinematicTree photonVertexFitTree = vertexFitter.fit(photonParticles);
        if(!photonVertexFitTree->isValid()) {
            edm::ParameterSet photonFitParams;
            photonFitParams.addParameter<double>("maxDistance", 3.0);
            photonFitParams.addParameter<int>("maxNbrOfIterations", 10000);
            KinematicParticleVertexFitter vertexFitter2(photonFitParams);
            photonVertexFitTree = vertexFitter2.fit(photonParticles);
        }
        if (!photonVertexFitTree || !photonVertexFitTree->isValid()) {
            edm::LogInfo("TrippleObjectVertexFit") << "Photon vertex fit after optimization failed.";
            continue;
        }
        KinematicConstraint *photonConstraint = new MassKinematicConstraint(zero_mass, zero_sigma);
        KinematicParticleFitter csFitterPhoton;
        photonVertexFitTree = csFitterPhoton.fit(photonConstraint, photonVertexFitTree);
        if (!photonVertexFitTree || !photonVertexFitTree->isValid()) {
            edm::LogInfo("TrippleObjectVertexFit") << "Photon mass-constrained fit failed.";
            continue;
        }
        photonVertexFitTree->movePointerToTheTop();
        RefCountedKinematicParticle fittedPhoton = photonVertexFitTree->currentParticle();
        convPhotons.push_back(fittedPhoton);
    }

    if(convPhotons.size() == 0) {
        return selectedConversions; // Return empty if no valid converted photons
    }

    // find the converted photon with the smallest deltaR to the dimuon system, and if it passes the selection criteria, add it to the selectedConversions vector
    double minDeltaR = std::numeric_limits<double>::max();
    pat::CompositeCandidate selectedConv;
    for (size_t i = 0; i < convPhotons.size(); i++)
    {
        GlobalVector convPhotonP3 = convPhotons[i]->currentState().kinematicParameters().momentum();
        double dR = deltaR(muonPairP4.eta(), muonPairP4.phi(), convPhotonP3.eta(), convPhotonP3.phi());
        if (dR < minDeltaR)        {
            minDeltaR = dR;
            selectedConv = conversions[convIndex[i]];
        }
    }
    selectedConversions.push_back(selectedConv);
    
    return selectedConversions;
}

std::vector<pat::CompositeCandidate> PhotonSelector::selectConvertedPhotons(
    const std::vector<pat::CompositeCandidate>& conversions,
    const std::vector<reco::Muon>& muons,
    const TransientTrackBuilder& transientTrackBuilder) {
    
    std::vector<pat::CompositeCandidate> selectedConversions;

    if(muons.size() < 2) {
        return selectedConversions;
    }
    
    reco::Candidate::LorentzVector muonPairP4 = muons[0].p4() + muons[1].p4();
    
    std::vector<RefCountedKinematicParticle> convPhotons;
    std::vector<int> convIndex;
    int convCount = 0;
    for (auto conv : conversions)
    {
        convIndex.push_back(convCount);
        convCount++;

        const ParticleMass zero_mass = 1e-4f; 
        float zero_sigma = 1e-4f;
        float eleSigma = 1e-4f;
        float muonSigma = 1e-4f;
        float chi = 1e-4f;
        float ndf = 1e-4f;
        float eleMass = 0.00051099893;
        KinematicParticleFactoryFromTransientTrack pFactory;
        const reco::Track eletk0 = *conv.userData<reco::Track>("track0");
        const reco::Track eletk1 = *conv.userData<reco::Track>("track1");
        std::vector<reco::TransientTrack> electronTT;
        electronTT.push_back(transientTrackBuilder.build(eletk0));
        electronTT.push_back(transientTrackBuilder.build(eletk1));

        std::vector<RefCountedKinematicParticle> photonParticles = {
        pFactory.particle(electronTT[0], eleMass, chi, ndf, eleSigma),
        pFactory.particle(electronTT[1], eleMass, chi, ndf, eleSigma)
    };
        KinematicParticleVertexFitter vertexFitter;
        RefCountedKinematicTree photonVertexFitTree = vertexFitter.fit(photonParticles);
        if(!photonVertexFitTree->isValid()) {
            edm::ParameterSet photonFitParams;
            photonFitParams.addParameter<double>("maxDistance", 3.0);
            photonFitParams.addParameter<int>("maxNbrOfIterations", 10000);
            KinematicParticleVertexFitter vertexFitter2(photonFitParams);
            photonVertexFitTree = vertexFitter2.fit(photonParticles);
        }
        if (!photonVertexFitTree || !photonVertexFitTree->isValid()) {
            edm::LogInfo("TrippleObjectVertexFit") << "Photon vertex fit after optimization failed.";
            continue;
        }
        KinematicConstraint *photonConstraint = new MassKinematicConstraint(zero_mass, zero_sigma);
        KinematicParticleFitter csFitterPhoton;
        photonVertexFitTree = csFitterPhoton.fit(photonConstraint, photonVertexFitTree);
        if (!photonVertexFitTree || !photonVertexFitTree->isValid()) {
            edm::LogInfo("TrippleObjectVertexFit") << "Photon mass-constrained fit failed.";
            continue;
        }
        photonVertexFitTree->movePointerToTheTop();
        RefCountedKinematicParticle fittedPhoton = photonVertexFitTree->currentParticle();
        convPhotons.push_back(fittedPhoton);
    }

    if(convPhotons.size() < 2) {
        return selectedConversions; // Return empty if no valid converted photons
    }

    // now add the converted photon pair with the smallest deltaR to the dimuon system
    if(convPhotons.size() > 1)
    {
        pat::CompositeCandidate selectedConv1;
        pat::CompositeCandidate selectedConv2;
        double minDeltaR = std::numeric_limits<double>::max();
        for (long unsigned int i = 0; i < convPhotons.size(); i++)
        {
            for (long unsigned int j = i + 1; j < convPhotons.size(); j++)
            {
                GlobalVector convPhotonP3_1 = convPhotons[i]->currentState().kinematicParameters().momentum();
                GlobalVector convPhotonP3_2 = convPhotons[j]->currentState().kinematicParameters().momentum();
                GlobalVector convPhotonPairP3 = convPhotonP3_1 + convPhotonP3_2;
                double dR = deltaR(muonPairP4.eta(), muonPairP4.phi(), convPhotonPairP3.eta(), convPhotonPairP3.phi());
                if (dR < minDeltaR)
                {
                    minDeltaR = dR;
                    selectedConv1 = conversions[convIndex[i]];
                    selectedConv2 = conversions[convIndex[j]];
                }
            }
        }
        selectedConversions.push_back(selectedConv1);
        selectedConversions.push_back(selectedConv2);
    }

    
    return selectedConversions;
}

bool PhotonSelector::passPhotonSelection(const reco::Photon& photon) const {
    
    return true;
}