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

        // scale factors for the cluster position uncertainties (sigma, not variance) along
        // e_phi and e_theta, the two directions perpendicular to the photon
        struct ClusterScale
        {
            double phi = 1.;
            double theta = 1.;
        };

        // ebScale, eeScale: cluster scale factors for barrel and endcap photons
        // energyErrorScale: scale factor for the photon energy uncertainty
        MMGVertexFit(ClusterScale ebScale = {1., 1.}, ClusterScale eeScale = {1., 1.}, double energyErrorScale = 1.)
            : ebScale_(ebScale), eeScale_(eeScale), energyErrorScale_(energyErrorScale) {}
        ~MMGVertexFit(){}

        // cluster position covariance used in the fit: V' = S V S with
        // S = s_phi e_phi e_phi^T + s_theta e_theta e_theta^T + u u^T, u the photon direction
        TMatrixD clusterCovariance(const reco::Photon& photon, const EcalClusterLazyTools& lazyTools) const;

        // photon energy uncertainty used in the fit
        double photonEnergyError(const reco::Photon& photon) const;

        // one Result per mu mu gamma combination that was fitted successfully
        std::vector<Result> fit(const std::vector<reco::Muon>& muons,
            const std::vector<reco::Photon>& photons,
            const EcalClusterLazyTools& lazyTools,
            const std::vector<reco::Vertex>& primaryVertices,
            const TransientTrackBuilder* TTBuilder);

    private:
        ClusterScale ebScale_;
        ClusterScale eeScale_;
        double energyErrorScale_;
};

#endif