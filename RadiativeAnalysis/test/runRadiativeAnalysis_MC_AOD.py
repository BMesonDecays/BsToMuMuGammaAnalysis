import sys
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.Eras.Era_Run3_cff import Run3

# Private Run3 MC (RECO/AOD content, everything in process "HLT"), e.g. JpsiGammaData/*FileList.txt
# interactive: cmsRun runRadiativeAnalysis_MC_AOD.py maxEvents=100 outputFile=test_MC.root inputFiles=/store/...
# submit.py:   cmsRun runRadiativeAnalysis_MC_AOD.py <jobId> "'file1' 'file2' ..."
options = VarParsing("analysis")
options.maxEvents  = -1
options.outputFile = "BsToMMG_MC_AOD.root"
options.inputFiles = ['/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToJpsiGamma_MCTunesRun3ECM13p6TeV/BsToJpsiGamma_CMSSW_12_4_11_patch3_19_01_2025/250119_174005/0000/private_BsToJpsiGamma_Run3_10.root']

# submit.py passes positional arguments (jobId, quoted file list) which VarParsing cannot parse
pyIdx   = next(i for i, a in enumerate(sys.argv) if a.endswith(".py"))
jobArgs = sys.argv[pyIdx + 1:]
if len(jobArgs) == 2 and "=" not in jobArgs[0]:
    jobId      = jobArgs[0]
    inputFiles = [f.strip("'") for f in jobArgs[1].split()]
    outputFile = "simpleAnalysis_jobs/RadiativeAnalysisMC" + jobId + ".root"
    maxEvents  = -1
    print("jobId:", jobId, "files:", inputFiles)
else:
    options.parseArguments()
    inputFiles = options.inputFiles
    outputFile = options.outputFile
    maxEvents  = options.maxEvents

process = cms.Process("Analysis", Run3)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
# silence the overabundant warnings of "predictedState error matrix inversion failed. An invalid vertex will be returned"
process.MessageLogger.KalmanVertexUpdator = cms.untracked.PSet(limit = cms.untracked.int32(0))

# ES products needed by the analyzer: B field, transient tracks, calo geometry/topology + ECAL conditions (EcalClusterLazyTools)
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.Geometry.GeometryRecoDB_cff")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:run3_mc_FULL", "")

# Run 3 GTs carry no CASTOR geometry; build CaloGeometry only from the subdetectors we need
process.CaloGeometryBuilder = cms.ESProducer("CaloGeometryBuilder",
    SelectedCalos = cms.vstring("HCAL", "EcalBarrel", "EcalEndcap")
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(maxEvents))
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(inputFiles))

# private MC has no rho in the event content -> recompute from particleFlow.
# PhotonMVAValueMapProducer (MVAVariableHelper) always reads both rho flavours, whichever MVA is configured
from RecoJets.JetProducers.fixedGridRhoProducerFastjet_cfi import fixedGridRhoFastjetAll
from RecoJets.JetProducers.fixedGridRhoProducer_cfi import fixedGridRhoAll
process.fixedGridRhoFastjetAll = fixedGridRhoFastjetAll.clone()
process.fixedGridRhoAll        = fixedGridRhoAll.clone()

# photon MVA values (Run3 Winter22) on AOD photons
from RecoEgamma.PhotonIdentification.PhotonMVAValueMapProducer_cfi import photonMVAValueMapProducer
from RecoEgamma.PhotonIdentification.Identification.mvaPhotonID_Winter22_122X_V1_cff import mvaPhoID_RunIIIWinter22_v1_producer_config
process.photonMVAValueMapProducer = photonMVAValueMapProducer.clone(
    src               = cms.InputTag("photons"),
    mvaConfigurations = cms.VPSet(mvaPhoID_RunIIIWinter22_v1_producer_config),
)

process.Analysis = cms.EDAnalyzer("RadiativeAnalysis",
    isMCstudy               = cms.bool(True),
    isMINIAOD               = cms.bool(False),
    genParticlesLabel       = cms.InputTag("genParticles"),
    MuonTag                 = cms.InputTag("muons"),
    PhotonTag               = cms.InputTag("photons"),
    PUInfo                  = cms.InputTag("addPileupInfo"),
    vertexBeamSpot          = cms.InputTag("offlineBeamSpot"),
    primaryvertex           = cms.InputTag("offlinePrimaryVertices"),
    triggerbits             = cms.InputTag("TriggerResults", "", "HLT"),
    TrackTag                = cms.InputTag("generalTracks"),
    ecalrechitEB            = cms.InputTag("reducedEcalRecHitsEB"),
    # private MC stores only EB rechits; EcalClusterLazyTools requires an EE collection,
    # so give it the EB one -> lookups for endcap DetIds find nothing (EE shower shapes are empty)
    ecalrechitEE            = cms.InputTag("reducedEcalRecHitsEB"),
    mvaValuesMap            = cms.InputTag("photonMVAValueMapProducer", "PhotonMVAEstimatorRunIIIWinter22v1Values"),
    # muon MVA (MuonMVAID)
    xgboost_models          = cms.vstring("Run2022-20231030-1731-Event0"),
    xgboost_variable_names  = cms.vstring("run3muonmva"),
    outputFile              = cms.untracked.string(outputFile),
)

process.path = cms.Path(process.fixedGridRhoFastjetAll * process.fixedGridRhoAll * process.photonMVAValueMapProducer * process.Analysis)
