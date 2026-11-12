#!/usr/bin/env python3
# Submit RadiativeAnalysis on all ParkingDoubleMuonLowMass[0-7] MINIAOD of 2024 E, F, G, H, I (48 CRAB tasks).
# Companion of multicrab_DATA_2024BCD.py (eras B, C, D); together they cover the full 2024 Golden JSON.
#   - 2024E:       2024CDEReprocessing (CMSSW_14_0_19); covers the full era (= PromptReco v1 + v2)
#   - 2024F, G, H: PromptReco-v1 (only processing readable by CMSSW_14_1_1)
#   - 2024I:       PromptReco-v1 (runs 386478-386693) AND PromptReco-v2 (runs 386694-386951): two halves of the era
#     (the MINIv6NANOv15 re-MINI is made with CMSSW_15_0_X and is not readable by CMSSW_14_1_1)
#
#   cd $CMSSW_BASE/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/crab_data
#   cmsenv; source /cvmfs/cms.cern.ch/common/crab-setup.sh
#   voms-proxy-init --voms cms                       # needed only while talking to the CRAB server
#   python3 multicrab_DATA_2024EFGHI.py --dryrun      # test one task without submitting jobs
#   python3 multicrab_DATA_2024EFGHI.py               # submit all 48 tasks
#   python3 multicrab_DATA_2024EFGHI.py --only 2024G  # submit a subset (substring match on the dataset name)
#
#   crab status -d crab_projects/<task dir>  ;  crab resubmit -d crab_projects/<task dir>
import os
import argparse
from multiprocessing import Process

from CRABClient.UserUtilities import config as crabConfig
from CRABAPI.RawCommand import crabCommand
from CRABClient.ClientExceptions import ClientException
from http.client import HTTPException

here = os.path.dirname(os.path.abspath(__file__))

outputFileName = 'RadiativeAnalysisDATA.root'

# label used in the CRAB request name -> processed dataset name
# (2024I has two PromptReco versions, so the label carries the version to keep the request names unique)
processings = {
    'Run2024E':    'Run2024E-2024CDEReprocessing-v1',
    'Run2024F':    'Run2024F-PromptReco-v1',
    'Run2024G':    'Run2024G-PromptReco-v1',
    'Run2024H':    'Run2024H-PromptReco-v1',
    'Run2024I_v1': 'Run2024I-PromptReco-v1',
    'Run2024I_v2': 'Run2024I-PromptReco-v2',
}
labelOf  = {proc: label for label, proc in processings.items()}
datasets = [f'/ParkingDoubleMuonLowMass{i}/{proc}/MINIAOD' for proc in processings.values() for i in range(8)]

# full-year 2024 Golden JSON (DQM certification, Aug 2026 version)
lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions24/Cert_Collisions2024_378981_386951_Golden.json'


def makeConfig(dataset):
    primary, processed, _ = dataset.strip('/').split('/')
    stream = primary.replace('ParkingDoubleMuonLowMass', 'PDMLM')
    label = labelOf[processed]

    config = crabConfig()

    config.General.requestName     = f'RadiativeAnalysis_{label}_{stream}'
    config.General.workArea        = 'crab_projects'
    config.General.transferOutputs = True
    config.General.transferLogs    = False

    config.JobType.pluginName  = 'Analysis'
    config.JobType.psetName    = os.path.join(here, '../test/runRadiativeAnalysis_MINIAOD.py')
    config.JobType.pyCfgParams = [f'outputFile={outputFileName}']   # no maxEvents: it would rename the output
    config.JobType.outputFiles = [outputFileName]                    # the analyzer writes its own TFile
    config.JobType.maxMemoryMB = 2500

    config.Data.inputDataset     = dataset
    config.Data.inputDBS         = 'global'
    config.Data.splitting        = 'Automatic'
    config.Data.unitsPerJob      = 180      # target job length in minutes
    config.Data.lumiMask         = lumiMask
    config.Data.publication      = False
    config.Data.outputDatasetTag = f'RadiativeAnalysis_{processed}'
    config.Data.outLFNDirBase    = '/store/user/psajdak/crab_out'   # CERNBox: /eos/user/p/psajdak/crab_out

    config.Site.storageSite = 'T3_CH_CERNBOX'
    return config


def submit(config, dryrun):
    try:
        if dryrun:
            # CRAB does not allow --dryrun with Automatic splitting: use a fixed number of events per job instead
            # (~34 ms per input event in a local 2024C test with the photon filter -> 200k events ~ 2 h)
            config.Data.splitting   = 'EventAwareLumiBased'
            config.Data.unitsPerJob = 200000
            crabCommand('submit', config=config, dryrun=True)
        else:
            crabCommand('submit', config=config)
    except HTTPException as e:
        print(f'Submission of {config.General.requestName} failed (HTTP): {e.headers}')
    except ClientException as e:
        print(f'Submission of {config.General.requestName} failed: {e}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dryrun', action='store_true', help='crab submit --dryrun on the first selected dataset only')
    parser.add_argument('--only', default='', help='submit only datasets whose name contains this string')
    args = parser.parse_args()

    selected = [d for d in datasets if args.only in d]
    if args.dryrun:
        selected = selected[:1]
    print(f'{len(selected)} dataset(s):')
    for d in selected:
        print('  ', d)

    for d in selected:
        # each submission in its own process: the CRAB client caches the config between calls
        p = Process(target=submit, args=(makeConfig(d), args.dryrun))
        p.start()
        p.join()
