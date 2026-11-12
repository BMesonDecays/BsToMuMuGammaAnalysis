import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

# Official Run3 MC in MINIAODSIM, e.g.
#   /BsToJPsiG-JPsiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2/MINIAODSIM
# MC counterpart of runRadiativeAnalysis_MINIAOD.py (data): isMCstudy = True, MC global tag, no photon filter
# cmsRun runRadiativeAnalysis_MC_MINIAOD.py maxEvents=100 outputFile=test_MC_MINIAOD.root
options = VarParsing("analysis")
options.maxEvents  = -1
options.outputFile = "BsToMMG_MC_MINIAOD.root"
options.inputFiles = ['/store/mc/Run3Summer22EEMiniAODv4/BsToJPsiG-JPsiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/MINIAODSIM/130X_mcRun3_2022_realistic_postEE_v6-v2/2820000/0131d8dc-06a2-4144-975b-e3f5dede79c4.root']
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
# 2022 post-EE MC conditions of this release (sample produced with 130X_mcRun3_2022_realistic_postEE_v6)
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase1_2022_realistic_postEE", "")

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
    isMCstudy               = cms.bool(True),
    isMINIAOD               = cms.bool(True),
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
    mvaValuesMap            = cms.InputTag("photonMVAValueMapProducer", "PhotonMVAEstimatorRunIIIWinter22v1Values"),
    # muon MVA (MuonMVAID)
    xgboost_models          = cms.vstring("Run2022-20231030-1731-Event0"),
    xgboost_variable_names  = cms.vstring("run3muonmva"),
    outputFile              = cms.untracked.string(options.outputFile),
)

# MC: every event is analysed (no photon filter, unlike the data config)
process.path = cms.Path(process.photonMVAValueMapProducer * process.Analysis)
