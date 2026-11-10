"""Flattening of the bmmg ntuple (one entry per event, vectors of objects and candidates)
into candidate-level tables (one row per B candidate).

For a channel with prefix <p> (e.g. mmg) the table has:
  - the candidate branches <p>_<var>          -> column <var>          (mass, vtxProb, mu1, pho, ...)
  - the objects referenced by the leg indices  -> column <leg>_<var>    (mu1_pt, pho_mva, dimu_mass, trk1_pt, ...)
  - the event-level scalars                    -> unchanged name        (evt_event, trg_..., pv_x, gen_B_mass, ...)
  - cand_idx (index of the candidate in the event) and n_cand (candidates of the channel in the event)
Missing objects (leg index < 0) give -999, as in the ntuple.
"""
import fnmatch

import awkward as ak
import numpy as np
import pandas as pd

MISSING = -999

# channel prefix -> {leg index branch: object collection prefix}
CHANNELS = {
    'mmg': {'mu1': 'mu', 'mu2': 'mu', 'dimu': 'dimu', 'pho': 'pho'},
    'mmcg': {'mu1': 'mu', 'mu2': 'mu', 'dimu': 'dimu', 'conv': 'conv'},
    'mmgg': {'mu1': 'mu', 'mu2': 'mu', 'dimu': 'dimu', 'pho1': 'pho', 'pho2': 'pho'},
    'mmkk': {'mu1': 'mu', 'mu2': 'mu', 'dimu': 'dimu', 'trk1': 'trk', 'trk2': 'trk'},
}
# event-level scalar branches copied to every candidate (jagged branches with these prefixes, e.g. gen_pho_*, are skipped)
EVENT_PREFIXES = ('evt_', 'bs_', 'pv_', 'trg_', 'gen_')


def collection_branches(fields, collection):
    """Branches of an object collection, e.g. 'mu' -> mu_pt, mu_eta, ... (not mmg_*, dimu_*)."""
    return [f for f in fields if f.startswith(collection + '_')]


def needed_branches(fields, channels):
    """Branches to read for the given channels (from the list of branches of a file)."""
    keep = set()
    for ch in channels:
        if ch + '_mass' not in fields:
            continue
        keep.update(f for f in fields if f.startswith(ch + '_'))
        for coll in set(CHANNELS[ch].values()):
            keep.update(collection_branches(fields, coll))
    keep.update(f for f in fields if f.startswith(EVENT_PREFIXES))
    return sorted(keep)


def _flat(x):
    return ak.to_numpy(ak.flatten(x, axis=1))


def build_table(arrays, channel, preselection=None, drop=()):
    """Candidate table (pandas DataFrame) of one channel from the arrays of one file (or chunk).

    arrays: awkward record array with the ntuple branches (one entry per event).
    preselection: pandas query string on the table columns, e.g. 'fitOK == 1 and vtxProb > 1e-3'.
    drop: glob patterns of columns not to store.
    Returns None if the channel is not in the arrays.
    """
    fields = set(arrays.fields)
    p = channel + '_'
    if p + 'mass' not in fields:
        return None
    template = arrays[p + 'mass']
    cols = {}
    # candidate branches
    for f in sorted(fields):
        if f.startswith(p):
            cols[f[len(p):]] = _flat(arrays[f])
    cols['cand_idx'] = _flat(ak.local_index(template, axis=1)).astype(np.int32)
    cols['n_cand'] = _flat(ak.broadcast_arrays(ak.num(template, axis=1), template)[0]).astype(np.int32)
    # objects through the leg indices
    for leg, coll in CHANNELS[channel].items():
        idx = arrays[p + leg]
        valid = _flat(idx >= 0)
        safe = ak.where(idx >= 0, idx, 0)
        for f in collection_branches(fields, coll):
            obj = arrays[f]
            if obj.ndim != 2:   # only per-object vectors
                continue
            gathered = _flat(obj[safe])
            # keep the branch type (int32 / float32); missing objects -> -999
            cols['%s_%s' % (leg, f[len(coll) + 1:])] = np.where(valid, gathered, MISSING).astype(gathered.dtype)
    # event-level scalars
    for f in sorted(fields):
        if f.startswith(EVENT_PREFIXES) and arrays[f].ndim == 1:
            cols[f] = _flat(ak.broadcast_arrays(arrays[f], template)[0])
    df = pd.DataFrame(cols)
    if preselection:
        df = df.query(preselection).copy()
    if drop:
        dropped = [c for c in df.columns if any(fnmatch.fnmatch(c, pat) for pat in drop)]
        protected = {'evt_run', 'evt_lumi', 'evt_event', 'cand_idx', 'n_cand'}
        df = df.drop(columns=[c for c in dropped if c not in protected])
    return df.reset_index(drop=True)
