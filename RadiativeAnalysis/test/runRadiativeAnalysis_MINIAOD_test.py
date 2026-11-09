import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

# cmsRun runRadiativeAnalysis_MINIAOD_test.py maxEvents=100 outputFile=test_MINIAOD.root
options = VarParsing("analysis")
options.maxEvents  = -1
options.outputFile = "BsToMMG_DATA_MINIAOD_test.root"
options.inputFiles = ['root://xrootd-cms.infn.it//store/data/Run2023D/ParkingDoubleMuonLowMass0/MINIAOD/22Sep2023_v1-v1/2550000/0419eec5-0ae4-4732-8f06-6d72dd25a149.root']
options.parseArguments()

process = cms.Process("Analysis")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
# failed Kalman updates are reported once per fit iteration (up to 100 lines per event) -> keep grid logs small
process.MessageLogger.cerr.KalmanVertexUpdator = cms.untracked.PSet(limit = cms.untracked.int32(10))

# ES products needed by the analyzer: B field, transient tracks, calo geometry/topology + ECAL conditions (EcalClusterLazyTools)
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.Geometry.GeometryRecoDB_cff")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:run3_data", "")

# Run 3 GTs carry no CASTOR geometry; build CaloGeometry only from the subdetectors we need
process.CaloGeometryBuilder = cms.ESProducer("CaloGeometryBuilder",
    SelectedCalos = cms.vstring("HCAL", "EcalBarrel", "EcalEndcap")
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(options.inputFiles))

# photon MVA values (Run3 Winter22) on MINIAOD photons; fixedGridRhoAll/fixedGridRhoFastjetAll are already in MINIAOD
from RecoEgamma.PhotonIdentification.PhotonMVAValueMapProducer_cfi import photonMVAValueMapProducer
from RecoEgamma.PhotonIdentification.Identification.mvaPhotonID_Winter22_122X_V1_cff import mvaPhoID_RunIIIWinter22_v1_producer_config
process.photonMVAValueMapProducer = photonMVAValueMapProducer.clone(
    src               = cms.InputTag("slimmedPhotons"),
    mvaConfigurations = cms.VPSet(mvaPhoID_RunIIIWinter22_v1_producer_config),
)

process.Analysis = cms.EDAnalyzer("RadiativeAnalysis",
    isMCstudy               = cms.bool(False),
    isMINIAOD               = cms.bool(True),
    # required by the analyzer, but only read when isMCstudy = True
    genParticlesLabel       = cms.InputTag("prunedGenParticles"),
    PUInfo                  = cms.InputTag("slimmedAddPileupInfo"),
    MuonTag                 = cms.InputTag("slimmedMuons"),
    PhotonTag               = cms.InputTag("slimmedPhotons"),
    vertexBeamSpot          = cms.InputTag("offlineBeamSpot"),
    primaryvertex           = cms.InputTag("offlineSlimmedPrimaryVertices"),
    triggerbits             = cms.InputTag("TriggerResults", "", "HLT"),
    TrackTag                = cms.InputTag("packedPFCandidates"),
    LostTrackTag            = cms.InputTag("lostTracks"),
    ecalrechitEB            = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
    ecalrechitEE            = cms.InputTag("reducedEgamma", "reducedEERecHits"),
    mvaValuesMap           = cms.InputTag("photonMVAValueMapProducer", "PhotonMVAEstimatorRunIIIWinter22v1Values"),
    # muon MVA (MuonMVAID)
    xgboost_models          = cms.vstring("Run2022-20231030-1731-Event0"),
    xgboost_variable_names  = cms.vstring("run3muonmva"),
    outputFile              = cms.untracked.string(options.outputFile),
)

process.path = cms.Path(process.photonMVAValueMapProducer * process.Analysis)
