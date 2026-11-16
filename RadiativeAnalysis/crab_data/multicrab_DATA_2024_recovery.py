#!/usr/bin/env python3
# Recovery tasks for 2024 data CRAB tasks that ended with unprocessed lumis.
# The original tasks were killed and `crab report` was run on them; each recovery task
# reuses the original configuration (makeConfig from the multicrab scripts) and processes
# only the lumis listed in the original task's results/notFinishedLumis.json.
#
#   cd $CMSSW_BASE/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/crab_data
#   cmsenv; source /cvmfs/cms.cern.ch/common/crab-setup.sh
#   voms-proxy-init --voms cms
#   python3 multicrab_DATA_2024_recovery.py --dryrun   # test the first task without submitting jobs
#   python3 multicrab_DATA_2024_recovery.py            # submit all recovery tasks
#   python3 multicrab_DATA_2024_recovery.py --only 2024H_PDMLM7
import os
import argparse
from multiprocessing import Process

import multicrab_DATA_2024BCD as bcd
import multicrab_DATA_2024EFGHI as efghi

here = os.path.dirname(os.path.abspath(__file__))

recoveryTag = 'recovery1'

# original task label -> input dataset (unprocessed lumis per task, from `crab report`)
tasks = {
    # submitted 13 Nov 2026, all recovery1 tasks finished 100%:
    # 'Run2024B_PDMLM4': '/ParkingDoubleMuonLowMass4/Run2024B-PromptReco-v1/MINIAOD',           # 843 lumis
    # 'Run2024D_PDMLM0': '/ParkingDoubleMuonLowMass0/Run2024D-2024CDEReprocessing-v1/MINIAOD',  # 309 lumis
    # 'Run2024F_PDMLM3': '/ParkingDoubleMuonLowMass3/Run2024F-PromptReco-v1/MINIAOD',           # 327 lumis
    # 'Run2024H_PDMLM7': '/ParkingDoubleMuonLowMass7/Run2024H-PromptReco-v1/MINIAOD',           #  20 lumis
    # crab report 13 Nov 2026 (evening), recovery1 tasks finished 100% (checked 15 Nov 2026):
    # 'Run2024G_PDMLM1':    '/ParkingDoubleMuonLowMass1/Run2024G-PromptReco-v1/MINIAOD',        #  61 lumis, failed with 8028 (file open error)
    # 'Run2024I_v2_PDMLM5': '/ParkingDoubleMuonLowMass5/Run2024I-PromptReco-v2/MINIAOD',        # 110 lumis, failed with 50660 (memory, 3.2 GB peak)
    # crab report 15 Nov 2026:
    'Run2024I_v1_PDMLM3': '/ParkingDoubleMuonLowMass3/Run2024I-PromptReco-v1/MINIAOD',        # 174 lumis (5 files), failed with 50664 (wall-clock) + 8028 (file open error)
}

# per-task overrides of the default recovery settings below
overrides = {
    # the remaining lumis are scattered over many files (10-23 files in a single failed job):
    # small jobs keep the number of files opened per job low, isolating a bad file and limiting memory
    # 'Run2024G_PDMLM1':    {'unitsPerJob': 10},
    # 'Run2024I_v2_PDMLM5': {'unitsPerJob': 10, 'maxMemoryMB': 3000},
    # recovery1 (unitsPerJob 10): one job hit the wall-clock limit (50664), one failed to open a file (8028).
    # recovery2 (crab report of recovery1, 16 Nov 2026): 40 lumis left; 4 jobs failed with 8028, all at T2_US_UCSD,
    # each sharing one of two suspect files (386616 fd0271db..., 386617 3488ca5d...). One file per job isolates them;
    # all 6 input files also have a disk replica at T2_BE_UCL, where the jobs are sent to avoid the UCSD copies.
    'Run2024I_v1_PDMLM3': {'tag': 'recovery2', 'fromTag': 'recovery1',
                           'splitting': 'FileBased', 'unitsPerJob': 1, 'siteWhitelist': ['T2_BE_UCL']},
}


def makeRecoveryConfig(label, dataset):
    module = bcd if dataset in bcd.datasets else efghi
    config = module.makeConfig(dataset)

    opts = overrides.get(label, {})
    tag  = opts.get('tag', recoveryTag)
    # task whose crab report provides the lumis: the original task, or a previous recovery task (opts['fromTag'])
    source = f'crab_RadiativeAnalysis_{label}' + (f'_{opts["fromTag"]}' if 'fromTag' in opts else '')

    config.General.requestName = f'RadiativeAnalysis_{label}_{tag}'
    # notFinishedLumis.json is already a subset of the Golden JSON used by the original task
    config.Data.lumiMask    = os.path.join(here, 'crab_projects', source, 'results', 'notFinishedLumis.json')
    # small tasks: fixed splitting avoids the Automatic-splitting probe stage
    # (~2000 lumis per 3 h in the original 2024B task -> 100 lumis ~ 10-30 min, more with scattered lumis)
    config.Data.splitting   = opts.get('splitting', 'LumiBased')
    config.Data.unitsPerJob = opts.get('unitsPerJob', 100)
    if 'maxMemoryMB' in opts:
        config.JobType.maxMemoryMB = opts['maxMemoryMB']
    if 'siteWhitelist' in opts:
        config.Site.whitelist = opts['siteWhitelist']
    return config


def submit(config, dryrun):
    try:
        bcd.crabCommand('submit', config=config, dryrun=dryrun) if dryrun else bcd.crabCommand('submit', config=config)
    except bcd.HTTPException as e:
        print(f'Submission of {config.General.requestName} failed (HTTP): {e.headers}')
    except bcd.ClientException as e:
        print(f'Submission of {config.General.requestName} failed: {e}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dryrun', action='store_true', help='crab submit --dryrun on the first selected task only')
    parser.add_argument('--only', default='', help='submit only tasks whose label contains this string')
    args = parser.parse_args()

    selected = [(l, d) for l, d in tasks.items() if args.only in l]
    if args.dryrun:
        selected = selected[:1]

    for label, dataset in selected:
        config = makeRecoveryConfig(label, dataset)
        if not os.path.isfile(config.Data.lumiMask):
            print(f'Skipping {label}: {config.Data.lumiMask} not found (run crab report first)')
            continue
        print(f'{config.General.requestName}: {dataset}')
        # each submission in its own process: the CRAB client caches the config between calls
        p = Process(target=submit, args=(config, args.dryrun))
        p.start()
        p.join()
