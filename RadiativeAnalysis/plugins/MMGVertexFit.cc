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



void MMGVertexFit::fit(const std::vector<reco::Muon>& muons, 
    const std::vector<reco::Photon>& photons,
    const EcalRecHitCollection& recHits,
    const EcalClusterLazyTools lazyTools,
    const MagneticField& field,
    const std::vector<reco::Vertex>& primaryVertices,
    const TransientTrackBuilder* theB)
    {
        // kinematic particle creation
        std::vector<RefCountedKinematicParticle> muonKinematicParticles;
        for(const auto& recoMu : muons)
        {
            reco::TrackRef muTrack = recoMu.track();
            if(!muTrack) continue;
            reco::TransientTrack muonTT = reco::TransientTrack(muTrack, &field);

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
      
          FreeTrajectoryState fts(vtx, p3, ch, &field);
      
          TMatrixD cov(lazyTools.covariancesXYZ(*recoPho.superCluster()));
          TMatrixD* covPtr(new TMatrixD(cov));
      
          
          AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
          AlgebraicVector6 diagonal(1., 1., 1., 1., 1., 1.);
          photonCov.SetDiagonal(diagonal);
      
          CartesianTrajectoryError photonErr(photonCov);
          fts.setCartesianError(photonErr);
      
          reco::TransientTrack phoTT = theB->build(fts);
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

                    KinematicParticleVertexFitter fitter;

                    RefCountedKinematicTree vertexFitTree = fitter.fit(allParticles);

                    if (!vertexFitTree->isValid()) continue;
                    // get the fitted particle and vertex
                    vertexFitTree->movePointerToTheTop();

                    // pointing constraint
                    //
                    GlobalPoint pvGlobalPoint(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
                    KinematicConstraint* pointingConstraint = new PointingKinematicConstraint(pvGlobalPoint);
                    KinematicParticleFitter kinematicFitter;
                    vertexFitTree = kinematicFitter.fit(pointingConstraint, vertexFitTree);
                    if (!vertexFitTree->isValid()) continue;

                    // get the fitted particle and vertex
                    vertexFitTree->movePointerToTheTop();
                    RefCountedKinematicParticle fitParticlePointing = vertexFitTree->currentParticle();
                    RefCountedKinematicVertex fitVertexPointing = vertexFitTree->currentDecayVertex();
                    if (!fitVertexPointing->vertexIsValid()) continue;

                    GlobalPoint fittedGlobalPointPointing = fitVertexPointing->position();

                }
            }
        }
    }