import FWCore.ParameterSet.Config as cms

process = cms.Process("XSEC")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

process.source = cms.Source(
    "PoolSource",
    fileNames = cms.untracked.vstring(
        'root://cms-xrd-global.cern.ch//store/mc/Run3Summer22DRPremix/BsTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/AODSIM/124X_mcRun3_2022_realistic_v12-v2/100000/24ac371a-2c39-4db3-b046-c91b49c12819.root',
        'root://cms-xrd-global.cern.ch//store/mc/Run3Summer22DRPremix/BsTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/AODSIM/124X_mcRun3_2022_realistic_v12-v2/100000/1576f693-e6cc-486f-88b5-55426e1151b9.root'
    )
)

process.load("GeneratorInterface.Core.genXSecAnalyzer_cfi")

process.p = cms.Path(process.genXSecAnalyzer)
