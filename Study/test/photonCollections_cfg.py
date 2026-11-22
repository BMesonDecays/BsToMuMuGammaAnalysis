# Counts B_s -> mu mu gamma signal events with the trigger, two gen-matched muons and a gen-matched photon,
# for several photon collections (plugins/PhotonCollectionStudy.cc). One module per photon collection, all in
# the same job: the gen, trigger and muon steps are the same, only the photon collection differs.
#
#   cmsRun photonCollections_cfg.py sample=legacy maxEvents=20000           # AOD: photons, gedPhotons
#   cmsRun photonCollections_cfg.py sample=new_aod inputFiles=root://...     # AOD: photons, gedPhotons
#   cmsRun photonCollections_cfg.py sample=new_miniaod inputFiles=root://... # MiniAOD: slimmedPhotons
# Output: photonCollections_<sample>.root, a directory per module (pho_<collection>). The expected yields:
#   python3 photonCollections.py
import glob

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

LEGACY = ('/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/'
          'Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV_09_07_2025/BsToMuMuGamma_CMSSW_12_4_11_patch3_09_07_2025/'
          '250709_075534/0000/*.root')

options = VarParsing('analysis')
options.register('sample', 'legacy', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 'legacy, new_aod or new_miniaod')
options.register('nFiles', 100, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'legacy: number of files of the sample directory')
options.maxEvents = 20000
options.parseArguments()

miniAOD = options.sample == 'new_miniaod'
files = list(options.inputFiles)
if not files:
    if options.sample != 'legacy':
        raise ValueError('give inputFiles for ' + options.sample)
    files = ['file:' + f for f in sorted(glob.glob(LEGACY))[:options.nFiles]]

process = cms.Process('PHOSTUDY')
process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 5000
process.source = cms.Source('PoolSource', fileNames=cms.untracked.vstring(files))
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.TFileService = cms.Service('TFileService', fileName=cms.string('photonCollections_%s.root' % options.sample))

common = dict(
    triggerResults=cms.InputTag('TriggerResults', '', 'HLT'),
    triggerPath=cms.string('HLT_DoubleMu4_3_LowMass_v'),
    genParticles=cms.InputTag('prunedGenParticles' if miniAOD else 'genParticles'),
    muons=cms.InputTag('slimmedMuons' if miniAOD else 'muons'),
    muonMaxDR=cms.double(0.05),
    photonMaxDR=cms.double(0.1),
)
collections = ['slimmedPhotons'] if miniAOD else ['photons', 'gedPhotons']
process.p = cms.Path()
for c in collections:
    m = cms.EDAnalyzer('PhotonCollectionStudy', photons=cms.InputTag(c), **common)
    setattr(process, 'pho_' + c, m)
    process.p += m
