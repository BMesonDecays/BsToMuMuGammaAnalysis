#!/usr/bin/env python3
"""Impact of each preselection cut on the signal MC and on the data sidebands, projected to Run 3.

    python3 scripts/presel_impact.py config/combine_mmg.yaml [--extra name=query ...]

Uses the Run 3 projection of a Combine configuration (config.load_combine_config: data x target/recorded
luminosity, signal MC at the equivalent good luminosity, B = br_scale). Tables (printed and written to
<output>/presel/):
  cutflow.csv   cumulative: the ntuple candidates, the event-level trigger flag (information only, implied by
                the trigger matching of the muons), then the preselection cuts in order, then the --extra cuts
                (default: the commented-out options of analysis.yaml);
  nminus1.csv   each preselection cut alone, and removed from the full preselection (N-1).
Columns: signal = gen-matched candidates (expected events, in [4.5, 6.5] GeV and in the signal window), signal
events (one per event), all signal-MC candidates in the signal window (with the combinatorial ones), the data
sideband candidates (raw and scaled to Run 3) and S / sqrt(B) with B = the sidebands scaled to the signal
window width (flat). The blind window of the data is never loaded.
The 'ntuple' row is what the tables hold: the v1 tables keep only candidates with fitOK == 1 and vtxProb > 1e-3
(convert.yaml preselection; the ntuplizer skim, skimMinVtxProb = 1e-3, is per event and weaker). The signal
candidates below 1e-3 (13-18% of the gen-matched ones, report.md 2026-11-22) are missing from the first rows;
the rows from the vertex cut (vtxProb > 0.01) on are not affected.
"""
import argparse
import os
import sys

import numpy as np
import pandas as pd

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import config, io, selection, weights  # noqa: E402

EXTRA = {'soft muons': 'mu1_isSoft == 1 and mu2_isSoft == 1', 'muon pt 4/3': 'mu1_pt > 4 and mu2_pt > 3'}


def counts(sig, bkg, cfg, factor, sb_to_window):
    mcol = cfg['mass']['column']
    lo, hi = cfg['combine']['mass_fit']['range']
    swlo, swhi = cfg['mass']['signal_window']
    t = sig[sig['genMatched'] == 1]
    in_range = t[(t[mcol] >= lo) & (t[mcol] < hi)]
    in_win = t[(t[mcol] >= swlo) & (t[mcol] < swhi)]
    allwin = sig[(sig[mcol] >= swlo) & (sig[mcol] < swhi)]
    s = in_win['weight'].sum()
    b = len(bkg) * factor * sb_to_window
    return {'S [4.5,6.5]': in_range['weight'].sum(), 'S window': s,
            'S events': in_range.drop_duplicates(selection.event_keys(in_range))['weight'].sum(),
            'MC cand window (all)': allwin['weight'].sum(), 'sideband raw': len(bkg),
            'sideband Run3': len(bkg) * factor, 'S/sqrt(B)': s / np.sqrt(b) if b > 0 else np.nan}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--extra', nargs='*', help='extra cumulative cuts after the preselection, name=query')
    args = ap.parse_args()
    cfg = config.load_combine_config(args.config)
    cc = cfg['combine']
    out = os.path.join(cfg['output'], 'presel')
    os.makedirs(out, exist_ok=True)
    mcol = cfg['mass']['column']
    presel = dict(cfg['preselection'])
    extra = dict(e.split('=', 1) for e in args.extra) if args.extra else EXTRA
    trig = {'event trigger': 'trg_DoubleMu4_3_LowMass == 1'}
    exprs = list(presel.values()) + list(extra.values()) + list(trig.values()) + ['genMatched == 1']
    factor = cc['data_factor']
    sb = cfg['mass']['sidebands']
    sb_width = sum(h - l for l, h in sb)
    swlo, swhi = cfg['mass']['signal_window']
    sb_to_window = (swhi - swlo) / sb_width

    s = cfg['samples']['signal']
    frames = []
    for ds in s['datasets']:
        df = io.load(cfg['parquet'], ds, cfg['channel'], [mcol], derived=cfg['derived'], exprs=exprs)
        meta = io.meta(cfg['parquet'], ds)
        df['weight'] = weights.mc_scale(meta, cc['signal_lumi_pb'], s.get('xsec_pb'), s.get('filter_eff'),
                                        s.get('br_scale', 1.0))
        produced = meta['xsec'] * meta['filterEff'] * s.get('br_scale', 1.0) * cc['signal_lumi_pb']
        print('%s: %d processed events, %.0f signal decays (gen filter) expected in Run 3'
              % (ds, meta['nEvents'], produced))
        frames.append(df)
    sig = pd.concat(frames, ignore_index=True)
    data = io.load(cfg['parquet'], cfg['samples']['data']['datasets'], cfg['channel'], [mcol],
                   derived=cfg['derived'], query=config.blind_query(cfg), exprs=exprs)
    data = data[selection.in_windows(data[mcol], sb)]
    print('signal MC: %d candidates; data sidebands: %d candidates (before the preselection)' % (len(sig), len(data)))

    # cumulative
    rows = [dict(cut='tables (vtxProb > 1e-3)', **counts(sig, data, cfg, factor, sb_to_window))]
    cs, cb = sig, data
    for name, q in list(trig.items()) + list(presel.items()) + list(extra.items()):
        cs, cb = cs.query(q), cb.query(q)
        tag = name if name in presel else name + (' (info)' if name in trig else ' (extra)')
        rows.append(dict(cut=tag, **counts(cs, cb, cfg, factor, sb_to_window)))
    cf = pd.DataFrame(rows).set_index('cut')
    cf['S eff step'] = cf['S window'] / cf['S window'].shift(1)
    cf['B eff step'] = cf['sideband raw'] / cf['sideband raw'].shift(1)

    # alone and N-1
    full = config.preselection_query(cfg)
    rows = []
    base = counts(sig, data, cfg, factor, sb_to_window)
    allp = counts(sig.query(full), data.query(full), cfg, factor, sb_to_window)
    for name, q in presel.items():
        a = counts(sig.query(q), data.query(q), cfg, factor, sb_to_window)
        others = [v for k, v in presel.items() if k != name]
        oq = ' and '.join('(%s)' % v for v in others) if others else 'index == index'
        n1 = counts(sig.query(oq), data.query(oq), cfg, factor, sb_to_window)
        rows.append({'cut': name,
                     'S eff alone': a['S window'] / base['S window'], 'B eff alone': a['sideband raw'] / base['sideband raw'],
                     'S eff N-1': allp['S window'] / n1['S window'], 'B eff N-1': allp['sideband raw'] / n1['sideband raw'],
                     'S window without it': n1['S window'], 'S/sqrt(B) without it': n1['S/sqrt(B)']})
    n1 = pd.DataFrame(rows).set_index('cut')

    pd.set_option('display.width', 250)
    fmt = lambda v: '%.4g' % v  # noqa: E731
    print('\nRun 3 projection: signal at %.0f pb^-1, B = %g; data x %.3f. S window = [%g, %g] GeV, '
          'B = sidebands x %.3f (window / sideband width)'
          % (cc['signal_lumi_pb'], s.get('br_scale', 1.0), factor, swlo, swhi, sb_to_window))
    print('\n=== cumulative\n' + cf.to_string(float_format=fmt))
    print('\n=== each cut alone (w.r.t. the ntuple) and N-1 (w.r.t. the other preselection cuts); S in the window\n'
          + n1.to_string(float_format=fmt))
    print('\nfull preselection: S window %.1f, S/sqrt(B) %.3f' % (allp['S window'], allp['S/sqrt(B)']))
    cf.to_csv(os.path.join(out, 'cutflow.csv'))
    n1.to_csv(os.path.join(out, 'nminus1.csv'))
    print('tables in %s' % out)


if __name__ == '__main__':
    main()
