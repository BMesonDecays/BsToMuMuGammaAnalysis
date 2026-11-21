import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_cff import Run3
from Configuration.AlCa.GlobalTag import GlobalTag

process = cms.Process("CONVFIT", Run3)

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(
        "file:/eos/user/s/sslawins/testing_mc/BsTo2MuG_miniaod.root"
    )
)
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(10000))

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:run3_mc_FULL", "")

process.convFit = cms.EDAnalyzer(
    "ConvFitStudy",
    miniAOD=cms.bool(True),
    conversions=cms.InputTag("oniaPhotonCandidates:conversions"),
    genParticles=cms.InputTag("prunedGenParticles"),
    pfCandidates=cms.InputTag("packedPFCandidates"),
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string("convFit.root")
)
process.p = cms.Path(process.convFit)