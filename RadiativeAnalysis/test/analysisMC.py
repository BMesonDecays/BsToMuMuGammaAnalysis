#!/cvmfs/cms.cern.ch/el8_amd64_gcc12/cms/cmssw/CMSSW_14_1_1/external/el8_amd64_gcc12/bin/python3

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


process.source = cms.Source('PoolSource', fileNames = cms.untracked.vstring(
    "/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToJpsiGamma_MCTunesRun3ECM13p6TeV/BsToJpsiGamma_CMSSW_12_4_11_patch3_19_01_2025/250119_174005/0000/private_BsToJpsiGamma_Run3_137.root"))
process.source.skipEvents = cms.untracked.uint32(0)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100))

process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL','')


process.analiza= cms.EDAnalyzer("CompCheckJpsiMassG_MC",
  outHist = cms.string('BsToJpsiGammaMC.root'),
  xgboost_models = cms.vstring(),
  xgboost_variable_names = cms.vstring(),
)

xgboost_models = [('Run2022-20231030-1731-Event0', 'run3muonmva'),]
for entry in xgboost_models:
  process.analiza.xgboost_models.append(entry[0]),
  process.analiza.xgboost_variable_names.append(entry[1])


process.MyPath = cms.Path(process.analiza)

print("All files set for analysis.")