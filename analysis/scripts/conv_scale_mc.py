#!/usr/bin/env python3
"""Converted-photon energy scale in MC from the truth, with the peak definition of conv_scale.py.

    python3 scripts/conv_scale_mc.py config/conv_scale_mc.yaml [--no-plots]

Input: gen-matched mmcg candidates of the B_s -> J/psi gamma MC (config/convert_convmc.yaml).
For the conversions from the signal B that match the leading gen photon (dR < 0.05):
  - r = conv_pt / gen pt: the photon energy response; its peak (Crystal Ball mean, n = 2 as in the data fit)
    is the MC scale s_MC, per bin of the true photon pt and |eta| -> conv_scale_mc.json (ScaleMap);
  - u = y / (m_Bs^2 - m_J/psi^2), y = m(mu mu gamma)^2 - m(mu mu)^2 from the unfitted momenta: the variable of
    the data method. Its peak equals the peak of r if the y method measures the photon scale (the dimuon
    resolution and the B kinematics do not shift it).
With a data map (data_map), the data and MC scales are compared in the pt range of the data.
"""
import argparse
import os
import sys

import numpy as np
import pandas as pd
import yaml

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import convscale as cs  # noqa: E402
from bmmg_ana import io  # noqa: E402

COLUMNS = ['conv_pt', 'conv_eta', 'conv_phi', 'dimu_pt', 'dimu_eta', 'dimu_phi', 'dimu_mass',
           'gen_pho1_pt', 'gen_pho1_eta', 'gen_pho1_phi', 'conv_genFromSignalB']


def dr(eta1, phi1, eta2, phi2):
    dphi = (np.asarray(phi1) - np.asarray(phi2) + np.pi) % (2 * np.pi) - np.pi
    return np.hypot(np.asarray(eta1) - np.asarray(eta2), dphi)


def peak(x, cfg):
    return cs.fit_peak(x, tuple(cfg.get('fit_range', (0.6, 1.3))), int(cfg.get('fit_bins', 140)))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--no-plots', action='store_true')
    args = ap.parse_args()
    with open(args.config) as f:
        cfg = yaml.safe_load(f)
    out = os.path.expanduser(cfg['output'])
    os.makedirs(out, exist_ok=True)
    exprs = [b['expr'] for b in cfg['binning']]
    df = io.load(cfg['parquet'], cfg['datasets'], 'mmcg', COLUMNS, query=cfg.get('selection'), exprs=exprs)
    # one entry per conversion: several candidates (other dimuons) can share it
    df = df.drop_duplicates(['evt_run', 'evt_lumi', 'evt_event', 'conv_pt', 'conv_eta'])
    match = (df['conv_genFromSignalB'] == 1) & (dr(df['conv_eta'], df['conv_phi'], df['gen_pho1_eta'], df['gen_pho1_phi']) < 0.05)
    df = df[match].copy()
    df['r'] = df['conv_pt'] / df['gen_pho1_pt']
    df['u'] = cs.y_from_vectors(df['dimu_pt'], df['dimu_eta'], df['dimu_phi'], df['dimu_mass'],
                                df['conv_pt'], df['conv_eta'], df['conv_phi']) / cs.Y_BS
    print('%d truth-matched conversions' % len(df))

    rows = []

    def record(label, sel, extra=None):
        fr, fu = peak(df['r'][sel], cfg), peak(df['u'][sel], cfg)
        row = {'fit': label, 'n': int(sel.sum()), 's_r': fr['values']['mean'], 's_r_err': fr['errors']['mean'],
               'valid_r': fr['valid'], 'sigma_r': fr['values']['sigma'], 'alpha_r': fr['values']['alpha'],
               's_u': fu['values']['mean'], 's_u_err': fu['errors']['mean'], 'valid_u': fu['valid'],
               'sigma_u': fu['values']['sigma'], 'median_r': float(np.median(df['r'][sel])) if sel.any() else np.nan}
        row.update(extra or {})
        rows.append(row)
        return row, fr, fu

    allsel = np.ones(len(df), dtype=bool)
    row, fr, fu = record('inclusive', allsel)
    print('inclusive: peak of E_reco/E_true %.4f ± %.4f (sigma %.3f), peak of y/Y_Bs %.4f ± %.4f (sigma %.3f); '
          'difference %+.4f; median E_reco/E_true %.4f'
          % (row['s_r'], row['s_r_err'], row['sigma_r'], row['s_u'], row['s_u_err'], row['sigma_u'],
             row['s_u'] - row['s_r'], row['median_r']))

    binning = cfg['binning']
    idx = cs.bin_index(df, binning)
    shape = tuple(len(b['edges']) - 1 for b in binning)
    scale, error = np.full(shape, row['s_r']), np.full(shape, row['s_r_err'])
    status = np.zeros(shape, dtype=int)
    print('\n%-34s %6s  %-17s %-17s %s' % ('bin (true pt, |eta|)', 'n', 's(E_reco/E_true)', 's(y/Y_Bs)', 'diff'))
    for b in cs.all_bins(binning):
        sel = np.all(idx == np.array(b), axis=1)
        rng = ', '.join('%s %g–%g' % (bb['name'], bb['edges'][k], bb['edges'][k + 1]) for k, bb in zip(b, binning))
        if sel.sum() < cfg.get('min_candidates', 0):
            status[b] = 2
            print('%-34s %6d  -' % (rng, sel.sum()))
            continue
        extra = {bb['name'] + '_lo': bb['edges'][k] for k, bb in zip(b, binning)}
        extra.update({bb['name'] + '_hi': bb['edges'][k + 1] for k, bb in zip(b, binning)})
        r, fr, fu = record('bin', sel, dict(extra, bin=list(b)))
        status[b] = 0 if fr['valid'] else 1
        if fr['valid']:
            scale[b], error[b] = r['s_r'], r['s_r_err']
        print('%-34s %6d  %.4f ± %.4f   %.4f ± %.4f   %+.4f' % (rng, r['n'], r['s_r'], r['s_r_err'], r['s_u'],
                                                               r['s_u_err'], r['s_u'] - r['s_r']))
    variables = [{'name': b['name'], 'expr': b['expr'], 'edges': [float(e) for e in b['edges']]} for b in binning]
    smap = cs.ScaleMap(variables, scale, error, status, inclusive={'s': rows[0]['s_r'], 's_err': rows[0]['s_r_err']},
                       info={'source': 'MC truth, peak of conv_pt / gen pt', 'datasets': cfg['datasets'],
                             'selection': cfg.get('selection'), 'n_conversions': int(len(df))})
    smap.to_json(os.path.join(out, 'conv_scale_mc.json'))
    pd.DataFrame(rows).to_csv(os.path.join(out, 'conv_scale_mc.csv'), index=False)

    data_map = os.path.expanduser(cfg.get('data_map') or '')
    if data_map and os.path.exists(data_map):
        dmap = cs.ScaleMap.from_json(data_map)
        print('\ndata / MC (data map bins; the MC bin at the data bin centre in true pt, open last bin: 1.5 x its low edge):')
        de = [np.asarray(v['edges']) for v in dmap.variables]
        for i in range(len(de[0]) - 1):
            for j in range(len(de[1]) - 1):
                if dmap.status[i, j] != 0:
                    continue
                pt = 0.5 * (de[0][i] + min(de[0][i + 1], 2 * de[0][i]))
                eta = 0.5 * (de[1][j] + de[1][j + 1])
                mi = tuple(int(np.clip(np.searchsorted(v['edges'], x, side='right') - 1, 0, len(v['edges']) - 2))
                           for v, x in zip(smap.variables, (pt, eta)))
                head = ('  pt %5.2f–%-6.2f |eta| %.1f–%.1f: data %.4f ± %.4f' % (
                    de[0][i], de[0][i + 1], de[1][j], de[1][j + 1], dmap.scale[i, j],
                    np.hypot(dmap.error[i, j], dmap.syst[i, j])))
                if smap.status[mi] != 0:   # no MC fit in this region: no comparison
                    print(head + '  MC -')
                    continue
                smc, emc = smap.scale[mi], smap.error[mi]
                print(head + '  MC %.4f ± %.4f  data/MC %.4f ± %.4f' % (
                    smc, emc, dmap.scale[i, j] / smc,
                    dmap.scale[i, j] / smc * np.hypot(np.hypot(dmap.error[i, j], dmap.syst[i, j]) / dmap.scale[i, j],
                                                      emc / smc)))
        if not args.no_plots:
            plot_compare(smap, dmap, os.path.join(out, 'scale_data_mc.png'))
    print('output in %s' % out)


def plot_compare(smap, dmap, path):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(figsize=(8, 5.5))
    me, de = [np.asarray(v['edges']) for v in smap.variables], [np.asarray(v['edges']) for v in dmap.variables]
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    for j in range(len(me[1]) - 1):
        ok = smap.status[:, j] == 0
        hi = np.minimum(me[0][1:], 2 * me[0][:-1])
        x = 0.5 * (me[0][:-1] + hi)
        ax.errorbar(x[ok], smap.scale[ok, j], yerr=smap.error[ok, j], fmt='s', mfc='none', color=colors[j % 10],
                    label='MC |η| %.1f–%.1f' % (me[1][j], me[1][j + 1]))
    for j in range(len(de[1]) - 1):
        ok = dmap.status[:, j] == 0
        hi = np.minimum(de[0][1:], 2 * de[0][:-1])
        x = 0.5 * (de[0][:-1] + hi)
        ax.errorbar(x[ok], dmap.scale[ok, j], yerr=np.hypot(dmap.error[ok, j], dmap.syst[ok, j]), fmt='o',
                    color=colors[j % 10], label='data |η| %.1f–%.1f' % (de[1][j], de[1][j + 1]))
    ax.set_xscale('log')
    ax.axhline(1, color='k', lw=0.5)
    ax.set_xlabel('true photon pt [GeV]')
    ax.set_ylabel('peak scale s')
    ax.legend(fontsize=8, ncol=2)
    fig.savefig(path, bbox_inches='tight')
    plt.close(fig)


if __name__ == '__main__':
    main()
