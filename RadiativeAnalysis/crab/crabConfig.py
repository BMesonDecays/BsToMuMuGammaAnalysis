# CRAB configuration of the RadiativeAnalysis ntuplizer (test/runNtuplizer_cfg.py), one config for MC and data.
# Set the sample with environment variables, then submit from this directory:
#
#   # official dataset (DAS):
#   BMMG_NAME=BsToMuMuGamma_2022 BMMG_DATASET=/BsTo2MuG_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/.../AODSIM \
#       crab submit -c crabConfig.py
#   # private files (list of LFNs, e.g. from makeFileList.py):
#   BMMG_NAME=BsToMuMuGamma_private BMMG_FILELIST=BsToMuMuGammaFileList.txt crab submit -c crabConfig.py
#   # data with a lumi mask:
#   BMMG_NAME=Parking2022C BMMG_DATASET=/ParkingDoubleMuonLowMass0/Run2022C-PromptReco-v1/AOD BMMG_ISMC=0 \
#       BMMG_LUMIMASK=/path/Cert.json crab submit -c crabConfig.py
#
# Variables:
#   BMMG_NAME      request name and output tag (required)
#   BMMG_DATASET   input dataset  |  BMMG_FILELIST  text file with LFNs (one of the two)
#   BMMG_ISMC      1 (default) or 0
#   BMMG_LUMIMASK  JSON lumi mask (data)
#   BMMG_GLOBALTAG global tag (default: see runNtuplizer_cfg.py)
#   BMMG_SKIM      1: store only the events with a valid B candidate (see 'skim' in runNtuplizer_cfg.py)
#   BMMG_SKIMMINVTXPROB  vertex-probability threshold of the skim (default: any successful fit)
#   BMMG_UNITS     files (FileBased) / lumis (LumiBased, data) per job, default 5 / 50
#   BMMG_OUTLFN    output LFN base, default /store/user/<CERN user name>/bmmg_ntuples
#   BMMG_SITE      storage site, default T3_CH_CERNBOX

import os
from CRABClient.UserUtilities import config, getUsername

def env(name, default=None):
    value = os.environ.get(name, default)
    if value is None:
        raise RuntimeError('crabConfig.py: set the environment variable %s' % name)
    return value

name = env('BMMG_NAME')
isMC = env('BMMG_ISMC', '1') == '1'
outputFile = 'bmmg_ntuple.root'

config = config()
config.General.requestName = name
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '../test/runNtuplizer_cfg.py'
config.JobType.outputFiles = [outputFile]
pyCfgParams = ['isMC=%d' % isMC, 'outputFile=' + outputFile]
if os.environ.get('BMMG_GLOBALTAG'):
    pyCfgParams.append('globalTag=' + os.environ['BMMG_GLOBALTAG'])
if os.environ.get('BMMG_SKIM', '0') == '1':
    pyCfgParams.append('skim=1')
    if os.environ.get('BMMG_SKIMMINVTXPROB'):
        pyCfgParams.append('skimMinVtxProb=' + os.environ['BMMG_SKIMMINVTXPROB'])
config.JobType.pyCfgParams = pyCfgParams
config.JobType.maxMemoryMB = 2500

if os.environ.get('BMMG_DATASET'):
    config.Data.inputDataset = os.environ['BMMG_DATASET']
    config.Data.inputDBS = 'global'
else:
    with open(env('BMMG_FILELIST')) as f:
        config.Data.userInputFiles = [line.strip() for line in f if line.strip()]
    config.Data.outputPrimaryDataset = name
    config.Site.whitelist = ['T2_CH_CERN']

if not isMC and os.environ.get('BMMG_DATASET'):
    config.Data.splitting = 'LumiBased'
    config.Data.unitsPerJob = int(env('BMMG_UNITS', '50'))
else:
    config.Data.splitting = 'FileBased'
    config.Data.unitsPerJob = int(env('BMMG_UNITS', '5'))
if os.environ.get('BMMG_LUMIMASK'):
    config.Data.lumiMask = os.environ['BMMG_LUMIMASK']   # applied by CRAB (no lumiMask option needed in the cfg)

config.Data.publication = False
config.Data.outputDatasetTag = name
config.Data.outLFNDirBase = env('BMMG_OUTLFN', '/store/user/%s/bmmg_ntuples' % getUsername())
config.Site.storageSite = env('BMMG_SITE', 'T3_CH_CERNBOX')
