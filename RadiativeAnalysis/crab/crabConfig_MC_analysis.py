import CRABClient
from CRABClient.UserUtilities import config

outputFileName = 'BsToMuMuGamma_analysis.root'
nEvents = -1

config = config()

config.General.requestName = 'BsToMuMuGamma_analysis'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '../test/makeTree_BsMuMuGamma_MC_AOD.py'
config.JobType.outputFiles = [outputFileName]
config.JobType.pyCfgParams = [f'outputFile={outputFileName}', f'nEvents={nEvents}']
config.JobType.numCores = 2
config.JobType.maxMemoryMB = 5000

config.Data.userInputFiles = open("fileList.txt").readlines()
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 50
config.Data.publication = False
config.Data.outputDatasetTag = 'BsToMuMuGamma_analysis'
config.Data.outputPrimaryDataset = 'BsToMuMuGamma_analysis'

config.Site.storageSite = 'T3_CH_CERNBOX'
config.Site.whitelist = ['T2_CH_CERN']
config.Data.outLFNDirBase = '/store/user/sslawins/bsmmg_output'
