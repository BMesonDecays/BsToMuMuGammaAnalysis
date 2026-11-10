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
    python3 submitCrab.py tasks.yaml -v ...                # full CRAB output instead of one line per task

Without --verbose every task prints one summary line (submitted / job states / failure with the end
of the CRAB output); the full CRAB output of each task is in <workArea>/logs/<task>.<command>.log.

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
from multiprocessing import Pipe, Process

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


# order of the job states in the short status summary
JOB_STATES = ['finished', 'transferring', 'running', 'idle', 'unsubmitted', 'cooloff', 'toRetry', 'held', 'failed', 'killed']


def logPath(t, command):
    """Log file with the full CRAB output of one command for one task (written when not --verbose)."""
    return os.path.join(t['workArea'], 'logs', '%s.%s.log' % (t['requestName'], command))


def _crab(command, kwargs, log, conn):
    """Run one crab command; runs in a child process (see the module docstring).

    Without a log file the CRAB output goes to the terminal. With one, stdout and stderr of the process
    (file descriptors, so the CRAB loggers are included) go to the log. The outcome is sent back
    to the parent as a dict: ok, error, result."""
    if log:
        os.makedirs(os.path.dirname(log), exist_ok=True)
        fd = os.open(log, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
        sys.stdout.flush()
        sys.stderr.flush()
        os.dup2(fd, 1)
        os.dup2(fd, 2)
        os.close(fd)
    out = {'ok': False, 'error': '', 'result': None}
    try:
        from CRABAPI.RawCommand import crabCommand
        from http.client import HTTPException
        try:
            result = crabCommand(command, **kwargs)
        except HTTPException as e:
            out['error'] = 'HTTP error: %s' % getattr(e, 'headers', e)
        except Exception as e:   # noqa: BLE001 - report and go on with the next task
            out['error'] = '%s: %s' % (type(e).__name__, e)
        else:
            out['result'] = result if isinstance(result, dict) else None
            status = (result or {}).get('commandStatus') if isinstance(result, dict) else None
            if result is None and command != 'submit':
                out['error'] = 'crab %s did nothing (see the log)' % command
            elif status == 'FAILED':
                out['error'] = 'crab %s failed (see the log)' % command
            else:
                out['ok'] = True
    finally:
        sys.stdout.flush()
        sys.stderr.flush()
        conn.send(out)
        conn.close()


def runCrab(command, kwargs, log):
    """Run a crab command in a child process; returns the dict sent by _crab."""
    parent, child = Pipe(duplex=False)
    p = Process(target=_crab, args=(command, kwargs, log, child))
    p.start()
    child.close()
    try:
        out = parent.recv()
    except EOFError:   # the child died before sending anything
        out = {'ok': False, 'error': 'the CRAB process exited with code %s' % p.exitcode, 'result': None}
    p.join()
    return out


def jobsSummary(jobs):
    """e.g. '120 jobs: finished 100 (83%), running 18, failed 2'."""
    total = sum(jobs.values())
    states = [s for s in JOB_STATES if s in jobs] + sorted(set(jobs) - set(JOB_STATES))
    parts = ['%s %d' % (s, jobs[s]) + (' (%.0f%%)' % (100. * jobs[s] / total) if s == 'finished' else '')
             for s in states]
    return '%4d jobs' % total + (': ' + ', '.join(parts) if parts else '')


def statusSummary(result):
    """Short status of one task: task status, job states, task failure message."""
    text = '%-10s %s' % (result.get('status') or '?', jobsSummary(result.get('jobsPerStatus') or {}))
    msg = result.get('taskFailureMsg') or result.get('statusFailureMsg')
    if msg:
        text += '  [%s]' % ' '.join(str(msg).split())[:150]
    return text


def logTail(log, n=15):
    try:
        with open(log, errors='replace') as f:
            return [line.rstrip() for line in f.readlines()[-n:]]
    except OSError:
        return []


def banner(i, n, command, t):
    """Separator printed before the output of each task (--verbose)."""
    line = '#' * 100
    print('\n%s\n#  [%d/%d]  %s  %s\n#  %s\n%s' % (line, i, n, command.upper(), t['requestName'],
                                                t.get('dataset') or 'file list ' + t.get('fileList', ''), line), flush=True)


class Reporter:
    """Output of the task loop: the full output under a banner (verbose) or one line per task."""

    def __init__(self, tasks, command, verbose):
        self.n, self.command, self.verbose = len(tasks), command, verbose
        self.width = max(len(t['requestName']) for t in tasks)

    def start(self, i, t):
        if self.verbose:
            banner(i, self.n, self.command, t)
        else:
            w = len(str(self.n))
            print('[%*d/%d] %-*s  ' % (w, i, self.n, self.width, t['requestName']), end='', flush=True)

    def line(self, text):
        """Outcome of the task (on the task's line when not verbose)."""
        print(('  ' if self.verbose else '') + text, flush=True)

    def failure(self, out, log):
        error = ' '.join((out['error'] or 'unknown error').split())
        self.line('FAILED: ' + (error if self.verbose else error[:200] + ('...' if len(error) > 200 else '')))
        if not self.verbose:
            for line in logTail(log):
                print('      | ' + line)
            print('      full output: %s' % log, flush=True)


def taskInfo(t):
    """One-line description of a task for the dry run."""
    what = t.get('dataset') or 'file list ' + t['fileList']
    return '%s  %s %s, %s %d%s, options: %s' % (
        what, 'MC' if t['isMC'] else 'data', t['dataFormat'], t['splitting'], t['unitsPerJob'],
        ', lumiMask' if t.get('lumiMask') else '', ' '.join(pyCfgParams(t)[3:]) or 'none')


# ------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('taskFile', help='YAML task file (see tasks_example.yaml)')
    ap.add_argument('--tasks', default='', help='only these tasks: comma-separated glob patterns on the request name')
    ap.add_argument('--command', default='submit', choices=['submit', 'status', 'resubmit', 'kill'],
                    help='crab command to run for every task (default submit)')
    ap.add_argument('--dry-run', action='store_true', help='print the CRAB configurations; do not contact CRAB')
    ap.add_argument('-v', '--verbose', action='store_true',
                    help='print the full CRAB output (and the full configurations with --dry-run); by default only '
                         'one summary line per task, the full output goes to <workArea>/logs/<task>.<command>.log')
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

    command = 'dry-run' if args.dry_run and args.command == 'submit' else args.command
    rep = Reporter(tasks, command, args.verbose)
    sep = '\n' + '#' * 100 if args.verbose else ''

    if args.command != 'submit':
        missing, failed, totals = [], [], {}
        for i, t in enumerate(tasks, 1):
            rep.start(i, t)
            if not os.path.isdir(projectDir(t)):
                rep.line('no project directory %s: not submitted' % projectDir(t))
                missing.append(t['requestName'])
                continue
            log = None if args.verbose else logPath(t, args.command)
            out = runCrab(args.command, {'dir': projectDir(t)}, log)
            if not out['ok']:
                rep.failure(out, log)
                failed.append(t['requestName'])
            elif args.command == 'status':
                rep.line(statusSummary(out['result']))
                for k, v in (out['result'].get('jobsPerStatus') or {}).items():
                    totals[k] = totals.get(k, 0) + v
            else:
                rep.line('%s: OK' % args.command)
        print(sep)
        if totals:
            print('all tasks: %s' % jobsSummary(totals).strip())
        print('crab %s for %d task(s); failed: %s; not submitted (no project directory): %s'
              % (args.command, len(tasks) - len(missing), failed or 'none', missing or 'none'))
        sys.exit(1 if failed else 0)

    user = '{user}'
    if not args.dry_run:
        from CRABClient.UserUtilities import getUsername
        user = getUsername()   # needs a valid grid proxy

    submitted, skipped, failed = [], [], []
    for i, t in enumerate(tasks, 1):
        config = crabConfig(t, user)
        rep.start(i, t)
        if args.dry_run:
            print(config if args.verbose else taskInfo(t))
            continue
        if os.path.exists(projectDir(t)):
            rep.line('skipped: %s exists (already submitted)' % projectDir(t))
            skipped.append(t['requestName'])
            continue
        log = None if args.verbose else logPath(t, 'submit')
        out = runCrab('submit', {'config': config}, log)
        if out['ok']:
            rep.line('submitted: %s' % ((out['result'] or {}).get('uniquerequestname') or t['requestName']))
            submitted.append(t['requestName'])
        else:
            rep.failure(out, log)
            failed.append(t['requestName'])

    if not args.dry_run:
        print(sep)
        print('submitted: %s\nskipped (existing project directory): %s\nfailed: %s'
              % (submitted or 'none', skipped or 'none', failed or 'none'))
        sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
