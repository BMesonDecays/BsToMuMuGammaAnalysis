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

// Sequential fit of B -> mu mu gamma:
//  1. Kalman vertex fit of the two muons and the photon (KinematicParticleVertexFitter).
//     The photon enters through its calorimeter cluster position and energy, so its
//     momentum is pointed from the fitted vertex to the cluster.
//  2. Pointing constraint on the top particle of the resulting tree (KinematicParticleFitter
//     with PointingKinematicConstraint): the B momentum must point from the primary vertex
//     to the B decay vertex. The constraint uses the full covariance of the B candidate,
//     including the correlations between the daughters from step 1.
std::vector<double> MMGVertexFit::fit(const std::vector<reco::Muon>& muons,
    const std::vector<reco::Photon>& photons,
    const EcalClusterLazyTools& lazyTools,
    const std::vector<reco::Vertex>& primaryVertices,
    const TransientTrackBuilder* TTBuilder)
    {
        std::vector<double> mass;
        if (primaryVertices.empty()) return mass;

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
        std::vector<std::unique_ptr<TMatrixD>> photonCovariances;
        std::vector<RefCountedKinematicParticle> photonKinematicParticles;
        for(const auto& recoPho : photons)
        {
          if(recoPho.isEB() == 0) continue;

          GlobalVector p3(recoPho.px(), recoPho.py(), recoPho.pz());
          TrackCharge ch = 0;

          const ParticleMass photon_mass(0.);
          float photon_sigma = 1E-6;

          // initial state: only used as the expansion point, the measurement is the cluster
          FreeTrajectoryState fts(pv, p3, ch, TTBuilder->field());
          AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
          fts.setCartesianError(CartesianTrajectoryError(photonCov));
          reco::TransientTrack phoTT = TTBuilder->build(fts);

          photonCovariances.push_back(std::make_unique<TMatrixD>(lazyTools.covariancesXYZ(*recoPho.superCluster())));
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

                    // invariant mass
                    mass.push_back(fitMass);
                }
            }
        }
        return mass;
    }
