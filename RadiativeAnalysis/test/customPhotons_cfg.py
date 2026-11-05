# Work in progress (plan.md Phase 9): BMMGCustomPhotonProducer on AOD, feeding the ntuplizer.
# The producer is a clone of the standard PhotonProducer; its configuration is the standard
# photons_cfi with AOD inputs and lower supercluster Et thresholds.
#   cmsRun customPhotons_cfg.py maxEvents=500 minSCEt=1.0
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('minSCEt', 1.0, VarParsing.multiplicity.singleton, VarParsing.varType.float,
                 'minimal supercluster Et of the custom photons [GeV] (standard photons: 10)')
options.register('testSample', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int, 'test_files.txt sample')
options.setDefault('outputFile', 'bmmg_customPhotons.root')
options._tagOrder = []
options.parseArguments()

from BsToMuMuGammaAnalysis.RadiativeAnalysis.testFiles import testFiles
from Configuration.Eras.Era_Run3_cff import Run3
process = cms.Process('CUSTOMPHO', Run3)
process.source = cms.Source('PoolSource', fileNames=cms.untracked.vstring(
    options.inputFiles if options.inputFiles else testFiles(options.testSample)))
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
process.MessageLogger.cerr.KalmanVertexUpdator = cms.untracked.PSet(limit=cms.untracked.int32(0))

process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')   # ES producers used by the photon algorithms
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL', '')

# custom photons: standard photon configuration, AOD inputs, low thresholds
from RecoEgamma.EgammaPhotonProducers.photons_cfi import photons
process.customPhotons = cms.EDProducer('BMMGCustomPhotonProducer', **photons.parameters_())
process.customPhotons.photonCoreProducer = cms.InputTag('photonCore')
process.customPhotons.barrelEcalHits = cms.InputTag('reducedEcalRecHitsEB')
process.customPhotons.endcapEcalHits = cms.InputTag('reducedEcalRecHitsEE')
process.customPhotons.hbheRecHits = cms.InputTag('reducedHcalRecHits', 'hbhereco')
process.customPhotons.primaryVertexProducer = cms.InputTag('offlinePrimaryVertices')
process.customPhotons.minSCEtBarrel = options.minSCEt
process.customPhotons.minSCEtEndcap = options.minSCEt
# RECO -> AOD input collections everywhere in the configuration (also in the nested PSets of the helpers)
aodInputs = {
    ('ecalRecHit', 'EcalRecHitsEB'): ('reducedEcalRecHitsEB', ''),
    ('ecalRecHit', 'EcalRecHitsEE'): ('reducedEcalRecHitsEE', ''),
    ('hbhereco', ''): ('reducedHcalRecHits', 'hbhereco'),
}
def toAOD(pset):
    for name in pset.parameterNames_():
        value = getattr(pset, name)
        if isinstance(value, cms.PSet):
            toAOD(value)
        elif isinstance(value, cms.InputTag):
            key = (value.getModuleLabel(), value.getProductInstanceLabel())
            if key in aodInputs:
                setattr(pset, name, cms.InputTag(*aodInputs[key]))
toAOD(process.customPhotons)

# ntuplizer on the custom photons (no photon ID: the VID is not set up for them)
from BsToMuMuGammaAnalysis.RadiativeAnalysis.radiativeAnalysis_cfi import radiativeAnalysis
process.bmmg = radiativeAnalysis.clone(isMC=True, photons='customPhotons', doMuMuConvGamma=False, doMuMuKK=False)
process.TFileService = cms.Service('TFileService', fileName=cms.string(options.outputFile))
process.p = cms.Path(process.customPhotons * process.bmmg)
