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




BMMGCustomPhotonProducer::BMMGCustomPhotonProducer(const edm::ParameterSet& iConfig)
    : caloGeomToken_(esConsumes()),
      topologyToken_(esConsumes()),
      flagsexclEB_{StringToEnumValue<EcalRecHit::Flags>(
          iConfig.getParameter<std::vector<std::string>>("RecHitFlagToBeExcludedEB"))},
      flagsexclEE_{StringToEnumValue<EcalRecHit::Flags>(
          iConfig.getParameter<std::vector<std::string>>("RecHitFlagToBeExcludedEE"))},
      severitiesexclEB_{StringToEnumValue<EcalSeverityLevel::SeverityLevel>(
          iConfig.getParameter<std::vector<std::string>>("RecHitSeverityToBeExcludedEB"))},
      severitiesexclEE_{StringToEnumValue<EcalSeverityLevel::SeverityLevel>(
          iConfig.getParameter<std::vector<std::string>>("RecHitSeverityToBeExcludedEE"))},
      photonIsolationCalculator_(iConfig.getParameter<edm::ParameterSet>("isolationSumsCalculatorSet"),
                                 flagsexclEB_,
                                 flagsexclEE_,
                                 severitiesexclEB_,
                                 severitiesexclEE_,
                                 consumesCollector()),
      photonMIPHaloTagger_(iConfig.getParameter<edm::ParameterSet>("mipVariableSet"), consumesCollector()),
      photonEnergyCorrector_(iConfig, consumesCollector()) {

  photonCoreProducer_ = consumes<reco::PhotonCoreCollection>(iConfig.getParameter<edm::InputTag>("photonCoreProducer"));
  barrelEcalHits_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("barrelEcalHits"));
  endcapEcalHits_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("endcapEcalHits"));
  vertexProducer_ = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexProducer"));
  hbheRecHits_ = consumes<HBHERecHitCollection>(iConfig.getParameter<edm::InputTag>("hbheRecHits"));
  hOverEConeSize_ = iConfig.getParameter<double>("hOverEConeSize");
  highEt_ = iConfig.getParameter<double>("highEt");
  // R9 value to decide converted/unconverted
  minR9Barrel_ = iConfig.getParameter<double>("minR9Barrel");
  minR9Endcap_ = iConfig.getParameter<double>("minR9Endcap");
  usePrimaryVertex_ = iConfig.getParameter<bool>("usePrimaryVertex");
  runMIPTagger_ = iConfig.getParameter<bool>("runMIPTagger");
  runMVABasedHaloTagger_ = iConfig.getParameter<bool>("runMVABasedHaloTagger");

  candidateP4type_ = iConfig.getParameter<std::string>("candidateP4type");

  edm::ParameterSet posCalcParameters = iConfig.getParameter<edm::ParameterSet>("posCalcParameters");
  posCalculator_ = PositionCalc(posCalcParameters);

  //Retrieve HCAL PF thresholds - from iConfig or from DB
  cutsFromDB_ = iConfig.getParameter<bool>("usePFThresholdsFromDB");
  if (cutsFromDB_) {
    hcalCutsToken_ = esConsumes<HcalPFCuts, HcalPFCutsRcd>(edm::ESInputTag("", "withTopo"));
  }

  ElectronHcalHelper::Configuration cfgCone, cfgBc;
  cfgCone.hOverEConeSize = hOverEConeSize_;
  if (cfgCone.hOverEConeSize > 0) {
    cfgCone.onlyBehindCluster = false;
    cfgCone.checkHcalStatus = false;

    cfgCone.hbheRecHits = hbheRecHits_;

    cfgCone.eThresHB = iConfig.getParameter<EgammaHcalIsolation::arrayHB>("recHitEThresholdHB");
    cfgCone.maxSeverityHB = iConfig.getParameter<int>("maxHcalRecHitSeverity");
    cfgCone.eThresHE = iConfig.getParameter<EgammaHcalIsolation::arrayHE>("recHitEThresholdHE");
    cfgCone.maxSeverityHE = cfgCone.maxSeverityHB;
  }

  cfgBc.hOverEConeSize = 0.;
  cfgBc.onlyBehindCluster = true;
  cfgBc.checkHcalStatus = false;

  cfgBc.hbheRecHits = hbheRecHits_;

  cfgBc.eThresHB = iConfig.getParameter<EgammaHcalIsolation::arrayHB>("recHitEThresholdHB");
  cfgBc.maxSeverityHB = iConfig.getParameter<int>("maxHcalRecHitSeverity");
  cfgBc.eThresHE = iConfig.getParameter<EgammaHcalIsolation::arrayHE>("recHitEThresholdHE");
  cfgBc.maxSeverityHE = cfgBc.maxSeverityHB;

  hcalHelperCone_ = std::make_unique<ElectronHcalHelper>(cfgCone, consumesCollector());
  hcalHelperBc_ = std::make_unique<ElectronHcalHelper>(cfgBc, consumesCollector());

  hcalRun2EffDepth_ = iConfig.getParameter<bool>("hcalRun2EffDepth");

  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("minSCEtBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("maxHoverEBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("ecalRecHitSumEtOffsetBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("ecalRecHitSumEtSlopeBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("hcalRecHitSumEtOffsetBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("hcalRecHitSumEtSlopeBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("nTrackSolidConeBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("nTrackHollowConeBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("trackPtSumSolidConeBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("trackPtSumHollowConeBarrel"));
  preselCutValuesBarrel_.push_back(iConfig.getParameter<double>("sigmaIetaIetaCutBarrel"));
  //
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("minSCEtEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("maxHoverEEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("ecalRecHitSumEtOffsetEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("ecalRecHitSumEtSlopeEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("hcalRecHitSumEtOffsetEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("hcalRecHitSumEtSlopeEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("nTrackSolidConeEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("nTrackHollowConeEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("trackPtSumSolidConeEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("trackPtSumHollowConeEndcap"));
  preselCutValuesEndcap_.push_back(iConfig.getParameter<double>("sigmaIetaIetaCutEndcap"));
  produces<reco::PhotonCollection>(PhotonCollection_);


  
  //outputCollectionName_ = iConfig.getParameter<std::string>("outputCollectionName");
  //produces<reco::PhotonCollection>(outputCollectionName_);
}

BMMGCustomPhotonProducer::~BMMGCustomPhotonProducer() {
  // Do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
}
// ------------ method called to produce the data  ------------
void BMMGCustomPhotonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   HcalPFCuts const* hcalCuts = nullptr;
  if (cutsFromDB_) {
    hcalCuts = &iSetup.getData(hcalCutsToken_);
  }
  using namespace edm;
  //  nEvt_++;

  reco::PhotonCollection outputPhotonCollection;
  auto outputPhotonCollection_p = std::make_unique<reco::PhotonCollection>();

  // Get the PhotonCore collection
  bool validPhotonCoreHandle = true;
  Handle<reco::PhotonCoreCollection> photonCoreHandle;
  iEvent.getByToken(photonCoreProducer_, photonCoreHandle);
  if (!photonCoreHandle.isValid()) {
    edm::LogError("PhotonProducer") << "Error! Can't get the photonCoreProducer";
    validPhotonCoreHandle = false;
  }

  // Get EcalRecHits
  bool validEcalRecHits = true;
  Handle<EcalRecHitCollection> barrelHitHandle;
  EcalRecHitCollection barrelRecHits;
  iEvent.getByToken(barrelEcalHits_, barrelHitHandle);
  if (!barrelHitHandle.isValid()) {
    edm::LogError("PhotonProducer") << "Error! Can't get the barrelEcalHits";
    validEcalRecHits = false;
  }
  if (validEcalRecHits)
    barrelRecHits = *(barrelHitHandle.product());

  Handle<EcalRecHitCollection> endcapHitHandle;
  iEvent.getByToken(endcapEcalHits_, endcapHitHandle);
  EcalRecHitCollection endcapRecHits;
  if (!endcapHitHandle.isValid()) {
    edm::LogError("PhotonProducer") << "Error! Can't get the endcapEcalHits";
    validEcalRecHits = false;
  }
  if (validEcalRecHits)
    endcapRecHits = *(endcapHitHandle.product());

  const CaloTopology* topology = &iSetup.getData(topologyToken_);

  // prepare access to hcal data
  hcalHelperCone_->beginEvent(iEvent, iSetup);
  hcalHelperBc_->beginEvent(iEvent, iSetup);

  // Get the primary event vertex
  Handle<reco::VertexCollection> vertexHandle;
  reco::VertexCollection vertexCollection;
  bool validVertex = true;
  if (usePrimaryVertex_) {
    iEvent.getByToken(vertexProducer_, vertexHandle);
    if (!vertexHandle.isValid()) {
      edm::LogWarning("PhotonProducer") << "Error! Can't get the product primary Vertex Collection "
                                        << "\n";
      validVertex = false;
    }
    if (validVertex)
      vertexCollection = *(vertexHandle.product());
  }
  int iSC = 0;  // index in photon collection
  // Loop over barrel and endcap SC collections and fill the  photon collection
  if (validPhotonCoreHandle)
    fillPhotonVector(iEvent,
                         iSetup,
                         photonCoreHandle,
                         topology,
                         hcalCuts,
                         &barrelRecHits,
                         &endcapRecHits,
                         *hcalHelperCone_,
                         *hcalHelperBc_,
                         vertexCollection,
                         outputPhotonCollection,
                         iSC);

  // put the product in the event
  edm::LogInfo("PhotonProducer") << " Put in the event " << iSC << " Photon Candidates \n";
  outputPhotonCollection_p->assign(outputPhotonCollection.begin(), outputPhotonCollection.end());

  // go back to run2-like 2 effective depths if desired - depth 1 is the normal depth 1, depth 2 is the sum over the rest
  if (hcalRun2EffDepth_) {
    for (auto& pho : *outputPhotonCollection_p)
      pho.hcalToRun2EffDepth();
  }

  iEvent.put(std::move(outputPhotonCollection_p), PhotonCollection_);

}


void BMMGCustomPhotonProducer::fillPhotonVector(edm::Event& evt,
                                          edm::EventSetup const& es,
                                          const edm::Handle<reco::PhotonCoreCollection>& photonCoreHandle,
                                          const CaloTopology* topology,
                                          const HcalPFCuts* hcalCuts,
                                          const EcalRecHitCollection* ecalBarrelHits,
                                          const EcalRecHitCollection* ecalEndcapHits,
                                          ElectronHcalHelper const& hcalHelperCone,
                                          ElectronHcalHelper const& hcalHelperBc,
                                          reco::VertexCollection& vertexCollection,
                                          reco::PhotonCollection& outputPhotonCollection,
                                          int& iSC) {
  // get the geometry from the event setup:
  const CaloGeometry* geometry = &es.getData(caloGeomToken_);
  const CaloSubdetectorGeometry* subDetGeometry = nullptr;
  const CaloSubdetectorGeometry* geometryES = geometry->getSubdetectorGeometry(DetId::Ecal, EcalPreshower);
  const EcalRecHitCollection* hits = nullptr;
  std::vector<double> preselCutValues;
  float minR9 = 0;
  photonEnergyCorrector_.init(es);
  std::vector<int> flags_, severitiesexcl_;
  for (unsigned int lSC = 0; lSC < photonCoreHandle->size(); lSC++) {
    reco::PhotonCoreRef coreRef(reco::PhotonCoreRef(photonCoreHandle, lSC));
    reco::SuperClusterRef scRef = coreRef->superCluster();
    iSC++;

    int subdet = scRef->seed()->hitsAndFractions()[0].first.subdetId();
    subDetGeometry = geometry->getSubdetectorGeometry(DetId::Ecal, subdet);

    if (subdet == EcalBarrel) {
      preselCutValues = preselCutValuesBarrel_;
      minR9 = minR9Barrel_;
      hits = ecalBarrelHits;
      flags_ = flagsexclEB_;
      severitiesexcl_ = severitiesexclEB_;
    } else if (subdet == EcalEndcap) {
      preselCutValues = preselCutValuesEndcap_;
      minR9 = minR9Endcap_;
      hits = ecalEndcapHits;
      flags_ = flagsexclEE_;
      severitiesexcl_ = severitiesexclEE_;
    } else {
      edm::LogWarning("") << "PhotonProducer: do not know if it is a barrel or endcap SuperCluster";
    }
    if (hits == nullptr)
      continue;
    // SC energy preselection
    if (scRef->energy() / cosh(scRef->eta()) <= preselCutValues[0])
      continue;

    // recalculate position of seed BasicCluster taking shower depth for unconverted photon
    math::XYZPoint unconvPos =
        posCalculator_.Calculate_Location(scRef->seed()->hitsAndFractions(), hits, subDetGeometry, geometryES);

    float maxXtal = EcalClusterTools::eMax(*(scRef->seed()), &(*hits));
    //AA
    //Change these to consider severity level of hits
    float e1x5 = EcalClusterTools::e1x5(*(scRef->seed()), &(*hits), &(*topology));
    float e2x5 = EcalClusterTools::e2x5Max(*(scRef->seed()), &(*hits), &(*topology));
    float e3x3 = EcalClusterTools::e3x3(*(scRef->seed()), &(*hits), &(*topology));
    float e5x5 = EcalClusterTools::e5x5(*(scRef->seed()), &(*hits), &(*topology));
    const auto& cov = EcalClusterTools::covariances(*(scRef->seed()), &(*hits), &(*topology), geometry);
    const auto& locCov = EcalClusterTools::localCovariances(*(scRef->seed()), &(*hits), &(*topology));

    float sigmaEtaEta = sqrt(cov[0]);
    float sigmaIetaIeta = sqrt(locCov[0]);
    float r9 = e3x3 / (scRef->rawEnergy());
    float full5x5_maxXtal = noZS::EcalClusterTools::eMax(*(scRef->seed()), &(*hits));
    //AA
    //Change these to consider severity level of hits
    float full5x5_e1x5 = noZS::EcalClusterTools::e1x5(*(scRef->seed()), &(*hits), &(*topology));
    float full5x5_e2x5 = noZS::EcalClusterTools::e2x5Max(*(scRef->seed()), &(*hits), &(*topology));
    float full5x5_e3x3 = noZS::EcalClusterTools::e3x3(*(scRef->seed()), &(*hits), &(*topology));
    float full5x5_e5x5 = noZS::EcalClusterTools::e5x5(*(scRef->seed()), &(*hits), &(*topology));
    const auto& full5x5_cov = noZS::EcalClusterTools::covariances(*(scRef->seed()), &(*hits), &(*topology), geometry);
    const auto& full5x5_locCov = noZS::EcalClusterTools::localCovariances(*(scRef->seed()), &(*hits), &(*topology));

    float full5x5_sigmaEtaEta = sqrt(full5x5_cov[0]);
    float full5x5_sigmaIetaIeta = sqrt(full5x5_locCov[0]);

    // compute position of ECAL shower
    math::XYZPoint caloPosition;
    if (r9 > minR9) {
      caloPosition = unconvPos;
    } else {
      caloPosition = scRef->position();
    }
//// energy determination -- Default to create the candidate. Afterwards corrections are applied
    double photonEnergy = 1.;
    math::XYZPoint vtx(0., 0., 0.);
    if (!vertexCollection.empty())
      vtx = vertexCollection.begin()->position();
    // compute momentum vector of photon from primary vertex and cluster position
    math::XYZVector direction = caloPosition - vtx;
    math::XYZVector momentum = direction.unit();

    // Create dummy candidate with unit momentum and zero energy to allow setting of all variables. The energy is set for last.
    math::XYZTLorentzVectorD p4(momentum.x(), momentum.y(), momentum.z(), photonEnergy);
    reco::Photon newCandidate(p4, caloPosition, coreRef, vtx);

    // Calculate fiducial flags and isolation variable. Blocked are filled from the isolationCalculator
    reco::Photon::FiducialFlags fiducialFlags;
    reco::Photon::IsolationVariables isolVarR03, isolVarR04;
    photonIsolationCalculator_.calculate(&newCandidate, evt, es, fiducialFlags, isolVarR04, isolVarR03, hcalCuts);
    newCandidate.setFiducialVolumeFlags(fiducialFlags);
    newCandidate.setIsolationVariables(isolVarR04, isolVarR03);
    /// fill shower shape block
    reco::Photon::ShowerShape showerShape;
    showerShape.e1x5 = e1x5;
    showerShape.e2x5 = e2x5;
    showerShape.e3x3 = e3x3;
    showerShape.e5x5 = e5x5;
    showerShape.maxEnergyXtal = maxXtal;
    showerShape.sigmaEtaEta = sigmaEtaEta;
    showerShape.sigmaIetaIeta = sigmaIetaIeta;
    for (uint id = 0; id < showerShape.hcalOverEcal.size(); ++id) {
      showerShape.hcalOverEcal[id] = hcalHelperCone.hcalESum(*scRef, id + 1, hcalCuts) / scRef->energy();
      showerShape.hcalOverEcalBc[id] = hcalHelperBc.hcalESum(*scRef, id + 1, hcalCuts) / scRef->energy();
    }
    showerShape.hcalTowersBehindClusters = hcalHelperBc.hcalTowersBehindClusters(*scRef);
    showerShape.pre7DepthHcal = false;
    newCandidate.setShowerShapeVariables(showerShape);
    /// fill full5x5 shower shape block
    reco::Photon::ShowerShape full5x5_showerShape;
    full5x5_showerShape.e1x5 = full5x5_e1x5;
    full5x5_showerShape.e2x5 = full5x5_e2x5;
    full5x5_showerShape.e3x3 = full5x5_e3x3;
    full5x5_showerShape.e5x5 = full5x5_e5x5;
    full5x5_showerShape.maxEnergyXtal = full5x5_maxXtal;
    full5x5_showerShape.sigmaEtaEta = full5x5_sigmaEtaEta;
    full5x5_showerShape.sigmaIetaIeta = full5x5_sigmaIetaIeta;
    for (uint id = 0; id < full5x5_showerShape.hcalOverEcal.size(); ++id) {
      full5x5_showerShape.hcalOverEcal[id] = hcalHelperCone.hcalESum(*scRef, id + 1, hcalCuts) / full5x5_e5x5;
      full5x5_showerShape.hcalOverEcalBc[id] = hcalHelperBc.hcalESum(*scRef, id + 1, hcalCuts) / full5x5_e5x5;
    }
    full5x5_showerShape.hcalTowersBehindClusters = hcalHelperBc.hcalTowersBehindClusters(*scRef);
    full5x5_showerShape.pre7DepthHcal = false;
    newCandidate.full5x5_setShowerShapeVariables(full5x5_showerShape);
    /// get ecal photon specific corrected energy
    /// plus values from regressions     and store them in the Photon
    // Photon candidate takes by default (set in photons_cfi.py)  a 4-momentum derived from the ecal photon-specific corrections.
    photonEnergyCorrector_.calculate(evt, newCandidate, subdet, vertexCollection, es);
    if (candidateP4type_ == "fromEcalEnergy") {
      newCandidate.setP4(newCandidate.p4(reco::Photon::ecal_photons));
      newCandidate.setCandidateP4type(reco::Photon::ecal_photons);
    } else if (candidateP4type_ == "fromRegression") {
      newCandidate.setP4(newCandidate.p4(reco::Photon::regression1));
      newCandidate.setCandidateP4type(reco::Photon::regression1);
    }

    // fill MIP Vairables for Halo: Block for MIP are filled from PhotonMIPHaloTagger
    if (subdet == EcalBarrel && runMIPTagger_) {
      auto mipVar = photonMIPHaloTagger_.mipCalculate(newCandidate, evt, es);
      newCandidate.setMIPVariables(mipVar);
    }
    /// Pre-selection loose  isolation cuts
    bool isLooseEM = true;
    if (newCandidate.pt() < highEt_) {
      if (newCandidate.hadronicOverEm() >= preselCutValues[1])
        isLooseEM = false;
      if (newCandidate.ecalRecHitSumEtConeDR04() > preselCutValues[2] + preselCutValues[3] * newCandidate.pt())
        isLooseEM = false;
      if (newCandidate.hcalTowerSumEtConeDR04() > preselCutValues[4] + preselCutValues[5] * newCandidate.pt())
        isLooseEM = false;
      if (newCandidate.nTrkSolidConeDR04() > int(preselCutValues[6]))
        isLooseEM = false;
      if (newCandidate.nTrkHollowConeDR04() > int(preselCutValues[7]))
        isLooseEM = false;
      if (newCandidate.trkSumPtSolidConeDR04() > preselCutValues[8])
        isLooseEM = false;
      if (newCandidate.trkSumPtHollowConeDR04() > preselCutValues[9])
        isLooseEM = false;
      if (newCandidate.sigmaIetaIeta() > preselCutValues[10])
        isLooseEM = false;
    }

    if (isLooseEM)
      outputPhotonCollection.push_back(newCandidate);
  }
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
  // Input collections and basic settings
  desc.add<edm::InputTag>("photonCoreProducer", edm::InputTag("islandPhotonCore"));
  desc.add<bool>("regressionWeightsFromDB", true);
  desc.add<std::string>("energyRegressionWeightsFileLocation", std::string(""));
  desc.add<std::string>("energyRegressionWeightsDBLocation", std::string(""));
  desc.add<std::string>("superClusterEnergyCorrFunction", std::string("EcalClusterEnergyCorrection"));
  desc.add<std::string>("superClusterEnergyErrorFunction", std::string("EcalClusterEnergyUncertainty"));
  desc.add<std::string>("superClusterCrackEnergyCorrFunction", std::string("EcalClusterCrackCorrection"));
  desc.add<std::string>("photonEcalEnergyCorrFunction", std::string("EcalClusterEnergyCorrectionObjectSpecific"));
  desc.add<std::string>("candidateP4type", std::string("fromEcalEnergy"));

  
  // Vertex
  desc.add<bool>("usePrimaryVertex", true);
  desc.add<edm::InputTag>("primaryVertexProducer", edm::InputTag("offlinePrimaryVerticesWithBS"));

  // Position calc parameters
  edm::ParameterSetDescription posCalcP; 
  posCalcP.add<double>("T0_barl", 7.4);
  posCalcP.add<double>("T0_endc", 6.3);
  posCalcP.add<double>("T0_endcPresh", 3.6);
  posCalcP.add<bool>("LogWeighted", true);
  posCalcP.add<double>("W0", 4.2);
  posCalcP.add<double>("X0", 0.89);
  desc.add<edm::ParameterSetDescription>("posCalcParameters", posCalcP);

  // Ecal and Hcal inputs
  desc.add<edm::InputTag>("barrelEcalHits", edm::InputTag("ecalRecHit","EcalRecHitsEB"));
  desc.add<edm::InputTag>("endcapEcalHits", edm::InputTag("ecalRecHit","EcalRecHitsEE"));
 // desc.add<edm::InputTag>("hbheRecHits", egammaHBHERecHit.hbheRecHits);
 // desc.add<EgammaHcalIsolation::arrayHB>("recHitEThresholdHB", egammaHBHERecHit.recHitEThresholdHB);
  //desc.add<EgammaHcalIsolation::arrayHE>("recHitEThresholdHE", egammaHBHERecHit.recHitEThresholdHE);
  //desc.add<bool>("usePFThresholdsFromDB", egammaHBHERecHit.usePFThresholdsFromDB);
  //desc.add<int>("maxHcalRecHitSeverity", egammaHBHERecHit.maxHcalRecHitSeverity);
  desc.add<bool>("hcalRun2EffDepth", false);

  // Halo and MIP tagging
  desc.add<bool>("runMIPTagger", true);
  desc.add<bool>("runMVABasedHaloTagger", false);

  // Preselection and isolation
  desc.add<double>("highEt", 100.0);
  desc.add<double>("minR9Barrel", 10.0);
  desc.add<double>("minR9Endcap", 10.0);

  // SC preselection values
  desc.add<double>("minSCEtBarrel", 5.0);
  desc.add<double>("minSCEtEndcap", 15.0);
  desc.add<double>("maxHoverEBarrel", 0.99);
  desc.add<double>("maxHoverEEndcap", 0.50);
  desc.add<double>("ecalRecHitSumEtOffsetBarrel", 1e9);
  desc.add<double>("ecalRecHitSumEtSlopeBarrel", 0.0);
  desc.add<double>("ecalRecHitSumEtOffsetEndcap", 1e9);
  desc.add<double>("ecalRecHitSumEtSlopeEndcap", 0.0);
  desc.add<double>("hcalRecHitSumEtOffsetBarrel", 1e9);
  desc.add<double>("hcalRecHitSumEtSlopeBarrel", 0.0);
  desc.add<double>("hcalRecHitSumEtOffsetEndcap", 1e9);
  desc.add<double>("hcalRecHitSumEtSlopeEndcap", 0.0);
  desc.add<double>("nTrackSolidConeBarrel", 1e9);
  desc.add<double>("nTrackHollowConeBarrel", 1e9);
  desc.add<double>("nTrackSolidConeEndcap", 1e9);
  desc.add<double>("nTrackHollowConeEndcap", 1e9);
  desc.add<double>("trackPtSumSolidConeBarrel", 1e9);
  desc.add<double>("trackPtSumHollowConeBarrel", 1e9);
  desc.add<double>("trackPtSumSolidConeEndcap", 1e9);
  desc.add<double>("trackPtSumHollowConeEndcap", 1e9);
  desc.add<double>("sigmaIetaIetaCutBarrel", 1e9);
  desc.add<double>("sigmaIetaIetaCutEndcap", 1e9);

  // RecHit exclusions
  desc.add<std::vector<std::string>>("RecHitFlagToBeExcludedEB", std::vector<std::string>());
  desc.add<std::vector<std::string>>("RecHitSeverityToBeExcludedEB", std::vector<std::string>());
  desc.add<std::vector<std::string>>("RecHitFlagToBeExcludedEE", std::vector<std::string>());
  desc.add<std::vector<std::string>>("RecHitSeverityToBeExcludedEE", std::vector<std::string>());

  descriptions.add("BMMGCustomPhotonProducer", desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(BMMGCustomPhotonProducer);
