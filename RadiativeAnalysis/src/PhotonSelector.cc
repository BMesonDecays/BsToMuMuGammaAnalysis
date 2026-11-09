#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/PhotonSelector.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MultiTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TMath.h"

#include <utility>
#include <numeric>
#include <vector>
#include <algorithm>

PhotonSelector::PhotonSelector() {}
PhotonSelector::~PhotonSelector() {}


template <typename PhotonCollection>
std::pair< std::vector<size_t>, std::vector<double> > PhotonSelector::selectRecoPhotons( const PhotonCollection& photons, const RefCountedKinematicParticle& dimuon,
    const math::XYZPoint dimuonVtxPosition, double max_dR )
{
    GlobalVector dimuonMomentum = dimuon->currentState().globalMomentum();

    std::vector<size_t> selectedPhotonIndices;  // output vector 1
    std::vector<double> selectedPhotonDeltaRs;  // output vector 2
    for (size_t i = 0; i < photons.size(); i++)
    {
        reco::Photon photon = photons.at(i);    // copy, because setVertex modifies the photon direction
        if ( !passPhotonSelection( photon ) )   continue;

        photon.setVertex(dimuonVtxPosition);
        double dR = reco::deltaR( dimuonMomentum.eta(), dimuonMomentum.phi(), photon.eta(), photon.phi() );

        if (dR > max_dR)    continue;

        selectedPhotonIndices.push_back(i);
        selectedPhotonDeltaRs.push_back(dR);
    }
    
    return std::make_pair( selectedPhotonIndices, selectedPhotonDeltaRs );
}
template <typename Photon>
bool PhotonSelector::passPhotonSelection(const Photon& photon) const 
{
    if (photon.superCluster().isNull())                         return false;
    if (photon.superCluster()->energy() < 1.0)                  return false; // Minimum energy cut for photons
    if (std::abs(photon.superCluster()->eta()) > 2.5)           return false; // ECAL acceptance

    return true;
}

// Explicit instantiations (the templates are defined in this .cc file, so used types must be listed here)
template std::pair< std::vector<size_t>, std::vector<double> > PhotonSelector::selectRecoPhotons<edm::View<reco::Photon>>(
    const edm::View<reco::Photon>&, const RefCountedKinematicParticle&, const math::XYZPoint, double);
template std::pair< std::vector<size_t>, std::vector<double> > PhotonSelector::selectRecoPhotons<std::vector<reco::Photon>>(
    const std::vector<reco::Photon>&, const RefCountedKinematicParticle&, const math::XYZPoint, double);
