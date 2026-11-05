# Driver configuration of the RadiativeAnalysis ntuplizer (B -> mu mu gamma), AOD or MiniAOD input.
#
# Examples:
#   cmsRun runNtuplizer_cfg.py                                   # MC AOD test file, all events
#   cmsRun runNtuplizer_cfg.py testSample=3 maxEvents=1000       # data AOD test file (isMC from the sample name)
#   cmsRun runNtuplizer_cfg.py testSample=1 maxEvents=1000       # MC MiniAOD test file (format from the sample name)
#   cmsRun runNtuplizer_cfg.py inputFiles=file:a.root isMC=0 lumiMask=Cert.json outputFile=out.root
#
# Options:
#   inputFiles   input files; if empty, sample "testSample" of test/test_files.txt is used
#   testSample   index of the sample in test/test_files.txt (default 0, the MC AOD test file)
#   testMaxFiles maximal number of files taken from the test sample (-1: all)
#   isMC         1 / 0; default (-1): data if the test sample name starts with "data", MC otherwise
#   dataFormat   AOD / MINIAOD; default: MINIAOD if the test sample name contains "miniaod", AOD otherwise
#   globalTag    default auto:run3_mc_FULL (MC) / auto:run3_data_prompt (data)
#   lumiMask     JSON file with the certified luminosity sections (data)
#   photonID     1 (default): run the photon MVA / cut-based ID; 0: skip it (pho_mva = -99, pho_tightId = -1),
#                e.g. for private samples without fixedGridRhoFastjetAll
#   maxEvents, outputFile (standard VarParsing options)

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('testSample', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'index of the sample in test/test_files.txt (used if inputFiles is empty)')
options.register('testMaxFiles', -1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'maximal number of files taken from the test sample')
options.register('isMC', -1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 '1: MC, 0: data, -1: from the test sample name')
options.register('dataFormat', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 "'AOD' or 'MINIAOD' (default: from the test sample name)")
options.register('globalTag', '', VarParsing.multiplicity.singleton, VarParsing.varType.string, 'global tag')
options.register('lumiMask', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 'JSON file with the certified luminosity sections')
options.register('photonID', 1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'run the photon ID (VID); needs fixedGridRhoFastjetAll, missing in some private samples')
options.register('channels', 'mmg,mmcg,mmgg,mmkk', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 'comma-separated list of the channels to build: mmg, mmcg, mmgg, mmkk')
options.register('trackCompatibility', 1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'compute the max track compatibility of the dimuon (slow-ish)')
options.register('verbose', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'print the trigger filters used for the muon matching')
options.setDefault('outputFile', 'bmmg_ntuple.root')
options._tagOrder = []   # no automatic '_numEventN' suffix in the output file name
options.parseArguments()

# ---------------------------------------------------------------- input
from BsToMuMuGammaAnalysis.RadiativeAnalysis.testFiles import testSamples, testFiles
if options.inputFiles:
    inputFiles = options.inputFiles
    sampleName = ''
else:
    inputFiles = testFiles(options.testSample, maxFiles=options.testMaxFiles)
    sampleName = testSamples()[options.testSample][0]
isMC = bool(options.isMC) if options.isMC >= 0 else not sampleName.lower().startswith('data')
dataFormat = options.dataFormat.upper() or ('MINIAOD' if 'miniaod' in sampleName.lower() else 'AOD')
if dataFormat not in ('AOD', 'MINIAOD'):
    raise ValueError('dataFormat must be AOD or MINIAOD, got ' + options.dataFormat)
print('RadiativeAnalysis: %d input file(s), %s, %s' % (len(inputFiles), 'MC' if isMC else 'data', dataFormat))

# collection names per input format
collections = {
    'AOD': dict(
        muons='muons', photons='gedPhotons', primaryVertices='offlinePrimaryVertices',
        ebRecHits='reducedEcalRecHitsEB', eeRecHits='reducedEcalRecHitsEE',
        conversions='allConversions', genParticles='genParticles', pileup='addPileupInfo',
    ),
    'MINIAOD': dict(
        muons='slimmedMuons', photons='slimmedPhotons', primaryVertices='offlineSlimmedPrimaryVertices',
        ebRecHits='reducedEgamma:reducedEBRecHits', eeRecHits='reducedEgamma:reducedEERecHits',
        conversions='reducedEgamma:reducedConversions', genParticles='prunedGenParticles',
        pileup='slimmedAddPileupInfo',
    ),
}[dataFormat]

from Configuration.Eras.Era_Run3_cff import Run3
process = cms.Process('BMMG', Run3)

process.source = cms.Source('PoolSource', fileNames=cms.untracked.vstring(inputFiles))
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
if options.lumiMask:
    import FWCore.PythonUtilities.LumiList as LumiList
    process.source.lumisToProcess = LumiList.LumiList(filename=options.lumiMask).getVLuminosityBlockRange()

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
# failed auxiliary vertex fits (track compatibility, Kalman fits) are expected and not worth a message each
process.MessageLogger.cerr.KalmanVertexUpdator = cms.untracked.PSet(limit=cms.untracked.int32(0))
process.options = cms.untracked.PSet(wantSummary=cms.untracked.bool(False))

# ---------------------------------------------------------------- conditions
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
globalTag = options.globalTag or ('auto:run3_mc_FULL' if isMC else 'auto:run3_data_prompt')
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')

# ---------------------------------------------------------------- photon ID (VID on gedPhotons / slimmedPhotons)
from PhysicsTools.SelectorUtils.tools.vid_id_tools import switchOnVIDPhotonIdProducer, setupAllVIDIdsInModule, \
    setupVIDPhotonSelection, DataFormat
switchOnVIDPhotonIdProducer(process, DataFormat.MiniAOD if dataFormat == 'MINIAOD' else DataFormat.AOD)
setupAllVIDIdsInModule(process, 'RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_RunIIIWinter22_122X_V1_cff',
                       setupVIDPhotonSelection)
process.photonMVAValueMapProducer.src = cms.InputTag(collections['photons'])
process.egmPhotonIDs.physicsObjectSrc = cms.InputTag(collections['photons'])

# ---------------------------------------------------------------- ntuplizer
from BsToMuMuGammaAnalysis.RadiativeAnalysis.radiativeAnalysis_cfi import radiativeAnalysis
channels = options.channels.split(',')
if dataFormat == 'MINIAOD' and 'mmcg' in channels:
    # MiniAOD conversions (reducedEgamma) point to tracks that are not stored: no e+ e- refit possible
    print('RadiativeAnalysis: the conversion channel (mmcg) is not available for MiniAOD and is switched off')
    channels.remove('mmcg')
process.bmmg = radiativeAnalysis.clone(
    isMC=isMC,
    verbose=bool(options.verbose),
    dataFormat=dataFormat,
    muons=collections['muons'],
    photons=collections['photons'],
    primaryVertices=collections['primaryVertices'],
    ebRecHits=collections['ebRecHits'],
    eeRecHits=collections['eeRecHits'],
    conversions=collections['conversions'],
    genParticles=collections['genParticles'],
    pileup=collections['pileup'],
    doMuMuGamma='mmg' in channels,
    doMuMuConvGamma='mmcg' in channels,
    doMuMuGammaGamma='mmgg' in channels,
    doMuMuKK='mmkk' in channels,
    computeTrackCompatibility=bool(options.trackCompatibility),
)
process.TFileService = cms.Service('TFileService', fileName=cms.string(options.outputFile))

if options.photonID:
    process.p = cms.Path(process.egmPhotonIDSequence * process.bmmg)
else:
    process.p = cms.Path(process.bmmg)
