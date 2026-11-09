#!/usr/bin/env python3
# Submit RadiativeAnalysis on all ParkingDoubleMuonLowMass[0-7] MINIAOD of 2024 B, C, D (24 CRAB tasks).
#   - 2024B: PromptReco (only processing available)
#   - 2024C, 2024D: 2024CDEReprocessing (CMSSW_14_0_19)
#     (the MINIv6NANOv15 re-MINI is made with CMSSW_15_0_2 and is not readable by CMSSW_14_1_1)
#
#   cd $CMSSW_BASE/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/crab_data
#   cmsenv; source /cvmfs/cms.cern.ch/common/crab-setup.sh
#   voms-proxy-init --voms cms --valid 192:00
#   python3 multicrab_DATA_2024BCD.py --dryrun      # test one task without submitting jobs
#   python3 multicrab_DATA_2024BCD.py               # submit all 24 tasks
#   python3 multicrab_DATA_2024BCD.py --only 2024B  # submit a subset (substring match on the dataset name)
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

processings = {
    'Run2024B': 'Run2024B-PromptReco-v1',
    'Run2024C': 'Run2024C-2024CDEReprocessing-v1',
    'Run2024D': 'Run2024D-2024CDEReprocessing-v1',
}
datasets = [f'/ParkingDoubleMuonLowMass{i}/{proc}/MINIAOD' for proc in processings.values() for i in range(8)]

# full-year 2024 Golden JSON (DQM certification, Aug 2026 version)
lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions24/Cert_Collisions2024_378981_386951_Golden.json'


def makeConfig(dataset):
    primary, processed, _ = dataset.strip('/').split('/')
    stream = primary.replace('ParkingDoubleMuonLowMass', 'PDMLM')
    era = processed.split('-')[0]

    config = crabConfig()

    config.General.requestName     = f'RadiativeAnalysis_{era}_{stream}'
    config.General.workArea        = 'crab_projects'
    config.General.transferOutputs = True
    config.General.transferLogs    = False

    config.JobType.pluginName  = 'Analysis'
    config.JobType.psetName    = os.path.join(here, '../test/runRadiativeAnalysis_MINIAOD_test.py')
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
