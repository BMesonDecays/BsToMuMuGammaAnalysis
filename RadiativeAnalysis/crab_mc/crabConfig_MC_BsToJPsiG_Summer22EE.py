# CRAB config: RadiativeAnalysis on the official Run3Summer22EE Bs -> J/psi(mu mu) gamma MC (MINIAODSIM)
# (the private AOD sample is processed with crabConfig_MC_BsToJpsiGamma.py)
#
#   cd $CMSSW_BASE/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/crab_mc
#   cmsenv; source /cvmfs/cms.cern.ch/common/crab-setup.sh
#   voms-proxy-init --voms cms                       # needed only while talking to the CRAB server
#   crab submit -c crabConfig_MC_BsToJPsiG_Summer22EE.py
#   crab status -d crab_projects/crab_RadiativeAnalysis_MC_BsToJPsiG_Run3Summer22EE
import os
from CRABClient.UserUtilities import config

here = os.path.dirname(os.path.abspath(__file__))

outputFileName = 'RadiativeAnalysisMC.root'   # must match the analyzer's outputFile (passed below)

config = config()

config.General.requestName     = 'RadiativeAnalysis_MC_BsToJPsiG_Run3Summer22EE'
config.General.workArea        = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs    = True

config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = os.path.join(here, '../test/runRadiativeAnalysis_MC_MINIAOD.py')
# no maxEvents here: VarParsing would append "_numEventN" to the output name; CRAB controls the event count itself
config.JobType.pyCfgParams = [f'outputFile={outputFileName}']
config.JobType.outputFiles = [outputFileName]   # the analyzer writes its own TFile (not TFileService)
config.JobType.numCores    = 1
config.JobType.maxMemoryMB = 2500

# 141 files, 4.58 M events (CMSSW_13_0_24); on disk at many T1/T2 sites -> no site whitelist needed
config.Data.inputDataset     = '/BsToJPsiG-JPsiTo2Mu_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2/MINIAODSIM'
config.Data.inputDBS         = 'global'
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 5           # ~33k events/file, ~13 ms/event locally -> ~40-60 min/job, 29 jobs
config.Data.publication      = False
config.Data.outputDatasetTag = 'RadiativeAnalysis_Run3Summer22EE_MINIAODSIM'
config.Data.outLFNDirBase    = '/store/user/psajdak/crab_out'   # CERNBox: /eos/user/p/psajdak/crab_out

config.Site.storageSite = 'T3_CH_CERNBOX'
