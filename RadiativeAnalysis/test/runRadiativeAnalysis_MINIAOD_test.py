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
process.MessageLogger.cerr.FwkReport.reportEvery = 10

# ES products needed by the analyzer: B field, transient tracks, calo geometry, ECAL conditions, HLT config
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

# MINIAOD has no std::vector<reco::Track>; rebuild it from packedPFCandidates + lostTracks
process.load("PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi")

process.Analysis = cms.EDAnalyzer("RadiativeAnalysis",
    isMCstudy               = cms.bool(False),
    isMINIAOD               = cms.bool(True),
    genParticlesLabel       = cms.InputTag("prunedGenParticles"),
    MuonTag                 = cms.InputTag("slimmedMuons"),
    JetTag                  = cms.InputTag("slimmedJets"),
    PhotonTag               = cms.InputTag("slimmedPhotons"),
    OOTPhotonTag            = cms.InputTag("slimmedOOTPhotons"),
    ElectronTag             = cms.InputTag("slimmedElectrons"),
    PUInfo                  = cms.InputTag("slimmedAddPileupInfo"),
    vertexBeamSpot          = cms.InputTag("offlineBeamSpot"),
    primaryvertex           = cms.InputTag("offlineSlimmedPrimaryVertices"),
    triggerbits             = cms.InputTag("TriggerResults", "", "HLT"),
    prescale                = cms.InputTag("patTrigger"),
    triggerobj              = cms.InputTag("slimmedPatTrigger"),
    triggerSummary          = cms.InputTag("hltTriggerSummaryAOD", "", "HLT"),  # not in MINIAOD, unused in analyze()
    HLTprocess              = cms.untracked.string("HLT"),
    pfCandTag               = cms.InputTag("unpackedTracksAndVertices"),
    PFCandTag               = cms.InputTag("packedPFCandidates"),
    IsoTrackTag             = cms.InputTag("isolatedTracks"),
    pfSupcluster            = cms.InputTag("reducedEgamma", "reducedSuperClusters"),
    ecalrechitEB            = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
    ecalrechitEE            = cms.InputTag("reducedEgamma", "reducedEERecHits"),
    ecalrechitES            = cms.InputTag("reducedEgamma", "reducedESRecHits"),
    convertedPhotons        = cms.InputTag("oniaPhotonCandidates", "conversions"),
    # photon VID is not run here -> photon MVA scores are set to -99
    mvaValuesMap            = cms.InputTag("photonMVAValueMapProducer", "PhotonMVAEstimatorRunIIIWinter22v1Values"),
    phoTightIDMap           = cms.InputTag("egmPhotonIDs", "cutBasedPhotonID-RunIIIWinter22-122X-V1-tight"),
    xgboost_models          = cms.vstring("Run2022-20231030-1731-Event0"),
    xgboost_variable_names  = cms.vstring("run3muonmva"),
    muMVACut                = cms.double(0.83),
    photonMVACut            = cms.double(-0.07),
    StoreDeDxInfo           = cms.bool(True),
    PionZeroMassWindowNoFit = cms.double(0.05),
    EtaMesonMassWindowNoFit = cms.double(0.05),
    EtaPrimeMassWindowNoFit = cms.double(0.05),
    JpsiMassWindowBeforeFit = cms.double(0.310),
    JpsiMassWindowAfterFit  = cms.double(0.150),
    PsiMassWindowBeforeFit  = cms.double(0.293),
    PsiMassWindowAfterFit   = cms.double(0.028),
    PhiMassWindowBeforeFit  = cms.double(0.03),
    PhiMassWindowAfterFit   = cms.double(0.02),
    JpsiPtCut               = cms.double(7),
    KaonTrackPtCut          = cms.double(0.7),
    BsLowerMassCutBeforeFit = cms.double(4.5),
    BsUpperMassCutBeforeFit = cms.double(6.5),
    BdLowerMassCutBeforeFit = cms.double(4.2),
    BdUpperMassCutBeforeFit = cms.double(6.2),
    BsLowerMassCutAfterFit  = cms.double(4.5),
    BsUpperMassCutAfterFit  = cms.double(6.5),
    BsPDGMass               = cms.double(5.3699),
    BdPDGMass               = cms.double(5.2794),
    BpPDGMass               = cms.double(5.2790),
    PionZeroPDGMass         = cms.double(0.1349),
    EtaMesonPDGMass         = cms.double(0.5478),
    EtaPrimePDGMass         = cms.double(0.9577),
    verbose                 = cms.bool(False),
    TestVerbose             = cms.bool(False),
    outputFile              = cms.untracked.string(options.outputFile),
)

process.path = cms.Path(process.unpackedTracksAndVertices * process.Analysis)
