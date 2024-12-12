import FWCore.ParameterSet.Config as cms
process = cms.Process("MUMUGamma")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("FWCore.MessageLogger.MessageLogger_cfi")

from PhysicsTools.PatAlgos.tools.coreTools import *
process.load("PhysicsTools.PatAlgos.patSequences_cff")
from PhysicsTools.PatAlgos.tools.pfTools import *

process.maxEvents = cms.untracked.PSet(  input = cms.untracked.int32(-1) )
process.source = cms.Source("PoolSource",
                            duplicateCheckMode = cms.untracked.string('noDuplicateCheck'),
                            skipEvents = cms.untracked.uint32(0),
                            fileNames = cms.untracked.vstring(
#'root://cms-xrd-global.cern.ch//store/mc/RunIISummer20UL16MiniAODAPVv2/BdToKPi_BMuonFilter_SoftQCDnonD_TuneCP5_13TeV-pythia8-evtgen/MINIAODSIM/BPH_106X_mcRun2_asymptotic_preVFP_v11-v2/2550000/220F4B68-DEFE-334E-9FCD-ECD84A0737DC.root',
#'root://xrootd-cms.infn.it//store/data/Run2023D/ParkingDoubleMuonLowMass0/MINIAOD/22Sep2023_v1-v1/2550000/0419eec5-0ae4-4732-8f06-6d72dd25a149.root',
#'root:///eos/user/a/almuhamm/05.PrivateMC/HeavyFlavorProduction/CMSSW_14_0_17/src/PrivateMCProduction/private_BsToMuMuGamma_Run3.root'
'root:///eos/home-a/almuhamm/05.PrivateMC/HeavyFlavorProduction/CMSSW_12_4_11_patch3/src/PrivateMCProduction/private_BsToMuMuGamma_Run3.root'
#'root://cms-xrd-global.cern.ch//store/mc/Run3Winter23MiniAOD/BsToMuMuG_MuGFilter_SoftQCDnonD_TuneCP5_13p6TeV_pythia8-evtgen/MINIAODSIM/GTv3Digi_GTv3_MiniGTv3_126X_mcRun3_2023_forPU65_v3-v2/2540000/27f6ecbd-6839-49f9-86e7-b3c957ae1f46.root',
)
)

"""
prefixPath = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/BsToMuMuGamma_14_0_17_22_10_2024/TSG-Run3Summer22EEGS_Run2022_BsToMuMuGamma_14_0_17_22_10_2024/BsToMuMuGamma_14_0_17_22_10_2024/241022_160351/0000'
import glob
fileList = glob.glob(prefixPath+'/*.root')
fileList = ['file:'+aFile for aFile in fileList]
process.source.fileNames = fileList
"""


from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL','')

#process.GlobalTag = GlobalTag(process.GlobalTag, '130X_dataRun3_HLT_v2','')

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


#-- ANALYZER TAGS AND PARAMETERS --#

process.bmmgVertexAnalysis = cms.EDAnalyzer("RadiativeAnalysis",
                                          isMCstudy                     = cms.bool(True),
                                          genParticlesLabel             = cms.InputTag("genParticles"),
                                          MuonTag                       = cms.InputTag("muons"),
                                          JetTag                        = cms.InputTag("jets"),
                                          PhotonTag                     = cms.InputTag("photons",'', 'RECO'),
                                          OOTPhotonTag                  = cms.InputTag("ootPhotons"),
                                          ElectronTag                   = cms.InputTag("electrons"),
                                          #SuperClusterTag               = cms.InputTag("reducedEgamma","reducedSuperClusters","PAT"),
                                          #OOTSuperClusterTag            = cms.InputTag("reducedEgamma","reducedOOTSuperClusters","PAT"),
                                          PUInfo                        = cms.InputTag("addPileupInfo"),
                                          vertexBeamSpot                = cms.InputTag("offlineBeamSpot"),
                                          primaryvertex                 = cms.InputTag("offlinePrimaryVertices"),
                                          triggerresults                = cms.InputTag("TriggerResults",'',"HLT"),
                                          pfCandTag                     = cms.InputTag("generalTracks"),
                                          #IsoTrackTag                   = cms.InputTag("isolatedTracks"),
                                          StoreDeDxInfo                 = cms.bool(True),
                                          PionZeroMassWindowNoFit       = cms.double(0.0005),
                                          EtaMesonMassWindowNoFit       = cms.double(0.017),
                                          EtaPrimeMassWindowNoFit       = cms.double(0.230),
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
                                          outputFile                    = cms.untracked.string("BsToMMG_MC_BsToMuMuG_MuGFilter_AOD.root"),                                          
)

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

#process.ntup = cms.Path(process.allPiTracks * process.allKTracks * process.kTracks * process.piTracks * process.bVertexAnalysis )
process.ntup = cms.Path(process.bmmgVertexAnalysis )
#process.filter = cms.Path(process.noScraping)
process.schedule = cms.Schedule(process.ntup)
