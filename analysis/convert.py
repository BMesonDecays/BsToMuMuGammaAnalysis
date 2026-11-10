#!/usr/bin/env python3
"""Convert the bmmg ntuples (thousands of small CRAB output files) into candidate-level Parquet tables.

    python3 convert.py config/convert.yaml [--datasets NAME ...] [--workers 16] [--dry-run]

For every dataset of the config the input files are split into chunks; each chunk is converted by
one worker process into <output>/<dataset>/<channel>/part-NNNNN.parquet (one row per B candidate,
see bmmg_ana/schema.py). Files are read one by one, so the memory per worker stays small.

The conversion is incremental: <output>/<dataset>/manifest.json lists the converted files of every
part. A re-run converts only the files that are not in the manifest yet (e.g. new CRAB outputs).
Unreadable files are reported, listed in the manifest and retried at the next run; a file that fails
in any channel contributes nothing (no rows, no RunInfo). Readers (bmmg_ana.io) use only the parts
listed in the manifest. The manifest records the conversion settings (channels, preselection,
drop_columns); a re-run with other settings is refused, so tables with different cuts are never mixed.
To convert a dataset again from scratch, delete its output directory.

<output>/<dataset>/meta.json has the sums over all converted files of the RunInfo tree
(nEvents = processed events, nEventsStored, cross section and filter efficiency for MC).
"""
import argparse
import glob
import hashlib
import json
import os
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed

import pandas as pd
import pyarrow as pa
import pyarrow.parquet as pq
import uproot
import yaml

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bmmg_ana.schema import CHANNELS, build_table, needed_branches  # noqa: E402

RUNINFO_SUM = ('nEvents', 'nEventsStored')
RUNINFO_MEAN = ('xsec', 'filterEff')   # nEvents-weighted mean over files with a positive value


def file_list(pattern):
    patterns = pattern if isinstance(pattern, list) else [pattern]
    files = []
    for p in patterns:
        files += glob.glob(os.path.expanduser(p), recursive=True)
    return sorted(set(files))


def parquet_codec(preferred='zstd'):
    """The preferred Parquet compression if this pyarrow build has it, else the best available one."""
    for codec in (preferred, 'zstd', 'lz4', 'snappy', 'gzip'):
        if codec and pa.Codec.is_available(codec):
            return codec
    return 'none'


def convert_file(f, channels, preselection, drop):
    """Tables and RunInfo of one file. Raises on any problem, so a file contributes all or nothing."""
    with uproot.open(f) as rf:
        tree = rf['bmmg/Events']
        arrays = tree.arrays(needed_branches(tree.keys(), channels))
        ri = rf['bmmg/RunInfo'].arrays(library='np')
        runinfo = {k: float(ri[k].sum()) if k in RUNINFO_SUM else float(ri[k][0])
                   for k in RUNINFO_SUM + RUNINFO_MEAN if k in ri}
        tables = {}
        for ch in channels:
            df = build_table(arrays, ch, preselection, drop)
            if df is not None and len(df):
                tables[ch] = df
    return tables, runinfo


def convert_chunk(dataset, part, files, outdir, channels, preselection, drop, codec):
    """Worker: convert a list of files into one Parquet file per channel. Returns a summary dict.

    The contributions of a file (rows of every channel and its RunInfo) are kept only if the whole
    file was converted. The parts are written under their final name, but readers only use the parts
    listed in the manifest, which the main process writes after the worker returned.
    """
    t0 = time.time()
    tables = {ch: [] for ch in channels}
    runinfo, good, bad = [], [], []
    for f in files:
        try:
            ftables, fri = convert_file(f, channels, preselection, drop)
        except Exception as e:   # noqa: BLE001 - a broken file must not stop the conversion
            bad.append('%s: %s' % (f, e))
            continue
        good.append(f)
        runinfo.append(fri)
        for ch, df in ftables.items():
            tables[ch].append(df)
    rows = {}
    for ch, dfs in tables.items():
        path = os.path.join(outdir, dataset, ch, 'part-%05d.parquet' % part)
        if not dfs:
            rows[ch] = 0
            if os.path.exists(path):   # stale part of an interrupted earlier run with the same number
                os.remove(path)
            continue
        df = pd.concat(dfs, ignore_index=True)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        pq.write_table(pa.Table.from_pandas(df, preserve_index=False), path + '.tmp', compression=codec)
        os.replace(path + '.tmp', path)   # never leave a half-written part
        rows[ch] = len(df)
    return dict(dataset=dataset, part=part, files=good, bad=bad, runinfo=runinfo, rows=rows,
                seconds=time.time() - t0)


def fingerprint(channels, preselection, drop):
    """Hash of the settings that define the content of the tables: parts with different settings must not be mixed."""
    key = json.dumps({'channels': sorted(channels), 'preselection': preselection or '', 'drop': sorted(drop)},
                     sort_keys=True)
    return hashlib.sha1(key.encode()).hexdigest()[:12]


def load_manifest(path):
    if os.path.exists(path):
        with open(path) as f:
            return json.load(f)
    return {'parts': {}, 'settings': None}


def save_json(obj, path):
    os.makedirs(os.path.dirname(path), exist_ok=True)   # also when no chunk of the dataset had rows yet
    with open(path + '.tmp', 'w') as f:
        json.dump(obj, f, indent=1)
    os.replace(path + '.tmp', path)


def make_meta(manifest, isMC, label):
    meta = {'isMC': isMC, 'label': label, 'nFiles': 0, 'rows': {}}
    for k in RUNINFO_SUM:
        meta[k] = 0
    wsum = {k: 0. for k in RUNINFO_MEAN}
    wnorm = {k: 0. for k in RUNINFO_MEAN}
    for part in manifest['parts'].values():
        meta['nFiles'] += len(part['files'])
        for ch, n in part['rows'].items():
            meta['rows'][ch] = meta['rows'].get(ch, 0) + n
        for ri in part['runinfo']:
            for k in RUNINFO_SUM:
                meta[k] += int(ri.get(k, 0))
            for k in RUNINFO_MEAN:
                if ri.get(k, -1) > 0:
                    wsum[k] += ri[k] * ri.get('nEvents', 1)
                    wnorm[k] += ri.get('nEvents', 1)
    for k in RUNINFO_MEAN:
        meta[k] = wsum[k] / wnorm[k] if wnorm[k] > 0 else -1.
    return meta


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--datasets', nargs='*', help='only these datasets of the config')
    ap.add_argument('--workers', type=int, help='parallel processes (default: config, else number of CPUs)')
    ap.add_argument('--dry-run', action='store_true', help='only list what would be converted')
    args = ap.parse_args()
    with open(args.config) as f:
        cfg = yaml.safe_load(f)

    outdir = os.path.expanduser(cfg['output'])
    channels = cfg.get('channels', list(CHANNELS))
    unknown = set(channels) - set(CHANNELS)
    if unknown:
        sys.exit('unknown channels %s (known: %s)' % (sorted(unknown), list(CHANNELS)))
    maxChunk = int(cfg.get('chunk_files', 200))
    workers = args.workers or cfg.get('workers') or os.cpu_count()
    codec = parquet_codec(cfg.get('compression', 'zstd'))
    preselection = cfg.get('preselection') or None
    drop = cfg.get('drop_columns') or []
    datasets = cfg['datasets']
    if args.datasets:
        missing = set(args.datasets) - set(datasets)
        if missing:
            sys.exit('datasets not in the config: %s' % sorted(missing))
        datasets = {k: v for k, v in datasets.items() if k in args.datasets}

    settings = {'fingerprint': fingerprint(channels, preselection, drop), 'channels': sorted(channels),
                'preselection': preselection, 'drop_columns': list(drop)}
    todo, manifests = {}, {}
    for name, ds in datasets.items():
        mpath = os.path.join(outdir, name, 'manifest.json')
        manifest = manifests[name] = load_manifest(mpath)
        old = manifest.get('settings')
        if manifest['parts'] and old is None:
            print('WARNING: %s: manifest without conversion settings (older convert.py); assuming the current ones' % name)
        elif manifest['parts'] and old['fingerprint'] != settings['fingerprint']:
            sys.exit('%s was converted with different settings (%s), now %s.\n'
                     'Delete %s to convert it again, or restore the old channels/preselection/drop_columns.'
                     % (name, old, settings, os.path.join(outdir, name)))
        manifest['settings'] = settings
        done = {f for p in manifest['parts'].values() for f in p['files']}   # unreadable files are retried
        todo[name] = [f for f in file_list(ds['files']) if f not in done]
        print('%-40s %6d files to convert (%d already converted)' % (name, len(todo[name]), len(done)))
    # chunk size: at most chunk_files, smaller if needed to give every worker something to do
    nTodo = sum(len(v) for v in todo.values())
    chunk = max(1, min(maxChunk, -(-nTodo // workers)))
    jobs = []
    for name, files in todo.items():
        nextPart = max((int(k) for k in manifests[name]['parts']), default=-1) + 1
        for i in range(0, len(files), chunk):
            jobs.append((name, nextPart + i // chunk, files[i:i + chunk]))
    if args.dry_run:
        print('%d chunks to convert (%d files per chunk), Parquet compression %s' % (len(jobs), chunk, codec))
        return

    os.makedirs(outdir, exist_ok=True)
    t0 = time.time()
    nbad = 0
    with ProcessPoolExecutor(max_workers=max(1, min(workers, len(jobs) or 1))) as pool:
        futures = {pool.submit(convert_chunk, name, part, files, outdir, channels, preselection, drop, codec):
                   (name, part, files) for name, part, files in jobs}
        for n, fut in enumerate(as_completed(futures), 1):
            try:
                r = fut.result()
            except Exception as e:   # noqa: BLE001 - not in the manifest: retried at the next run
                name, part, files = futures[fut]
                print('[%d/%d] %s part %d FAILED (%d files, retried at the next run): %s'
                      % (n, len(jobs), name, part, len(files), e), flush=True)
                nbad += len(files)
                continue
            name = r['dataset']
            manifests[name]['parts'][str(r['part'])] = {k: r[k] for k in ('files', 'bad', 'runinfo', 'rows')}
            # the manifest is written after every chunk: an interrupted conversion can be resumed
            save_json(manifests[name], os.path.join(outdir, name, 'manifest.json'))
            nbad += len(r['bad'])
            for b in r['bad']:
                print('  BAD FILE %s' % b)
            print('[%d/%d] %s part %d: %d files, rows %s, %.0f s'
                  % (n, len(jobs), name, r['part'], len(r['files']), r['rows'], r['seconds']), flush=True)
    for name in manifests:
        os.makedirs(os.path.join(outdir, name), exist_ok=True)
        save_json(make_meta(manifests[name], bool(datasets[name].get('isMC', False)), datasets[name].get('label', name)),
                  os.path.join(outdir, name, 'meta.json'))
    print('done in %.0f s; %d unreadable files (listed in the manifests; retried at the next run)' % (time.time() - t0, nbad))


if __name__ == '__main__':
    main()
