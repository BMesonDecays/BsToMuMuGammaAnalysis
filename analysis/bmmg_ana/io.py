"""Loading of the candidate tables written by convert.py."""
import glob
import json
import os
import re

import pandas as pd
import pyarrow.dataset as pads

KEYS = ['evt_run', 'evt_lumi', 'evt_event']


def parts(base, dataset, channel):
    return sorted(glob.glob(os.path.join(os.path.expanduser(base), dataset, channel, 'part-*.parquet')))


def meta(base, dataset):
    """meta.json of a dataset (nEvents, nEventsStored, xsec, filterEff, isMC, label, rows)."""
    with open(os.path.join(os.path.expanduser(base), dataset, 'meta.json')) as f:
        return json.load(f)


def columns(base, dataset, channel):
    """Column names of a dataset/channel."""
    files = parts(base, dataset, channel)
    return pads.dataset(files, format='parquet').schema.names if files else []


def _referenced(expr, available):
    """Column names used in a pandas expression."""
    return {w for w in re.findall(r'[A-Za-z_][A-Za-z0-9_]*', expr or '') if w in available}


def load(base, datasets, channel, cols=None, query=None, derived=None, keys=True, exprs=()):
    """Candidates of one channel for one or more datasets as a pandas DataFrame.

    base: output directory of convert.py; datasets: name or list of names.
    cols: columns to load (None: all). Columns used by `query` and `derived` are loaded automatically.
    query: pandas query applied after loading (e.g. 'vtxProb > 0.01 and pho_pt > 2').
    derived: {name: pandas expression} of extra columns (computed before the query), e.g. {'lxySig': 'lxy / lxyErr'}.
    keys: also load the event ids (evt_run, evt_lumi, evt_event) and cand_idx.
    exprs: more expressions (e.g. cuts applied later) whose columns are loaded too.
    Adds the column 'dataset' (categorical). Columns missing in a dataset (e.g. gen_* in data) are NaN.
    """
    if isinstance(datasets, str):
        datasets = [datasets]
    derived = derived or {}
    frames = []
    for ds in datasets:
        files = parts(base, ds, channel)
        if not files:
            continue
        dset = pads.dataset(files, format='parquet')
        available = set(dset.schema.names)
        if cols is None:
            want = list(dset.schema.names)
        else:
            want = list(dict.fromkeys(list(cols) + (KEYS + ['cand_idx'] if keys else [])))
            for expr in list(derived.values()) + [query] + list(exprs):
                want += sorted(_referenced(expr, available) - set(want))
        present = [c for c in want if c in available]
        df = dset.to_table(columns=present).to_pandas()
        for name, expr in derived.items():
            df[name] = df.eval(expr)
        if query:
            df = df.query(query).copy()
        df['dataset'] = ds
        frames.append(df)
    if not frames:
        raise FileNotFoundError('no Parquet files for channel %s of %s under %s' % (channel, datasets, base))
    df = pd.concat(frames, ignore_index=True)
    df['dataset'] = df['dataset'].astype('category')
    return df
