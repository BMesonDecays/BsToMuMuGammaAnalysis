#!/usr/bin/env python3
"""Working-point scan: expected signal and background vs the BDT (or MLP) threshold, best figure of merit.

    python3 scripts/scan_wp.py config/analysis.yaml [--classifier mlp]

S: signal MC (weighted, see lumi_pb) in the signal mass window. B: data in the sidebands, extrapolated
into the signal window with a flat or exponential shape fitted to the sideband mass distribution
(after the preselection, before the classifier cut). Scores are out-of-fold (models of train_bdt.py,
or of train_mlp.py with --classifier mlp); the results are written next to the models.

With significance.projection_lumi_pb (and lumi_pb) set, S and B are scaled to that luminosity.
With significance.shape set, the expected significance of a binned mass fit in score categories is
computed too (significance.binned_asimov; categories optimised by significance.optimise_categories).
"""
import argparse
import os
import sys

import numpy as np
import pandas as pd

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import config, plotting, selection, significance  # noqa: E402

FOMS = ['punzi', 'asimov_z', 's_over_sqrt_b']


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--classifier', choices=['bdt', 'mlp'], default='bdt',
                    help='bdt (train_bdt.py) or mlp (train_mlp.py)')
    ap.add_argument('--cpu', action='store_true', help='mlp: hide the GPUs from TensorFlow')
    args = ap.parse_args()
    if args.cpu:
        os.environ['CUDA_VISIBLE_DEVICES'] = ''
    # import only the classifier in use (TensorFlow is slow to import)
    if args.classifier == 'mlp':
        from bmmg_ana import mlp as clf
    else:
        from bmmg_ana import bdt as clf
    cfg = config.load_config(args.config)
    scfg = cfg.get('significance', {})
    mcol = cfg['mass']['column']
    outdir = os.path.join(cfg['output'], args.classifier)
    models, features = clf.load(outdir)

    samples = config.load_samples(cfg, which=('signal', 'background'))
    sig, bkg = samples['signal'], samples['background']
    sig_all = sig
    sig = sig[selection.in_windows(sig[mcol], [cfg['mass']['signal_window']])]
    sig_score = clf.predict(models, sig, features)
    bkg_score = clf.predict(models, bkg, features)

    model = scfg.get('background_model', 'exp')
    factor = significance.sideband_factor(cfg['mass']['signal_window'], cfg['mass']['sidebands'], model, bkg[mcol])
    print('signal candidates in the window: %d (weighted %.4g); sideband candidates: %d; '
          'sideband -> signal-window factor (%s): %.4f' % (len(sig), sig['weight'].sum(), len(bkg), model, factor))

    scale = 1.0
    if scfg.get('projection_lumi_pb'):
        if not cfg.get('lumi_pb'):
            sys.exit('significance.projection_lumi_pb needs lumi_pb (the luminosity of the data)')
        scale = scfg['projection_lumi_pb'] / cfg['lumi_pb']
        print('S and B projected to %.4g fb^-1 (x %.4g)' % (scfg['projection_lumi_pb'] / 1e3, scale))
    res = significance.scan(sig_score, sig['weight'] * scale, bkg_score, factor * scale, punzi_a=scfg.get('punzi_a', 3),
                            min_sideband=scfg.get('min_sideband', 10))
    res.to_csv(os.path.join(outdir, 'scan.csv'), index=False)
    fom = scfg.get('fom', 'punzi')
    best = significance.best(res, fom)
    plotting.scan(res, FOMS, os.path.join(outdir, 'scan.png'), best_threshold=best['threshold'],
                  xlabel='%s score threshold' % args.classifier.upper())
    print('\nbest working point (%s):' % fom)
    print(best.to_string(float_format=lambda x: '%.5g' % x))
    for other in FOMS:
        if other != fom and res[other].notna().any():
            b = significance.best(res, other)
            print('  (best %s at threshold %.4f: S=%.4g B=%.4g)' % (other, b['threshold'], b['S'], b['B']))
    if scfg.get('shape'):
        shape_fit(cfg, scfg, sig_all, clf.predict(models, sig_all, features), bkg, bkg_score, scale, outdir)
    print('\nscan table and plot in %s' % outdir)


def shape_fit(cfg, scfg, sig, sig_score, bkg, bkg_score, scale, outdir):
    """Expected significance of a binned mass fit in score categories (boundaries optimised on a grid)."""
    mcol = cfg['mass']['column']
    sh = scfg['shape']
    lo, hi, width = sh.get('mass_bins', [cfg['mass']['blind'][0], cfg['mass']['blind'][1], 0.025])
    edges = np.linspace(lo, hi, int(round((hi - lo) / width)) + 1)
    # exponential slope from the loosest sideband-efficiency slice (the tight categories have few candidates)
    slope_min = float(np.quantile(bkg_score, 1 - sh.get('slope_bkg_eff', 0.01)))
    kw = dict(sidebands=cfg['mass']['sidebands'], mass_bins=edges, lumi_scale=scale, slope_min_score=slope_min,
              min_sideband=sh.get('min_sideband', 10))
    if sh.get('eff_grid'):
        kw['eff_grid'] = sh['eff_grid']
    print('\nbinned fit of %s in [%g, %g] (%d bins), score categories from sideband efficiencies:' % (mcol, lo, hi, len(edges) - 1))
    rows = []
    for n in range(1, sh.get('max_categories', 3) + 1):
        r = significance.optimise_categories(sig[mcol], sig['weight'], sig_score, bkg[mcol], bkg_score, bkg['evt_event'],
                                             n_categories=n, **kw)
        z, effs, bounds, cats = r['full']
        zh, halves = r['honest']
        print('  %d categories: Z = %.3f, unbiased (boundaries from the other half of the sideband events) Z = %.3f (%s)'
              % (n, z, zh, ', '.join('%.3f' % x for x in halves)))
        print('    boundaries at sideband efficiency %s:\n%s' % (effs, cats.to_string(index=False, float_format=lambda x: '%.4g' % x)))
        rows.append(cats.assign(n_categories=n, Z_total=z, Z_unbiased=zh))
    pd.concat(rows).to_csv(os.path.join(outdir, 'shape_categories.csv'), index=False)


if __name__ == '__main__':
    main()
