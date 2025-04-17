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

#include <vector>

class MMGVertexFit
{
    public:
        MMGVertexFit(){};
        ~MMGVertexFit(){}

        std::vector<double> fit(const std::vector<reco::Muon>& muons, 
            const std::vector<reco::Photon>& photons,
            const EcalClusterLazyTools& lazyTools,
            const std::vector<reco::Vertex>& primaryVertices,
            const TransientTrackBuilder* TTBuilder);
    
};

#endif