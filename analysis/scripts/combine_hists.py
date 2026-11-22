#!/usr/bin/env python3
"""Mass histograms of the signal MC and of the data sidebands at several BDT score cuts, for Combine.

    python3 scripts/combine_hists.py config/combine_mmg.yaml [--retrain] [--jobs N]

Runs in the LCG view (pandas, XGBoost). Steps:
  1. samples of the base analysis configuration (bmmg_ana.config.load_samples): preselection, data blinded
     (nothing in the blind window is loaded), signal MC normalised to the good luminosity of the Run 3 target;
  2. the k-fold BDT of the base configuration (features, parameters), trained on the signal MC against the data
     sidebands and saved in <output>/bdt/, or loaded from there (--retrain: train again). Every candidate is
     scored by the model of its fold, so the training candidates get out-of-fold scores;
  3. per cut (score > cut): histograms of the mass in mass_fit.range, written to <output>/mmg_mass_hists.root
     (directory cut_<cut>, e.g. cut_0p99):
       signal       signal MC, weighted to the Run 3 target luminosity
       data_sb      data sidebands x (target / recorded luminosity), the blind window empty
       data_sb_raw  the unscaled sideband candidates (Poisson errors for the background fit)
     and <output>/yields.csv (yields, efficiencies, sideband counts per cut).
The fits, datacards and Combine runs: scripts/combine_fit.py (in the Combine CMSSW area).
"""
import argparse
import os
import sys

import numpy as np
import pandas as pd

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import bdt, config, plotting, selection  # noqa: E402

HIST_FILE = 'mmg_mass_hists.root'


def cut_name(cut):
    return 'cut_' + ('%g' % cut).replace('.', 'p')


def train(cfg, sig, bkg, outdir, jobs):
    bcfg = cfg['bdt']
    features = bcfg['features']
    params = dict(bcfg.get('params') or {})
    kw = dict(k=bcfg.get('kfold', 5), n_jobs=jobs, early_stopping=bcfg.get('early_stopping', 20))
    print('training on %d signal and %d sideband candidates, %d features, %d folds'
          % (len(sig), len(bkg), len(features), kw['k']))
    try:
        models, info = bdt.train_kfold(sig, bkg, features, params=params, **kw)
    except Exception as e:   # noqa: BLE001 - e.g. XGBoost without CUDA
        if params.get('device', 'cpu') == 'cpu':
            raise
        print('training with device=%s failed (%s): CPU' % (params['device'], e))
        params['device'] = 'cpu'
        models, info = bdt.train_kfold(sig, bkg, features, params=params, **kw)
    bdt.save(models, features, outdir, extra={'channel': cfg['channel'], 'params': params,
                                              'parquet': cfg['parquet']})
    oof = info['oof']
    auc = plotting.roc(oof['label'], oof['score'], os.path.join(outdir, 'roc.png'))
    data = oof.assign(**{f: np.concatenate([sig[f].to_numpy(), bkg[f].to_numpy()]) for f in features})
    tr, te = data[data['fold'] != 0], data[data['fold'] == 0]
    train_score = models[0].predict_proba(tr[features])[:, 1]
    plotting.overtraining({'signal': train_score[tr['label'].to_numpy() == 1],
                           'background': train_score[tr['label'].to_numpy() == 0]},
                          {'signal': te.loc[te['label'] == 1, 'score'], 'background': te.loc[te['label'] == 0, 'score']},
                          os.path.join(outdir, 'overtraining_fold0.png'))
    plotting.importance(info['importance'], os.path.join(outdir, 'importance.png'))
    print('out-of-fold AUC = %.4f' % auc)
    print('train vs test KS p-values per fold:\n%s' % info['ks'].to_string(index=False))
    return models, features


def histogram(values, weights, lo, hi, nbins):
    import hist
    h = hist.Hist(hist.axis.Regular(nbins, lo, hi, name='mass', label='m(#mu#mu#gamma) [GeV]'),
                  storage=hist.storage.Weight())
    h.fill(mass=np.asarray(values, dtype=float), weight=np.asarray(weights, dtype=float))
    return h


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--retrain', action='store_true', help='train the BDT even if models exist in <output>/bdt')
    ap.add_argument('--jobs', type=int, default=-1, help='threads for XGBoost (default: all cores)')
    args = ap.parse_args()
    cfg = config.load_combine_config(args.config)
    cc = cfg['combine']
    out = cfg['output']
    os.makedirs(out, exist_ok=True)
    mcol = cfg['mass']['column']
    lo, hi = cc['mass_fit']['range']
    nbins = int(round((hi - lo) / cc['mass_fit']['bin_width']))
    factor = cc['data_factor']
    print('Run 3 projection: data x %.4f (%.0f / %.0f pb^-1 recorded); signal MC at %.0f pb^-1 of good data'
          % (factor, cc['lumi']['target_recorded_pb'], cc['lumi']['data_recorded_pb'], cc['signal_lumi_pb']))

    samples = config.load_samples(cfg, which=('signal', 'data'))
    sig, data = samples['signal'], samples['data']
    sb = selection.in_windows(data[mcol], cfg['mass']['sidebands'])
    bkg = data[sb].copy()
    print('signal: %d candidates (%.2f expected); data outside the blind window: %d, in the sidebands: %d'
          % (len(sig), sig['weight'].sum(), len(data), len(bkg)))

    bdtdir = os.path.join(out, 'bdt')
    if args.retrain or not os.path.exists(os.path.join(bdtdir, 'bdt.json')):
        models, features = train(cfg, sig, bkg, bdtdir, args.jobs)
    else:
        models, features = bdt.load(bdtdir)
        if list(features) != list(cfg['bdt']['features']):
            sys.exit('the models in %s have other features than the configuration: use --retrain' % bdtdir)
        print('BDT models loaded from %s' % bdtdir)
    sig['score'] = bdt.predict(models, sig, features)
    bkg['score'] = bdt.predict(models, bkg, features)

    import uproot
    rows = []
    sig_in = (sig[mcol] >= lo) & (sig[mcol] < hi)
    bkg_in = (bkg[mcol] >= lo) & (bkg[mcol] < hi)
    swlo, swhi = cfg['mass']['signal_window']
    sig_tot = sig.loc[sig_in, 'weight'].sum()
    nsb_tot = int(bkg_in.sum())
    path = os.path.join(out, HIST_FILE)
    with uproot.recreate(path) as f:
        for cut in cc['bdt_cuts']:
            s = sig[sig_in & (sig['score'] > cut)]
            b = bkg[bkg_in & (bkg['score'] > cut)]
            d = cut_name(cut)
            f[d + '/signal'] = histogram(s[mcol], s['weight'], lo, hi, nbins)
            f[d + '/data_sb'] = histogram(b[mcol], np.full(len(b), factor), lo, hi, nbins)
            f[d + '/data_sb_raw'] = histogram(b[mcol], np.ones(len(b)), lo, hi, nbins)
            low = int((b[mcol] < cfg['mass']['blind'][0]).sum())
            rows.append({'cut': cut, 'name': d, 'S': s['weight'].sum(),
                         'S_window': s.loc[(s[mcol] >= swlo) & (s[mcol] < swhi), 'weight'].sum(),
                         'eff_sig': s['weight'].sum() / sig_tot if sig_tot > 0 else np.nan,
                         'n_sig_mc': len(s), 'n_sb_raw': len(b), 'n_sb_low': low, 'n_sb_high': len(b) - low,
                         'n_sb_scaled': len(b) * factor, 'eff_sb': len(b) / nsb_tot if nsb_tot else np.nan,
                         'fit': len(b) >= cc.get('min_sideband', 3)})
    y = pd.DataFrame(rows)
    y.to_csv(os.path.join(out, 'yields.csv'), index=False)
    pd.set_option('display.width', 200)
    print('\n' + y.to_string(index=False, float_format=lambda v: '%.4g' % v))
    print('\nhistograms in %s' % path)


if __name__ == '__main__':
    main()
