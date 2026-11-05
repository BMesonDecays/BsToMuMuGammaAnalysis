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

# input data format: 'AOD' or 'MINIAOD'
dataFormat = 'AOD'

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

# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV_09_07_2025/BsToMuMuGamma_CMSSW_12_4_11_patch3_09_07_2025/250709_075534/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_Pi0ToGammaGamma_Pi0PythiaGun/Pi0ToGammaGamma_CMSSW_12_4_11_patch3_12_12_2024/241212_131944/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToMuMuGamma_MCTunesRun3ECM13p6TeV/BdToMuMuGamma_CMSSW_12_4_11_patch3_06_12_2024/241221_163854/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToKStarGamma_MCTunesRun3ECM13p6TeV/BdToKStarGamma_CMSSW_12_4_11_patch3_23_12_2024/241223_125459/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToPhiGamma_MCTunesRun3ECM13p6TeV/BsToPhiGamma_CMSSW_12_4_11_patch3_06_12_2024/241206_105826/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToKStarGamma_MCTunesRun3ECM13p6TeV/BsToKStarGamma_CMSSW_12_4_11_patch3_02_01_2024/250110_222730/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToKStarGamma_MCTunesRun3ECM13p6TeV/BdToKStarGamma_CMSSW_12_4_11_patch3_23_12_2024/250113_144624/0000/'

# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToMuMuGamma_MCTunesRun3ECM13p6TeV/BdToMuMuGamma_CMSSW_12_4_11_patch3_15_01_2024/250115_124459/0000/'

dataDir = '/eos/user/s/sslawins/filtered_BsToMuMuGamma_official/BsTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/BsToMuMuGamma_official/261102_143603/0000/'

lsCommand = 'ls -1 ' + dataDir + '| grep root'
#print('Command: ', lsCommand)

dir = subprocess.Popen(lsCommand, stdout=subprocess.PIPE, shell=True, text=True)
lsOutput = dir.communicate()[0]

files = []
for f in lsOutput.split():
    #print(dataDir + f)
    files.append('file:' + dataDir + f)  # Full path to the files with 'file:' prefix


# files = []
# with open('BsTo2MuG_files.txt', 'r') as f:
#     for line in f:
#         files.append('root://cms-xrd-global.cern.ch/' + line.strip())

# for f in files:
#     print(f)

print('Number of files: ', len(files))

# input files (up to 255 files accepted)

# process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring("file:/eos/user/s/sslawins/condor_output/BsToMuMuGamma_filtered.root") )
process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring(files) )
process.source.skipEvents = cms.untracked.uint32(0)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(20000))

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