#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MMGVertexFit.h"

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

std::optional<RefCountedKinematicParticle> MMGVertexFit::muonParticle(const reco::Muon& muon,
    const TransientTrackBuilder* TTBuilder)
    {
        const reco::TrackRef muTrack = muon.track();
        if (!muTrack) return std::nullopt;
        const reco::TransientTrack muonTT = TTBuilder->build(muTrack);

        const ParticleMass muon_mass(0.105658);
        float muon_sigma = 1E-6;

        KinematicParticleFactoryFromTransientTrack pFactory;
        return pFactory.particle(muonTT, muon_mass, float(0), float(0), muon_sigma);
    }

// Sequential fit of B -> mu mu gamma:
//  1. Kalman vertex fit of the two muons and the photon (KinematicParticleVertexFitter).
//     The photon enters through its calorimeter cluster position and energy, so its
//     momentum is pointed from the fitted vertex to the cluster.
//  2. Pointing constraint on the top particle of the resulting tree (KinematicParticleFitter
//     with PointingKinematicConstraint): the B momentum must point from the primary vertex
//     to the B decay vertex. The constraint uses the full covariance of the B candidate,
//     including the correlations between the daughters from step 1.
std::optional<MMGVertexFit::Result> MMGVertexFit::fitCandidate(const RefCountedKinematicParticle& mu1,
    const RefCountedKinematicParticle& mu2,
    const reco::Photon& photon,
    const EcalClusterLazyTools& lazyTools,
    const TransientTrackBuilder* TTBuilder,
    const GlobalPoint& pv) const
    {
        if (!hasClusterHits(photon)) return std::nullopt;

        // copy of the photon with the rescaled energy uncertainty; the energy is unchanged.
        // clone() keeps the full type, e.g. pat::Photon with its embedded supercluster.
        // The kinematic particle keeps raw pointers to the copy and to its cluster covariance;
        // both are owned by the Result, so they live as long as the fitted particles.
        std::shared_ptr<reco::Photon> fitPhoton(photon.clone());
        const reco::Photon::P4type p4type = energyType(*fitPhoton);
        fitPhoton->setCorrectedEnergy(p4type, fitPhoton->getCorrectedEnergy(p4type), photonEnergyError(photon), false);
        auto photonCovariance = std::make_shared<TMatrixD>(clusterCovariance(*fitPhoton, lazyTools));

        const GlobalVector p3(fitPhoton->px(), fitPhoton->py(), fitPhoton->pz());
        const TrackCharge ch = 0;
        const ParticleMass photon_mass(0.);
        float photon_sigma = 1E-6;

        // initial state: only used as the expansion point, the measurement is the cluster
        FreeTrajectoryState fts(pv, p3, ch, TTBuilder->field());
        AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
        fts.setCartesianError(CartesianTrajectoryError(photonCov));
        const reco::TransientTrack phoTT = TTBuilder->build(fts);

        KinematicParticleFactoryFromTransientTrack pFactory;
        std::vector<RefCountedKinematicParticle> allParticles{
            mu1, mu2,
            pFactory.particle(phoTT, photon_mass, float(0), float(0), photon_sigma, fitPhoton.get(), photonCovariance.get())};

        KinematicParticleVertexFitter vertexFitter;
        KinematicParticleFitter constraintFitter;
        PointingKinematicConstraint pointingConstraint(pv);

        RefCountedKinematicTree vertexFitTree;
        std::vector<RefCountedKinematicParticle> daughters;
        try
        {
            // 1. vertex fit with the photon cluster
            vertexFitTree = vertexFitter.fit(allParticles);
            if (!vertexFitTree->isValid()) return std::nullopt;

            // 2. pointing constraint on the B candidate
            vertexFitTree->movePointerToTheTop();
            vertexFitTree = constraintFitter.fit(&pointingConstraint, vertexFitTree);
            if (!vertexFitTree->isValid()) return std::nullopt;
        }
        catch (const VertexException&)
        {
            return std::nullopt;
        }

        // fitted daughters (the pointing constraint only changes the top particle)
        daughters = vertexFitTree->finalStateParticles();

        // get the fitted particle and vertex
        vertexFitTree->movePointerToTheTop();
        RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
        RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
        if (!fitVertex->vertexIsValid()) return std::nullopt;

        // the constraint fit can diverge and still return a valid tree
        const double fitMass = fitParticle->currentState().mass();
        if (!std::isfinite(fitVertex->chiSquared()) || !std::isfinite(fitMass) || fitMass <= 0.) return std::nullopt;

        Result result;
        result.particle = fitParticle;
        result.vertex = fitVertex;
        result.daughters = std::move(daughters);
        result.fitPhoton = std::move(fitPhoton);
        result.fitPhotonCovariance = std::move(photonCovariance);
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

        // kinematic particles of the muons, built once
        std::vector<std::optional<RefCountedKinematicParticle>> muonKinematicParticles;
        muonKinematicParticles.reserve(muons.size());
        for (const reco::Muon* recoMu : muons)
            muonKinematicParticles.push_back(muonParticle(*recoMu, TTBuilder));

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
