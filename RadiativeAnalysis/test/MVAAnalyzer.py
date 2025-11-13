#!/cvmfs/cms.cern.ch/el8_amd64_gcc12/cms/cmssw/CMSSW_14_1_1/external/el8_amd64_gcc12/bin/python3

import FWCore.ParameterSet.Config as cms

import os
import sys
import subprocess


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

dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV_09_07_2025/BsToMuMuGamma_CMSSW_12_4_11_patch3_09_07_2025/250709_075534/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_Pi0ToGammaGamma_Pi0PythiaGun/Pi0ToGammaGamma_CMSSW_12_4_11_patch3_12_12_2024/241212_131944/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToMuMuGamma_MCTunesRun3ECM13p6TeV/BdToMuMuGamma_CMSSW_12_4_11_patch3_06_12_2024/241221_163854/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToKStarGamma_MCTunesRun3ECM13p6TeV/BdToKStarGamma_CMSSW_12_4_11_patch3_23_12_2024/241223_125459/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToPhiGamma_MCTunesRun3ECM13p6TeV/BsToPhiGamma_CMSSW_12_4_11_patch3_06_12_2024/241206_105826/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToKStarGamma_MCTunesRun3ECM13p6TeV/BsToKStarGamma_CMSSW_12_4_11_patch3_02_01_2024/250110_222730/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToKStarGamma_MCTunesRun3ECM13p6TeV/BdToKStarGamma_CMSSW_12_4_11_patch3_23_12_2024/250113_144624/0000/'

# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToMuMuGamma_MCTunesRun3ECM13p6TeV/BdToMuMuGamma_CMSSW_12_4_11_patch3_15_01_2024/250115_124459/0000/'

lsCommand = 'ls -1 ' + dataDir + '| grep root'
#print('Command: ', lsCommand)

dir = subprocess.Popen(lsCommand, stdout=subprocess.PIPE, shell=True, text=True)
lsOutput = dir.communicate()[0]

files = []
for f in lsOutput.split():
    #print(dataDir + f)
    files.append('file:' + dataDir + f)  # Full path to the files with 'file:' prefix

print('Number of files: ', len(files))

# input files (up to 255 files accepted)

# process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring("file:") )
process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring(files) )
process.source.skipEvents = cms.untracked.uint32(0)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10000))

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


process.analiza= cms.EDAnalyzer("MVAAnalyzer",
  outHist = cms.string('MVAAnalysis.root'),
  xgboost_models = cms.vstring(),
  xgboost_variable_names = cms.vstring(),
)

xgboost_models = [
    ('Run2022-20231030-1731-Event0', 'run3muonmva'),
]
for entry in xgboost_models:
    process.analiza.xgboost_models.append(entry[0]),
    process.analiza.xgboost_variable_names.append(entry[1])

# process.out = cms.OutputModule("PoolOutputModule", fileName = cms.untracked.string("test2.root"))

process.MyPath = cms.Path(process.analiza)
#process.schedule = cms.Schedule(process.MyPath)

#process.outpath = cms.EndPath(process.out)
print("All files set for analysis.")