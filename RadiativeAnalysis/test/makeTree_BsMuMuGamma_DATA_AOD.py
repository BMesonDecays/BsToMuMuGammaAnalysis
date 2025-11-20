import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from pathlib import Path
import os
import random
import sys
import re
from os import listdir
from os.path import isfile, join
import glob

options = VarParsing("analysis")
options.register("nEvents", 5000, VarParsing.multiplicity.singleton, VarParsing.varType.int, "Number of events to process")
# options.register("outputFile", "default_output.root", VarParsing.multiplicity.singleton, VarParsing.varType.string, "Output file name")
options.parseArguments()

process = cms.Process("MUMUGamma")


process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_data_FULL','')
# process.GlobalTag = GlobalTag(process.GlobalTag, '124X_mcRun3_2022_realistic_v12','')
process.load("FWCore.MessageLogger.MessageLogger_cfi")
#process.load("Geometry.CaloEventSetup.CaloGeometry_cfi")
#process.load("Configuration.Geometry.GeometryECALHCAL_cff")
#process.load("Geometry.CaloEventSetup.CaloTopology_cfi")
#process.load("Geometry.CaloEventSetup.EcalTrigTowerConstituents_cfi")
#process.ZdcHardcodeGeometryEP = cms.ESPrefer("ZdcGeometryFromDBEP")
#process.es_prefer_HcalGeometry = cms.ESPrefer("HcalHardcodeGeometryEP", "")
#process.es_prefer_CastorGeometry = cms.ESPrefer("CastorGeometryFromDBEP", "")  # Add this
#process.es_prefer_CaloTowerGeometry = cms.ESPrefer("CaloTowerGeometryFromDBEP", "")
#process.es_prefer_EcalBarrelGeometry = cms.ESPrefer("EcalBarrelGeometryFromDBEP", "")


import HeavyFlavorAnalysis.Onia2MuMu.OniaPhotonConversionProducer_cfi
process.oniaPhotonCandidates = HeavyFlavorAnalysis.Onia2MuMu.OniaPhotonConversionProducer_cfi.PhotonCandidates.clone()
# process.oniaPhotonCandidates.conversions 
process.oniaPhotonCandidates.primaryVertexTag = cms.InputTag('offlinePrimaryVerticesWithBS')

from PhysicsTools.PatAlgos.tools.coreTools import *
process.load("PhysicsTools.PatAlgos.patSequences_cff")
from PhysicsTools.PatAlgos.tools.pfTools import *
print("events", options.nEvents)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.nEvents))
"""
if options.inputFiles:
    # If input files are provided through the argument
    process.source = cms.Source("PoolSource",
                                fileNames = cms.untracked.vstring(options.inputFiles)
                                )
else:
    # If no input files are provided, use the default list (for example)
    prefixPath = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV/BsToMuMuGamma_CMSSW_12_4_11_patch3_14_12_2024/241214_121515/0000'
    fileList = glob.glob(prefixPath + '/*.root')
    fileList = ['file:' + aFile for aFile in fileList]
    process.source = cms.Source("PoolSource",
                                fileNames = cms.untracked.vstring(fileList)
                                )



"""
# process.source = cms.Source("PoolSource",
#                             duplicateCheckMode = cms.untracked.string('noDuplicateCheck'),
#                             skipEvents = cms.untracked.uint32(0),
#                             fileNames = cms.untracked.vstring(
#          #'root://xrootd-cms.infn.it//store/data/Run2023D/ParkingDoubleMuonLowMass0/MINIAOD/22Sep2023_v1-v1/2550000/0419eec5-0ae4-4732-8f06-6d72dd25a149.root',
#         #  'root://cms-xrd-global.cern.ch//store/data/Run2022C/ParkingDoubleMuonLowMass0/AOD/10Dec2022-v2/2540000/019a2652-4483-484d-9892-0887e18413ec.root',
#         #  'root://cms-xrd-global.cern.ch//store/data/Run2022C/ParkingDoubleMuonLowMass0/AOD/10Dec2022-v2/2540000/01da8b6d-946e-4331-9ef5-bb12e6d9a94f.root',
#         #  'root://cms-xrd-global.cern.ch//store/data/Run2022C/ParkingDoubleMuonLowMass0/AOD/10Dec2022-v2/2540000/02cb4e7e-bf61-4038-8ecd-223583720404.root',
#         #  'root://cms-xrd-global.cern.ch//store/data/Run2022C/ParkingDoubleMuonLowMass0/AOD/10Dec2022-v2/2540000/0383f1aa-aa59-4e47-abb0-3d7391213bd8.root',
#         #  'root://cms-xrd-global.cern.ch//store/data/Run2022C/ParkingDoubleMuonLowMass0/AOD/10Dec2022-v2/2540000/056a1f3c-8c9e-4869-b3df-51f81cc517df.root',
#             'root://cms-xrd-global.cern.ch//store/data/Run2024I/MuonEG/AOD/PromptReco-v1/000/386/618/00000/0fcdac9d-3651-45e8-8cbb-94fa236c6c63.root'
# )
# )

process.options.numberOfThreads=cms.untracked.uint32(2)
process.options.numberOfStreams=cms.untracked.uint32(1)

process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring())


#--PatOverlap, mu/ele--#
process.load("PhysicsTools.PatAlgos.cleaningLayer1.genericTrackCleaner_cfi")
process.cleanPatTracks.checkOverlaps.muons.requireNoOverlaps     = cms.bool(False)
process.cleanPatTracks.checkOverlaps.electrons.requireNoOverlaps = cms.bool(False)
#--Pat Matching --#
#MUON MC-MATCHING VALUES FROM BsMuMu MUON-ID STUDIES
process.load("PhysicsTools.PatAlgos.mcMatchLayer0.muonMatch_cfi")
process.muonMatch.matched = cms.InputTag("prunedGenParticles")
process.muonMatch.maxDeltaR = cms.double(0.12)
process.muonMatch.maxDPtRel = cms.double(0.3)
process.muonMatch.checkCharge = cms.bool(True)
process.muonMatch.resolveAmbiguities = cms.bool(True)
process.muonMatch.resolveByMatchQuality = cms.bool(True)

"""
--Do we need electron exclusion? In case we deal with MINIAOD rechit collections : 
--in that case again it will going back to collecting the rec hits in ECAL and exclude the GSF tracks from the tracker to retain only photon signals 
--Not sure for the moment but this requires some attention 
edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> >    "reducedEgamma"             "reducedEBRecHits"   "PAT"
edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> >    "reducedEgamma"             "reducedEERecHits"   "PAT"
edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> >    "reducedEgamma"             "reducedESRecHits"   "PAT"
"""

#-- PAT MC MATCHING ele --#
#This twiki is old : ELECTRON MC-MATCHING VALUES FROM: https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATMCMatching
# object        electron        photon  muon    tau to jet      jet
# maxDPtRel     0.5             1.0     0.5     3.0             3.0
# maxDeltaR     0.5             0.2     0.5     0.1             0.4
process.load("PhysicsTools.PatAlgos.mcMatchLayer0.electronMatch_cfi")
process.electronMatch.matched = cms.InputTag("prunedGenParticles")
process.electronMatch.maxDeltaR = cms.double(0.5)
process.electronMatch.maxDPtRel = cms.double(0.5)
process.electronMatch.checkCharge = cms.bool(True)
process.electronMatch.resolveAmbiguities = cms.bool(True)
process.electronMatch.resolveByMatchQuality = cms.bool(True)

process.CaloGeometryBuilder = cms.ESProducer("CaloGeometryBuilder",
    SelectedCalos = cms.vstring("HCAL", "EcalBarrel", "EcalEndcap")
)
#-- ANALYZER TAGS AND PARAMETERS --#

process.bmmgVertexAnalysis = cms.EDAnalyzer("RadiativeAnalysis",
                                          isMCstudy                     = cms.bool(False),
                                          isMINIAOD                     = cms.bool(False),
                                          genParticlesLabel             = cms.InputTag("genParticles"),
                                          MuonTag                       = cms.InputTag("muons"),
                                          JetTag                        = cms.InputTag("jets"),
                                          PhotonTag                     = cms.InputTag("photons"),
                                          OOTPhotonTag                  = cms.InputTag("ootPhotons"),
                                          ElectronTag                   = cms.InputTag("electrons"),
                                          #SuperClusterTag               = cms.InputTag("reducedEgamma","reducedSuperClusters","PAT"),
                                          #OOTSuperClusterTag            = cms.InputTag("reducedEgamma","reducedOOTSuperClusters","PAT"),
                                          PUInfo                        = cms.InputTag("addPileupInfo"),
                                          vertexBeamSpot                = cms.InputTag("offlineBeamSpot"),
                                          primaryvertex                 = cms.InputTag("offlinePrimaryVertices"),
                                          triggerbits                   = cms.InputTag("TriggerResults",'',"HLT"),
                                          pfCandTag                     = cms.InputTag("generalTracks"),
                                          pfSupcluster                  = cms.InputTag("particleFlowSuperClusterECAL","particleFlowSuperClusterECALBarrel"),
                                          ecalrechit                    = cms.InputTag("reducedEcalRecHitsEB"),
                                          convertedPhotons              = cms.InputTag("oniaPhotonCandidates","conversions"),
                                          PFCandTag                   = cms.InputTag("particleFlow"),
                                          #IsoTrackTag                   = cms.InputTag("isolatedTracks"),
                                          StoreDeDxInfo                 = cms.bool(True),
                                          PionZeroMassWindowNoFit       = cms.double(0.05),#05),
                                          EtaMesonMassWindowNoFit       = cms.double(0.05),#017),
                                          EtaPrimeMassWindowNoFit       = cms.double(0.05),#230),
                                          JpsiMassWindowBeforeFit       = cms.double(0.310),
                                          JpsiMassWindowAfterFit        = cms.double(0.150),
                                          PsiMassWindowBeforeFit        = cms.double(0.293), 
                                          PsiMassWindowAfterFit         = cms.double(0.028),
                                          PhiMassWindowBeforeFit        = cms.double(0.03),
                                          PhiMassWindowAfterFit         = cms.double(0.02),
                                          MuonPtCut                     = cms.double(4),
                                          JpsiPtCut                     = cms.double(7),
                                          KaonTrackPtCut                = cms.double(0.7),
                                          BsLowerMassCutBeforeFit       = cms.double(4.5),
                                          BsUpperMassCutBeforeFit       = cms.double(6.5),
                                          BsLowerMassCutAfterFit        = cms.double(4.5),
                                          BsUpperMassCutAfterFit        = cms.double(6.5),
                                          verbose                       = cms.bool(True),
                                          TestVerbose                   = cms.bool(True),
                                          BsPDGMass                     = cms.double(5.3699),
                                          BdPDGMass                     = cms.double(5.2794),
                                          BpPDGMass                     = cms.double(5.2790),
                                          PionZeroPDGMass               = cms.double(0.1349),
                                          EtaMesonPDGMass               = cms.double(0.5478),
                                          EtaPrimePDGMass               = cms.double(0.9577),
                                          PsiPDGMass                    = cms.double(3.6860),
                                          outputFile                    = cms.untracked.string(options.outputFile),
                                          xgboost_models = cms.vstring(),
                                          xgboost_variable_names = cms.vstring(),
)

xgboost_models = [
    ('Run2022-20231030-1731-Event0', 'run3muonmva'),
]
for entry in xgboost_models:
    process.bmmgVertexAnalysis.xgboost_models.append(entry[0]),
    process.bmmgVertexAnalysis.xgboost_variable_names.append(entry[1])

process.load("MuonAnalysis.MuonAssociators.patMuonsWithTrigger_cff")
import PhysicsTools.PatAlgos.producersLayer1.muonProducer_cfi
process.patMuonsWithoutTrigger = PhysicsTools.PatAlgos.producersLayer1.muonProducer_cfi.patMuons.clone()

from MuonAnalysis.MuonAssociators.patMuonsWithTrigger_cff import addMCinfo, changeRecoMuonInput, useL1MatchingWindowForSinglets, changeTriggerProcessName, switchOffAmbiguityResolution
useL1MatchingWindowForSinglets(process)
#changeTriggerProcessName(process, "REDIGI36X")
switchOffAmbiguityResolution(process) # Switch off ambiguity resolution: allow multiple reco muons to match to the same trigger muon
process.muonMatchHLTL3.maxDeltaR = 0.1
process.muonMatchHLTL3.maxDPtRel = 10.0
process.muonMatchHLTCtfTrack.maxDeltaR = 0.1
process.muonMatchHLTCtfTrack.maxDPtRel = 10.0
process.muonMatchHLTTrackMu.maxDeltaR = 0.1
process.muonMatchHLTTrackMu.maxDPtRel = 10.0

"""
### ==== Apply some final selection (none by default) ====
process.patMuons = cms.EDFilter("PATMuonSelector",
    src = cms.InputTag("patMuonsWithTrigger"),
    cut = cms.string("p>2 && abs(eta)<2.4"),
)

#apply the scraping event filter here
process.noScraping= cms.EDFilter("FilterOutScraping",
    applyfilter = cms.untracked.bool(True),
    debugOn = cms.untracked.bool(False), ## Or 'True' to get some per-event info
    numtrack = cms.untracked.uint32(10),
    thresh = cms.untracked.double(0.2)
)

process.primaryVertexFilter = cms.EDFilter("GoodVertexFilter",
                                           vertexCollection = cms.InputTag('offlinePrimaryVertices'),
                                           minimumNDOF = cms.uint32(4),
                                           maxAbsZ = cms.double(15),
                                           maxd0 = cms.double(2)
                                           )
"""


# can I do a replace of patMuons with the sequence that includes the trigger matching?
#process.patDefaultSequence.replace(process.patMuons,process.patMuonsWithoutTrigger * process.patTriggerMatching * process.patMuons)
#process.vertex = cms.Path(process.inclusiveVertexing * process.inclusiveMergedVertices * process.selectedVertices * process.bcandidates)
#process.pat = cms.Path( process.patDefaultSequence )
#process.pat = cms.Path(process.patDefaultSequence)
#print(process.pat)

process.SimpleMemoryCheck = cms.Service("SimpleMemoryCheck", ignoreTotal = cms.untracked.int32(1))
process.dumpES = cms.EDAnalyzer("PrintEventSetupContent")
process.espath = cms.Path(process.dumpES)

#process.ntup = cms.Path(process.allPiTracks * process.allKTracks * process.kTracks * process.piTracks * process.bVertexAnalysis )
process.ntup = cms.Path(process.oniaPhotonCandidates*process.bmmgVertexAnalysis )
#process.filter = cms.Path(process.noScraping)
process.schedule = cms.Schedule(process.ntup,process.espath)

