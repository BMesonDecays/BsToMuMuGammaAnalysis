#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_TripleObjectVertexFit_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_TripleObjectVertexFit_h

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonClosestApproachCalculator.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/PhotonSelector.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DecayChainVariables.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "DataFormats/MuonReco/interface/MuonChamberMatch.h"

#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include <iostream>

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include "TMath.h"


class TripleObjectVertexFit {
public:
    TripleObjectVertexFit();
    ~TripleObjectVertexFit() {}

    struct MassLimits {
        static constexpr double BsMassCutLower = 4.5;
        static constexpr double BsMassCutUpper = 6.0;
        static constexpr double BdMassCutLower = 4.5;
        static constexpr double BdMassCutUpper = 6.0;
        static constexpr double BsPDGMass = 5.36677;
    };

    const double nominalMuonMass = 0.1056583715;
    typedef reco::Candidate::LorentzVector LorentzVector;

    struct MmgFitResults {
        // Dimuon variables
        int resonanceFlag = -999;
        double dimuonVtxProb = -9999.0;
        double dimuonMass = -9999.0;
        double dimuonEta = -9999.0;
        double dimuonPhi = -9999.0;
        double dimuonPt = -9999.0;

        // Photon variables
        std::vector<double> photonDeltaRs;
        std::vector<double> photonPt;
        std::vector<double> photonEta;
        std::vector<double> photonPhi;
        std::vector<double> photonEnergy;
        std::vector<double> photonDimuonMass;
        std::vector<double> photonDimuonPt;
        std::vector<double> photonDimuonEta;
        std::vector<double> photonDimuonPhi;

        // Bs meson variables
        std::vector<double> bsVtxProb;
        std::vector<double> bsMass;
        std::vector<double> bsEnergy;
        std::vector<double> bsPt;
        std::vector<double> bsEta;
        std::vector<double> bsPhi;
        std::vector<bool> bsFitSuccessful;

        // Constructor
        explicit MmgFitResults(size_t size = 0)
            : photonDeltaRs(size),
              photonPt(size),
              photonEta(size),
              photonPhi(size),
              photonEnergy(size),
              photonDimuonMass(size),
              photonDimuonPt(size),
              photonDimuonEta(size),
              photonDimuonPhi(size),
              bsVtxProb(size),
              bsMass(size),
              bsEnergy(size),
              bsPt(size),
              bsEta(size),
              bsPhi(size),
              bsFitSuccessful(size, false) {}
    };


    // The TripleObjectVertexFit::fitMmgVertices function takes the considered muon pair and all photons in the event
    // It returns a struct, where resulting quantities for a given photon constitute single entries in the vector elements of the struct
    // PhotonCollection can be e.g. std::vector<reco::Photon> or edm::View<reco::Photon>
    template <typename Muon, typename PhotonCollection>
    MmgFitResults fitMmgVertices(
        const std::pair< Muon, Muon >& muonPair,
        const PhotonCollection& photons,
        const std::vector<reco::Vertex>& PVs,
        const EcalClusterLazyTools& lazyTools,
        const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
        const MagneticField& bField,
        const TransientTrackBuilder& transientTrackBuilder);
};

#endif