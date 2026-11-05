#!/cvmfs/cms.cern.ch/el8_amd64_gcc12/cms/cmssw/CMSSW_14_1_1/external/el8_amd64_gcc12/bin/python3

import FWCore.ParameterSet.Config as cms

import os
import sys
import subprocess
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('dataFormat', 'AOD', VarParsing.multiplicity.singleton, VarParsing.varType.string, "'AOD' or 'MINIAOD'")
options.register('testSample', -1, VarParsing.multiplicity.singleton, VarParsing.varType.int, 'index in test/test_files.txt (-1: AOD/MINIAOD test file)')
options.register('testMaxFiles', -1, VarParsing.multiplicity.singleton, VarParsing.varType.int, 'max number of files from the test sample')
options.setDefault('maxEvents', 20000)
options.setDefault('outputFile', 'histosMMGVertexFitExample.root')
options.parseArguments()


# process = cms.Process("MojaAnaliza")

from Configuration.Eras.Era_Run3_cff import Run3
process = cms.Process("EenTest", Run3)

# MessageLogger & co.
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(1)
process.MessageLogger.suppressWarning  = cms.untracked.vstring('Geometry','AfterSource','L1T')
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))
# process.MessageLogger.cerr.threshold = "DEBUG"
# process.MessageLogger.debugModules = ["*"]

# input data format: 'AOD' or 'MINIAOD'
dataFormat = options.dataFormat

# collection names for each data format
collections = {
    'AOD': dict(
        genParticles    = 'genParticles',
        muons           = 'muons',
        photons         = 'gedPhotons',
        ebRecHits       = 'reducedEcalRecHitsEB',
        eeRecHits       = 'reducedEcalRecHitsEE',
        beamSpot        = 'offlineBeamSpot',
        primaryVertices = 'offlinePrimaryVertices',
    ),
    'MINIAOD': dict(
        genParticles    = 'prunedGenParticles',
        muons           = 'slimmedMuons',
        photons         = 'slimmedPhotons',
        ebRecHits       = 'reducedEgamma:reducedEBRecHits',
        eeRecHits       = 'reducedEgamma:reducedEERecHits',
        beamSpot        = 'offlineBeamSpot',
        primaryVertices = 'offlineSlimmedPrimaryVertices',
    ),
}[dataFormat]

# input: explicit inputFiles=..., otherwise sample testSample from test/test_files.txt
# (default: the AOD or MINIAOD test file, depending on dataFormat)
from BsToMuMuGammaAnalysis.RadiativeAnalysis.testFiles import testFiles
if options.inputFiles:
    files = options.inputFiles
else:
    sample = options.testSample if options.testSample >= 0 else (0 if dataFormat == 'AOD' else 1)
    files = testFiles(sample, maxFiles=options.testMaxFiles)
print('Number of files: ', len(files))
process.source = cms.Source('PoolSource', fileNames = cms.untracked.vstring(files))
process.source.skipEvents = cms.untracked.uint32(0)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents))

process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")

process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
# process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_data', '')
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL','')


process.analiza= cms.EDAnalyzer("MMGVertexFitExample",
  outHist = cms.string(options.outputFile),
  # input collections, see dataFormat above
  genParticles    = cms.InputTag(collections['genParticles']),
  muons           = cms.InputTag(collections['muons']),
  photons         = cms.InputTag(collections['photons']),
  ebRecHits       = cms.InputTag(collections['ebRecHits']),
  eeRecHits       = cms.InputTag(collections['eeRecHits']),
  beamSpot        = cms.InputTag(collections['beamSpot']),
  primaryVertices = cms.InputTag(collections['primaryVertices']),
  # scale factors for the cluster position uncertainties (sigma) along e_phi and e_theta,
  # perpendicular to the photon, for barrel and endcap photons,
  # tuned so that the core width of the cluster pulls is 1 (gedPhotons)
  clusterCovScalePhiEB = cms.untracked.double(0.16),
  clusterCovScaleThetaEB = cms.untracked.double(0.165),
  clusterCovScalePhiEE = cms.untracked.double(0.20),
  clusterCovScaleThetaEE = cms.untracked.double(0.24),
  # scale factors for the photon energy uncertainty, for barrel and endcap photons,
  # from the core width of the photon energy pull in bins of E_gen (roughly flat, gedPhotons)
  energyErrorScaleEB = cms.untracked.double(1.2),
  energyErrorScaleEE = cms.untracked.double(1.3),
  # ECAL region of the matched photons used in the fit: 'EB', 'EE' or 'both'
  photonRegion = cms.untracked.string('both'),
)

# process.out = cms.OutputModule("PoolOutputModule", fileName = cms.untracked.string("test2.root"))

process.MyPath = cms.Path(process.analiza)
#process.schedule = cms.Schedule(process.MyPath)

#process.outpath = cms.EndPath(process.out)
print("All files set for analysis.")