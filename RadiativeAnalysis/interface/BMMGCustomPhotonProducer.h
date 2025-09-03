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
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"


#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"
#include "DataFormats/EgammaReco/interface/BasicCluster.h"
#include "DataFormats/EgammaReco/interface/BasicClusterFwd.h"
#include "DataFormats/EgammaReco/interface/BasicClusterShapeAssociation.h"
#include "DataFormats/EgammaReco/interface/ClusterShape.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/PhotonCore.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"


#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"


#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"


#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Common/interface/Handle.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/CaloTopology/interface/HcalTopology.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"


#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "RecoEcal/EgammaCoreTools/interface/PositionCalc.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalSeverityLevelAlgo.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalSeverityLevelAlgoRcd.h"


#include "RecoEgamma/EgammaPhotonAlgos/interface/PhotonEnergyCorrector.h"
#include "RecoEgamma/PhotonIdentification/interface/PhotonIsolationCalculator.h"
#include "RecoEgamma/PhotonIdentification/interface/PhotonMIPHaloTagger.h"
#include "RecoEgamma/PhotonIdentification/interface/PhotonMVABasedHaloTagger.h"
#include "RecoEgamma/EgammaElectronAlgos/interface/ElectronHcalHelper.h"


#include "RecoCaloTools/Selectors/interface/CaloConeSelector.h"

#include "CondFormats/EcalObjects/interface/EcalChannelStatus.h"
#include "CondFormats/EcalObjects/interface/EcalFunctionParameters.h"
#include "CondFormats/DataRecord/interface/EcalChannelStatusRcd.h"
#include "CondFormats/DataRecord/interface/HcalPFCutsRcd.h"
#include "CondTools/Hcal/interface/HcalPFCutsHandler.h"


#include "CommonTools/Utils/interface/StringToEnumValue.h"


#include "CommonTools/Utils/interface/StringToEnumValue.h"
#include "CondFormats/EcalObjects/interface/EcalFunctionParameters.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonCore.h"
#include "DataFormats/EgammaReco/interface/BasicCluster.h"
#include "DataFormats/EgammaReco/interface/BasicClusterShapeAssociation.h"
#include "DataFormats/EgammaReco/interface/ClusterShape.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "RecoCaloTools/Selectors/interface/CaloConeSelector.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/PositionCalc.h"
#include "RecoEgamma/EgammaPhotonAlgos/interface/PhotonEnergyCorrector.h"
#include "RecoEgamma/PhotonIdentification/interface/PhotonIsolationCalculator.h"
#include "RecoEgamma/PhotonIdentification/interface/PhotonMIPHaloTagger.h"
#include "RecoEgamma/PhotonIdentification/interface/PhotonMVABasedHaloTagger.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalSeverityLevelAlgo.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalSeverityLevelAlgoRcd.h"
#include "RecoEgamma/EgammaElectronAlgos/interface/ElectronHcalHelper.h"
#include "CondFormats/DataRecord/interface/HcalPFCutsRcd.h"
#include "CondTools/Hcal/interface/HcalPFCutsHandler.h"
#include "Geometry/CaloTopology/interface/HcalTopology.h"
#include <vector>


class BMMGCustomPhotonProducer : public edm::stream::EDProducer<> {
public:
  explicit BMMGCustomPhotonProducer(const edm::ParameterSet& ps);
  ~BMMGCustomPhotonProducer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  void produce(edm::Event& evt, const edm::EventSetup& es) override;
private:
  void beginStream(edm::StreamID) override;
  
  void endStream() override;
  void fillPhotonVector(edm::Event& evt,
                            edm::EventSetup const& es,
                            const edm::Handle<reco::PhotonCoreCollection>& photonCoreHandle,
                            const CaloTopology* topology,
                            const HcalPFCuts* hcalCuts,
                            const EcalRecHitCollection* ecalBarrelHits,
                            const EcalRecHitCollection* ecalEndcapHits,
                            ElectronHcalHelper const& hcalHelperCone,
                            ElectronHcalHelper const& hcalHelperBc,
                            reco::VertexCollection& pvVertices,
                            reco::PhotonCollection& outputCollection,
                            int& iSC);
  
  // ----------member data ---------------------------
  
  // std::string PhotonCoreCollection_;
  std::string PhotonCollection_;
  edm::EDGetTokenT<reco::PhotonCoreCollection> photonCoreProducer_;
  edm::EDGetTokenT<EcalRecHitCollection> barrelEcalHits_;
  edm::EDGetTokenT<EcalRecHitCollection> endcapEcalHits_;
  edm::EDGetTokenT<HBHERecHitCollection> hbheRecHits_;
  edm::EDGetTokenT<reco::VertexCollection> vertexProducer_;
  const edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeomToken_;
  const edm::ESGetToken<CaloTopology, CaloTopologyRecord> topologyToken_;

  //AA
  //Flags and severities to be excluded from calculations

  std::vector<int> flagsexclEB_;
  std::vector<int> flagsexclEE_;
  std::vector<int> severitiesexclEB_;
  std::vector<int> severitiesexclEE_;

  double hOverEConeSize_;
  double highEt_;
  double minR9Barrel_;
  double minR9Endcap_;
  bool runMIPTagger_;
  bool runMVABasedHaloTagger_;

  bool validConversions_;

  bool usePrimaryVertex_;

  PositionCalc posCalculator_;

  bool validPixelSeeds_;
  PhotonIsolationCalculator photonIsolationCalculator_;

  //MIP
  const PhotonMIPHaloTagger photonMIPHaloTagger_;
  //MVA based Halo tagger for the EE photons
  std::unique_ptr<const PhotonMVABasedHaloTagger> photonMVABasedHaloTagger_ = nullptr;

  std::vector<double> preselCutValuesBarrel_;
  std::vector<double> preselCutValuesEndcap_;

  PhotonEnergyCorrector photonEnergyCorrector_;
  std::string candidateP4type_;

  // additional configuration and helpers
  std::unique_ptr<ElectronHcalHelper> hcalHelperCone_;
  std::unique_ptr<ElectronHcalHelper> hcalHelperBc_;
  bool hcalRun2EffDepth_;

  edm::ESGetToken<HcalPFCuts, HcalPFCutsRcd> hcalCutsToken_;
  bool cutsFromDB_;
};

#endif
