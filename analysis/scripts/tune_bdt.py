#!/usr/bin/env python3
"""Random search of the BDT hyperparameters with the k-fold training of train_bdt.py.

    python3 scripts/tune_bdt.py config/analysis.yaml [--trials 40] [--seed 42] [--bkg-eff 2e-4]

Every trial trains the k folds (features, kfold, early_stopping of the bdt block; the sampled parameters
replace bdt.params) and records, from the out-of-fold scores:
  - eff_sig at sideband efficiencies 5e-5, 2e-4 (the ranking objective, --bkg-eff) and 1e-3,
  - the AUC and the train/test KS p-values,
  - the best cut-and-count Asimov Z (signal window, background from the sidebands as in scan_wp.py,
    projected to significance.projection_lumi_pb if set).
The trials are ranked by the signal efficiency at a fixed background efficiency rather than by the best Z,
because the best Z sits at a handful of sideband candidates and fluctuates. Trial 0 is the bdt.params of the
config, as a reference. The table is written to <output>/bdt_tuning/trials.csv; copy the chosen parameters
to bdt.params.
"""
import argparse
import os
import sys
import time

import numpy as np
import pandas as pd
from sklearn.metrics import roc_auc_score

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import bdt, config, selection, significance  # noqa: E402


def sample_params(rng, device):
    return dict(n_estimators=4000, max_depth=int(rng.integers(3, 9)), learning_rate=float(10 ** rng.uniform(-1.7, -0.9)),
                subsample=float(rng.uniform(0.5, 1.0)), colsample_bytree=float(rng.uniform(0.3, 1.0)),
                min_child_weight=float(10 ** rng.uniform(0, 2.3)), reg_lambda=float(10 ** rng.uniform(-1, 2)),
                gamma=float(rng.choice([0, 0, 0.1, 0.5, 1, 3])), max_bin=int(rng.choice([256, 512])), device=device)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--trials', type=int, default=40)
    ap.add_argument('--seed', type=int, default=42)
    ap.add_argument('--bkg-eff', type=float, default=2e-4, help='sideband efficiency of the ranking objective')
    ap.add_argument('--device', default=None, help='XGBoost device (default: bdt.params.device, else cpu)')
    args = ap.parse_args()
    cfg = config.load_config(args.config)
    bcfg, scfg = cfg['bdt'], cfg.get('significance', {})
    features, k = bcfg['features'], bcfg.get('kfold', 5)
    device = args.device or (bcfg.get('params') or {}).get('device', 'cpu')
    outdir = os.path.join(cfg['output'], 'bdt_tuning')
    os.makedirs(outdir, exist_ok=True)
    mcol = cfg['mass']['column']

    samples = config.load_samples(cfg, which=('signal', 'background'))
    sig, bkg = samples['signal'], samples['background']
    win = np.asarray(selection.in_windows(sig[mcol], [cfg['mass']['signal_window']]))
    factor = significance.sideband_factor(cfg['mass']['signal_window'], cfg['mass']['sidebands'],
                                          scfg.get('background_model', 'exp'), bkg[mcol])
    scale = scfg['projection_lumi_pb'] / cfg['lumi_pb'] if scfg.get('projection_lumi_pb') and cfg.get('lumi_pb') else 1.0
    print('%d signal, %d sideband candidates, %d features, %d folds, device %s; Z projected x %.3g'
          % (len(sig), len(bkg), len(features), k, device, scale))

    rng = np.random.default_rng(args.seed)
    trials = [dict(bcfg.get('params') or {}, device=device)] + [sample_params(rng, device) for _ in range(args.trials)]
    rows = []
    for i, p in enumerate(trials):
        t = time.time()
        models, info = bdt.train_kfold(sig, bkg, features, k=k, params=p, early_stopping=bcfg.get('early_stopping', 20))
        oof = info['oof']
        ssc, bsc = oof['score'].to_numpy()[:len(sig)], oof['score'].to_numpy()[len(sig):]
        res = significance.scan(ssc[win], sig['weight'].to_numpy()[win] * scale, bsc, factor * scale,
                                min_sideband=scfg.get('min_sideband', 10))
        best = significance.best(res, 'asimov_z')
        row = dict(trial=i, **{'eff_sig@%g' % e: float((ssc > np.quantile(bsc, 1 - e)).mean()) for e in (5e-5, args.bkg_eff, 1e-3)},
                   auc=roc_auc_score(oof['label'], oof['score']), Z_cut=best['asimov_z'], n_sideband_at_Z=best['n_sideband'],
                   ks_min=float(info['ks'][['ks_p_sig', 'ks_p_bkg']].min().min()), n_trees=float(info['ks']['n_trees'].mean()),
                   seconds=time.time() - t, **{k_: v for k_, v in p.items() if k_ != 'device'})
        rows.append(row)
        print(' '.join('%s=%.4g' % (k_, v) if isinstance(v, float) else '%s=%s' % (k_, v) for k_, v in row.items()), flush=True)
        pd.DataFrame(rows).to_csv(os.path.join(outdir, 'trials.csv'), index=False)
    df = pd.DataFrame(rows).sort_values('eff_sig@%g' % args.bkg_eff, ascending=False)
    print('\nbest trials (by signal efficiency at sideband efficiency %g):\n%s' % (args.bkg_eff, df.head(10).to_string(index=False)))
    print('\ntable in %s' % os.path.join(outdir, 'trials.csv'))


if __name__ == '__main__':
    main()
