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
//   reco::TransientTrack tt1 = ttBuilder->build(muon1.innerTrack()), tt2 = ...;   // must outlive the particles!
//   auto mu1 = MMGVertexFit::muonParticle(tt1);                  // once per muon and event
//   auto mu2 = MMGVertexFit::muonParticle(tt2);
//
// LIFETIME: KinematicParticleFactoryFromTransientTrack keeps a raw pointer to the TransientTrack it
// is given, so every TransientTrack passed to muonParticle / trackParticle / conversionParticle must
// stay alive (and must not be moved) as long as the particles and fit results are used. Photon
// tracks are owned by PhotonParticle / Result.
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

            // angle [rad] between the fitted B momentum and SV - PV after the pointing constraint:
            // ~0 if the constraint is satisfied. The CMSSW parent fitter can return a tree that does not
            // satisfy it (e.g. after the iteration cap), up to anti-parallel (pi).
            double pointingAngle = -1.;
            // stage 1, common-vertex fit before the pointing constraint (unconstrained geometry)
            double vertexOnlyChi2 = -1., vertexOnlyNdof = -1.;
            GlobalPoint vertexOnlyPosition;
            GlobalVector vertexOnlyMomentum;

            // indices of the inputs in the vectors passed to fit(); -1 when fitCandidate() is called directly
            int mu1Index = -1;
            int mu2Index = -1;
            int photonIndex = -1;

            // The fitted photon particles keep raw pointers to the photon copies and their cluster
            // covariances; they are owned here so that they live as long as the result.
            std::vector<std::shared_ptr<const reco::Photon>> fitPhotons;
            std::vector<std::shared_ptr<const TMatrixD>> fitPhotonCovariances;
            std::vector<std::shared_ptr<const reco::TransientTrack>> fitPhotonTracks;
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

        // energy type used by the fit: the candidate P4 type, ecal_photons if it is undefined
        // (getCorrectedEnergy(Error) throws for an undefined type)
        static reco::Photon::P4type energyType(const reco::Photon& photon);

        // can the photon be used as a measurement in the fit? The fit needs a positive finite energy and
        // energy uncertainty, and a cluster covariance with two independent position measurements
        // (perpendicular to the photon). A one-crystal cluster has zero spread, for example.
        enum PhotonInputStatus { kPhotonOK = 0, kNoClusterHits = 1, kBadEnergy = 2, kBadEnergyError = 3,
                                 kBadCovariance = 4 };
        PhotonInputStatus photonInputStatus(const reco::Photon& photon, const EcalClusterLazyTools& lazyTools) const;

        // kinematic particle of a muon (from its inner track); empty if the muon has no inner track.
        // Build it once per muon and event and reuse it for all candidates.
        static RefCountedKinematicParticle muonParticle(const reco::TransientTrack& muonTrack);

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
            std::shared_ptr<const reco::TransientTrack> track;   // the particle points to it
        };
        std::optional<PhotonParticle> photonParticle(const reco::Photon& photon, const EcalClusterLazyTools& lazyTools,
            const TransientTrackBuilder* TTBuilder, const GlobalPoint& expansionPoint) const;

        // generic B fit: vertex fit of the particles, then the pointing constraint to pv.
        // Returns nothing if the fit fails or diverges (non-finite chi2 or mass).
        static std::optional<Result> fitParticles(const std::vector<RefCountedKinematicParticle>& particles,
            const GlobalPoint& pv);

        // angle [rad] between p and d (atan2 of |p x d| and p.d); -1 if one of them is null
        static double pointingAngle(const GlobalVector& p, const GlobalVector& d);

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
