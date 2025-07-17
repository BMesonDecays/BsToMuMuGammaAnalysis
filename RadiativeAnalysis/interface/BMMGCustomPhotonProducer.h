#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_BMMGCustomPhotonProducer_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_BMMGCustomPhotonProducer_h

// -*- C++ -*-
//
// Package:    BsToMuMuGammaAnalysis/BMMGCustomPhotonProducer
// Class:      BMMGCustomPhotonProducer
//
/**\class BMMGCustomPhotonProducer BMMGCustomPhotonProducer.h BsToMuMuGammaAnalysis/BMMGCustomPhotonProducer/interface/BMMGCustomPhotonProducer.h

 Description: Custom photon producer for BsToMuMuGamma analysis that creates photon objects from SuperClusters
              with comprehensive photon variables including low pT photons starting from 0 GeV.

 Implementation:
     Creates reco::Photon objects from SuperClusters with full shower shape calculations,
     isolation variables, and H/E measurements using ECAL/HCAL information.
*/
//
// Original Author:  Alibordi Muhammad
//         Created:  Fri, 11 Jul 2025 10:22:23 GMT
//
//

#include <memory>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"
#include "DataFormats/EgammaReco/interface/BasicClusterFwd.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidate.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalSeverityLevelAlgo.h"

#include "CondFormats/EcalObjects/interface/EcalChannelStatus.h"
#include "CondFormats/DataRecord/interface/EcalChannelStatusRcd.h"

#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"


class BMMGCustomPhotonProducer : public edm::stream::EDProducer<> {
public:
  explicit BMMGCustomPhotonProducer(const edm::ParameterSet&);
  ~BMMGCustomPhotonProducer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override;

  reco::Photon::ShowerShape calculateShowerShape(const reco::SuperCluster& sc,
                                                 const EcalRecHitCollection& ebRecHits,
                                                 const EcalRecHitCollection& eeRecHits,
                                                 const CaloGeometry& geometry,
                                                 const CaloTopology& topology);
  
  reco::Photon::IsolationVariables calculateIsolation(const reco::SuperCluster& sc,
                                                      const HBHERecHitCollection& hbheRecHits,
                                                      const CaloTowerCollection& caloTowers,
                                                      const CaloGeometry& geometry);
  
  double calculateHoE(const reco::SuperCluster& sc,
                     const CaloTowerCollection& caloTowers,
                     const CaloGeometry& geometry);
  
  reco::Photon::MIPVariables calculateMIPVariables(const reco::SuperCluster& sc);
  
  reco::Photon::FiducialFlags calculateFiducialFlags(const reco::SuperCluster& sc,
                                                     const CaloGeometry& geometry);
  
  bool passesKinematicCuts(const reco::SuperCluster& sc) const;
  math::XYZPoint calculateCaloPosition(const reco::SuperCluster& sc) const;
  
  // ----------member data ---------------------------
  
  edm::EDGetTokenT<reco::SuperClusterCollection> ebSuperClustersToken_;
  edm::EDGetTokenT<reco::SuperClusterCollection> eeSuperClustersToken_;
  edm::EDGetTokenT<EcalRecHitCollection> ebRecHitsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> eeRecHitsToken_;
  edm::EDGetTokenT<HBHERecHitCollection> hbheRecHitsToken_;
  edm::EDGetTokenT<CaloTowerCollection> caloTowersToken_;
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeomToken_;
  edm::ESGetToken<CaloTopology, CaloTopologyRecord> caloTopoToken_;
  
  double minPt_;
  double maxEta_;
  double minEnergy_;
  bool useAllClusters_;
  bool calculateIsolation_;
  bool calculateShowerShapes_;
  
  double isolationConeSize_;
  double isolationConeInnerSize_;
  double hOverEConeSize_;
  
  std::string outputCollectionName_;
};

#endif
