#!/usr/bin/env python3
"""Submit (and follow) CRAB tasks of the RadiativeAnalysis ntuplizer for many datasets, from a YAML task file.

Setup (once per shell):
    cmsenv
    source /cvmfs/cms.cern.ch/common/crab-setup.sh
    voms-proxy-init -voms cms -valid 192:00

Usage (from any directory):
    python3 submitCrab.py tasks.yaml                       # submit every task of the file
    python3 submitCrab.py tasks.yaml --dry-run             # only print the CRAB configurations, nothing is submitted
    python3 submitCrab.py tasks.yaml --tasks 'Parking*'    # only the tasks whose name matches (glob, comma-separated)
    python3 submitCrab.py tasks.yaml --command status      # crab status of every task (also: resubmit, kill)

The task file format is documented in tasks_example.yaml. Every task becomes one CRAB task
crab_<name> in the work area. A task whose project directory already exists is not submitted again.

Following https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRABClientLibraryAPI, each submission
runs in its own process: CRAB imports the CMSSW configuration once per Python session, so the
per-task cmsRun options would otherwise not be applied.
"""
import argparse
import copy
import fnmatch
import os
import re
import sys
from multiprocessing import Process

import yaml

HERE = os.path.dirname(os.path.abspath(__file__))
PSET = os.path.normpath(os.path.join(HERE, '..', 'test', 'runNtuplizer_cfg.py'))
OUTPUT_FILE = 'bmmg_ntuple.root'

# task keys (everything else in a task is an error, to catch typos)
TASK_KEYS = {
    'name', 'dataset', 'datasets', 'fileList', 'isMC', 'dataFormat', 'lumiMask', 'runRange',
    'splitting', 'unitsPerJob', 'totalUnits', 'maxMemoryMB', 'maxJobRuntimeMin', 'numCores',
    'workArea', 'outLFNDirBase', 'storageSite', 'tag', 'whitelist', 'blacklist', 'options',
}
# runNtuplizer_cfg.py options set by the script itself (not allowed in 'options')
RESERVED_OPTIONS = {'isMC', 'dataFormat', 'outputFile', 'inputFiles', 'testSample', 'testMaxFiles', 'maxEvents'}

DEFAULTS = {
    'workArea': 'crab_projects',
    'outLFNDirBase': '/store/user/{user}/bmmg_ntuples',
    'storageSite': 'T3_CH_CERNBOX',
    'maxMemoryMB': 2500,
    'options': {},
}


# ------------------------------------------------------------------ task file
def merge(base, override):
    """Merge two task dictionaries; 'options' is merged key by key."""
    out = copy.deepcopy(base)
    for k, v in override.items():
        if k == 'options' and isinstance(v, dict):
            out['options'] = {**out.get('options', {}), **v}
        else:
            out[k] = copy.deepcopy(v)
    return out


def tier(dataset):
    return dataset.rstrip('/').split('/')[-1].upper()


def defaultName(dataset):
    """Request name from a dataset name: <primary>_<processed>, shortened to fit the CRAB limit."""
    _, primary, processed, _ = dataset.split('/')
    name = '%s_%s' % (primary, processed)
    if len(name) > 90:
        name = '%s_%s' % (primary, processed.split('-')[0])   # MC: keep only the campaign
    return name


def expandTasks(doc, path):
    """List of fully configured tasks (dicts) from the parsed YAML document."""
    if not isinstance(doc, dict) or 'tasks' not in doc:
        raise ValueError('%s: the file needs a "tasks" list (and optionally "defaults")' % path)
    defaults = merge(DEFAULTS, doc.get('defaults') or {})
    unknown = set(doc) - {'defaults', 'tasks'}
    if unknown:
        raise ValueError('%s: unknown top-level keys %s' % (path, sorted(unknown)))
    tasks = []
    for i, entry in enumerate(doc['tasks']):
        unknown = set(entry) - TASK_KEYS
        if unknown:
            raise ValueError('%s, task %d: unknown keys %s (allowed: %s)' % (path, i, sorted(unknown), sorted(TASK_KEYS)))
        entry = merge(defaults, entry)
        datasets = entry.pop('datasets', None)
        if datasets:
            if 'dataset' in entry or 'fileList' in entry:
                raise ValueError('%s, task %d: use either "datasets" or "dataset"/"fileList"' % (path, i))
            for ds in datasets:
                t = dict(entry, dataset=ds)
                t.pop('name', None)   # one name per dataset
                tasks.append(t)
        else:
            tasks.append(entry)
    for t in tasks:
        finalize(t, path)
    names = [t['requestName'] for t in tasks]
    dup = sorted({n for n in names if names.count(n) > 1})
    if dup:
        raise ValueError('%s: duplicate request names %s (set "name" or "tag")' % (path, dup))
    return tasks


def finalize(t, path):
    """Check a task, fill the defaults that depend on the input and build its request name."""
    where = '%s, task %s' % (path, t.get('name') or t.get('dataset') or t.get('fileList'))
    if bool(t.get('dataset')) == bool(t.get('fileList')):
        raise ValueError('%s: give exactly one of "dataset" / "datasets" / "fileList"' % where)
    if t.get('dataset'):
        if not re.match(r'^/[^/]+/[^/]+/[^/]+$', t['dataset']):
            raise ValueError('%s: malformed dataset name %s' % (where, t['dataset']))
        dsTier = tier(t['dataset'])
        # data type: from the data tier unless given; if given it must agree with the tier
        tierFormat = 'MINIAOD' if dsTier.startswith('MINIAOD') else ('AOD' if dsTier.startswith('AOD') else None)
        tierMC = dsTier.endswith('SIM')
        t.setdefault('dataFormat', tierFormat)
        t.setdefault('isMC', tierMC)
        if tierFormat and t['dataFormat'].upper() != tierFormat:
            raise ValueError('%s: dataFormat %s does not match the data tier %s' % (where, t['dataFormat'], dsTier))
        if bool(t['isMC']) != tierMC:
            raise ValueError('%s: isMC=%s does not match the data tier %s' % (where, t['isMC'], dsTier))
        t.setdefault('name', defaultName(t['dataset']))
    else:
        for key in ('name', 'isMC', 'dataFormat'):
            if key not in t:
                raise ValueError('%s: a fileList task needs "%s"' % (where, key))
    if t.get('dataFormat') is None:
        raise ValueError('%s: set "dataFormat" (AOD or MINIAOD)' % where)
    t['dataFormat'] = t['dataFormat'].upper()
    if t['dataFormat'] not in ('AOD', 'MINIAOD'):
        raise ValueError('%s: dataFormat must be AOD or MINIAOD' % where)
    t['isMC'] = bool(t['isMC'])
    if not t['isMC'] and not t.get('lumiMask'):
        print('WARNING: %s: data without a lumiMask' % where)
    bad = set(t['options']) & RESERVED_OPTIONS
    if bad:
        raise ValueError('%s: options %s are set by the script (use the task keys isMC / dataFormat)' % (where, sorted(bad)))
    # splitting: data by lumi sections, MC and file lists by files
    if 'splitting' not in t:
        t['splitting'] = 'LumiBased' if (not t['isMC'] and t.get('dataset')) else 'FileBased'
    if 'unitsPerJob' not in t:
        t['unitsPerJob'] = {'LumiBased': 50, 'FileBased': 5, 'EventAwareLumiBased': 100000, 'Automatic': 480}.get(t['splitting'], 5)
    name = t['name'] + ('_' + str(t['tag']) if t.get('tag') else '')
    if not re.match(r'^[A-Za-z0-9_-]+$', name) or len(name) > 100:
        raise ValueError('%s: request name "%s" must have <= 100 characters out of [A-Za-z0-9_-]' % (where, name))
    t['requestName'] = name
    if not os.path.isabs(t['workArea']):
        t['workArea'] = os.path.join(os.path.dirname(os.path.abspath(path)), t['workArea'])


def pyCfgParams(t):
    params = ['isMC=%d' % t['isMC'], 'dataFormat=' + t['dataFormat'], 'outputFile=' + OUTPUT_FILE]
    for k, v in sorted(t['options'].items()):
        if isinstance(v, bool):
            v = int(v)
        params.append('%s=%s' % (k, v))
    return params


# ------------------------------------------------------------------ CRAB
def crabConfig(t, user):
    from CRABClient.UserUtilities import config as crabConfigBase
    c = crabConfigBase()
    c.General.requestName = t['requestName']
    c.General.workArea = t['workArea']
    c.General.transferOutputs = True
    c.General.transferLogs = False

    c.JobType.pluginName = 'Analysis'
    c.JobType.psetName = PSET
    c.JobType.pyCfgParams = pyCfgParams(t)
    c.JobType.outputFiles = [OUTPUT_FILE]
    c.JobType.maxMemoryMB = int(t['maxMemoryMB'])
    if t.get('maxJobRuntimeMin'):
        c.JobType.maxJobRuntimeMin = int(t['maxJobRuntimeMin'])
    if t.get('numCores'):
        c.JobType.numCores = int(t['numCores'])

    if t.get('dataset'):
        c.Data.inputDataset = t['dataset']
        c.Data.inputDBS = 'global'
    else:
        listPath = t['fileList']
        if not os.path.isabs(listPath):
            listPath = os.path.join(HERE, listPath)
        with open(listPath) as f:
            c.Data.userInputFiles = [line.strip() for line in f if line.strip() and not line.startswith('#')]
        c.Data.outputPrimaryDataset = t['name']
    c.Data.splitting = t['splitting']
    c.Data.unitsPerJob = int(t['unitsPerJob'])
    if t.get('totalUnits'):
        c.Data.totalUnits = int(t['totalUnits'])
    if t.get('lumiMask'):
        c.Data.lumiMask = t['lumiMask']   # applied by CRAB
    if t.get('runRange'):
        c.Data.runRange = str(t['runRange'])
    c.Data.publication = False
    c.Data.outputDatasetTag = t['requestName']
    c.Data.outLFNDirBase = t['outLFNDirBase'].replace('{user}', user)

    c.Site.storageSite = t['storageSite']
    if t.get('whitelist'):
        c.Site.whitelist = list(t['whitelist'])
    elif t.get('fileList'):
        c.Site.whitelist = ['T2_CH_CERN']   # private files on /eos/cms
    if t.get('blacklist'):
        c.Site.blacklist = list(t['blacklist'])
    return c


def projectDir(t):
    return os.path.join(t['workArea'], 'crab_' + t['requestName'])


def _submit(config):
    # runs in a child process (see the module docstring)
    from CRABAPI.RawCommand import crabCommand
    from CRABClient.ClientExceptions import ClientException
    from http.client import HTTPException
    try:
        crabCommand('submit', config=config)
    except HTTPException as e:
        print('Failed submitting task %s: %s' % (config.General.requestName, e.headers))
        sys.exit(1)
    except ClientException as e:
        print('Failed submitting task %s: %s' % (config.General.requestName, e))
        sys.exit(1)


def _command(command, directory):
    from CRABAPI.RawCommand import crabCommand
    try:
        result = crabCommand(command, dir=directory)
    except Exception as e:   # noqa: BLE001 - report and go on with the next task
        print('crab %s failed for %s: %s' % (command, directory, e))
        sys.exit(1)
    if command == 'status' and isinstance(result, dict):
        jobs = result.get('jobsPerStatus', {})
        total = sum(jobs.values())
        summary = ', '.join('%s %d' % (k, v) for k, v in sorted(jobs.items()))
        print('SUMMARY %s: task %s, %d jobs: %s' % (os.path.basename(directory), result.get('status'), total, summary))


def runInProcess(target, *args):
    p = Process(target=target, args=args)
    p.start()
    p.join()
    return p.exitcode == 0


# ------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('taskFile', help='YAML task file (see tasks_example.yaml)')
    ap.add_argument('--tasks', default='', help='only these tasks: comma-separated glob patterns on the request name')
    ap.add_argument('--command', default='submit', choices=['submit', 'status', 'resubmit', 'kill'],
                    help='crab command to run for every task (default submit)')
    ap.add_argument('--dry-run', action='store_true', help='print the CRAB configurations; do not contact CRAB')
    args = ap.parse_args()

    try:
        with open(args.taskFile) as f:
            tasks = expandTasks(yaml.safe_load(f), args.taskFile)
    except (ValueError, yaml.YAMLError) as e:
        sys.exit('ERROR: %s' % e)
    if args.tasks:
        patterns = [p.strip() for p in args.tasks.split(',') if p.strip()]
        tasks = [t for t in tasks if any(fnmatch.fnmatch(t['requestName'], p) for p in patterns)]
    if not tasks:
        sys.exit('no task selected')

    if args.command != 'submit':
        missing = [t['requestName'] for t in tasks if not os.path.isdir(projectDir(t))]
        failed = [t['requestName'] for t in tasks if t['requestName'] not in missing
                  and not runInProcess(_command, args.command, projectDir(t))]
        print('\ncrab %s for %d task(s); failed: %s; not submitted (no project directory): %s'
              % (args.command, len(tasks) - len(missing), failed or 'none', missing or 'none'))
        sys.exit(1 if failed else 0)

    user = '{user}'
    if not args.dry_run:
        from CRABClient.UserUtilities import getUsername
        user = getUsername()   # needs a valid grid proxy

    submitted, skipped, failed = [], [], []
    for t in tasks:
        config = crabConfig(t, user)
        print('=' * 100)
        print('task %s' % t['requestName'])
        if args.dry_run:
            print(config)
            continue
        if os.path.exists(projectDir(t)):
            print('  %s exists: already submitted, skipped' % projectDir(t))
            skipped.append(t['requestName'])
            continue
        (submitted if runInProcess(_submit, config) else failed).append(t['requestName'])

    if not args.dry_run:
        print('\nsubmitted: %s\nskipped (existing project directory): %s\nfailed: %s'
              % (submitted or 'none', skipped or 'none', failed or 'none'))
        sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
