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

'''
dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV/BsToMuMuGamma_CMSSW_12_4_11_patch3_14_12_2024/241214_121515/0000/'

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

process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring("file:/eos/user/s/sslawins/condor_output/BsToMuMuGamma_filtered.root") )
# process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring(files) )
'''

process.source = cms.Source('PoolSource', fileNames = cms.untracked.vstring("file:/eos/user/p/psajdak/data/BsToMuMuGamma_filtered.root"))
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
# process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_data', '')
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL','')


process.analiza= cms.EDAnalyzer("MMGVertexFitExample",
  outHist = cms.string('histosMMGVertexFitExample.root'),
)

# process.out = cms.OutputModule("PoolOutputModule", fileName = cms.untracked.string("test2.root"))

process.MyPath = cms.Path(process.analiza)
#process.schedule = cms.Schedule(process.MyPath)

#process.outpath = cms.EndPath(process.out)
print("All files set for analysis.")