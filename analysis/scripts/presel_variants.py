#!/usr/bin/env python3
"""Compare preselection variants (resonance-veto windows, vertex probability cut) by the expected significance.

    python3 scripts/presel_variants.py config/combine_mmg.yaml [--retrain NAME ...] [--variants NAME ...]

The candidates are loaded once with the loosest selection of all variants (the tables already require
vtxProb > 1e-3), every variant is a pandas query on them. Per variant, after the BDT:
  thresholds t (1 - t log-spaced from 1e-1 to 1e-4): S = signal MC in the signal window (Run 3 projection of the
  Combine configuration), B = data sidebands above t, x the exponential sideband -> window factor fitted at that t,
  x target/recorded luminosity; Z = Asimov significance with known background, only where >= --min-sb raw
  sideband candidates are left.
Scores: by default the BDT of the Combine study (<output>/bdt, trained with the baseline preselection; its fold
models score the candidates of the events they did not see, so the scores stay out-of-fold also for the new
candidates). --retrain NAME: train a BDT on the candidates of that variant (<output>/presel_variants/<NAME>/bdt,
reused if it exists) and score that variant with it.
Output: <output>/presel_variants/summary.csv, scan_<name>.csv, z_vs_s.png.
"""
import argparse
import os
import sys

import numpy as np
import pandas as pd

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import bdt, config, io, selection, significance, weights  # noqa: E402

TRIGGER = 'mu1_trg_DoubleMu4_3_LowMass == 1 and mu2_trg_DoubleMu4_3_LowMass == 1'
RHO = '(dimu_mass < 0.70 or dimu_mass > 0.85)'
PHI = (0.97, 1.07)


def veto(jpsi, psi2s, phi=PHI):
    return ' and '.join('not (dimu_mass > %g and dimu_mass < %g)' % w for w in (phi, jpsi, psi2s))


def make_variants():
    v = {}
    def add(name, jpsi=(2.9, 3.3), psi2s=(3.55, 3.8), vtx=0.01):
        v[name] = dict(query='vtxProb > %g and %s and %s and %s' % (vtx, veto(jpsi, psi2s), RHO, TRIGGER),
                       jpsi=jpsi, psi2s=psi2s, vtx=vtx)
    add('baseline')                                   # = analysis.yaml (dimu_resonance == 0)
    add('vtx_3e-3', vtx=3e-3)
    add('vtx_1e-3', vtx=1e-3)
    add('jpsi_2.95-3.20', jpsi=(2.95, 3.20))
    add('jpsi_3.00-3.20', jpsi=(3.00, 3.20))
    add('jpsi_3.02-3.17', jpsi=(3.02, 3.17))
    add('jpsi_3.04-3.15', jpsi=(3.04, 3.15))
    add('jpsi_3.02-3.17_psi_3.62-3.74', jpsi=(3.02, 3.17), psi2s=(3.62, 3.74))
    # asymmetric: the radiative tail below the J/psi stays vetoed
    add('jpsi_2.90-3.20', jpsi=(2.90, 3.20))
    add('jpsi_2.90-3.15', jpsi=(2.90, 3.15))
    add('psi_3.55-3.75', psi2s=(3.55, 3.75))
    add('jpsi_2.90-3.20_psi_3.55-3.75', jpsi=(2.90, 3.20), psi2s=(3.55, 3.75))
    add('jpsi_2.90-3.20_vtx_1e-3', jpsi=(2.90, 3.20), vtx=1e-3)
    add('jpsi_2.90-3.20_psi_3.55-3.75_vtx_1e-3', jpsi=(2.90, 3.20), psi2s=(3.55, 3.75), vtx=1e-3)
    return v


def scan_variant(sig, bkg, cfg, factor, thresholds, min_sb):
    mcol = cfg['mass']['column']
    swlo, swhi = cfg['mass']['signal_window']
    sbs = cfg['mass']['sidebands']
    sw = sig[(sig[mcol] >= swlo) & (sig[mcol] < swhi)]
    rows = []
    for t in thresholds:
        s = sw.loc[sw['score'] > t, 'weight'].sum()
        m = bkg.loc[bkg['score'] > t, mcol].to_numpy()
        n = len(m)
        if n >= 2:
            f = significance.sideband_factor((swlo, swhi), sbs, 'exp', m)
        else:
            f = significance.sideband_factor((swlo, swhi), sbs, 'flat')
        b = n * factor * f
        z = float(significance.asimov_z(s, b)) if n >= min_sb and b > 0 else np.nan
        rows.append({'threshold': t, 'S': s, 'B': b, 'n_sb': n, 'Z': z})
    return pd.DataFrame(rows)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--variants', nargs='*', help='only these variants (default: all)')
    ap.add_argument('--retrain', nargs='*', default=[], help='variants scored with their own BDT')
    ap.add_argument('--min-sb', type=int, default=10, help='minimum raw sideband candidates for a Z value')
    ap.add_argument('--jobs', type=int, default=-1)
    ap.add_argument('--device', default='cpu', help='XGBoost device of the --retrain trainings (cpu, cuda)')
    args = ap.parse_args()
    cfg = config.load_combine_config(args.config)
    cc = cfg['combine']
    out = os.path.join(cfg['output'], 'presel_variants')
    os.makedirs(out, exist_ok=True)
    mcol = cfg['mass']['column']
    factor = cc['data_factor']
    variants = make_variants()
    if args.variants:
        variants = {k: variants[k] for k in args.variants}
    features = list(cfg['bdt']['features'])

    loose = '%s and %s' % (RHO, TRIGGER)
    cols = [mcol, 'dimu_mass', 'vtxProb'] + [f for f in features if f not in cfg['derived']]
    s = cfg['samples']['signal']
    frames = []
    for ds in s['datasets']:
        df = io.load(cfg['parquet'], ds, cfg['channel'], cols, query=loose, derived=cfg['derived'],
                     exprs=[s['query']])
        df = df.query(s['query']).copy()
        df['weight'] = weights.mc_scale(io.meta(cfg['parquet'], ds), cc['signal_lumi_pb'], s.get('xsec_pb'),
                                        s.get('filter_eff'), s.get('br_scale', 1.0))
        frames.append(df)
    sig = pd.concat(frames, ignore_index=True)
    data = io.load(cfg['parquet'], cfg['samples']['data']['datasets'], cfg['channel'], cols,
                   query=config.blind_query(cfg) + ' and ' + loose, derived=cfg['derived'])
    bkg = data[selection.in_windows(data[mcol], cfg['mass']['sidebands'])].copy()
    del data
    print('loose selection: %d signal, %d sideband candidates' % (len(sig), len(bkg)))

    base_models, base_feat = bdt.load(os.path.join(cfg['output'], 'bdt'))
    if list(base_feat) != features:
        sys.exit('the Combine-study BDT has other features than the configuration')
    sig['score_base'] = bdt.predict(base_models, sig, features)
    bkg['score_base'] = bdt.predict(base_models, bkg, features)

    thresholds = 1 - np.logspace(-1, -4, 61)
    rows, scans = [], {}
    for name, v in variants.items():
        vs, vb = sig.query(v['query']).copy(), bkg.query(v['query']).copy()
        tag = 'baseline BDT'
        if name in args.retrain:
            d = os.path.join(out, name, 'bdt')
            if os.path.exists(os.path.join(d, 'bdt.json')):
                models, _ = bdt.load(d)
            else:
                bcfg = cfg['bdt']
                params = dict(bcfg.get('params') or {}, device=args.device)
                print('training the BDT of %s on %d signal and %d sideband candidates' % (name, len(vs), len(vb)))
                models, info = bdt.train_kfold(vs, vb, features, params=params, k=bcfg.get('kfold', 5),
                                               n_jobs=args.jobs, early_stopping=bcfg.get('early_stopping', 20))
                bdt.save(models, features, d, extra={'variant': name, 'query': v['query']})
            vs['score'] = bdt.predict(models, vs, features)
            vb['score'] = bdt.predict(models, vb, features)
            tag = 'own BDT'
        else:
            vs['score'], vb['score'] = vs['score_base'], vb['score_base']
        sc = scan_variant(vs, vb, cfg, factor, thresholds, args.min_sb)
        key = name + ('' if tag == 'baseline BDT' else ' (own BDT)')
        scans[key] = sc
        sc.to_csv(os.path.join(out, 'scan_%s.csv' % key.replace(' ', '_').replace('(', '').replace(')', '')),
                  index=False)
        swlo, swhi = cfg['mass']['signal_window']
        s_pre = vs.loc[(vs[mcol] >= swlo) & (vs[mcol] < swhi), 'weight'].sum()
        b = sc.loc[sc['Z'].idxmax()] if sc['Z'].notna().any() else None
        row = {'variant': key, 'S presel': s_pre, 'sideband presel': len(vb)}
        for t in (0.99, 0.995, 0.998):
            r = scan_variant(vs, vb, cfg, factor, [t], args.min_sb).iloc[0]
            row['Z@%g' % t] = r['Z']
            row['S@%g' % t] = r['S']
            row['nsb@%g' % t] = r['n_sb']
        if b is not None:
            row.update({'Z best': b['Z'], 't best': b['threshold'], 'S best': b['S'], 'B best': b['B'],
                        'nsb best': b['n_sb']})
        rows.append(row)
        print('%-36s S presel %6.1f  sidebands %7d  Z best %.3f' % (key, s_pre, len(vb), row.get('Z best', np.nan)))

    summ = pd.DataFrame(rows).set_index('variant')
    summ.to_csv(os.path.join(out, 'summary.csv'))
    pd.set_option('display.width', 250)
    print('\n' + summ.to_string(float_format=lambda x: '%.4g' % x))

    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(figsize=(8, 5.5))
    for key, sc in scans.items():
        ax.plot(sc['S'], sc['Z'], marker='.', label=key)
    ax.set_xlabel('S in the signal window (Run 3)')
    ax.set_ylabel('Asimov Z (exponential background, known)')
    ax.grid(alpha=0.3)
    ax.legend(fontsize=7)
    fig.tight_layout()
    fig.savefig(os.path.join(out, 'z_vs_s.png'), dpi=120)
    print('outputs in %s' % out)


if __name__ == '__main__':
    main()
