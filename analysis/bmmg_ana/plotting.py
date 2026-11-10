"""Plots with matplotlib + mplhep (CMS style) and hist."""
import os

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa: E402
import mplhep as hep  # noqa: E402
import numpy as np  # noqa: E402
from hist import Hist  # noqa: E402

hep.style.use('CMS')


def _label(ax, data=False, lumi=None, label='Preliminary'):
    """CMS label; data=False adds 'Simulation'."""
    kw = dict(ax=ax, data=data, label=label, loc=0, com=13.6)
    if lumi:
        kw['lumi'] = '%.3g' % lumi
    try:
        hep.cms.label(**kw)
    except TypeError:   # other mplhep signatures
        hep.cms.text(label if data else 'Simulation ' + label, ax=ax)


def save(fig, path):
    os.makedirs(os.path.dirname(os.path.abspath(path)) or '.', exist_ok=True)
    fig.savefig(path, bbox_inches='tight')
    if path.endswith('.png'):
        fig.savefig(path[:-4] + '.pdf', bbox_inches='tight')
    plt.close(fig)


def histogram(values, bins, rng, weights=None):
    h = Hist.new.Reg(bins, *rng, flow=False).Weight()
    v = np.asarray(values, dtype=float)
    ok = np.isfinite(v)
    h.fill(v[ok], weight=None if weights is None else np.asarray(weights, dtype=float)[ok])
    return h


def shapes(samples, var, bins, rng, path, xlabel=None, logy=False, data=True):
    """Normalised shape comparison. samples: {label: (values, weights or None)}."""
    fig, ax = plt.subplots()
    for label, (values, weights) in samples.items():
        h = histogram(values, bins, rng, weights)
        total = h.sum().value
        if total > 0:
            hep.histplot(h / total, ax=ax, label=label, histtype='step', linewidth=2)
    ax.set_xlabel(xlabel or var)
    ax.set_ylabel('fraction of candidates / bin')
    if logy:
        ax.set_yscale('log')
    ax.legend()
    _label(ax, data=data)
    save(fig, path)


def data_vs_mc(data_values, mc, var, bins, rng, path, xlabel=None, blind=None, lumi=None, logy=False):
    """Data points over stacked MC with a data/MC ratio panel.

    mc: {label: (values, weights)}; blind: (lo, hi) window not shown for data.
    """
    if mc:
        fig, (ax, rax) = plt.subplots(2, 1, sharex=True, gridspec_kw=dict(height_ratios=[3, 1], hspace=0.05))
    else:
        fig, ax = plt.subplots()
    d = np.asarray(data_values, dtype=float)
    if blind:
        d = d[(d < blind[0]) | (d >= blind[1])]
    hd = histogram(d, bins, rng)
    centers = hd.axes[0].centers
    hidden = ((centers >= blind[0]) & (centers < blind[1])) if blind else np.zeros(len(centers), bool)
    hmc = [histogram(v, bins, rng, w) for v, w in mc.values()]
    if hmc:
        hep.histplot(hmc, ax=ax, stack=True, histtype='fill', label=list(mc.keys()))
    ax.errorbar(centers[~hidden], hd.values()[~hidden], yerr=np.sqrt(hd.variances()[~hidden]),
                fmt='o', color='black', markersize=4, label='data')
    if blind:
        ax.axvspan(*blind, color='grey', alpha=0.3, label='blinded')
    ax.set_xlim(*rng)
    ax.set_ylim(bottom=0)
    ax.set_ylabel('candidates / bin')
    if logy:
        ax.set_yscale('log')
    ax.legend()
    _label(ax, data=True, lumi=lumi)
    if not hmc:
        ax.set_xlabel(xlabel or var)
        save(fig, path)
        return
    ax.set_xlabel('')
    tot = sum(h.values() for h in hmc)
    with np.errstate(divide='ignore', invalid='ignore'):
        ratio = np.where(hidden, np.nan, hd.values() / tot)
        err = np.sqrt(hd.variances()) / tot
    rax.errorbar(centers, ratio, yerr=err, fmt='o', color='black', markersize=3)
    rax.axhline(1, color='grey', linestyle='--')
    rax.set_ylim(0, 2)
    rax.set_ylabel('data / MC')
    rax.set_xlabel(xlabel or var)
    save(fig, path)


def roc(labels, scores, path, weights=None):
    from sklearn.metrics import roc_auc_score, roc_curve
    fpr, tpr, _ = roc_curve(labels, scores, sample_weight=weights)
    auc = roc_auc_score(labels, scores, sample_weight=weights)
    fig, ax = plt.subplots()
    ax.plot(tpr, 1 - fpr, label='AUC = %.4f' % auc)
    ax.set_xlabel('signal efficiency')
    ax.set_ylabel('background rejection')
    ax.legend(loc='lower left')
    save(fig, path)
    return auc


def overtraining(train, test, path, bins=40):
    """train/test: {'signal': scores, 'background': scores}; train as histograms, test as points."""
    fig, ax = plt.subplots()
    for (cls, color) in (('signal', 'tab:blue'), ('background', 'tab:red')):
        h = histogram(train[cls], bins, (0, 1))
        hep.histplot(h / max(h.sum().value, 1e-12), ax=ax, histtype='fill', alpha=0.3, color=color, label=cls + ' (train)')
        h = histogram(test[cls], bins, (0, 1))
        hep.histplot(h / max(h.sum().value, 1e-12), ax=ax, histtype='errorbar', color=color, label=cls + ' (test)')
    ax.set_xlabel('BDT score')
    ax.set_ylabel('fraction / bin')
    ax.set_yscale('log')
    ax.legend()
    save(fig, path)


def importance(series, path):
    fig, ax = plt.subplots(figsize=(8, 0.4 * len(series) + 2))
    s = series[::-1]
    ax.barh(range(len(s)), s.values)
    ax.set_yticks(range(len(s)))
    ax.set_yticklabels(s.index, fontsize=14)
    ax.set_xlabel('mean gain')
    save(fig, path)


def scan(scan_df, foms, path, best_threshold=None):
    fig, axes = plt.subplots(len(foms) + 1, 1, sharex=True, figsize=(9, 3 * (len(foms) + 1)))
    axes[0].plot(scan_df['threshold'], scan_df['S'], label='S')
    axes[0].plot(scan_df['threshold'], scan_df['B'], label='B')
    axes[0].set_yscale('log')
    axes[0].legend()
    for ax, fom in zip(axes[1:], foms):
        ax.plot(scan_df['threshold'], scan_df[fom])
        ax.set_ylabel(fom)
    for ax in axes:
        if best_threshold is not None:
            ax.axvline(best_threshold, color='grey', linestyle='--')
    axes[-1].set_xlabel('BDT score threshold')
    save(fig, path)
