#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_TripleObjectVertexFit_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_TripleObjectVertexFit_h

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceResonance.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/PhotonSelector.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GenChain.h"

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

    struct MassLimits {     // mu mu gamma mass window applied before the Bs fit
        static constexpr double BsMassCutLower = 4.5;
        static constexpr double BsMassCutUpper = 6.0;
    };

    const double nominalMuonMass = 0.1056583715;

    struct MmgFitResults {
        // Dimuon variables
        int resonanceFlag = -999;
        double dimuonVtxProb = -9999.0;
        double dimuonMass = -9999.0;        // measured, from the two muon four-vectors
        double dimuonFitMass = -9999.0;     // after the dimuon kinematic fit (= PDG mass if mass-constrained)
        double dimuonEta = -9999.0;
        double dimuonPhi = -9999.0;
        double dimuonPt = -9999.0;

        // Photon variables
        std::vector<double> photonDeltaRs;
        std::vector<double> photonPt;
        std::vector<double> photonEta;
        std::vector<double> photonPhi;
        std::vector<double> photonEnergy;
        std::vector<double> photonMVA;          // photon MVA ID score
        std::vector<double> photonDimuonMass;
        std::vector<double> photonDimuonPt;
        std::vector<double> photonDimuonEta;
        std::vector<double> photonDimuonPhi;
        std::vector<double> photonDimuonLxy;        // PV -> dimuon vertex, transverse
        std::vector<double> photonDimuonLxySig;     // Lxy / sigma(Lxy), with the dimuon vertex and PV covariances
        std::vector<double> photonDimuonCosAlpha;   // angle between PV -> dimuon vertex and the mu mu gamma momentum, 3D
        std::vector<double> photonDimuonCosAlpha2D; // same in the transverse plane
        std::vector<double> photonGenDeltaR;   // MC study only; photon direction taken after setting its vertex at the SV

        // Primary vertex used in the Bs fit (lowest DCA to the mu mu gamma direction from the SV)
        std::vector<int> pvIndex;
        std::vector<double> pvX;
        std::vector<double> pvY;
        std::vector<double> pvZ;
        std::vector<double> pvErrX;
        std::vector<double> pvErrY;
        std::vector<double> pvErrZ;
        std::vector<double> pvCovXX;
        std::vector<double> pvCovXY;
        std::vector<double> pvCovXZ;
        std::vector<double> pvCovYY;
        std::vector<double> pvCovYZ;
        std::vector<double> pvCovZZ;
        std::vector<double> pvNdof;
        std::vector<double> pvRho;
        std::vector<int> pvNTracks;

        // Bs meson variables
        std::vector<double> bsVtxProb;
        std::vector<double> bsVtxX;                 // fitted B vertex
        std::vector<double> bsVtxY;
        std::vector<double> bsVtxZ;
        std::vector<double> bsLxy;                  // PV -> B vertex, transverse
        std::vector<double> bsLxySig;               // Lxy / sigma(Lxy), with the B vertex and PV covariances
        std::vector<double> bsCt;                   // L(3D) * m / p
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
              photonMVA(size, -9999.0),
              photonDimuonMass(size),
              photonDimuonPt(size),
              photonDimuonEta(size),
              photonDimuonPhi(size),
              photonDimuonLxy(size, -9999.0),
              photonDimuonLxySig(size, -9999.0),
              photonDimuonCosAlpha(size, -9999.0),
              photonDimuonCosAlpha2D(size, -9999.0),
              photonGenDeltaR(size, -9999.0),
              pvIndex(size, -999),
              pvX(size, -9999.0),
              pvY(size, -9999.0),
              pvZ(size, -9999.0),
              pvErrX(size, -9999.0),
              pvErrY(size, -9999.0),
              pvErrZ(size, -9999.0),
              pvCovXX(size, -9999.0),
              pvCovXY(size, -9999.0),
              pvCovXZ(size, -9999.0),
              pvCovYY(size, -9999.0),
              pvCovYZ(size, -9999.0),
              pvCovZZ(size, -9999.0),
              pvNdof(size, -9999.0),
              pvRho(size, -9999.0),
              pvNTracks(size, -999),
              bsVtxProb(size, -9999.0),     // Bs variables stay at -9999 if the mass window or the fit fails
              bsVtxX(size, -9999.0),
              bsVtxY(size, -9999.0),
              bsVtxZ(size, -9999.0),
              bsLxy(size, -9999.0),
              bsLxySig(size, -9999.0),
              bsCt(size, -9999.0),
              bsMass(size, -9999.0),
              bsEnergy(size, -9999.0),
              bsPt(size, -9999.0),
              bsEta(size, -9999.0),
              bsPhi(size, -9999.0),
              bsFitSuccessful(size, false) {}
    };


    // The TripleObjectVertexFit::fitMmgVertices function takes the considered muon pair and all photons in the event
    // It returns a struct, where resulting quantities for a given photon constitute single entries in the vector elements of the struct
    // PhotonCollection can be e.g. std::vector<reco::Photon> or edm::View<reco::Photon>
    // photonMVAs: MVA ID scores, parallel to photons (same size and order)
    // genChain (MC study only, nullptr otherwise) is used to fill photonGenDeltaR
    template <typename Muon, typename PhotonCollection>
    MmgFitResults fitMmgVertices(
        const std::pair< Muon, Muon >& muonPair,
        const PhotonCollection& photons,
        const std::vector<float>& photonMVAs,
        const std::vector<reco::Vertex>& PVs,
        const EcalClusterLazyTools& lazyTools,
        const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
        const MagneticField& bField,
        const TransientTrackBuilder& transientTrackBuilder,
        const GenDecayChain* genChain = nullptr);
};

#endif