"""Analysis configuration (config/analysis.yaml) and the samples it defines."""
import os

import yaml

from . import io, selection, weights


def load_config(path):
    with open(path) as f:
        cfg = yaml.safe_load(f)
    for key in ('parquet', 'output'):
        cfg[key] = os.path.expanduser(cfg[key])
    cfg.setdefault('derived', {})
    cfg.setdefault('preselection', {})
    cfg.setdefault('best_candidate', None)
    check_windows(cfg['mass'])
    return cfg


def check_windows(mass):
    """The sidebands must lie outside the blind window, and the signal window inside it."""
    blo, bhi = mass['blind']
    if not blo < bhi:
        raise ValueError('mass.blind must be [lo, hi] with lo < hi, not %s' % (mass['blind'],))
    for lo, hi in mass['sidebands']:
        if not lo < hi:
            raise ValueError('sideband [%s, %s]: lo must be below hi' % (lo, hi))
        if lo < bhi and hi > blo:
            raise ValueError('sideband [%s, %s] overlaps the blind window %s' % (lo, hi, mass['blind']))
    slo, shi = mass['signal_window']
    if not (blo <= slo < shi <= bhi):
        raise ValueError('signal window %s must lie inside the blind window %s' % (mass['signal_window'], mass['blind']))


def blind_query(cfg):
    """pandas query that keeps only data outside the blind window."""
    lo, hi = cfg['mass']['blind']
    m = cfg['mass']['column']
    return '(%s < %r or %s >= %r)' % (m, lo, m, hi)


def preselection_query(cfg):
    cuts = cfg['preselection']
    if not cuts:
        return None
    exprs = list(cuts.values()) if isinstance(cuts, dict) else ([cuts] if isinstance(cuts, str) else list(cuts))
    return ' and '.join('(%s)' % e for e in exprs)


def load_samples(cfg, columns=(), which=('signal', 'data')):
    """DataFrames of the samples after the preselection (and best-candidate choice), with a 'weight' column.

    'data': data candidates OUTSIDE the blind window. The best-candidate choice (if any) is made
            before the blinding, i.e. as it will be after unblinding; it must not use the mass.
    'background': data candidates in the mass sidebands.
    'signal': signal MC candidates passing samples.signal.query; weight = MC normalisation to lumi_pb
              (1 per candidate if lumi_pb is not set).
    """
    mcol = cfg['mass']['column']
    cols = list(dict.fromkeys([mcol] + list(columns) + list(cfg.get('bdt', {}).get('features', []))))
    # derived columns are computed in io.load; ask for their inputs, not for themselves
    cols = [c for c in cols if c not in cfg['derived']] + list(cfg['derived'])
    base_cols = [c for c in cols if c not in cfg['derived']]
    presel = preselection_query(cfg)
    out = {}
    if 'data' in which or 'background' in which:
        data = io.load(cfg['parquet'], cfg['samples']['data']['datasets'], cfg['channel'], base_cols,
                       query=presel, derived=cfg['derived'])
        if cfg['best_candidate']:
            data = selection.best_candidate(data, cfg['best_candidate'])
        data = data.query(blind_query(cfg)).copy()   # nothing in the blind window leaves this function
        data['weight'] = 1.0
        if 'data' in which:
            out['data'] = data
        if 'background' in which:
            out['background'] = data[selection.in_windows(data[mcol], cfg['mass']['sidebands'])].copy()
    if 'signal' in which:
        s = cfg['samples']['signal']
        query = ' and '.join('(%s)' % q for q in (presel, s.get('query')) if q)
        frames = []
        for ds in s['datasets']:
            df = io.load(cfg['parquet'], ds, cfg['channel'], base_cols, query=query or None, derived=cfg['derived'])
            if cfg['best_candidate']:
                df = selection.best_candidate(df, cfg['best_candidate'])
            if cfg.get('lumi_pb'):
                df['weight'] = weights.mc_scale(io.meta(cfg['parquet'], ds), cfg['lumi_pb'], s.get('xsec_pb'),
                                                s.get('filter_eff'), s.get('br_scale', 1.0))
            else:
                df['weight'] = 1.0
            frames.append(df)
        import pandas as pd
        out['signal'] = pd.concat(frames, ignore_index=True)
    return out
