#!/usr/bin/env python3
"""BDT working-point scan: expected signal and background vs the BDT threshold, best figure of merit.

    python3 scripts/scan_wp.py config/analysis.yaml

S: signal MC (weighted, see lumi_pb) in the signal mass window. B: data in the sidebands, extrapolated
into the signal window with a flat or exponential shape fitted to the sideband mass distribution
(after the preselection, before the BDT cut). Scores are out-of-fold (models of train_bdt.py).
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import bdt, config, plotting, selection, significance  # noqa: E402

FOMS = ['punzi', 'asimov_z', 's_over_sqrt_b']


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    args = ap.parse_args()
    cfg = config.load_config(args.config)
    scfg = cfg.get('significance', {})
    mcol = cfg['mass']['column']
    outdir = os.path.join(cfg['output'], 'bdt')
    models, features = bdt.load(outdir)

    samples = config.load_samples(cfg, which=('signal', 'background'))
    sig, bkg = samples['signal'], samples['background']
    sig = sig[selection.in_windows(sig[mcol], [cfg['mass']['signal_window']])]
    sig_score = bdt.predict(models, sig, features)
    bkg_score = bdt.predict(models, bkg, features)

    model = scfg.get('background_model', 'exp')
    factor = significance.sideband_factor(cfg['mass']['signal_window'], cfg['mass']['sidebands'], model, bkg[mcol])
    print('signal candidates in the window: %d (weighted %.4g); sideband candidates: %d; '
          'sideband -> signal-window factor (%s): %.4f' % (len(sig), sig['weight'].sum(), len(bkg), model, factor))

    res = significance.scan(sig_score, sig['weight'], bkg_score, factor, punzi_a=scfg.get('punzi_a', 3),
                            min_sideband=scfg.get('min_sideband', 10))
    res.to_csv(os.path.join(outdir, 'scan.csv'), index=False)
    fom = scfg.get('fom', 'punzi')
    best = significance.best(res, fom)
    plotting.scan(res, FOMS, os.path.join(outdir, 'scan.png'), best_threshold=best['threshold'])
    print('\nbest working point (%s):' % fom)
    print(best.to_string(float_format=lambda x: '%.5g' % x))
    for other in FOMS:
        if other != fom and res[other].notna().any():
            b = significance.best(res, other)
            print('  (best %s at threshold %.4f: S=%.4g B=%.4g)' % (other, b['threshold'], b['S'], b['B']))
    print('\nscan table and plot in %s' % outdir)


if __name__ == '__main__':
    main()
