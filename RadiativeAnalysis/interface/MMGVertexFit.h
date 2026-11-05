#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_MMGVertexFit_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_MMGVertexFit_h

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicVertex.h"
#include "TMatrixD.h"

#include <memory>
#include <optional>
#include <vector>

// Kinematic fit of B -> mu mu gamma with the photon measured by its ECAL cluster
// (needs the modified RecoVertex/KinematicFitPrimitives and RecoEcal/EgammaCoreTools).
//
// Typical use in an analyzer, one fit per candidate:
//   MMGVertexFit fitter(ebScale, eeScale);
//   auto mu1 = fitter.muonParticle(muon1, ttBuilder);           // once per muon and event
//   auto mu2 = fitter.muonParticle(muon2, ttBuilder);
//   auto result = fitter.fitCandidate(*mu1, *mu2, photon, lazyTools, ttBuilder, pv);
//   if (result) { result->particle, result->vertex, result->daughters ... }
class MMGVertexFit
{
    public:
        // fitted B candidate (after the pointing constraint) and its decay vertex
        struct Result
        {
            // note: particle->lastConstraint() points to the constraint object local to fitCandidate()
            // and must not be used after the fit
            RefCountedKinematicParticle particle;               // B candidate after the pointing constraint
            RefCountedKinematicVertex vertex;                   // B decay vertex
            std::vector<RefCountedKinematicParticle> daughters; // fitted mu1, mu2, photon (in this order)

            // indices of the inputs in the vectors passed to fit(); -1 when fitCandidate() is called directly
            int mu1Index = -1;
            int mu2Index = -1;
            int photonIndex = -1;

            // The fitted photon particles keep raw pointers to the photon copies and their cluster
            // covariances; they are owned here so that they live as long as the result.
            std::vector<std::shared_ptr<const reco::Photon>> fitPhotons;
            std::vector<std::shared_ptr<const TMatrixD>> fitPhotonCovariances;
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

        // cluster position covariance used in the fit: V' = S V S with
        // S = s_phi e_phi e_phi^T + s_theta e_theta e_theta^T + u u^T, u the photon direction
        TMatrixD clusterCovariance(const reco::Photon& photon, const EcalClusterLazyTools& lazyTools) const;

        // false if the supercluster has no rechits, so its covariance cannot be computed;
        // happens in MiniAOD for photons that fail the reducedEgamma selection
        static bool hasClusterHits(const reco::Photon& photon);

        // photon energy uncertainty used in the fit
        double photonEnergyError(const reco::Photon& photon) const;

        // kinematic particle of a muon (from its inner track); empty if the muon has no inner track.
        // Build it once per muon and event and reuse it for all candidates.
        static std::optional<RefCountedKinematicParticle> muonParticle(const reco::Muon& muon,
            const TransientTrackBuilder* TTBuilder);

        // kinematic particle of a charged track with the given mass hypothesis (e.g. a kaon)
        static RefCountedKinematicParticle trackParticle(const reco::TransientTrack& track, double mass);

        // photon from a conversion: vertex fit of the e+ e- tracks with a zero-mass constraint;
        // returns the photon particle at the conversion vertex (empty if the fit fails)
        static std::optional<RefCountedKinematicParticle> conversionParticle(const reco::TransientTrack& e1,
            const reco::TransientTrack& e2);

        // photon measured by its ECAL cluster; the photon copy and the covariance the particle points to
        // are returned together with it and must be kept alive (see Result::fitPhotons)
        struct PhotonParticle
        {
            RefCountedKinematicParticle particle;
            std::shared_ptr<const reco::Photon> photon;
            std::shared_ptr<const TMatrixD> covariance;
        };
        std::optional<PhotonParticle> photonParticle(const reco::Photon& photon, const EcalClusterLazyTools& lazyTools,
            const TransientTrackBuilder* TTBuilder, const GlobalPoint& expansionPoint) const;

        // generic B fit: vertex fit of the particles, then the pointing constraint to pv.
        // Returns nothing if the fit fails or diverges (non-finite chi2 or mass).
        static std::optional<Result> fitParticles(const std::vector<RefCountedKinematicParticle>& particles,
            const GlobalPoint& pv);

        // fit one mu mu gamma candidate; pv is the primary vertex used in the pointing constraint.
        // Returns nothing if the photon has no cluster hits or if the fit fails or diverges.
        std::optional<Result> fitCandidate(const RefCountedKinematicParticle& mu1,
            const RefCountedKinematicParticle& mu2,
            const reco::Photon& photon,
            const EcalClusterLazyTools& lazyTools,
            const TransientTrackBuilder* TTBuilder,
            const GlobalPoint& pv) const;

        // the same with any number of photons (e.g. mu mu gamma gamma); daughters: mu1, mu2, photons...
        std::optional<Result> fitCandidate(const RefCountedKinematicParticle& mu1,
            const RefCountedKinematicParticle& mu2,
            const std::vector<const reco::Photon*>& photons,
            const EcalClusterLazyTools& lazyTools,
            const TransientTrackBuilder* TTBuilder,
            const GlobalPoint& pv) const;

        // one Result per mu mu gamma combination that was fitted successfully, all with the
        // pointing constraint to primaryVertices[0]; the Result indices refer to muons and photons.
        // Muons and photons are passed by pointer, so pat::Muon and pat::Photon (MiniAOD) work as well.
        std::vector<Result> fit(const std::vector<const reco::Muon*>& muons,
            const std::vector<const reco::Photon*>& photons,
            const EcalClusterLazyTools& lazyTools,
            const std::vector<reco::Vertex>& primaryVertices,
            const TransientTrackBuilder* TTBuilder) const;

    private:
        ErrorScale ebScale_;
        ErrorScale eeScale_;
};

#endif
