#!/usr/bin/env python3
from CRABClient.UserUtilities import config
from CRABAPI.RawCommand import crabCommand

config = config()


config.section_('General')
config.General.requestName = 'BsToJpsiG_PreEE'
#config.General.requestName = 'BsToJPsiPi0-JPsiTo2Mu-Pi0To2G_SoftQCD_TuneCP5_13p6TeV_pythia8_PostEE_AODSIM'
#request name is the name of the folder where crab log is saved
#config.General.workArea = 'BsTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8_PreEE_AODSIM_23June2026'

#config.General.workArea = 'BsToJPsiPi0-JPsiTo2Mu-Pi0To2G_SoftQCD_TuneCP5_13p6TeV_pythia8_PostEE_AODSIM_27June2026'
config.General.workArea = 'BsToJpsiG_PreEE'
config.General.transferOutputs = True
config.General.transferLogs = False


config.section_('JobType')
config.JobType.psetName = '/eos/home-a/almuhamm/06.Analysis/FlavourAnalysis/CMSSW_16_1_0/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/test/makeTree_BsMuMuGamma_MC_AOD.py'
config.JobType.pluginName = 'Analysis'
config.JobType.outputFiles = ['output.root']
config.JobType.allowUndistributedCMSSW = True

config.section_('Data')
#config.Data.lumiMask = 'Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt'
#config.Data.runRange = '299370-300238' # '193093-194075'
#config.Data.inputDataset = '/BsTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM'
#config.Data.inputDataset = '/BsTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM'
config.Data.inputDataset  = '/BsToJPsiG-JPsiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM'
#config.Data.inputDataset  = '/BsToJPsiG-JPsiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM'
#config.Data.inputDataset  = '/BsToKstarG-KstarTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM'
#config.Data.inputDataset = '/BsToKstarG-KstarTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM'
#config.Data.inputDataset = '/BsToJPsiEta-JPsiTo2Mu-EtaTo2G_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM'
#config.Data.inputDataset = '/BsToJPsiEta-JPsiTo2Mu-EtaTo2G_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM'
#config.Data.inputDataset= '/BsToJPsiPi0-JPsiTo2Mu-Pi0To2G_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM'
#config.Data.inputDataset ='/BsToJPsiPi0-JPsiTo2Mu-Pi0To2G_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM'
config.Data.unitsPerJob     = 7000#5000
config.Data.totalUnits      = 70000000
config.Data.splitting       = 'EventAwareLumiBased'
config.JobType.maxMemoryMB = 2900#8000
#config.Data.splitting = 'LumiBased'


config.Data.outLFNDirBase = '/store/user/almuhamm/MuSampleSharedDirectory/BPHCentralSampleCrab'
config.Data.publication = False
config.section_('Site')
config.Site.storageSite = 'T3_CH_CERNBOX'

"""
datasets = [
    ("BsToPhiG_PreEE",
     "/BsToPhiG-PhiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM"),

    ("BsToPhiG_PostEE",
     "/BsToPhiG-PhiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM"),

    ("BdTo2MuG_PreEE",
     "/BdTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM"),

    ("BdTo2MuG_PostEE",
     "/BdTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM"),

    ("BdToKstarG_PreEE",
     "/BdToKstarG-KstarTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM"),

    ("BdToKstarG_PostEE",
     "/BdToKstarG-KstarTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM")
]

for requestName, dataset in datasets:
    config.General.requestName = requestName
    config.Data.inputDataset = dataset
    try:
        crabCommand('submit', config=config)
    except Exception as e:
        print(f"Failed to submit {dataset}: {e}")





datasets = [
   '/BsToPhiG-PhiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM',
   '/BsToPhiG-PhiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM',
   '/BdTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM',
   '/BdTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM',
   '/BdToKstarG-KstarTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22DRPremix-124X_mcRun3_2022_realistic_v12-v2/AODSIM',
   '/BdToKstarG-KstarTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEDRPremix-124X_mcRun3_2022_realistic_postEE_v1-v2/AODSIM'
]

for dataset in datasets:
    shortname = dataset.split('/')[1] + '_' + dataset.split('/')[2]  # e.g. Muon0_Run2025F-ZMu-PromptReco-v1
    config.General.requestName = 'RareRadiative_' + shortname
    config.Data.inputDataset = dataset
    try:
        crabCommand('submit', config=config)
    except Exception as e:
        print(f"Failed to submit {dataset}: {e}")
"""
