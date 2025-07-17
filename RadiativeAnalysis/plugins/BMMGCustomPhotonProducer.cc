// -*- C++ -*-
//
// Package:    BsToMuMuGammaAnalysis/RadiativeAnalysis
// Class:      BMMGCustomPhotonProducer
//
/**\class BMMGCustomPhotonProducer BMMGCustomPhotonProducer.cc BsToMuMuGammaAnalysis/RadiativeAnalysis/plugins/BMMGCustomPhotonProducer.cc

 Description: Custom photon producer for BsToMuMuGamma analysis

 Implementation:
     Creates comprehensive photon objects from SuperClusters with all variables
     needed for photon analysis including low pT photons starting from 0 GeV.
*/
// Original Author:  Alibordi Muhammad
// Created:  Fri, 11 Jul 2025 10:22:23 GMT


#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BMMGCustomPhotonProducer.h"




BMMGCustomPhotonProducer::BMMGCustomPhotonProducer(const edm::ParameterSet& iConfig) {
  ebSuperClustersToken_ = consumes<reco::SuperClusterCollection>(iConfig.getParameter<edm::InputTag>("ebSuperClusters"));
  eeSuperClustersToken_ = consumes<reco::SuperClusterCollection>(iConfig.getParameter<edm::InputTag>("eeSuperClusters"));
  ebRecHitsToken_       = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("ebRecHits"));
  eeRecHitsToken_       = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("eeRecHits"));
  hbheRecHitsToken_     = consumes<HBHERecHitCollection>(iConfig.getParameter<edm::InputTag>("hbheRecHits"));
  caloTowersToken_      = consumes<CaloTowerCollection>(iConfig.getParameter<edm::InputTag>("caloTowers"));
  caloGeomToken_        = esConsumes<CaloGeometry, CaloGeometryRecord>();
  caloTopoToken_        = esConsumes<CaloTopology, CaloTopologyRecord>();

  // Configuration parameters
  minPt_ = iConfig.getParameter<double>("minPt");
  maxEta_ = iConfig.getParameter<double>("maxEta");
  minEnergy_ = iConfig.getParameter<double>("minEnergy");
  useAllClusters_ = iConfig.getParameter<bool>("useAllClusters");
  calculateIsolation_ = iConfig.getParameter<bool>("calculateIsolation");
  calculateShowerShapes_ = iConfig.getParameter<bool>("calculateShowerShapes");
  
  // Isolation parameters
  isolationConeSize_ = iConfig.getParameter<double>("isolationConeSize");
  isolationConeInnerSize_ = iConfig.getParameter<double>("isolationConeInnerSize");
  hOverEConeSize_ = iConfig.getParameter<double>("hOverEConeSize");
  
  // Output collection name
  outputCollectionName_ = iConfig.getParameter<std::string>("outputCollectionName");
  
  // Register products
  produces<reco::PhotonCollection>(outputCollectionName_);
}

BMMGCustomPhotonProducer::~BMMGCustomPhotonProducer() {
  // Do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
}
// ------------ method called to produce the data  ------------
void BMMGCustomPhotonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  
  Handle<reco::SuperClusterCollection> ebSuperClusters;
  iEvent.getByToken(ebSuperClustersToken_, ebSuperClusters);
  
  Handle<reco::SuperClusterCollection> eeSuperClusters;
  iEvent.getByToken(eeSuperClustersToken_, eeSuperClusters);
  
  Handle<EcalRecHitCollection> ebRecHits;
  iEvent.getByToken(ebRecHitsToken_, ebRecHits);
  
  Handle<EcalRecHitCollection> eeRecHits;
  iEvent.getByToken(eeRecHitsToken_, eeRecHits);
  
  Handle<HBHERecHitCollection> hbheRecHits;
  iEvent.getByToken(hbheRecHitsToken_, hbheRecHits);
  
  Handle<CaloTowerCollection> caloTowers;
  iEvent.getByToken(caloTowersToken_, caloTowers);

  const auto& caloGeom              = iSetup.getData(caloGeomToken_);
  const auto& caloTopo              = iSetup.getData(caloTopoToken_);

  // ESHandle<CaloTopology> topologyHandle;
  // iSetup.get<CaloTopologyRecord>().get(topologyHandle);
  // const CaloTopology& topology = *topologyHandle;

  //  output collection
  std::unique_ptr<reco::PhotonCollection> photons(new reco::PhotonCollection());
  
  // Process barrel SuperClusters
  if (ebSuperClusters.isValid()) {
    for (size_t i = 0; i < ebSuperClusters->size(); ++i) {
      const reco::SuperCluster& sc = (*ebSuperClusters)[i];
      
      if (!passesKinematicCuts(sc)) continue;
      reco::Photon::Point vtx(0, 0, 0);
      math::XYZPoint caloposition = calculateCaloPosition(sc);
      reco::Photon::ShowerShape showerShape;
      if (calculateShowerShapes_) {
        showerShape = calculateShowerShape(sc, *ebRecHits, *eeRecHits, caloGeom, caloTopo);
      }
      reco::Photon::IsolationVariables isolationVars;
      if (calculateIsolation_) {
        isolationVars = calculateIsolation(sc, *hbheRecHits, *caloTowers, caloGeom);
      }
      double hoe = calculateHoE(sc, *caloTowers, caloGeom);
      reco::Photon::MIPVariables mipVars = calculateMIPVariables(sc);
      reco::Photon::FiducialFlags fiducialFlags = calculateFiducialFlags(sc, caloGeom);
      double energy = sc.energy();
      double eta = sc.eta();
      double phi = sc.phi();
      double pt = energy * sin(2 * atan(exp(-eta)));
      
      math::XYZTLorentzVector p4(pt * cos(phi), pt * sin(phi), pt * sinh(eta), energy);
      
      reco::SuperClusterRef scRef(ebSuperClusters, i);
      reco::Photon photon(
          reco::Photon::Point(0, 0, 0),  // vertex position
          scRef,                          // SuperCluster ref
          caloposition,                   // position in ECAL
          reco::Photon::LorentzVector(p4) // momentum
      );

      photon.setShowerShapeVariables(showerShape);
      photon.setIsolationVariables(isolationVars);
      photon.setHadronicOverEm(hoe);
      photon.setMIPVariables(mipVars);
      photon.setFiducialVolumeFlags(fiducialFlags);
      photons->push_back(photon);
    }
  }
  
  // Process endcap SuperClusters
  if (eeSuperClusters.isValid()) {
    for (size_t i = 0; i < eeSuperClusters->size(); ++i) {
      const reco::SuperCluster& sc = (*eeSuperClusters)[i];
      
      if (!passesKinematicCuts(sc)) continue;
      
      reco::Photon::Point vtx(0, 0, 0);
      
      math::XYZPoint caloposition = calculateCaloPosition(sc);
      
      reco::Photon::ShowerShape showerShape;
      if (calculateShowerShapes_) {
        showerShape = calculateShowerShape(sc, *ebRecHits, *eeRecHits, caloGeom, caloTopo);
      }
      
      reco::Photon::IsolationVariables isolationVars;
      if (calculateIsolation_) {
        isolationVars = calculateIsolation(sc, *hbheRecHits, *caloTowers, caloGeom);
      }
      
      double hoe = calculateHoE(sc, *caloTowers, caloGeom);
      
      reco::Photon::MIPVariables mipVars = calculateMIPVariables(sc);
      
      reco::Photon::FiducialFlags fiducialFlags = calculateFiducialFlags(sc, caloGeom);
      
      double energy = sc.energy();
      double eta = sc.eta();
      double phi = sc.phi();
      double pt = energy * sin(2 * atan(exp(-eta)));
      
      math::XYZTLorentzVector p4(pt * cos(phi), pt * sin(phi), pt * sinh(eta), energy);
      
      reco::SuperClusterRef scRef(eeSuperClusters, i);
      reco::Photon photon(
          reco::Photon::Point(0, 0, 0),  // vertex position
          scRef,                          // SuperCluster ref
          caloposition,                   // position in ECAL
          reco::Photon::LorentzVector(p4) // momentum
      );
      
      photon.setShowerShapeVariables(showerShape);
      photon.setIsolationVariables(isolationVars);
      photon.setHadronicOverEm(hoe);
      photon.setMIPVariables(mipVars);
      photon.setFiducialVolumeFlags(fiducialFlags);
      photons->push_back(photon);
    }
  }
  
  // Put collection in event
  iEvent.put(std::move(photons), outputCollectionName_);
}

// ------------ Helper methods ------------

reco::Photon::ShowerShape BMMGCustomPhotonProducer::calculateShowerShape(
    const reco::SuperCluster& sc,
    const EcalRecHitCollection& ebRecHits,
    const EcalRecHitCollection& eeRecHits,
    const CaloGeometry& geometry,
    const CaloTopology& topology) {
  
  reco::Photon::ShowerShape showerShape;
  
  try {
    // Calculate local covariances
    std::vector<float> localCov = EcalClusterTools::localCovariances(sc, ebRecHits, eeRecHits, topology);
    if (localCov.size() >= 3) {
      showerShape.sigmaIetaIeta = sqrt(localCov[0]);
      showerShape.sigmaIetaIphi = localCov[1];
      showerShape.sigmaIphiIphi = sqrt(localCov[2]);
    }
    
    // Calculate SC local covariances
    std::vector<float> scLocalCov = EcalClusterTools::scLocalCovariances(sc, ebRecHits, eeRecHits, topology);
    if (scLocalCov.size() >= 3) {
      showerShape.sigmaEtaEta = sqrt(scLocalCov[0]);
    }
    
    // Calculate energy ratios
    showerShape.e1x5 = EcalClusterTools::e1x5(sc, ebRecHits, eeRecHits, topology);
    showerShape.e2x5 = EcalClusterTools::e2x5Max(sc, ebRecHits, eeRecHits, topology);
    showerShape.e3x3 = EcalClusterTools::e3x3(sc, ebRecHits, eeRecHits, topology);
    showerShape.e5x5 = EcalClusterTools::e5x5(sc, ebRecHits, eeRecHits, topology);
    
    // Calculate R9
    if (sc.energy() > 0) {
      showerShape.r9 = showerShape.e3x3 / sc.energy();
    }
    
    // Calculate maximum energy crystal
    showerShape.maxEnergyXtal = EcalClusterTools::eMax(sc, ebRecHits, eeRecHits);
    
    // Calculate effective sigma RR
    if (localCov.size() >= 3) {
      showerShape.effSigmaRR = sqrt(localCov[0] + localCov[2]);
    }
    
  } catch (const std::exception& e) {
    edm::LogWarning("BMMGCustomPhotonProducer") << "Exception in calculateShowerShape: " << e.what();
  }
  
  return showerShape;
}

reco::Photon::IsolationVariables BMMGCustomPhotonProducer::calculateIsolation(
    const reco::SuperCluster& sc,
    const HBHERecHitCollection& hbheRecHits,
    const CaloTowerCollection& caloTowers,
    const CaloGeometry& geometry) {
  
  reco::Photon::IsolationVariables isolationVars;
  
  // Initialize isolation variables, I am overestimating this part since , we might not need this many isolation 
  isolationVars.ecalRecHitSumEt = 0;
  isolationVars.hcalRecHitSumEt = 0;
  isolationVars.solidConeTrkSumPt = 0;
  isolationVars.nTrkSolidCone = 0;
  isolationVars.nTrkHollowCone = 0;
  isolationVars.hollowConeTrkSumPt = 0;
  isolationVars.ecalRecHitSumEtConeDR04 = 0;
  isolationVars.hcalRecHitSumEtConeDR04 = 0;
  isolationVars.solidConeTrkSumPtConeDR04 = 0;
  isolationVars.nTrkSolidConeConeDR04 = 0;
  isolationVars.nTrkHollowConeConeDR04 = 0;
  isolationVars.hollowConeTrkSumPtConeDR04 = 0;
  
  // Calculate HCAL isolation using CaloTowers
  double hcalIso = 0;
  double hcalIso04 = 0;
  double scEta = sc.eta();
  double scPhi = sc.phi();
  
  for (const auto& tower : caloTowers) {
    double dEta = tower.eta() - scEta;
    double dPhi = deltaPhi(tower.phi(), scPhi);
    double dR = sqrt(dEta*dEta + dPhi*dPhi);
    
    // DR < 0.3 cone
    if (dR < isolationConeSize_ && dR > isolationConeInnerSize_) {
      hcalIso += tower.hadEt();
    }
    
    // DR < 0.4 cone
    if (dR < 0.4 && dR > 0.15) {
      hcalIso04 += tower.hadEt();
    }
  }
  
  isolationVars.hcalRecHitSumEt = hcalIso;
  isolationVars.hcalRecHitSumEtConeDR04 = hcalIso04;
  
  return isolationVars;
}

double BMMGCustomPhotonProducer::calculateHoE(const reco::SuperCluster& sc,
                                              const CaloTowerCollection& caloTowers,
                                              const CaloGeometry& geometry) {
  double hcalEnergy = 0;
  double scEta = sc.eta();
  double scPhi = sc.phi();
  
  for (const auto& tower : caloTowers) {
    double dEta = tower.eta() - scEta;
    double dPhi = deltaPhi(tower.phi(), scPhi);
    double dR = sqrt(dEta*dEta + dPhi*dPhi);
    
    if (dR < hOverEConeSize_) {
      hcalEnergy += tower.hadEnergy();
    }
  }
  
  return (sc.energy() > 0) ? hcalEnergy / sc.energy() : 0;
}

reco::Photon::MIPVariables BMMGCustomPhotonProducer::calculateMIPVariables(const reco::SuperCluster& sc) {
  reco::Photon::MIPVariables mipVars;
  
  // Initialize MIP variables
  mipVars.mipChi2 = 0;
  mipVars.mipTotEnergy = 0;
  mipVars.mipSlope = 0;
  mipVars.mipIntercept = 0;
  mipVars.mipNhitCone = 0;
  mipVars.mipIsHalo = false;
  
  return mipVars;
}

reco::Photon::FiducialFlags BMMGCustomPhotonProducer::calculateFiducialFlags(const reco::SuperCluster& sc,
                                                                            const CaloGeometry& geometry) {
  reco::Photon::FiducialFlags fiducialFlags;
  
  double scEta = sc.eta();
  
  fiducialFlags.isEB = (std::abs(scEta) < 1.479);
  fiducialFlags.isEE = (std::abs(scEta) > 1.479);
  fiducialFlags.isEBEtaGap = false;
  fiducialFlags.isEBPhiGap = false;
  fiducialFlags.isEERingGap = false;
  fiducialFlags.isEEDeeGap = false;
  fiducialFlags.isEBEEGap = (std::abs(scEta) > 1.4442 && std::abs(scEta) < 1.566);
  
  return fiducialFlags;
}

bool BMMGCustomPhotonProducer::passesKinematicCuts(const reco::SuperCluster& sc) const {
  double scEt = sc.energy() * sin(2 * atan(exp(-sc.eta())));
  
  if (scEt < minPt_) return false;
  if (std::abs(sc.eta()) > maxEta_) return false;
  if (sc.energy() < minEnergy_) return false;
  
  return true;
}

math::XYZPoint BMMGCustomPhotonProducer::calculateCaloPosition(const reco::SuperCluster& sc) const {
  return math::XYZPoint(sc.x(), sc.y(), sc.z());
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void BMMGCustomPhotonProducer::beginStream(edm::StreamID) {
  // Nothing to do here for now
}

// ------------ method called once each stream after processing all runs, lumis or events  ------------
void BMMGCustomPhotonProducer::endStream() {
  // Nothing to do here for now
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void BMMGCustomPhotonProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  
  //python config parse
  desc.add<edm::InputTag>("ebSuperClusters", edm::InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALBarrel"));
  desc.add<edm::InputTag>("eeSuperClusters", edm::InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALEndcapWithPreshower"));
  desc.add<edm::InputTag>("ebRecHits", edm::InputTag("reducedEcalRecHitsEB"));
  desc.add<edm::InputTag>("eeRecHits", edm::InputTag("reducedEcalRecHitsEE"));
  desc.add<edm::InputTag>("hbheRecHits", edm::InputTag("reducedHcalRecHits", "hbhereco"));
  desc.add<edm::InputTag>("caloTowers", edm::InputTag("caloTowers"));
  
  // Selection 
  desc.add<double>("minPt", 0.0);
  desc.add<double>("maxEta", 2.5);
  desc.add<double>("minEnergy", 0.0);
  desc.add<bool>("useAllClusters", true);
  desc.add<bool>("calculateIsolation", true);
  desc.add<bool>("calculateShowerShapes", true);
  
  // Isolation 
  desc.add<double>("isolationConeSize", 0.3);
  desc.add<double>("isolationConeInnerSize", 0.15);
  desc.add<double>("hOverEConeSize", 0.15);
  
  desc.add<std::string>("outputCollectionName", "");
  
  descriptions.add("BMMGCustomPhotonProducer", desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(BMMGCustomPhotonProducer);
