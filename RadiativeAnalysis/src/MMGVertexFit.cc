#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MMGVertexFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Constants.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"

#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/KinematicFitPrimitives/interface/ParticleMass.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "TrackingTools/TrajectoryParametrization/interface/CartesianTrajectoryError.h"
#include "RecoVertex/KinematicFit/interface/PointingKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicTree.h"
#include "RecoVertex/VertexPrimitives/interface/VertexException.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include <cmath>
#include <memory>

namespace
{
    // energy type used by the fit (ParticleKinematicLinearizedTrackState::computePhotonJacobians)
    reco::Photon::P4type energyType(const reco::Photon& photon)
    {
        return photon.getCandidateP4type() == reco::Photon::undefined ? reco::Photon::ecal_photons
                                                                       : photon.getCandidateP4type();
    }
}

// covariancesXYZ gives the spatial spread of the shower, which overestimates the
// uncertainty of the cluster position; it is rescaled with factors from the cluster pulls.
// The scaling is done along e_phi and e_theta, the directions perpendicular to the photon
// that the fit measures, so the factors mean the same in the barrel and the endcap.
// The component along the photon (shower depth) drops out of the fit and is not scaled.
TMatrixD MMGVertexFit::clusterCovariance(const reco::Photon& photon, const EcalClusterLazyTools& lazyTools) const
    {
        const TMatrixD cov = lazyTools.covariancesXYZ(*photon.superCluster());
        const ErrorScale& scale = photon.isEB() ? ebScale_ : eeScale_;

        const double phi = photon.phi(), theta = photon.theta();
        const double ePhi[3] = {-std::sin(phi), std::cos(phi), 0.};
        const double eTheta[3] = {std::cos(theta) * std::cos(phi), std::cos(theta) * std::sin(phi), -std::sin(theta)};
        const double u[3] = {std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta)};

        TMatrixD S(3, 3);
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
                S(a, b) = scale.clusterPhi * ePhi[a] * ePhi[b] + scale.clusterTheta * eTheta[a] * eTheta[b] + u[a] * u[b];
        return S * cov * S;
    }

bool MMGVertexFit::hasClusterHits(const reco::Photon& photon)
    {
        return !photon.superCluster()->hitsAndFractions().empty();
    }

// The energy uncertainty of the photon candidate does not match the spread of E_reco - E_gen;
// it is rescaled with a factor from the photon energy pulls.
double MMGVertexFit::photonEnergyError(const reco::Photon& photon) const
    {
        const ErrorScale& scale = photon.isEB() ? ebScale_ : eeScale_;
        return scale.energy * photon.getCorrectedEnergyError(energyType(photon));
    }

RefCountedKinematicParticle MMGVertexFit::muonParticle(const reco::TransientTrack& muonTrack)
    {
        return trackParticle(muonTrack, bmmg::pdg::muonMass);
    }

RefCountedKinematicParticle MMGVertexFit::trackParticle(const reco::TransientTrack& track, double mass)
    {
        KinematicParticleFactoryFromTransientTrack pFactory;
        float chi = 0.f, ndf = 0.f, sigma = 1E-6;
        return pFactory.particle(track, ParticleMass(mass), chi, ndf, sigma);
    }

std::optional<RefCountedKinematicParticle> MMGVertexFit::conversionParticle(const reco::TransientTrack& e1,
    const reco::TransientTrack& e2)
    {
        const std::vector<RefCountedKinematicParticle> electrons{trackParticle(e1, bmmg::pdg::electronMass),
                                                                 trackParticle(e2, bmmg::pdg::electronMass)};
        try
        {
            KinematicParticleVertexFitter vertexFitter;
            RefCountedKinematicTree tree = vertexFitter.fit(electrons);
            if (!tree->isValid()) return std::nullopt;
            MassKinematicConstraint zeroMass(ParticleMass(0.), float(1E-4));
            KinematicParticleFitter constraintFitter;
            tree = constraintFitter.fit(&zeroMass, tree);
            if (!tree->isValid()) return std::nullopt;
            tree->movePointerToTheTop();
            return tree->currentParticle();
        }
        catch (const VertexException&)
        {
            return std::nullopt;
        }
    }

std::optional<MMGVertexFit::PhotonParticle> MMGVertexFit::photonParticle(const reco::Photon& photon,
    const EcalClusterLazyTools& lazyTools, const TransientTrackBuilder* TTBuilder, const GlobalPoint& expansionPoint) const
    {
        if (!hasClusterHits(photon)) return std::nullopt;

        // copy of the photon with the rescaled energy uncertainty; the energy is unchanged.
        // clone() keeps the full type, e.g. pat::Photon with its embedded supercluster.
        std::shared_ptr<reco::Photon> fitPhoton(photon.clone());
        const reco::Photon::P4type p4type = energyType(*fitPhoton);
        fitPhoton->setCorrectedEnergy(p4type, fitPhoton->getCorrectedEnergy(p4type), photonEnergyError(photon), false);
        auto covariance = std::make_shared<TMatrixD>(clusterCovariance(*fitPhoton, lazyTools));

        // initial state: only used as the expansion point, the measurement is the cluster
        const GlobalVector p3(fitPhoton->px(), fitPhoton->py(), fitPhoton->pz());
        FreeTrajectoryState fts(expansionPoint, p3, TrackCharge(0), TTBuilder->field());
        AlgebraicSymMatrix66 cov{ROOT::Math::SMatrixIdentity()};
        fts.setCartesianError(CartesianTrajectoryError(cov));
        // owned by the result: the kinematic particle keeps a pointer to it
        auto phoTT = std::make_shared<const reco::TransientTrack>(TTBuilder->build(fts));

        KinematicParticleFactoryFromTransientTrack pFactory;
        float chi = 0.f, ndf = 0.f, sigma = 1E-6;
        PhotonParticle result;
        result.particle = pFactory.particle(*phoTT, ParticleMass(0.), chi, ndf, sigma, fitPhoton.get(), covariance.get());
        result.track = std::move(phoTT);
        result.photon = std::move(fitPhoton);
        result.covariance = std::move(covariance);
        return result;
    }

// Sequential fit of a B candidate:
//  1. Kalman vertex fit of the daughters (KinematicParticleVertexFitter). A photon enters through
//     its calorimeter cluster position and energy, so its momentum is pointed from the fitted
//     vertex to the cluster.
//  2. Pointing constraint on the top particle of the resulting tree (KinematicParticleFitter
//     with PointingKinematicConstraint): the B momentum must point from the primary vertex
//     to the B decay vertex. The constraint uses the full covariance of the B candidate,
//     including the correlations between the daughters from step 1.
std::optional<MMGVertexFit::Result> MMGVertexFit::fitParticles(const std::vector<RefCountedKinematicParticle>& particles,
    const GlobalPoint& pv)
    {
        KinematicParticleVertexFitter vertexFitter;
        KinematicParticleFitter constraintFitter;
        PointingKinematicConstraint pointingConstraint(pv);

        RefCountedKinematicTree tree;
        try
        {
            tree = vertexFitter.fit(particles);
            if (!tree->isValid()) return std::nullopt;
            tree->movePointerToTheTop();
            tree = constraintFitter.fit(&pointingConstraint, tree);
            if (!tree->isValid()) return std::nullopt;
        }
        catch (const VertexException&)
        {
            return std::nullopt;
        }

        Result result;
        // fitted daughters (the pointing constraint only changes the top particle); read after the
        // constraint fit: reading them before changed the result of the constraint fit (see report.md P1.2)
        result.daughters = tree->finalStateParticles();
        tree->movePointerToTheTop();
        result.particle = tree->currentParticle();
        result.vertex = tree->currentDecayVertex();
        if (!result.vertex->vertexIsValid()) return std::nullopt;

        // the constraint fit can diverge and still return a valid tree: such fits (non-finite chi2,
        // non-positive or absurd mass or momentum) are treated as failed
        const double fitMass = result.particle->currentState().mass();
        const double fitP = result.particle->currentState().globalMomentum().mag();
        if (!std::isfinite(result.vertex->chiSquared()) || !std::isfinite(fitMass) || fitMass <= 0. || fitMass > 100. ||
            !std::isfinite(fitP) || fitP > 1e4)
            return std::nullopt;
        return result;
    }

std::optional<MMGVertexFit::Result> MMGVertexFit::fitCandidate(const RefCountedKinematicParticle& mu1,
    const RefCountedKinematicParticle& mu2,
    const reco::Photon& photon,
    const EcalClusterLazyTools& lazyTools,
    const TransientTrackBuilder* TTBuilder,
    const GlobalPoint& pv) const
    {
        return fitCandidate(mu1, mu2, std::vector<const reco::Photon*>{&photon}, lazyTools, TTBuilder, pv);
    }

std::optional<MMGVertexFit::Result> MMGVertexFit::fitCandidate(const RefCountedKinematicParticle& mu1,
    const RefCountedKinematicParticle& mu2,
    const std::vector<const reco::Photon*>& photons,
    const EcalClusterLazyTools& lazyTools,
    const TransientTrackBuilder* TTBuilder,
    const GlobalPoint& pv) const
    {
        std::vector<RefCountedKinematicParticle> particles{mu1, mu2};
        std::vector<PhotonParticle> photonParticles;
        for (const reco::Photon* pho : photons)
        {
            auto pp = photonParticle(*pho, lazyTools, TTBuilder, pv);
            if (!pp) return std::nullopt;
            particles.push_back(pp->particle);
            photonParticles.push_back(std::move(*pp));
        }
        auto result = fitParticles(particles, pv);
        if (!result) return std::nullopt;
        for (auto& pp : photonParticles)
        {
            result->fitPhotons.push_back(std::move(pp.photon));
            result->fitPhotonCovariances.push_back(std::move(pp.covariance));
            result->fitPhotonTracks.push_back(std::move(pp.track));
        }
        return result;
    }

std::vector<MMGVertexFit::Result> MMGVertexFit::fit(const std::vector<const reco::Muon*>& muons,
    const std::vector<const reco::Photon*>& photons,
    const EcalClusterLazyTools& lazyTools,
    const std::vector<reco::Vertex>& primaryVertices,
    const TransientTrackBuilder* TTBuilder) const
    {
        std::vector<Result> results;
        if (primaryVertices.empty()) return results;

        const GlobalPoint pv(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());

        // kinematic particles of the muons, built once; the TransientTracks must outlive the particles
        std::vector<reco::TransientTrack> muonTracks;
        muonTracks.reserve(muons.size());   // no reallocation: the particles point to the elements
        std::vector<std::optional<RefCountedKinematicParticle>> muonKinematicParticles;
        for (const reco::Muon* recoMu : muons)
        {
            if (recoMu->track().isNull()) { muonKinematicParticles.push_back(std::nullopt); continue; }
            muonTracks.push_back(TTBuilder->build(recoMu->track()));
            muonKinematicParticles.push_back(muonParticle(muonTracks.back()));
        }

        for (unsigned int i = 0; i < muons.size(); i++)
        {
            if (!muonKinematicParticles[i]) continue;
            for (unsigned int j = i+1; j < muons.size(); j++)
            {
                if (!muonKinematicParticles[j]) continue;
                for (unsigned int k = 0; k < photons.size(); k++)
                {
                    auto result = fitCandidate(*muonKinematicParticles[i], *muonKinematicParticles[j], *photons[k], lazyTools, TTBuilder, pv);
                    if (!result) continue;
                    result->mu1Index = i;
                    result->mu2Index = j;
                    result->photonIndex = k;
                    results.push_back(std::move(*result));
                }
            }
        }
        return results;
    }
