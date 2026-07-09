from WMCore.Configuration import Configuration
config = Configuration()

config.section_('General')
config.General.requestName = 'ParkingDoubleMuonLowMass0_Run2022C-PromptReco-v1_AOD'
config.General.workArea = 'ParkingDoubleMuonLowMass0_Run2022C-PromptReco-v1_AOD_4thJuly2026'
config.General.transferOutputs = True
config.General.transferLogs = False


config.section_('JobType')
config.JobType.psetName = '/eos/home-a/almuhamm/06.Analysis/FlavourAnalysis/CMSSW_16_1_0/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/test/makeTree_BsMuMuGamma_DATA_AOD.py'
config.JobType.pluginName = 'Analysis'
config.JobType.outputFiles = ['output.root']
config.JobType.allowUndistributedCMSSW = True
config.JobType.maxMemoryMB = 2900#8000

config.section_('Data')
config.Data.lumiMask = '/eos/home-a/almuhamm/06.Analysis/FlavourAnalysis/CMSSW_16_1_0/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/test/json/json2022/Cert_Collisions2022_eraC_355862_357482_Golden.json'
#config.Data.runRange = '299370-300238' # '193093-194075'
config.Data.inputDataset ='/ParkingDoubleMuonLowMass0/Run2022C-PromptReco-v1/AOD'
config.Data.unitsPerJob     = 7000#5000
config.Data.totalUnits      = 70000000
config.Data.splitting = 'LumiBased'
config.Data.outLFNDirBase = '/store/user/almuhamm/MuSampleSharedDirectory/BPHCentralSampleCrab'
config.Data.publication = False

config.section_('Site')
config.Site.storageSite = 'T3_CH_CERNBOX'

