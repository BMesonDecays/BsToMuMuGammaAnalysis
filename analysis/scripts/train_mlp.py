#!/usr/bin/env python3
"""Train the k-fold MLP (TensorFlow): signal = truth-matched signal MC, background = data in the mass sidebands.

    python3 scripts/train_mlp.py config/analysis.yaml [--cpu] [--verbose 2]

The analogue of train_bdt.py, configured by the `mlp` block of the config. Runs on the GPU if TensorFlow
sees one. Writes the models, the preprocessing and the feature list to <output>/mlp/, and ROC,
overtraining, permutation-importance and loss-curve plots.
"""
import argparse
import os
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--cpu', action='store_true', help='hide the GPUs from TensorFlow')
    ap.add_argument('--verbose', type=int, default=2, help='Keras verbosity: 0 silent, 1 progress bar, 2 one line per epoch')
    args = ap.parse_args()
    if args.cpu:
        os.environ['CUDA_VISIBLE_DEVICES'] = ''
    from bmmg_ana import config, mlp, plotting   # TensorFlow is imported here, after CUDA_VISIBLE_DEVICES

    cfg = config.load_config(args.config)
    mcfg = cfg['mlp']
    features = mcfg['features']
    k = mcfg.get('kfold', 5)
    outdir = os.path.join(cfg['output'], 'mlp')

    samples = config.load_samples(cfg, which=('signal', 'background'))
    sig, bkg = samples['signal'], samples['background']
    print('training on %d signal and %d background (sideband) candidates, %d features, %d folds, device: %s'
          % (len(sig), len(bkg), len(features), k, ', '.join(mlp.devices()) or 'CPU'))
    models, info = mlp.train_kfold(sig, bkg, features, k=k, params=mcfg.get('params'),
                                   early_stopping=mcfg.get('early_stopping', 10), verbose=args.verbose)
    mlp.save(models, features, outdir, extra={'channel': cfg['channel'], 'params': mcfg.get('params')})

    oof = info['oof']
    auc = plotting.roc(oof['label'], oof['score'], os.path.join(outdir, 'roc.png'))
    # overtraining: model of fold 0 on its training folds vs its test fold
    data = oof.assign(**{f: np.concatenate([sig[f].to_numpy(), bkg[f].to_numpy()]) for f in features})
    tr, te = data[data['fold'] != 0], data[data['fold'] == 0]
    train_score = models[0].predict_proba(tr[features])
    plotting.overtraining({'signal': train_score[tr['label'].to_numpy() == 1], 'background': train_score[tr['label'].to_numpy() == 0]},
                          {'signal': te.loc[te['label'] == 1, 'score'], 'background': te.loc[te['label'] == 0, 'score']},
                          os.path.join(outdir, 'overtraining_fold0.png'), xlabel='MLP score')
    plotting.importance(info['importance'], os.path.join(outdir, 'importance.png'), xlabel='AUC decrease when shuffled')
    plotting.history(info['history'], os.path.join(outdir, 'loss.png'))
    print('\nout-of-fold AUC = %.4f' % auc)
    print('\ntrain vs test KS p-values per fold (small values = overtraining):\n%s' % info['ks'].to_string(index=False))
    print('\nfeature importance (AUC decrease when the feature is shuffled in the test fold):\n%s' % info['importance'].to_string())
    print('\nmodels and plots in %s' % outdir)


if __name__ == '__main__':
    main()
