#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_MMGVertexFit_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_MMGVertexFit_h

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TMatrixD.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicVertex.h"

#include <vector>

class MMGVertexFit
{
    public:
        // fitted B candidate (after the pointing constraint) and its decay vertex
        struct Result
        {
            RefCountedKinematicParticle particle;
            RefCountedKinematicVertex vertex;
        };

        // scale factors for the photon measurement uncertainties (sigma, not variance):
        // cluster position along e_phi and e_theta, the two directions perpendicular to the photon,
        // and photon energy
        struct ErrorScale
        {
            double clusterPhi = 1.;
            double clusterTheta = 1.;
            double energy = 1.;
        };

        // ebScale, eeScale: scale factors for barrel and endcap photons
        MMGVertexFit(ErrorScale ebScale = {1., 1., 1.}, ErrorScale eeScale = {1., 1., 1.})
            : ebScale_(ebScale), eeScale_(eeScale) {}
        ~MMGVertexFit(){}

        // cluster position covariance used in the fit: V' = S V S with
        // S = s_phi e_phi e_phi^T + s_theta e_theta e_theta^T + u u^T, u the photon direction
        TMatrixD clusterCovariance(const reco::Photon& photon, const EcalClusterLazyTools& lazyTools) const;

        // false if the supercluster has no rechits, so its covariance cannot be computed;
        // happens in MiniAOD for photons that fail the reducedEgamma selection
        static bool hasClusterHits(const reco::Photon& photon);

        // photon energy uncertainty used in the fit
        double photonEnergyError(const reco::Photon& photon) const;

        // one Result per mu mu gamma combination that was fitted successfully;
        // muons and photons are passed by pointer, so pat::Muon and pat::Photon (MiniAOD) work as well
        std::vector<Result> fit(const std::vector<const reco::Muon*>& muons,
            const std::vector<const reco::Photon*>& photons,
            const EcalClusterLazyTools& lazyTools,
            const std::vector<reco::Vertex>& primaryVertices,
            const TransientTrackBuilder* TTBuilder);

    private:
        ErrorScale ebScale_;
        ErrorScale eeScale_;
};

#endif