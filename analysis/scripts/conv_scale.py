#!/usr/bin/env python3
"""Energy scale of the converted photons from chi_c1,2 -> J/psi gamma(conversion) in data.

    python3 scripts/conv_scale.py config/conv_scale.yaml [--no-variations] [--no-plots]

Input: the chic tables of convert.py (config/convert_chic.yaml). Fit variable y = m^2 - m(mumu)^2, linear in
the photon energy; the chi_c1/chi_c2 peaks sit at s * (m_chic^2 - m_J/psi^2) (bmmg_ana/convscale.py).

Steps:
  1. inclusive fit (all selected candidates); reference fits per bin of the `shape_from` variables (e.g. |eta|
     slices), which fix the parameters `fix_in_bins` (tail alpha, N(chi_c2)/N(chi_c1)) of the map bins;
  2. fit in every bin of `binning` -> scale map; bins with too few candidates or a failed fit get the
     scale of their reference fit and a status != 0;
  3. model variations (`variations`): the largest deviation per bin is the systematic error;
  4. one-dimensional cross-checks (`cross_checks`, e.g. the conversion radius);
  5. closure: y of every candidate divided by the interpolated map value, inclusive and per-bin refits (s ~ 1).

Output (in `output`): conv_scale.json (the map: bmmg_ana.convscale.ScaleMap), conv_scale.csv (all fits),
fit plots per bin (fits/), summary plots.
"""
import argparse
import os
import sys
import time

import numpy as np
import pandas as pd
import yaml

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import convscale as cs  # noqa: E402
from bmmg_ana import io  # noqa: E402

COLUMNS = ['mass', 'dimu_mass', 'q', 'conv_pt']


def fit_sample(y, model, inclusive, fix, min_candidates, r21_factor=1.):
    """Fit one bin with the inclusive result as start; returns (result or None, status, n in range)."""
    lo, hi = model['range']
    n = int(np.count_nonzero((y >= lo) & (y < hi)))
    if n < min_candidates:
        return None, 2, n
    start = dict(inclusive['values'], ntot=inclusive['ntot'])
    start['r21'] *= r21_factor
    r = cs.fit(y, model, start=start, fixed=fix)
    return r, (0 if r['valid'] else 1), n


def reference_fits(df, y, cfg, model, inclusive):
    """Fits that provide the fixed shape parameters (fix_in_bins) of the map bins.

    shape_from: names of map variables. For every bin of these variables (all other variables merged, inside
    the map range) one fit; the map bins take their fixed parameters from it. A reference fit that fails or
    has too few candidates falls back to the inclusive fit. Without shape_from: the inclusive fit for all.
    r21_from: selection of a region with resolved chi_c1/chi_c2 peaks; its N(chi_c2)/N(chi_c1) is fixed in the
    reference fits and in the bins (returned model).
    r21_per: names of map variables: N(chi_c2)/N(chi_c1) from a fit of the r21_from region per bin of these
    variables (the other variables merged), fixed in the bins (returned model['r21_bins'], see r21_for_bin).
    The reference fits keep the r21_from value of the whole region.
    Returns {projected bin tuple: fit result}, the indices of the shape_from variables, summary rows, and the
    model for the bins.
    """
    binning = cfg['binning']
    names = [b['name'] for b in binning]
    dims = [names.index(n) for n in cfg.get('shape_from') or []]
    rows = []
    if cfg.get('r21_from'):
        # N(chi_c2)/N(chi_c1) from a region with resolved peaks, fixed in all reference and bin fits
        sel = np.asarray(df.eval(cfg['r21_from']), dtype=bool)
        r = cs.fit(y[sel], model, start=dict(inclusive['values'], ntot=inclusive['ntot']))
        if not r['valid']:
            raise RuntimeError('the r21_from fit (%s) is not valid' % cfg['r21_from'])
        print('r21 from %s: %.4f ± %.4f (s = %.4f)' % (cfg['r21_from'], r['values']['r21'], r['errors']['r21'], r['s']))
        rows.append({'fit': 'r21_from', 'bin': [], 'n_range': r['n'], 'status': 0, 's': r['s'], 's_err': r['s_err'],
                     'r21': r['values']['r21'], 'alpha': r['values']['alpha']})
        inclusive = dict(inclusive, values=dict(inclusive['values'], r21=r['values']['r21']))
        model = dict(model, fixed=dict(model.get('fixed') or {}, r21=r['values']['r21']))
    idx = cs.bin_index(df, binning)
    r21_bins = None
    if cfg.get('r21_from') and cfg.get('r21_per'):
        # the production ratio changes with pt (0.49 -> 0.31 over pt_est 1 -> >3 GeV in 2022 data)
        rdims = [names.index(n) for n in cfg['r21_per']]
        rbins = [binning[d] for d in rdims]
        values = np.full(tuple(len(b['edges']) - 1 for b in rbins), model['fixed']['r21'])
        for pb in cs.all_bins(rbins):
            selb = sel & np.all(idx[:, rdims] == np.array(pb), axis=1)
            r = cs.fit(y[selb], dict(model, fixed={k: v for k, v in model['fixed'].items() if k != 'r21'}),
                       start=dict(inclusive['values'], ntot=inclusive['ntot']))
            rng = ', '.join('%s %g–%g' % (b['name'], b['edges'][k], b['edges'][k + 1]) for b, k in zip(rbins, pb))
            if r['valid']:
                values[pb] = r['values']['r21']
                print('r21 from %s, %s: %.4f ± %.4f (s = %.4f)' % (cfg['r21_from'], rng, r['values']['r21'],
                                                                   r['errors']['r21'], r['s']))
            else:
                print('WARNING: r21 fit in %s, %s not valid: the value of the whole region' % (cfg['r21_from'], rng))
            rows.append({'fit': 'r21_from', 'bin': list(pb), 'n_range': r['n'], 'status': 0 if r['valid'] else 1,
                         's': r['s'], 's_err': r['s_err'], 'r21': r['values']['r21'], 'r21_err': r['errors']['r21'],
                         'alpha': r['values']['alpha'],
                         **{b['name'] + '_lo': b['edges'][k] for b, k in zip(rbins, pb)},
                         **{b['name'] + '_hi': b['edges'][k + 1] for b, k in zip(rbins, pb)}})
        r21_bins = {'vars': [{'name': b['name'], 'expr': b['expr'], 'edges': [float(e) for e in b['edges']]}
                             for b in rbins], 'values': values}
    if not dims:
        return {(): inclusive}, dims, rows, (dict(model, r21_bins=r21_bins) if r21_bins else model)
    inmap = np.all(idx >= 0, axis=1)
    refs = {}
    lo, hi = model['range']
    for pb in cs.all_bins([binning[d] for d in dims]):
        sel = inmap & np.all(idx[:, dims] == np.array(pb), axis=1)
        n = int(np.count_nonzero((y[sel] >= lo) & (y[sel] < hi)))
        r = cs.fit(y[sel], model, start=dict(inclusive['values'], ntot=inclusive['ntot'])) \
            if n >= cfg.get('min_candidates', 0) else None
        ok = r is not None and r['valid']
        refs[pb] = r if ok else inclusive
        rows.append({'fit': 'reference', 'bin': list(pb), 'n_range': n, 'status': 0 if ok else (1 if r else 2),
                     **({'s': r['s'], 's_err': r['s_err'], 'sigmaRel': r['values']['sigmaRel'], 'n_sig': r['n_sig'],
                         'chi2': r['chi2'], 'ndf': r['ndf'], 'alpha': r['values']['alpha'],
                         'r21': r['values']['r21']} if r else {}),
                     **{binning[d]['name'] + '_lo': binning[d]['edges'][k] for d, k in zip(dims, pb)},
                     **{binning[d]['name'] + '_hi': binning[d]['edges'][k + 1] for d, k in zip(dims, pb)}})
    return refs, dims, rows, (dict(model, r21_bins=r21_bins) if r21_bins else model)


def r21_for_bin(r21_bins, binning, b):
    """N(chi_c2)/N(chi_c1) of bin b of `binning` from the per-bin values (r21_bins of reference_fits).

    Every r21 variable is matched to the binning variable with the same expression; its value is taken at the
    centre of that bin (clamped to the outer r21 bins). None if a variable is not in the binning (e.g. the
    conversion-radius cross-check): the fit keeps the r21 of the whole r21_from region.
    """
    exprs = [bb['expr'] for bb in binning]
    k = []
    for v in r21_bins['vars']:
        if v['expr'] not in exprs:
            return None
        d = exprs.index(v['expr'])
        c = 0.5 * (binning[d]['edges'][b[d]] + binning[d]['edges'][b[d] + 1])
        k.append(int(np.clip(np.searchsorted(v['edges'], c, side='right') - 1, 0, len(v['edges']) - 2)))
    return float(r21_bins['values'][tuple(k)])


def run_map(df, y, cfg, model, inclusive, label, r21_factor=1., refs=None, fix=None):
    """Per-bin fits of the map binning. Returns (rows, scale, error, status arrays, results).

    refs: (reference fits, dims) of reference_fits(); default: the inclusive fit for every bin.
    fix: parameters fixed to the reference values (default: fix_in_bins of the config).
    """
    fix = cfg.get('fix_in_bins', []) if fix is None else fix
    binning = cfg['binning']
    r21_bins = model.get('r21_bins')
    model = {k: v for k, v in model.items() if k != 'r21_bins'}
    refs, dims = refs or ({(): inclusive}, [])
    idx = cs.bin_index(df, binning)
    shape = tuple(len(b['edges']) - 1 for b in binning)
    scale, error = np.full(shape, inclusive['s']), np.full(shape, inclusive['s_err'])
    status = np.zeros(shape, dtype=int)
    rows, results = [], {}
    for b in cs.all_bins(binning):
        sel = np.all(idx == np.array(b), axis=1)
        ref = refs[tuple(b[d] for d in dims)]
        r21 = r21_for_bin(r21_bins, binning, b) if r21_bins else None
        mb = dict(model, fixed=dict(model['fixed'], r21=r21)) if r21 is not None else model
        r, st, n = fit_sample(y[sel], mb, ref, fix, cfg.get('min_candidates', 0), r21_factor)
        status[b] = st
        row = {'fit': label, 'bin': list(b), 'n_range': n, 'status': st}
        for k, bb in zip(b, binning):
            row[bb['name'] + '_lo'], row[bb['name'] + '_hi'] = bb['edges'][k], bb['edges'][k + 1]
        if r is not None:
            row.update(s=r['s'], s_err=r['s_err'], sigmaRel=r['values']['sigmaRel'], n_sig=r['n_sig'],
                       chi2=r['chi2'], ndf=r['ndf'])
            if st == 0:
                scale[b], error[b] = r['s'], r['s_err']
            else:   # fallback: the scale of the reference fit (status says so)
                scale[b], error[b] = ref['s'], ref['s_err']
            results[b] = r
        else:
            scale[b], error[b] = ref['s'], ref['s_err']
        rows.append(row)
    return rows, scale, error, status, results


def plot_fit(r, path, title):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    m = r['model']
    e = m.edges
    c = 0.5 * (e[1:] + e[:-1])
    fig, (ax, axp) = plt.subplots(2, 1, sharex=True, figsize=(8, 7), gridspec_kw={'height_ratios': [3, 1]})
    n = r['counts']
    ax.errorbar(c, n, yerr=np.sqrt(np.maximum(n, 1)), fmt='ko', ms=3, label='data')
    comps = m.components(r['values'])
    ax.plot(c, sum(comps.values()), 'b-', label='fit')
    ax.plot(c, comps['bkg'], 'b--', label='background')
    for k, v in comps.items():
        if k != 'bkg':
            ax.plot(c, v + comps['bkg'], ':', label=r'$\chi_{c%s}$ + background' % k[-1])
    ax.set_ylabel('candidates / %.3f GeV$^2$' % (e[1] - e[0]))
    ax.set_title(title, fontsize=10)
    v, er = r['values'], r['errors']
    ax.text(0.98, 0.95, 's = %.4f ± %.4f\nσ/y = %.3f\nN(χc1) = %.0f, N(χc2)/N(χc1) = %.2f\nχ²/ndf = %.1f/%d'
            % (v['s'], er['s'], v['sigmaRel'], v['N1'], v['r21'], r['chi2'], r['ndf']),
            transform=ax.transAxes, ha='right', va='top', fontsize=9)
    ax.legend(loc='upper left', fontsize=8)
    pull = (n - r['expected']) / np.sqrt(np.maximum(r['expected'], 1e-9))
    axp.bar(c, pull, width=e[1] - e[0], color='gray')
    axp.axhline(0, color='k', lw=0.5)
    axp.set_ylabel('pull')
    axp.set_xlabel(r'$y = m(\mu\mu\gamma)^2 - m(\mu\mu)^2$ [GeV$^2$]')
    os.makedirs(os.path.dirname(path), exist_ok=True)
    fig.savefig(path, bbox_inches='tight')
    plt.close(fig)


def plot_summary(smap, path):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    v = smap.variables
    fig, ax = plt.subplots(figsize=(8, 5.5))
    e0 = np.asarray(v[0]['edges'], dtype=float)
    # last edge open-ended: draw the last bin with the width of the previous one
    x = 0.5 * (e0[1:] + e0[:-1])
    if len(e0) > 2:
        x[-1] = e0[-2] + 0.5 * (e0[-2] - e0[-3])
    nother = smap.scale.shape[1] if smap.scale.ndim > 1 else 1
    for j in range(nother):
        s = smap.scale[:, j] if smap.scale.ndim > 1 else smap.scale
        st = smap.error[:, j] if smap.scale.ndim > 1 else smap.error
        sy = smap.syst[:, j] if smap.scale.ndim > 1 else smap.syst
        ok = (smap.status[:, j] if smap.scale.ndim > 1 else smap.status) == 0
        lab = '%s %.1f–%.1f' % (v[1]['name'], v[1]['edges'][j], v[1]['edges'][j + 1]) if len(v) > 1 else ''
        dx = (j - (nother - 1) / 2) * 0.02 * (x[-1] - x[0] if len(x) > 1 else 1)
        h = ax.errorbar(x[ok] + dx, s[ok], yerr=np.hypot(st[ok], sy[ok]), fmt='o', capsize=3, label=lab)
        ax.errorbar(x[ok] + dx, s[ok], yerr=st[ok], fmt='none', color=h[0].get_color(), elinewidth=3)
    inc = smap.inclusive
    ax.axhline(inc['s'], color='gray', ls='--', label='inclusive %.4f' % inc['s'])
    ax.axhline(1, color='k', lw=0.5)
    ax.set_xlabel('%s (%s) [GeV]' % (v[0]['name'], v[0]['expr']))
    ax.set_ylabel('photon energy scale s')
    ax.legend(fontsize=9)
    ax.set_title('thick: statistical, thin: stat ⊕ syst', fontsize=9)
    fig.savefig(path, bbox_inches='tight')
    plt.close(fig)


def plot_1d(rows, name, path):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    r = [x for x in rows if x['status'] == 0]
    if not r:
        return
    lo, hi = np.array([x[name + '_lo'] for x in r]), np.array([x[name + '_hi'] for x in r])
    hi = np.where(hi > 3 * np.maximum(lo, 1e-9) + 5, lo + (lo - np.r_[lo[0], lo[:-1]]).clip(min=0.1), hi)
    fig, ax = plt.subplots(figsize=(7, 4.5))
    ax.errorbar(0.5 * (lo + hi), [x['s'] for x in r], xerr=0.5 * (hi - lo), yerr=[x['s_err'] for x in r], fmt='o')
    ax.axhline(1, color='k', lw=0.5)
    ax.set_xlabel(name)
    ax.set_ylabel('s')
    fig.savefig(path, bbox_inches='tight')
    plt.close(fig)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--no-variations', action='store_true', help='skip the systematic model variations')
    ap.add_argument('--no-plots', action='store_true')
    args = ap.parse_args()
    with open(args.config) as f:
        cfg = yaml.safe_load(f)
    out = os.path.expanduser(cfg['output'])
    os.makedirs(out, exist_ok=True)
    model = dict(cs.DEFAULT_MODEL, **cfg.get('model', {}))
    exprs = [b['expr'] for b in cfg['binning'] + cfg.get('cross_checks', [])]
    for v in (cfg.get('variations') or {}).values():
        if 'fixed' in v:   # variations replace the fixed parameters of the nominal model
            v['fixed'] = dict(model.get('fixed') or {}, **v['fixed'])

    t0 = time.time()
    df = io.load(cfg['parquet'], cfg['datasets'], 'chic', COLUMNS, query=cfg.get('selection'), exprs=exprs)
    y = cs.y_from_masses(df['mass'], df['dimu_mass'])
    df['y'] = y
    df['pt_est'] = cs.pt_est(df['conv_pt'], y)
    print('%d selected candidates (%.0f s)' % (len(df), time.time() - t0))

    # 1. inclusive
    inc = cs.fit(y, model)
    print('inclusive: s = %.4f ± %.4f, sigma/y = %.4f, N1 = %.0f, N2/N1 = %.3f, chi2/ndf = %.1f/%d, valid %s'
          % (inc['s'], inc['s_err'], inc['values']['sigmaRel'], inc['values']['N1'], inc['values']['r21'],
             inc['chi2'], inc['ndf'], inc['valid']))
    if not inc['valid']:
        print('WARNING: the inclusive fit is not valid')
    rows = [{'fit': 'inclusive', 'bin': [], 'n_range': inc['n'], 'status': 0 if inc['valid'] else 1, 's': inc['s'],
             's_err': inc['s_err'], 'sigmaRel': inc['values']['sigmaRel'], 'n_sig': inc['n_sig'],
             'chi2': inc['chi2'], 'ndf': inc['ndf']}]

    # 2. map: shape references (e.g. per |eta| slice), then the bins
    refs, dims, rrows, bmodel = reference_fits(df, y, cfg, model, inc)
    rows += rrows
    for r in rrows:
        print('%s %s: n %d status %d s = %s alpha = %s r21 = %s' % (
            r['fit'], ', '.join('%s %.2f–%.2f' % (k[:-3], r[k], r[k[:-3] + '_hi']) for k in r if k.endswith('_lo')),
            r['n_range'], r['status'], '%.4f ± %.4f' % (r['s'], r['s_err']) if 's' in r else '-',
            '%.2f' % r['alpha'] if 'alpha' in r else '-', '%.3f' % r['r21'] if 'r21' in r else '-'))
    mrows, scale, error, status, results = run_map(df, y, cfg, bmodel, inc, 'nominal', refs=(refs, dims))
    rows += mrows

    # 3. variations
    syst = np.zeros_like(scale)
    if not args.no_variations:
        for name, over in (cfg.get('variations') or {}).items():
            over = dict(over)
            r21_factor = over.pop('r21_factor', 1.)
            vm = dict(model, **over)
            vinc = cs.fit(y, vm)
            rows.append({'fit': 'inclusive_' + name, 'bin': [], 'n_range': vinc['n'],
                         'status': 0 if vinc['valid'] else 1, 's': vinc['s'], 's_err': vinc['s_err'],
                         'sigmaRel': vinc['values']['sigmaRel'], 'n_sig': vinc['n_sig'], 'chi2': vinc['chi2'],
                         'ndf': vinc['ndf']})
            try:
                vrefs, vdims, _, vbm = reference_fits(df, y, cfg, vm, vinc)
            except RuntimeError as e:   # one failed variation does not stop the map
                print('WARNING: variation %s skipped: %s' % (name, e))
                continue
            if r21_factor != 1.:
                vbm = dict(vbm, fixed=dict(vbm['fixed'], r21=vbm['fixed']['r21'] * r21_factor))
                if vbm.get('r21_bins'):
                    vbm['r21_bins'] = dict(vbm['r21_bins'], values=vbm['r21_bins']['values'] * r21_factor)
            vrows, vscale, _, vstatus, _ = run_map(df, y, cfg, vm if 'r21' not in (vbm.get('fixed') or {}) else vbm,
                                                   vinc, name, 1. if 'r21' in (vbm.get('fixed') or {}) else r21_factor,
                                                   refs=(vrefs, vdims))
            rows += vrows
            ok = (status == 0) & (vstatus == 0)
            syst = np.where(ok, np.maximum(syst, np.abs(vscale - scale)), syst)
            print('variation %-14s inclusive s = %.4f (%+.4f); per-bin max |ds| = %.4f'
                  % (name, vinc['s'], vinc['s'] - inc['s'], np.max(np.abs(vscale - scale)[ok]) if ok.any() else 0))

    variables = [{'name': b['name'], 'expr': b['expr'], 'edges': [float(e) for e in b['edges']]} for b in cfg['binning']]
    smap = cs.ScaleMap(variables, scale, error, status, syst=syst, inclusive={'s': inc['s'], 's_err': inc['s_err']},
                       info={'selection': cfg.get('selection'), 'model': model, 'datasets': cfg['datasets'],
                             'n_candidates': int(len(df)), 'fixed_in_bins': cfg.get('fix_in_bins', []),
                             'fixed_values': {k: inc['values'][k] for k in cfg.get('fix_in_bins', [])},
                             'r21_fixed': bmodel['fixed'].get('r21'), 'r21_from': cfg.get('r21_from'),
                             'r21_bins': (dict(bmodel['r21_bins'], values=bmodel['r21_bins']['values'].tolist())
                                          if bmodel.get('r21_bins') else None)})
    smap.to_json(os.path.join(out, 'conv_scale.json'))

    # 4. cross-checks
    for cc in cfg.get('cross_checks', []):
        ccfg = dict(cfg, binning=[cc], min_candidates=cfg.get('min_candidates', 0))
        # free tail: the shape changes along most cross-check variables (r21 stays fixed if r21_from is set)
        crow, _, _, _, cres = run_map(df, y, ccfg, bmodel, inc, 'check_' + cc['name'], fix=[])
        rows += crow
        print('cross-check %s:' % cc['name'])
        for r in crow:
            print('  %8.2f–%-8.2f n %7d status %d  s = %s' % (r[cc['name'] + '_lo'], r[cc['name'] + '_hi'], r['n_range'],
                  r['status'], '%.4f ± %.4f' % (r['s'], r['s_err']) if 's' in r else '-'))
        if not args.no_plots:
            plot_1d(crow, cc['name'], os.path.join(out, 'check_%s.png' % cc['name']))

    # 5. closure: the map applied per candidate, refits inclusive and in the (finer) cross-check bins.
    # In the map bins themselves the closure is exact by construction when the bin values are used.
    # Only the candidates inside the map binning (e.g. pt_est above the lowest edge).
    interpolate = bool(cfg.get('interpolate', False))
    inmap = np.all(cs.bin_index(df, cfg['binning']) >= 0, axis=1)
    df, y = df[inmap], y[inmap]
    yc = y / smap.lookup_df(df, interpolate=interpolate)
    # start at the inclusive shape with s = 1: from the generic start values the fit of 10^6-10^7 candidates
    # can end in a false minimum (alpha at its limit)
    clo = cs.fit(yc, bmodel, start=dict(inc['values'], s=1.0, ntot=inc['ntot']))
    rows.append({'fit': 'closure_inclusive', 'bin': [], 'n_range': clo['n'], 'status': 0 if clo['valid'] else 1,
                 's': clo['s'], 's_err': clo['s_err'], 'sigmaRel': clo['values']['sigmaRel'], 'n_sig': clo['n_sig'],
                 'chi2': clo['chi2'], 'ndf': clo['ndf']})
    print('closure (%s, %d candidates inside the map): inclusive s = %.4f ± %.4f (valid %s)'
          % ('interpolated' if interpolate else 'bin values', len(df), clo['s'], clo['s_err'], clo['valid']))
    for cc in cfg.get('cross_checks', []):
        ccfg = dict(cfg, binning=[cc])
        crow, cscale, cerr, cstatus, _ = run_map(df, yc, ccfg, bmodel, clo, 'closure_' + cc['name'], fix=[])
        rows += crow
        ok = cstatus == 0
        print('  closure %-8s: %s' % (cc['name'], '  '.join('%.4f±%.4f' % (a, b) if o else '-'
                                                          for a, b, o in zip(cscale, cerr, ok))))

    pd.DataFrame(rows).to_csv(os.path.join(out, 'conv_scale.csv'), index=False)

    # map printout
    names = [b['name'] for b in cfg['binning']]
    print('\nmap (%s): s ± stat ± syst [status]' % ' x '.join(names))
    for b in cs.all_bins(cfg['binning']):
        rng = ', '.join('%s %.2f–%.2f' % (bb['name'], bb['edges'][k], bb['edges'][k + 1]) for k, bb in zip(b, cfg['binning']))
        print('  %-40s %.4f ± %.4f ± %.4f [%d]' % (rng, scale[b], error[b], syst[b], status[b]))

    if not args.no_plots:
        plot_fit(inc, os.path.join(out, 'fit_inclusive.png'), 'inclusive')
        plot_fit(clo, os.path.join(out, 'fit_closure_inclusive.png'), 'inclusive, after the correction')
        for b, r in results.items():
            rng = ', '.join('%s %.2f–%.2f' % (bb['name'], bb['edges'][k], bb['edges'][k + 1]) for k, bb in zip(b, cfg['binning']))
            plot_fit(r, os.path.join(out, 'fits', 'fit_' + '_'.join(map(str, b)) + '.png'), rng)
        plot_summary(smap, os.path.join(out, 'scale_map.png'))
    print('output in %s (%.0f s)' % (out, time.time() - t0))


if __name__ == '__main__':
    main()
