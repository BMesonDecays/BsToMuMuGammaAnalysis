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
        const ClusterScale& scale = photon.isEB() ? ebScale_ : eeScale_;

        const double phi = photon.phi(), theta = photon.theta();
        const double ePhi[3] = {-std::sin(phi), std::cos(phi), 0.};
        const double eTheta[3] = {std::cos(theta) * std::cos(phi), std::cos(theta) * std::sin(phi), -std::sin(theta)};
        const double u[3] = {std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta)};

        TMatrixD S(3, 3);
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
                S(a, b) = scale.phi * ePhi[a] * ePhi[b] + scale.theta * eTheta[a] * eTheta[b] + u[a] * u[b];
        return S * cov * S;
    }

// The energy uncertainty of the photon candidate underestimates the spread of E_reco - E_gen;
// it is rescaled with a factor from the photon energy pulls.
double MMGVertexFit::photonEnergyError(const reco::Photon& photon) const
    {
        return energyErrorScale_ * photon.getCorrectedEnergyError(energyType(photon));
    }

// Sequential fit of B -> mu mu gamma:
//  1. Kalman vertex fit of the two muons and the photon (KinematicParticleVertexFitter).
//     The photon enters through its calorimeter cluster position and energy, so its
//     momentum is pointed from the fitted vertex to the cluster.
//  2. Pointing constraint on the top particle of the resulting tree (KinematicParticleFitter
//     with PointingKinematicConstraint): the B momentum must point from the primary vertex
//     to the B decay vertex. The constraint uses the full covariance of the B candidate,
//     including the correlations between the daughters from step 1.
std::vector<MMGVertexFit::Result> MMGVertexFit::fit(const std::vector<reco::Muon>& muons,
    const std::vector<reco::Photon>& photons,
    const EcalClusterLazyTools& lazyTools,
    const std::vector<reco::Vertex>& primaryVertices,
    const TransientTrackBuilder* TTBuilder)
    {
        std::vector<Result> results;
        if (primaryVertices.empty()) return results;

        const GlobalPoint pv(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());

        KinematicParticleFactoryFromTransientTrack pFactory;

        // kinematic particle creation
        std::vector<RefCountedKinematicParticle> muonKinematicParticles;
        for(const auto& recoMu : muons)
        {
            reco::TrackRef muTrack = recoMu.track();
            if(!muTrack) continue;
            reco::TransientTrack muonTT = TTBuilder->build(muTrack);

            const ParticleMass muon_mass(0.105658);
            float muon_sigma = 1E-6;

            muonKinematicParticles.push_back(pFactory.particle(muonTT, muon_mass, float(0), float(0), muon_sigma));
        }

        // the kinematic particles keep raw pointers to the photon and to its cluster covariance,
        // so both have to stay alive until all fits are done
        std::vector<std::unique_ptr<reco::Photon>> fitPhotons;
        std::vector<std::unique_ptr<TMatrixD>> photonCovariances;
        std::vector<RefCountedKinematicParticle> photonKinematicParticles;
        for(const auto& inputPho : photons)
        {

          // copy of the photon with the rescaled energy uncertainty; the energy is unchanged
          fitPhotons.push_back(std::make_unique<reco::Photon>(inputPho));
          reco::Photon& recoPho = *fitPhotons.back();
          const reco::Photon::P4type p4type = energyType(recoPho);
          recoPho.setCorrectedEnergy(p4type, recoPho.getCorrectedEnergy(p4type), photonEnergyError(inputPho), false);

          GlobalVector p3(recoPho.px(), recoPho.py(), recoPho.pz());
          TrackCharge ch = 0;

          const ParticleMass photon_mass(0.);
          float photon_sigma = 1E-6;

          // initial state: only used as the expansion point, the measurement is the cluster
          FreeTrajectoryState fts(pv, p3, ch, TTBuilder->field());
          AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
          fts.setCartesianError(CartesianTrajectoryError(photonCov));
          reco::TransientTrack phoTT = TTBuilder->build(fts);

          photonCovariances.push_back(std::make_unique<TMatrixD>(clusterCovariance(recoPho, lazyTools)));
          photonKinematicParticles.push_back(pFactory.particle(phoTT, photon_mass, float(0), float(0), photon_sigma, &recoPho, photonCovariances.back().get()));
        }

        KinematicParticleVertexFitter vertexFitter;
        KinematicParticleFitter constraintFitter;
        PointingKinematicConstraint pointingConstraint(pv);

        for (unsigned int i = 0; i < muonKinematicParticles.size(); i++)
        {
            for (unsigned int j = i+1; j < muonKinematicParticles.size(); j++)
            {
                for (unsigned int k = 0; k < photonKinematicParticles.size(); k++)
                {
                    std::vector<RefCountedKinematicParticle> allParticles;
                    allParticles.push_back(muonKinematicParticles.at(i));
                    allParticles.push_back(muonKinematicParticles.at(j));
                    allParticles.push_back(photonKinematicParticles.at(k));

                    RefCountedKinematicTree vertexFitTree;
                    try
                    {
                        // 1. vertex fit with the photon cluster
                        vertexFitTree = vertexFitter.fit(allParticles);
                        if (!vertexFitTree->isValid()) continue;

                        // 2. pointing constraint on the B candidate
                        vertexFitTree->movePointerToTheTop();
                        vertexFitTree = constraintFitter.fit(&pointingConstraint, vertexFitTree);
                        if (!vertexFitTree->isValid()) continue;
                    }
                    catch (const VertexException&)
                    {
                        continue;
                    }

                    // get the fitted particle and vertex
                    vertexFitTree->movePointerToTheTop();
                    RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
                    RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
                    if (!fitVertex->vertexIsValid()) continue;

                    // the constraint fit can diverge and still return a valid tree
                    const double fitMass = fitParticle->currentState().mass();
                    if (!std::isfinite(fitVertex->chiSquared()) || !std::isfinite(fitMass) || fitMass <= 0.) continue;

                    results.push_back({fitParticle, fitVertex});
                }
            }
        }
        return results;
    }
