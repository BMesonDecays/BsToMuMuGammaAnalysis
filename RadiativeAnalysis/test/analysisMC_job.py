import FWCore.ParameterSet.Config as cms
import os
import sys
import subprocess

from Configuration.Eras.Era_Run3_cff import Run3
process = cms.Process("analysis", Run3)

# MessageLogger & co.
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(1)
process.MessageLogger.suppressWarning  = cms.untracked.vstring('Geometry','AfterSource','L1T')
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))

jobId = sys.argv[1]
files2 = []
for f in sys.argv[2].split():
  files2.append(f.strip('\''))
print('files2:', files2)


# input files (up to 255 files accepted)
process.source = cms.Source('PoolSource',
  fileNames = cms.untracked.vstring(files2))
process.source.skipEvents = cms.untracked.uint32(0)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1))

process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL','')


process.analiza= cms.EDAnalyzer("JpsiGEndMarch_MC",
  outHist = cms.string('JpsiGEndMarch_jobs/BsToJpsiGammaMC'+jobId+'.root'),
  xgboost_models = cms.vstring(),
  xgboost_variable_names = cms.vstring(),
)

xgboost_models = [('Run2022-20231030-1731-Event0', 'run3muonmva'),]
for entry in xgboost_models:
  process.analiza.xgboost_models.append(entry[0]),
  process.analiza.xgboost_variable_names.append(entry[1])


process.MyPath = cms.Path(process.analiza)
process.schedule = cms.Schedule(process.MyPath)