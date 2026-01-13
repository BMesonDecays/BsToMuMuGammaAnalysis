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
    "/store/data/Run2024B/ParkingDoubleMuonLowMass0/MINIAOD/PromptReco-v1/000/378/981/00000/5c8cde04-e7ac-4c56-849a-85bd77ae9624.root"))
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
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_data', '')
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL','')


process.analiza= cms.EDAnalyzer("JpsiGammaMiniAOD",
  outHist = cms.string('BsToJpsiGammaData.root'),
)



process.MyPath = cms.Path(process.analiza)

print("All files set for analysis.")