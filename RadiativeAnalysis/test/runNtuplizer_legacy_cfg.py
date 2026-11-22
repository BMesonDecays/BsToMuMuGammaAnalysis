# RadiativeAnalysis ntuplizer for legacy AOD samples without the endcap ECAL rechits, e.g. the private
# zero-bias, no-pileup B_s -> mu mu gamma MC of 2025-07-09 (CMSSW_12_4_11_patch3; test sample bsmmg_legacy_aod,
# no generator cut on the photon pt).
#
# It runs runNtuplizer_cfg.py (same options, same ntuple format) and then:
#   - removes the endcap reco photons (photonMaxAbsSCEta = 1.479, the barrel edge): their cluster
#     variables and the ECAL photon fits need the endcap rechits. mmg / mmgg have barrel photons only;
#   - gives the ntuplizer an empty endcap rechit collection (EmptyEcalRecHitCollectionProducer,
#     plugins/RadiativeAnalysis_legacy.cc), which is never looked up;
#   - produces fixedGridRhoAll / fixedGridRhoFastjetAll from particleFlow (not stored in these samples): the
#     photon MVA (VID, photonID=1) and the photon MVA without the muons use it.
# The converted photons (mmcg, chic) use only the tracker and work in the endcaps as usual.
#
# Examples:
#   cmsRun runNtuplizer_legacy_cfg.py maxEvents=1000                       # bsmmg_legacy_aod, all files
#   cmsRun runNtuplizer_legacy_cfg.py testMaxFiles=10 outputFile=legacy.root
#   cmsRun runNtuplizer_legacy_cfg.py inputFiles=file:a.root
# Options: as runNtuplizer_cfg.py; the default test sample is bsmmg_legacy_aod.

import os
import sys

import FWCore.ParameterSet.Config as cms

# default input: the legacy sample (unless the input is given)
if not any(a.startswith(('testSample=', 'inputFiles=', 'dataset=')) for a in sys.argv[2:]):
    sys.argv.append('testSample=bsmmg_legacy_aod')

_mainCfg = os.path.join(os.environ['CMSSW_BASE'], 'src', 'BsToMuMuGammaAnalysis', 'RadiativeAnalysis', 'test',
                        'runNtuplizer_cfg.py')
if not os.path.exists(_mainCfg):   # the package is taken from the release area
    _mainCfg = os.path.join(os.environ['CMSSW_RELEASE_BASE'], 'src', 'BsToMuMuGammaAnalysis', 'RadiativeAnalysis',
                            'test', 'runNtuplizer_cfg.py')
with open(_mainCfg) as _f:
    exec(compile(_f.read(), _mainCfg, 'exec'))

if dataFormat != 'AOD':
    raise ValueError('runNtuplizer_legacy_cfg.py is for AOD samples without endcap rechits, got ' + dataFormat)

# endcap photons off, empty endcap rechits
process.emptyEERecHits = cms.EDProducer('EmptyEcalRecHitCollectionProducer')
process.bmmg.eeRecHits = cms.InputTag('emptyEERecHits')
process.bmmg.photonMaxAbsSCEta = 1.479

# rho from the PF candidates (the producers keep their standard labels, as the photon ID expects)
from RecoJets.JetProducers.fixedGridRhoProducer_cfi import fixedGridRhoAll
from RecoJets.JetProducers.fixedGridRhoProducerFastjet_cfi import fixedGridRhoFastjetAll
process.fixedGridRhoAll = fixedGridRhoAll.clone()
process.fixedGridRhoFastjetAll = fixedGridRhoFastjetAll.clone()

process.legacyTask = cms.Task(process.emptyEERecHits, process.fixedGridRhoAll, process.fixedGridRhoFastjetAll)
process.p.associate(process.legacyTask)
print('RadiativeAnalysis legacy: endcap reco photons removed (|SC eta| < 1.479), empty endcap rechits, rho from PF')
