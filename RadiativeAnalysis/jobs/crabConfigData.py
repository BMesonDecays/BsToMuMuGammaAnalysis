from WMCore.Configuration import Configuration

config = Configuration()
config.section_("General")
#config.General.requestName = setEra+setType 
#config.General.workArea = setType+setId+'_jobVer'+jobVer
config.General.workArea = 'BsToJpsiGamma24B'
config.General.requestName = 'FL_dR_dataset'
config.General.transferLogs = True 
config.General.transferOutputs = True 

config.section_("Data")

config.Data.inputDataset = '/ParkingDoubleMuonLowMass4/Run2024B-PromptReco-v1/MINIAOD'
config.Data.lumiMask= '/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/2024B_Golden.json'

#config.Data.runRange = '362719-362760'

config.Data.useParent = False 
config.Data.inputDBS = 'global'
config.Data.splitting = 'LumiBased'
#config.Data.splitting = 'Automatic'
#config.Data.splitting = 'EventAwareLumiBased'
config.Data.unitsPerJob = 2 #number of files per jobs
config.Data.totalUnits =  -1 #number of event
#config.Data.outLFNDirBase = '/store/user/konec/crabout/'
config.Data.outLFNDirBase = '/store/user/psajdak/crab_out/'
config.Data.outputDatasetTag = 'BsToJpsiGamma_FL_dR'

config.section_("JobType")
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '../test/analysis.py'
#config.JobType.disableAutomaticOutputCollection = True
config.JobType.outputFiles = ['BsToJpsiGammaData.root']
config.JobType.maxJobRuntimeMin = 60
#config.JobType.maxMemoryMB = 3000

config.section_("Site")
#config.Site.whitelist = ['T3_CH_CERNCAF']
#config.Site.whitelist = ['T2_CH_CERN']
#config.Site.storageSite = 'T2_PL_Swierk'
config.Site.storageSite = 'T3_CH_CERNBOX'
#config.Site.blacklist = ['T2_KR_*','T2_CN_*','T2_BR_*','T2_US_Florida','T2_US_UCSD']
