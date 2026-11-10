"""Cuts, cut flows and best-candidate selection on candidate tables (pandas)."""
import numpy as np
import pandas as pd

from .io import KEYS


def apply(df, cuts):
    """Apply cuts: a pandas query string, or a list / dict of query strings (all are ANDed)."""
    if not cuts:
        return df
    if isinstance(cuts, str):
        return df.query(cuts)
    exprs = list(cuts.values()) if isinstance(cuts, dict) else list(cuts)
    return df.query(' and '.join('(%s)' % e for e in exprs))


def cutflow(df, cuts, weight=None, per_event=True):
    """Cut flow table for an ordered dict {name: query}.

    Columns: candidates, events (unique event ids), and the weighted sum if `weight` (column name)
    is given; efficiencies relative to the first row and to the previous cut.
    """
    rows = []
    cur = df
    rows.append(_counts('all', cur, weight, per_event))
    for name, expr in cuts.items():
        cur = cur.query(expr)
        rows.append(_counts(name, cur, weight, per_event))
    out = pd.DataFrame(rows).set_index('cut')
    base = 'weighted' if weight else ('events' if per_event else 'candidates')
    out['eff_total'] = out[base] / out[base].iloc[0] if out[base].iloc[0] else np.nan
    out['eff_step'] = out[base] / out[base].shift(1)
    return out


def _counts(name, df, weight, per_event):
    r = {'cut': name, 'candidates': len(df)}
    if per_event:
        r['events'] = len(df.drop_duplicates(event_keys(df))) if len(df) else 0
    if weight:
        r['weighted'] = float(df[weight].sum())
    return r


def event_keys(df):
    """Columns identifying an event (dataset + run, lumi, event)."""
    return (['dataset'] if 'dataset' in df.columns else []) + KEYS


def best_candidate(df, key='vtxProb', highest=True):
    """One candidate per event: the one with the highest (or lowest) value of `key`."""
    if df.empty:
        return df
    order = df.sort_values(key, ascending=not highest, kind='stable')
    return order.drop_duplicates(event_keys(df), keep='first').sort_index()


def in_windows(x, windows):
    """Mask: x inside any of the [lo, hi) windows."""
    x = np.asarray(x)
    m = np.zeros(len(x), dtype=bool)
    for lo, hi in windows:
        m |= (x >= lo) & (x < hi)
    return m
