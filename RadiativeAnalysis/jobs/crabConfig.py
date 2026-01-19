import CRABClient
from CRABClient.UserUtilities import config

outputFileName = 'BsToJpsiGammaMC.root'
nEvents = -1

config = config()

config.General.requestName = 'triggerBit'
config.General.workArea = 'BsToJpsiGamma_MC_GenMatched'
config.General.transferOutputs = True

config.JobType.pluginName = 'ANALYSIS'
config.JobType.psetName = '../test/analysis.py'
config.JobType.outputFiles = [outputFileName]
#config.JobType.pyCfgParams = [f'outputFile={outputFileName}', f'nEvents={nEvents}']
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500

config.Data.userInputFiles = open("../JpsiGammaData/BsToJpsiGammaFileList.txt").readlines()
config.Data.unitsPerJob     = 5#5000
config.Data.totalUnits      = 70000000
config.Data.splitting       = 'FileBased'
config.Data.publication = False
config.Data.outputDatasetTag = 'BsToJpsiGamma_MC_triggerBit'
config.Data.outputPrimaryDataset = 'BsToJpsiGamma_analysis'

config.Site.storageSite = 'T3_CH_CERNBOX'
config.Site.whitelist = ['T2_CH_CERN']
#config.Data.outLFNDirBase = '/store/user/sslawins/bsmmg_output'
config.Data.outLFNDirBase = '/store/user/psajdak/crab_out'