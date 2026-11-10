#!/usr/bin/env python3
"""Cut flow and basic plots: signal MC vs data sidebands (shapes), blinded data mass distribution.

    python3 scripts/plot_basic.py config/analysis.yaml
"""
import argparse
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import config, io, plotting, selection  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    args = ap.parse_args()
    cfg = config.load_config(args.config)
    out = os.path.join(cfg['output'], 'plots')
    os.makedirs(out, exist_ok=True)
    mcol = cfg['mass']['column']
    plot_vars = cfg.get('plots', {})

    # cut flows (from the converted tables, before the preselection of the analysis config)
    cols = list(plot_vars) + [mcol]
    cuts0 = dict(cfg['preselection']) if isinstance(cfg['preselection'], dict) else {'preselection': config.preselection_query(cfg)}
    for name, sample in (('data', cfg['samples']['data']), ('signal', cfg['samples']['signal'])):
        # data: the blind window is removed at loading, so no count (not even the first row) includes it
        query = config.blind_query(cfg) if name == 'data' else sample.get('query')
        df = io.load(cfg['parquet'], sample['datasets'], cfg['channel'],
                     [c for c in cols if c not in cfg['derived']], derived=cfg['derived'],
                     query=query, exprs=list(cuts0.values()))
        cf = selection.cutflow(df, cuts0)
        print('\n=== cut flow, %s\n%s' % (name, cf.to_string(float_format=lambda x: '%.4g' % x)))
        cf.to_csv(os.path.join(out, 'cutflow_%s.csv' % name))

    samples = config.load_samples(cfg, columns=cols, which=('signal', 'data', 'background'))
    sig, bkg, data = samples['signal'], samples['background'], samples['data']
    print('\nafter the preselection: signal %d, data outside the blind window %d candidates (%d in the sidebands)'
          % (len(sig), len(data), len(bkg)))

    for var, (bins, lo, hi) in plot_vars.items():
        if var not in sig.columns:
            print('skipping %s: not a column' % var)
            continue
        plotting.shapes({'signal MC': (sig[var], sig['weight']), 'data sidebands': (bkg[var], None)},
                        var, bins, (lo, hi), os.path.join(out, 'shape_%s.png' % var))

    bins, lo, hi = plot_vars.get(mcol, (80, 4.5, 6.5))
    mc = {'signal MC': (sig[mcol], sig['weight'])} if cfg.get('lumi_pb') else {}
    plotting.data_vs_mc(data[mcol], mc, mcol, bins, (lo, hi), os.path.join(out, 'data_mass_blinded.png'),
                        xlabel='m(μμγ) [GeV]', blind=cfg['mass']['blind'],
                        lumi=cfg['lumi_pb'] / 1000. if cfg.get('lumi_pb') else None)
    print('plots in %s' % out)


if __name__ == '__main__':
    main()
