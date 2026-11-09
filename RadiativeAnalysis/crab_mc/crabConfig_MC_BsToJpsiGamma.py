# CRAB config: RadiativeAnalysis on the private Run3 Bs -> J/psi gamma MC (AOD)
# Same input files and cmsRun config as test/submit.py, run on the grid instead of lxplus.
#
#   cd $CMSSW_BASE/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/crab_mc
#   cmsenv; voms-proxy-init --voms cms --valid 168:00
#   crab submit -c crabConfig_MC_BsToJpsiGamma.py      (add --dryrun first to test one job)
#   crab status -d crab_projects/crab_RadiativeAnalysis_MC_BsToJpsiGamma
import os
from CRABClient.UserUtilities import config

here = os.path.dirname(os.path.abspath(__file__))

outputFileName = 'RadiativeAnalysisMC.root'   # must match the analyzer's outputFile (passed below)

config = config()

config.General.requestName     = 'RadiativeAnalysis_MC_BsToJpsiGamma'
config.General.workArea        = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs    = True

config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = os.path.join(here, '../test/runRadiativeAnalysis_MC_AOD.py')
# no maxEvents here: VarParsing would append "_numEventN" to the output name; CRAB controls the event count itself
config.JobType.pyCfgParams = [f'outputFile={outputFileName}']
config.JobType.outputFiles = [outputFileName]   # the analyzer writes its own TFile (not TFileService)
config.JobType.numCores    = 1
config.JobType.maxMemoryMB = 2500

# 1233 private-MC LFNs under /store/group/phys_bphys (stored at CERN EOS -> T2_CH_CERN)
with open(os.path.join(here, '../JpsiGammaData/BsToJpsiGammaFileList.txt')) as f:
    config.Data.userInputFiles = [l.strip() for l in f if l.strip()]
config.Data.splitting            = 'FileBased'
config.Data.unitsPerJob          = 20          # ~950 events/file, ~35 s/file locally -> ~15 min/job, 62 jobs
config.Data.publication          = False
config.Data.outputPrimaryDataset = 'BsToJpsiGamma_Run3_privateMC'
config.Data.outputDatasetTag     = 'RadiativeAnalysis_MC_AOD'
config.Data.outLFNDirBase        = '/store/user/psajdak/crab_out'   # CERNBox: /eos/user/p/psajdak/crab_out

# user input files carry no location info -> run where the files are
config.Site.whitelist   = ['T2_CH_CERN']
config.Site.storageSite = 'T3_CH_CERNBOX'
