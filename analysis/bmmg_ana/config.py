"""Analysis configuration (config/analysis.yaml) and the samples it defines."""
import os

import yaml


def _read_yaml(path):
    """A configuration file, with its base configuration (key "base", a path relative to the file) merged in.

    The keys of the file replace those of the base; dictionaries are merged recursively, and a key set to null
    in a merged dictionary removes the base entry (e.g. preselection: {vertex: null} drops the vertex cut).
    Lists and other values replace the base value. Bases can have bases.
    """
    with open(path) as f:
        cfg = yaml.safe_load(f) or {}
    base = cfg.pop('base', None)
    if base is None:
        return cfg
    if not os.path.isabs(base):
        base = os.path.join(os.path.dirname(os.path.abspath(path)), base)
    return merge(_read_yaml(base), cfg)


def merge(base, over):
    """Recursive merge of configuration dictionaries (see _read_yaml)."""
    out = dict(base)
    for k, v in over.items():
        if isinstance(v, dict) and isinstance(out.get(k), dict):
            out[k] = {kk: vv for kk, vv in merge(out[k], v).items() if vv is not None}
        else:
            out[k] = v
    return out


def load_config(path):
    cfg = _read_yaml(path)
    for key in ('parquet', 'output'):
        cfg[key] = os.path.expanduser(cfg[key])
    cfg.setdefault('derived', {})
    cfg.setdefault('preselection', {})
    cfg.setdefault('best_candidate', None)
    check_windows(cfg['mass'])
    return cfg


def load_combine_config(path):
    """Configuration of the Combine significance scripts (e.g. config/combine_mmg.yaml).

    The file names a base analysis configuration ("analysis", relative to the file) whose settings are used as
    they are (preselection, features, BDT, windows); "override" replaces top-level keys of the base (e.g.
    parquet, output). The other keys of the file are returned under cfg['combine'].
    """
    with open(path) as f:
        cc = yaml.safe_load(f)
    base = cc.pop('analysis')
    if not os.path.isabs(base):
        base = os.path.join(os.path.dirname(os.path.abspath(path)), base)
    cfg = _read_yaml(base)
    cfg.update(cc.pop('override', None) or {})
    for key in ('parquet', 'output'):
        cfg[key] = os.path.expanduser(cfg[key])
    cfg.setdefault('derived', {})
    cfg.setdefault('preselection', {})
    cfg.setdefault('best_candidate', None)
    check_windows(cfg['mass'])
    cfg['combine'] = cc
    lumi = cc['lumi']
    # Run 3 projection: data x target/recorded (recorded luminosities); the signal MC is normalised to the good
    # (golden-JSON) luminosity equivalent to the target, golden x target/recorded
    cc['data_factor'] = lumi['target_recorded_pb'] / lumi['data_recorded_pb']
    cc['signal_lumi_pb'] = lumi['data_golden_pb'] * cc['data_factor']
    cfg['lumi_pb'] = cc['signal_lumi_pb']
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
    'signal': signal MC candidates passing samples.signal.query (truth label, applied after the
              best-candidate choice, as in data); weight = MC normalisation to lumi_pb
              (1 per candidate if lumi_pb is not set).
    """
    # imported here: the configuration functions also work without pyarrow (e.g. in a CMSSW area)
    from . import io, selection, weights
    mcol = cfg['mass']['column']
    cols = list(dict.fromkeys([mcol] + list(columns) + list(cfg.get('bdt', {}).get('features', []))
                              + list(cfg.get('mlp', {}).get('features', []))
                              + ([cfg['best_candidate']] if cfg['best_candidate'] else [])))
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
        truth = s.get('query')
        frames = []
        for ds in s['datasets']:
            df = io.load(cfg['parquet'], ds, cfg['channel'], base_cols, query=presel, derived=cfg['derived'],
                         exprs=[truth] if truth else ())
            # same candidate policy as data: rank all preselected candidates, then apply the truth label
            if cfg['best_candidate']:
                df = selection.best_candidate(df, cfg['best_candidate'])
            if truth:
                df = df.query(truth).copy()
            if cfg.get('lumi_pb'):
                df['weight'] = weights.mc_scale(io.meta(cfg['parquet'], ds), cfg['lumi_pb'], s.get('xsec_pb'),
                                                s.get('filter_eff'), s.get('br_scale', 1.0))
            else:
                df['weight'] = 1.0
            frames.append(df)
        import pandas as pd
        out['signal'] = pd.concat(frames, ignore_index=True)
    return out
