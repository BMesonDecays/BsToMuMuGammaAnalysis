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
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicConstraint.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MultiTrackPointingKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"



std::vector<double> MMGVertexFit::fit(const std::vector<reco::Muon>& muons, 
    const std::vector<reco::Photon>& photons,
    const EcalClusterLazyTools& lazyTools,
    const std::vector<reco::Vertex>& primaryVertices,
    const TransientTrackBuilder* TTBuilder)
    {
        std::vector<double> mass;
        // kinematic particle creation
        std::vector<RefCountedKinematicParticle> muonKinematicParticles;
        for(const auto& recoMu : muons)
        {
            reco::TrackRef muTrack = recoMu.track();
            if(!muTrack) continue;
            reco::TransientTrack muonTT = TTBuilder->build(muTrack);

            const ParticleMass muon_mass(0.105658);
            float muon_sigma = 1E-6;

            KinematicParticleFactoryFromTransientTrack pFactory;
            muonKinematicParticles.push_back(pFactory.particle(muonTT, muon_mass, float(0), float(0), muon_sigma));
        }

        std::vector<RefCountedKinematicParticle> photonKinematicParticles;
        for(const auto& recoPho : photons)
        {
          reco::Photon* recoPhoPtr(new reco::Photon(recoPho));
          if(recoPho.isEB() == 0) continue;
      
          GlobalPoint vtx(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
          GlobalVector p3(recoPho.px(), recoPho.py(), recoPho.pz());
          TrackCharge ch = 0;
      
          const ParticleMass photon_mass(0.);
          float photon_sigma = 1E-6;
      
          FreeTrajectoryState fts(vtx, p3, ch, TTBuilder->field());
      
          TMatrixD cov(lazyTools.covariancesXYZ(*recoPho.superCluster()));
          TMatrixD* covPtr(new TMatrixD(cov));
      
          
          AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
          AlgebraicVector6 diagonal(1., 1., 1., 1., 1., 1.);
          photonCov.SetDiagonal(diagonal);
      
          CartesianTrajectoryError photonErr(photonCov);
          fts.setCartesianError(photonErr);
      
          reco::TransientTrack phoTT = TTBuilder->build(fts);
          KinematicParticleFactoryFromTransientTrack pFactory;
          photonKinematicParticles.push_back(pFactory.particle(phoTT, photon_mass, float(0), float(0), photon_sigma, recoPhoPtr, covPtr));
        }

        for (unsigned int i = 0; i < muonKinematicParticles.size(); i++)
        {
            for (unsigned int j = i+1; j < muonKinematicParticles.size(); j++)
            {
                for (unsigned int k = 0; k < photonKinematicParticles.size(); k++)
                {
                    RefCountedKinematicParticle mu1 = muonKinematicParticles.at(i);
                    RefCountedKinematicParticle mu2 = muonKinematicParticles.at(j);
                    RefCountedKinematicParticle pho = photonKinematicParticles.at(k);
                    std::vector<RefCountedKinematicParticle> allParticles;
                    allParticles.push_back(mu1);
                    allParticles.push_back(mu2);
                    allParticles.push_back(pho);

                    // kalman vertex fitter
                    KinematicParticleVertexFitter fitter;

                    RefCountedKinematicTree vertexFitTree = fitter.fit(allParticles);

                    if (!vertexFitTree->isValid()) continue;

                    // global fit with pointing constraint

                    // first recreate kinamatic particles from already fitted particles from the previous fit
                    KinematicParticleFactoryFromTransientTrack pFactory;
                    std::vector<RefCountedKinematicParticle> allParticlesGlobal;
                    vertexFitTree->movePointerToTheTop();
                    vertexFitTree->movePointerToTheFirstChild();
                    RefCountedKinematicParticle mu1Global = vertexFitTree->currentParticle();
                    FreeTrajectoryState fts(mu1Global->currentState().globalPosition(), mu1Global->currentState().globalMomentum(), mu1Global->currentState().particleCharge(), TTBuilder->field());
                    AlgebraicSymMatrix66 muonCov;
                    AlgebraicSymMatrix77 muonCov77 = mu1Global->currentState().kinematicParametersError().matrix();
                    for (int i = 0; i < 6; i++)
                    {
                        for (int j = 0; j < 6; j++)
                        {
                            muonCov(i, j) = muonCov77(i, j);
                        }
                    }
                    CartesianTrajectoryError muonErr(muonCov);
                    fts.setCartesianError(muonErr);
                    reco::TransientTrack muonTTGlobal = TTBuilder->build(fts);
                    float sigma = 0.001;
                    mu1Global = pFactory.particle(muonTTGlobal, mu1Global->currentState().mass(), float(0), float(0), sigma);
                    allParticlesGlobal.push_back(mu1Global);

                    vertexFitTree->movePointerToTheNextChild();
                    RefCountedKinematicParticle mu2Global = vertexFitTree->currentParticle();
                    FreeTrajectoryState fts2(mu2Global->currentState().globalPosition(), mu2Global->currentState().globalMomentum(), mu2Global->currentState().particleCharge(), TTBuilder->field());
                    AlgebraicSymMatrix66 muonCov2;
                    AlgebraicSymMatrix77 muonCov77_2 = mu2Global->currentState().kinematicParametersError().matrix();
                    for (int i = 0; i < 6; i++)
                    {
                        for (int j = 0; j < 6; j++)
                        {
                            muonCov2(i, j) = muonCov77_2(i, j);
                        }
                    }
                    CartesianTrajectoryError muonErr2(muonCov2);
                    fts2.setCartesianError(muonErr2);
                    reco::TransientTrack muonTTGlobal2 = TTBuilder->build(fts2);
                    mu2Global = pFactory.particle(muonTTGlobal2, mu2Global->currentState().mass(), float(0), float(0), sigma);
                    allParticlesGlobal.push_back(mu2Global);

                    vertexFitTree->movePointerToTheNextChild();
                    RefCountedKinematicParticle phoGlobal = vertexFitTree->currentParticle();
                    FreeTrajectoryState fts3(phoGlobal->currentState().globalPosition(), phoGlobal->currentState().globalMomentum(), phoGlobal->currentState().particleCharge(), TTBuilder->field());
                    AlgebraicSymMatrix66 photonCov;
                    AlgebraicSymMatrix77 photonCov77 = phoGlobal->currentState().kinematicParametersError().matrix();
                    for (int i = 0; i < 6; i++)
                    {
                        for (int j = 0; j < 6; j++)
                        {
                            photonCov(i, j) = photonCov77(i, j);
                        }
                    }
                    CartesianTrajectoryError photonErr(photonCov);
                    fts3.setCartesianError(photonErr);
                    reco::TransientTrack phoTTGlobal = TTBuilder->build(fts3);
                    float sigma3 = 0.001;
                    phoGlobal = pFactory.particle(phoTTGlobal, phoGlobal->currentState().mass(), float(0), float(0), sigma3);

                    allParticlesGlobal.push_back(phoGlobal);

                    vertexFitTree->movePointerToTheTop();

                    // create the constraint
                    GlobalPoint pvGlobalPoint(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
                    MultiTrackKinematicConstraint* multiPointingConstraint = new MultiTrackPointingKinematicConstraint(pvGlobalPoint);
                    KinematicConstrainedVertexFitter constrainedFitter;
                    RefCountedKinematicTree vertexFitTreeGlobal = constrainedFitter.fit(allParticlesGlobal, multiPointingConstraint);
                    if (!vertexFitTreeGlobal->isValid()) continue;
                    // get the fitted particle and vertex
                    vertexFitTreeGlobal->movePointerToTheTop();
                    RefCountedKinematicParticle fitParticleGlobal = vertexFitTreeGlobal->currentParticle();
                    RefCountedKinematicVertex fitVertexGlobal = vertexFitTreeGlobal->currentDecayVertex();
                    if (!fitVertexGlobal->vertexIsValid()) continue;

                    // invariant mass
                    mass.push_back(fitParticleGlobal->currentState().mass());

                }
            }
        }
        return mass;
    }