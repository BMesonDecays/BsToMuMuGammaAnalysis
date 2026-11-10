#!/usr/bin/env python3
"""Train the k-fold BDT: signal = truth-matched signal MC, background = data in the mass sidebands.

    python3 scripts/train_bdt.py config/analysis.yaml [--jobs 16]

Writes the models and the feature list to <output>/bdt/, and ROC, overtraining and feature-importance plots.
"""
import argparse
import os
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import bdt, config, plotting  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--jobs', type=int, default=-1, help='threads for XGBoost (default: all cores)')
    args = ap.parse_args()
    cfg = config.load_config(args.config)
    bcfg = cfg['bdt']
    features = bcfg['features']
    outdir = os.path.join(cfg['output'], 'bdt')

    samples = config.load_samples(cfg, which=('signal', 'background'))
    sig, bkg = samples['signal'], samples['background']
    print('training on %d signal and %d background (sideband) candidates, %d features, %d folds'
          % (len(sig), len(bkg), len(features), bcfg.get('kfold', 5)))
    models, info = bdt.train_kfold(sig, bkg, features, k=bcfg.get('kfold', 5), params=bcfg.get('params'),
                                   n_jobs=args.jobs, early_stopping=bcfg.get('early_stopping', 20))
    bdt.save(models, features, outdir, extra={'channel': cfg['channel'], 'params': bcfg.get('params')})

    oof = info['oof']
    auc = plotting.roc(oof['label'], oof['score'], os.path.join(outdir, 'roc.png'))
    # overtraining: model of fold 0 on its training folds vs its test fold
    data = oof.assign(**{f: np.concatenate([sig[f].to_numpy(), bkg[f].to_numpy()]) for f in features})
    tr, te = data[data['fold'] != 0], data[data['fold'] == 0]
    train_score = models[0].predict_proba(tr[features])[:, 1]
    plotting.overtraining({'signal': train_score[tr['label'].to_numpy() == 1], 'background': train_score[tr['label'].to_numpy() == 0]},
                          {'signal': te.loc[te['label'] == 1, 'score'], 'background': te.loc[te['label'] == 0, 'score']},
                          os.path.join(outdir, 'overtraining_fold0.png'))
    plotting.importance(info['importance'], os.path.join(outdir, 'importance.png'))
    print('\nout-of-fold AUC = %.4f' % auc)
    print('\ntrain vs test KS p-values per fold (small values = overtraining):\n%s' % info['ks'].to_string(index=False))
    print('\nfeature importance (mean gain):\n%s' % info['importance'].to_string())
    print('\nmodels and plots in %s' % outdir)


if __name__ == '__main__':
    main()
