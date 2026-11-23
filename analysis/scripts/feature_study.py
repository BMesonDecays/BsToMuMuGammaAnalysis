#!/usr/bin/env python3
"""Compare BDT feature sets: one k-fold training per set, out-of-fold figures of merit.

    python3 scripts/feature_study.py config/analysis_v3.yaml config/features_v3.yaml [--sets NAME ...]
        [--seeds 1 2] [--device cuda]

The feature-set file: {sets: {name: {base: other set | null, add: [...], remove: [...]}}}; a set without base
starts from bdt.features of the analysis configuration. The bdt block of the configuration gives the parameters
(device overridden by --device), k and early stopping; --seeds repeats every training with other XGBoost seeds
(the spread of the metrics is the training noise).
Metrics (as scripts/tune_bdt.py): signal efficiency at sideband efficiencies 5e-5, 2e-4, 1e-3; AUC; the best
cut-and-count Asimov Z (signal window, exponential background from the sidebands, projected to
significance.projection_lumi_pb); the top-10 features by gain.
Output: <output>/feature_study/results.csv (one row per set and seed), importance_<set>.csv.
"""
import argparse
import os
import sys
import time

import numpy as np
import pandas as pd
import yaml
from sklearn.metrics import roc_auc_score

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import bdt, config, selection, significance  # noqa: E402


def resolve(sets, name, default, seen=()):
    s = sets[name]
    if name in seen:
        raise ValueError('feature set %s: circular base' % name)
    base = resolve(sets, s['base'], default, seen + (name,)) if s.get('base') else list(default)
    rm = set(s.get('remove') or [])
    missing = rm - set(base)
    if missing:
        raise ValueError('feature set %s removes features not in its base: %s' % (name, sorted(missing)))
    out = [f for f in base if f not in rm]
    return out + [f for f in (s.get('add') or []) if f not in out]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('sets')
    ap.add_argument('--sets', dest='only', nargs='*', help='only these sets')
    ap.add_argument('--seeds', type=int, nargs='*', default=[1])
    ap.add_argument('--device', default='cuda')
    args = ap.parse_args()
    cfg = config.load_config(args.config)
    with open(args.sets) as f:
        sets = yaml.safe_load(f)['sets']
    bcfg, scfg = cfg['bdt'], cfg.get('significance', {})
    default = list(bcfg['features'])
    names = args.only or list(sets)
    feats = {n: resolve(sets, n, default) for n in names}
    allf = sorted({f for v in feats.values() for f in v})
    outdir = os.path.join(cfg['output'], 'feature_study')
    os.makedirs(outdir, exist_ok=True)
    mcol = cfg['mass']['column']

    samples = config.load_samples(cfg, columns=allf, which=('signal', 'background'))
    sig, bkg = samples['signal'], samples['background']
    for f in allf:
        if f not in sig.columns:
            sys.exit('feature %s is not a column' % f)
    win = np.asarray(selection.in_windows(sig[mcol], [cfg['mass']['signal_window']]))
    factor = significance.sideband_factor(cfg['mass']['signal_window'], cfg['mass']['sidebands'],
                                          scfg.get('background_model', 'exp'), bkg[mcol])
    scale = scfg['projection_lumi_pb'] / cfg['lumi_pb'] if scfg.get('projection_lumi_pb') and cfg.get('lumi_pb') else 1.0
    print('%d signal, %d sideband candidates; Z projected x %.3g' % (len(sig), len(bkg), scale), flush=True)

    path = os.path.join(outdir, 'results.csv')
    rows = []
    for name in names:
        for seed in args.seeds:
            p = dict(bcfg.get('params') or {}, device=args.device, random_state=seed)
            t = time.time()
            models, info = bdt.train_kfold(sig, bkg, feats[name], k=bcfg.get('kfold', 5), params=p,
                                           early_stopping=bcfg.get('early_stopping', 20))
            oof = info['oof']
            ssc, bsc = oof['score'].to_numpy()[:len(sig)], oof['score'].to_numpy()[len(sig):]
            res = significance.scan(ssc[win], sig['weight'].to_numpy()[win] * scale, bsc, factor * scale,
                                    min_sideband=scfg.get('min_sideband', 10))
            best = significance.best(res, 'asimov_z')
            row = dict(set=name, seed=seed, n_features=len(feats[name]),
                       **{'eff_sig@%g' % e: float((ssc > np.quantile(bsc, 1 - e)).mean()) for e in (5e-5, 2e-4, 1e-3)},
                       auc=roc_auc_score(oof['label'], oof['score']), Z_cut=best['asimov_z'],
                       n_sb_at_Z=best['n_sideband'], n_trees=float(info['ks']['n_trees'].mean()),
                       ks_min=float(info['ks'][['ks_p_sig', 'ks_p_bkg']].min().min()), seconds=time.time() - t,
                       top=' '.join(info['importance'].index[:10]))
            rows.append(row)
            info['importance'].to_csv(os.path.join(outdir, 'importance_%s.csv' % name), header=['gain'])
            print('%-28s seed %d: eff@5e-5 %.3f eff@2e-4 %.3f eff@1e-3 %.3f AUC %.4f Z %.2f (%d sb) %.0f trees %.0f s'
                  % (name, seed, row['eff_sig@5e-05'], row['eff_sig@0.0002'], row['eff_sig@0.001'], row['auc'],
                     row['Z_cut'], row['n_sb_at_Z'], row['n_trees'], row['seconds']), flush=True)
            pd.DataFrame(rows).to_csv(path, index=False)
    df = pd.DataFrame(rows)
    agg = df.groupby('set', sort=False)[['eff_sig@5e-05', 'eff_sig@0.0002', 'eff_sig@0.001', 'auc', 'Z_cut']].agg(['mean', 'std'])
    pd.set_option('display.width', 250)
    print('\n' + agg.to_string(float_format=lambda v: '%.4f' % v))
    print('\nresults in %s' % path)


if __name__ == '__main__':
    main()
