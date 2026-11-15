#!/usr/bin/env python3
"""Mass-sculpting test of the BDT: can it find the B mass from its inputs?

    python3 scripts/check_sculpting.py config/analysis.yaml [--inner 0.2] [--device cuda]

The BDT is trained on the sidebands without their inner strips (the 'inner' GeV next to the blind window),
with the features and parameters of the bdt block. Its efficiency on the inner strips, which it has never
seen, is compared with that on the adjacent outer strips of the same width, at several background
efficiencies. If the BDT had learnt the mass, it would select the unseen masses near the signal window
more often (ratio > 1); a ratio compatible with 1 means no sign of sculpting. The background inside the
signal window is extrapolated from the sidebands, so sculpting would bias it.
"""
import argparse
import os
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import bdt, config, selection  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--inner', type=float, default=0.2, help='width of the inner strips [GeV]')
    ap.add_argument('--effs', type=float, nargs='+', default=[1e-2, 1e-3, 3e-4])
    ap.add_argument('--device', default=None)
    args = ap.parse_args()
    cfg = config.load_config(args.config)
    bcfg = cfg['bdt']
    features = bcfg['features']
    mcol = cfg['mass']['column']
    params = dict(bcfg.get('params') or {})
    if args.device:
        params['device'] = args.device
    samples = config.load_samples(cfg, which=('signal', 'background'))
    sig, bkg = samples['signal'], samples['background']
    blo, bhi = cfg['mass']['blind']
    w = args.inner
    inner_w = [[blo - w, blo], [bhi, bhi + w]]
    near_w = [[blo - 2 * w, blo - w], [bhi + w, bhi + 2 * w]]
    for lo, hi in inner_w + near_w:
        if not any(slo <= lo and hi <= shi for slo, shi in cfg['mass']['sidebands']):
            sys.exit('strip [%g, %g] is not inside the sidebands; use a smaller --inner' % (lo, hi))
    inner = np.asarray(selection.in_windows(bkg[mcol], inner_w))
    outer = bkg[~inner]
    print('training on %d signal and %d outer-sideband candidates; inner strips %s: %d candidates'
          % (len(sig), len(outer), inner_w, inner.sum()))
    models, info = bdt.train_kfold(sig, outer, features, k=bcfg.get('kfold', 5), params=params,
                                   early_stopping=bcfg.get('early_stopping', 20))
    sc_outer = info['oof']['score'].to_numpy()[len(sig):]
    sc_inner = bdt.predict(models, bkg[inner], features)
    near = np.asarray(selection.in_windows(outer[mcol], near_w))
    print('\n%-10s %-22s %-22s %s' % ('bkg eff', 'inner strips (unseen)', 'adjacent strips %s' % near_w, 'ratio inner/adjacent'))
    for e in args.effs:
        thr = np.quantile(sc_outer, 1 - e)
        ni, nn = int((sc_inner > thr).sum()), int((sc_outer[near] > thr).sum())
        ei, en = ni / inner.sum(), nn / near.sum()
        r = ei / en if en > 0 else np.nan
        dr = r * np.sqrt(1 / max(ni, 1) + 1 / max(nn, 1))
        print('%-10.0e %6d (eff %.2e)      %6d (eff %.2e)      %.2f +- %.2f' % (e, ni, ei, nn, en, r, dr))


if __name__ == '__main__':
    main()
