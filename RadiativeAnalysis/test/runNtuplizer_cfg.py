# Driver configuration of the RadiativeAnalysis ntuplizer (B -> mu mu gamma), AOD or MiniAOD input.
#
# Examples:
#   cmsRun runNtuplizer_cfg.py                                   # MC AOD test file, all events
#   cmsRun runNtuplizer_cfg.py testSample=data_aod maxEvents=1000        # data AOD test file (isMC from the sample)
#   cmsRun runNtuplizer_cfg.py testSample=bsmmg_miniaod maxEvents=1000   # MC MiniAOD test file (format from the sample)
#   cmsRun runNtuplizer_cfg.py inputFiles=file:a.root isMC=0 lumiMask=Cert.json outputFile=out.root
#
# Options:
#   inputFiles   input files; if empty (and no dataset), the test sample "testSample" is used
#   dataset      dataset name, the files are found using the DAS client
#   testSample   name of the sample in test/test_files.yaml (default bsmmg_aod, the MC AOD test file);
#                python3 ../python/testFiles.py lists the samples
#   testMaxFiles maximal number of files taken from the test sample (-1: all)
#   isMC         1 / 0; default (-1): the isMC attribute of the test sample (MC for inputFiles / dataset)
#   dataFormat   AOD / MINIAOD; default: the dataFormat attribute of the test sample (AOD for inputFiles / dataset)
#   globalTag    default auto:run3_mc_FULL (MC) / auto:run3_data_prompt (data)
#   lumiMask     JSON file with the certified luminosity sections (data)
#   skim         1: write only the events with >= 1 B candidate (any enabled channel) whose fit succeeded and,
#                if skimMinVtxProb >= 0, whose vertex probability is > skimMinVtxProb (default 0: all events);
#                RunInfo nEvents counts all processed events, nEventsStored the written ones
#   photonID     1 (default): run the photon MVA ID (score and wp90); 0: skip it (pho_mva = -99, pho_tightId = -1),
#                e.g. for private samples without fixedGridRhoFastjetAll
#   convScaleMap converted-photon energy-scale map (JSON of analysis/scripts/conv_scale.py), applied to the mmcg
#                photon (p -> p / s; conv_scale stores s, conv_* and chic stay uncorrected): 'default' (the 2022
#                data map, data/conv_scale_2022.json, also for MC, as AN-2023/035), 'none', or a path (absolute
#                or relative to the CMSSW search path)
#   maxEvents, outputFile (standard VarParsing options)

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('testSample', 'bsmmg_aod', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 'name of the sample in test/test_files.yaml (used if inputFiles and dataset are empty)')
options.register('dataset', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 'dataset name, the files are found using the DAS client')
options.register('testMaxFiles', -1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'maximal number of files taken from the test sample')
options.register('isMC', -1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 '1: MC, 0: data, -1: from the test sample')
options.register('dataFormat', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 "'AOD' or 'MINIAOD' (default: from the test sample)")
options.register('globalTag', '', VarParsing.multiplicity.singleton, VarParsing.varType.string, 'global tag')
options.register('lumiMask', '', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 'JSON file with the certified luminosity sections')
options.register('photonID', 1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'run the photon ID (VID); needs fixedGridRhoFastjetAll, missing in some private samples')
options.register('channels', 'mmg,mmcg,mmgg,mmkk,chic', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 'comma-separated list of the channels to build: mmg, mmcg, mmgg, mmkk, chic (chi_c -> J/psi gamma(conv))')
options.register('trackCompatibility', 1, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'compute the max track compatibility of the dimuon (slow-ish)')
options.register('skim', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 '1: write only the events with >= 1 candidate whose fit succeeded (and vtxProb > skimMinVtxProb)')
options.register('skimMinVtxProb', -1., VarParsing.multiplicity.singleton, VarParsing.varType.float,
                 'vertex probability a candidate needs for the skim (< 0: any successful fit)')
options.register('convScaleMap', 'default', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 "converted-photon energy-scale map: 'default' (2022 data map), 'none', or a JSON file")
options.register('verbose', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int,
                 'print the trigger filters used for the muon matching')
options.setDefault('outputFile', 'bmmg_ntuple.root')
options._tagOrder = []   # no automatic '_numEventN' suffix in the output file name
options.parseArguments()

# ---------------------------------------------------------------- input
from BsToMuMuGammaAnalysis.RadiativeAnalysis.testFiles import testSample, testFiles, datasetFiles
sample = dict(isMC=True, dataFormat='AOD')   # defaults for inputFiles / dataset
if options.inputFiles:
    inputFiles = options.inputFiles
elif options.dataset:
    inputFiles = datasetFiles(options.dataset)
else:
    sample = testSample(options.testSample)
    inputFiles = testFiles(options.testSample, maxFiles=options.testMaxFiles)
isMC = bool(options.isMC) if options.isMC >= 0 else sample['isMC']
dataFormat = options.dataFormat.upper() or sample['dataFormat']
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
        conversions='oniaPhotonCandidates:conversions', genParticles='prunedGenParticles',
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

# ---------------------------------------------------------------- ntuplizer
from BsToMuMuGammaAnalysis.RadiativeAnalysis.radiativeAnalysis_cfi import radiativeAnalysis
channels = [c.strip() for c in options.channels.split(',') if c.strip()]
knownChannels = ('mmg', 'mmcg', 'mmgg', 'mmkk', 'chic')
unknownChannels = [c for c in channels if c not in knownChannels]
if unknownChannels or not channels:
    # an unknown name would silently disable the channel; with skim=1 that empties the output
    raise ValueError('channels=%s: unknown channel(s) %s; expected a comma-separated subset of %s'
                     % (options.channels, unknownChannels, ','.join(knownChannels)))
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
    doChiC='chic' in channels,
    computeTrackCompatibility=bool(options.trackCompatibility),
    onlyEventsWithCandidates=bool(options.skim),
    minCandidateVtxProb=options.skimMinVtxProb,
    conversionScaleMap={'default': 'BsToMuMuGammaAnalysis/RadiativeAnalysis/data/conv_scale_2022.json',
                        'none': ''}.get(options.convScaleMap, options.convScaleMap),
)
process.TFileService = cms.Service('TFileService', fileName=cms.string(options.outputFile))

# ---------------------------------------------------------------- photon ID (VID), on the photons of the ntuplizer
from PhysicsTools.SelectorUtils.tools.vid_id_tools import switchOnVIDPhotonIdProducer, setupAllVIDIdsInModule, \
    setupVIDPhotonSelection, DataFormat
switchOnVIDPhotonIdProducer(process, DataFormat.MiniAOD if dataFormat == 'MINIAOD' else DataFormat.AOD)
setupAllVIDIdsInModule(process, 'RecoEgamma.PhotonIdentification.Identification.mvaPhotonID_Winter22_122X_V1_cff',
                       setupVIDPhotonSelection)
# The value maps are keyed by the photon collection, so both producers must read the ntuplizer's photons.
# switchOnVIDPhotonIdProducer sets egmPhotonIDs to gedPhotons (AOD) / slimmedPhotons (MiniAOD), but
# photonMVAValueMapProducer always defaults to slimmedPhotons (an AOD job fails without the setting below).
process.egmPhotonIDs.physicsObjectSrc = process.bmmg.photons
process.photonMVAValueMapProducer.src = process.bmmg.photons

if options.photonID:
    process.p = cms.Path(process.egmPhotonIDSequence * process.bmmg)
else:
    process.p = cms.Path(process.bmmg)
