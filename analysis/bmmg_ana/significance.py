"""Working-point scan: expected signal and background above every BDT threshold, and figures of merit.

S(t): weighted signal MC in the signal mass window with score >= t.
B(t): data in the mass sidebands with score >= t, extrapolated into the signal window with a factor
      from a flat or exponential background shape (fitted to the sidebands).
All thresholds are computed at once from sorted scores and cumulative sums.
"""
import numpy as np
import pandas as pd
from scipy.optimize import minimize_scalar


def cumulative_above(scores, weights, thresholds):
    """Sum of the weights with score >= t, for every t in thresholds."""
    scores = np.asarray(scores, dtype=float)
    weights = np.ones_like(scores) if weights is None else np.asarray(weights, dtype=float)
    order = np.argsort(scores, kind='stable')
    s, w = scores[order], weights[order]
    above = np.concatenate([np.cumsum(w[::-1])[::-1], [0.]])   # above[i] = sum of w[i:]
    return above[np.searchsorted(s, np.asarray(thresholds, dtype=float), side='left')]


def _window_width(windows):
    return sum(hi - lo for lo, hi in windows)


def exp_slope(mass, sidebands):
    """Unbinned ML fit of exp(-lambda m), normalised over the sidebands; returns lambda."""
    mass = np.asarray(mass, dtype=float)
    if len(mass) < 2:
        return 0.

    def integral(lam):
        if abs(lam) < 1e-9:
            return _window_width(sidebands)
        return sum((np.exp(-lam * lo) - np.exp(-lam * hi)) / lam for lo, hi in sidebands)

    def nll(lam):
        return lam * mass.sum() + len(mass) * np.log(integral(lam))

    return minimize_scalar(nll, bounds=(-20., 20.), method='bounded').x


def sideband_factor(signal_window, sidebands, model='flat', sideband_mass=None):
    """Expected background in the signal window per background candidate in the sidebands."""
    sw = [tuple(signal_window)]
    if model == 'flat':
        return _window_width(sw) / _window_width(sidebands)
    if model == 'exp':
        lam = exp_slope(sideband_mass, sidebands)

        def integral(windows):
            if abs(lam) < 1e-9:
                return _window_width(windows)
            return sum((np.exp(-lam * lo) - np.exp(-lam * hi)) / lam for lo, hi in windows)

        return integral(sw) / integral(sidebands)
    raise ValueError('background model must be flat or exp, not %s' % model)


def asimov_z(s, b):
    """Median discovery significance for s signal over b background (Asimov)."""
    s, b = np.asarray(s, dtype=float), np.asarray(b, dtype=float)
    with np.errstate(divide='ignore', invalid='ignore'):
        z = np.sqrt(2 * ((s + b) * np.log1p(s / b) - s))
    return np.where(b > 0, z, np.nan)


def scan(sig_scores, sig_weights, bkg_scores, bkg_factor, thresholds=None, punzi_a=3.0, min_sideband=10,
         bkg_weights=None):
    """Figures of merit vs the score threshold.

    sig_scores/sig_weights: signal MC candidates inside the signal window.
    bkg_scores: data candidates in the sidebands; bkg_factor: sideband -> signal-window factor.
    thresholds: default 1000 points from the score quantiles of signal and background.
    min_sideband: thresholds with fewer sideband candidates get NaN figures of merit (B too uncertain).
    Returns a DataFrame with threshold, S, B, n_sideband, eff_sig, eff_bkg, s_over_sqrt_b, asimov_z, punzi.
    """
    sig_scores, bkg_scores = np.asarray(sig_scores, dtype=float), np.asarray(bkg_scores, dtype=float)
    if thresholds is None:
        q = np.linspace(0, 1, 1000)
        thresholds = np.unique(np.concatenate([np.quantile(sig_scores, q), np.quantile(bkg_scores, q)]))
    thresholds = np.asarray(thresholds, dtype=float)
    S = cumulative_above(sig_scores, sig_weights, thresholds)
    nsb = cumulative_above(bkg_scores, bkg_weights, thresholds)
    nraw = cumulative_above(bkg_scores, None, thresholds)
    B = nsb * bkg_factor
    s_tot = cumulative_above(sig_scores, sig_weights, [-np.inf])[0]
    b_tot = cumulative_above(bkg_scores, bkg_weights, [-np.inf])[0]
    out = pd.DataFrame({'threshold': thresholds, 'S': S, 'B': B, 'n_sideband': nraw,
                        'eff_sig': S / s_tot if s_tot else np.nan, 'eff_bkg': nsb / b_tot if b_tot else np.nan})
    ok = nraw >= min_sideband
    with np.errstate(divide='ignore', invalid='ignore'):
        out['s_over_sqrt_b'] = np.where(ok & (B > 0), S / np.sqrt(B), np.nan)
        out['asimov_z'] = np.where(ok, asimov_z(S, B), np.nan)
        out['punzi'] = np.where(ok, out['eff_sig'] / (punzi_a / 2 + np.sqrt(B)), np.nan)
    return out


def best(scan_df, fom='punzi'):
    """Row of the scan with the highest figure of merit."""
    if scan_df[fom].notna().sum() == 0:
        raise ValueError('no threshold with a valid %s (too few sideband candidates?)' % fom)
    return scan_df.loc[scan_df[fom].idxmax()]
