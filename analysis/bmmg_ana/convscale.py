"""Energy scale of the converted photons from chi_c -> J/psi gamma(conversion) (channel chic).

Fit variable (per candidate, from the chic table):
    y = m(mu mu gamma)^2 - m(mu mu)^2 = 2 p(mu mu).p(gamma)   [GeV^2]
(both masses from the muon momentum sum, the photon massless). y is linear in the photon energy, so a
photon energy scale s moves the chi_cJ peaks to s * (m_chicJ^2 - m_J/psi^2). The variant with the PDG
J/psi mass, Q (Q + 2 m_J/psi), gives the same scale and width on data.

Model (binned extended likelihood, iminuit):
    chi_c0 (optional), chi_c1, chi_c2: Crystal Ball shapes with the tail on the low side (bremsstrahlung),
        mean s*Y_J, width sigmaRel*s*Y_J, common alpha and n; yields N1, N2 = r21*N1, N0 = r01*N1
        (where the resolution is poor the two peaks merge and the ratio r21 is degenerate with s: fix it);
    background: Chebyshev polynomial (order from the config) in the fit range.
The fitted s is the peak (core) scale; the correction of the photon momentum is 1/s.
"""
import itertools
import json

import numpy as np

M_JPSI = 3.096900
M_CHIC = {0: 3.41471, 1: 3.51067, 2: 3.55617}
Y_PEAK = {j: m ** 2 - M_JPSI ** 2 for j, m in M_CHIC.items()}   # m_chicJ^2 - m_J/psi^2


def y_from_masses(mass, dimu_mass):
    """Fit variable y = m(mu mu gamma)^2 - m(mu mu)^2 [GeV^2]."""
    mass, dimu_mass = np.asarray(mass, dtype=float), np.asarray(dimu_mass, dtype=float)
    return mass ** 2 - dimu_mass ** 2


def pt_est(conv_pt, y):
    """conv_pt * Y1 / y: the true photon pt of a chi_c1 candidate, independent of the photon energy response."""
    return np.asarray(conv_pt, dtype=float) * Y_PEAK[1] / np.asarray(y, dtype=float)


def q_from_y(y):
    """Q = m - m_J/psi for a dimuon at the PDG J/psi mass (display units)."""
    return np.sqrt(M_JPSI ** 2 + np.asarray(y, dtype=float)) - M_JPSI


def corrected_mass(mass, dimu_mass, s):
    """m(mu mu gamma) with the photon momentum divided by s: m'^2 = m_mumu^2 + (m^2 - m_mumu^2) / s."""
    mass, dimu_mass = np.asarray(mass, dtype=float), np.asarray(dimu_mass, dtype=float)
    return np.sqrt(dimu_mass ** 2 + (mass ** 2 - dimu_mass ** 2) / np.asarray(s, dtype=float))


# ---------------------------------------------------------------------------------------------- model

def crystal_ball(x, mean, sigma, alpha, n):
    """Unnormalised Crystal Ball, power-law tail below mean - alpha*sigma."""
    t = (np.asarray(x) - mean) / sigma
    out = np.exp(-0.5 * t * t)
    low = t < -alpha
    if np.any(low):
        a = (n / alpha) ** n * np.exp(-0.5 * alpha * alpha)
        b = n / alpha - alpha
        out = np.where(low, a * np.power(np.maximum(b - t, 1e-12), -n), out)
    return out


DEFAULT_MODEL = {
    'range': [1.6, 5.2],      # y [GeV^2], Q ~ 0.25 - 0.75 GeV
    'bins': 90,
    'chic0': False,           # include the chi_c0 peak (BR(J/psi gamma) 1.4%: <1% of the signal)
    'bkg_order': 2,           # Chebyshev order of the background
    'sub': 8,                 # integration points per bin
    'fixed': {'n': 2.0},      # parameters fixed to these values (the CB n is degenerate with the background)
}

PEAK_PARS = ('s', 'sigmaRel', 'alpha', 'n')


class Model:
    """Expected counts per bin of the chi_c peaks + background, for a fixed histogram binning."""

    def __init__(self, cfg=None):
        self.cfg = dict(DEFAULT_MODEL, **(cfg or {}))
        lo, hi = self.cfg['range']
        self.edges = np.linspace(lo, hi, int(self.cfg['bins']) + 1)
        k = int(self.cfg['sub'])
        w = np.diff(self.edges)
        # midpoints of k sub-intervals per bin
        self.grid = (self.edges[:-1, None] + w[:, None] * (np.arange(k) + 0.5) / k)
        self.dx = (w / k)[:, None]
        self.peaks = [0, 1, 2] if self.cfg['chic0'] else [1, 2]
        self.nbkg = int(self.cfg['bkg_order'])
        self.par_names = (list(PEAK_PARS) + ['N1', 'r21'] + (['r01'] if 0 in self.peaks else []) + ['Nbkg']
                          + ['c%d' % i for i in range(1, self.nbkg + 1)])

    @staticmethod
    def yields(p):
        """Yields of the peaks, {J: N_J}."""
        y = {1: p['N1'], 2: p['r21'] * p['N1']}
        if 'r01' in p:
            y[0] = p['r01'] * p['N1']
        return y

    def _norm_counts(self, dens):
        tot = (dens * self.dx).sum()
        return (dens * self.dx).sum(axis=1) / tot if tot > 0 else np.zeros(len(self.edges) - 1)

    def components(self, p):
        """Expected counts per bin of each component, dict name -> array."""
        out = {}
        yields = self.yields(p)
        for j in self.peaks:
            m = p['s'] * Y_PEAK[j]
            out['chic%d' % j] = yields[j] * self._norm_counts(
                crystal_ball(self.grid, m, p['sigmaRel'] * m, p['alpha'], p['n']))
        lo, hi = self.cfg['range']
        x = (2 * self.grid - lo - hi) / (hi - lo)
        coef = [1.] + [p['c%d' % i] for i in range(1, self.nbkg + 1)]
        bkg = np.maximum(np.polynomial.chebyshev.chebval(x, coef), 1e-9)
        out['bkg'] = p['Nbkg'] * self._norm_counts(bkg)
        return out

    def expected(self, p):
        return sum(self.components(p).values())


# ------------------------------------------------------------------------------------------------ fit

def fit(y, cfg=None, start=None, fixed=(), limits=None):
    """Fit the y values of one sample. Returns a result dict.

    start: starting values (e.g. from the inclusive fit); fixed: names of parameters kept at start.
    cfg['fixed']: {name: value} of parameters fixed to a value (overrides start).
    Result keys: values, errors, s, s_err, n_sig, n, valid, at_limit, chi2, ndf, nll, counts, expected, model.
    valid: Minuit converged with an accurate covariance and no free shape parameter at a limit.
    """
    from iminuit import Minuit

    model = Model(cfg)
    counts, _ = np.histogram(np.asarray(y, dtype=float), bins=model.edges)
    ntot = counts.sum()
    p0 = {'s': 0.97, 'sigmaRel': 0.04, 'alpha': 1.0, 'n': 3.0, 'Nbkg': 0.8 * ntot, 'N1': 0.12 * ntot, 'r21': 0.5,
          'r01': 0.05}
    for i in range(1, model.nbkg + 1):
        p0['c%d' % i] = 0.
    if start:
        p0.update({k: v for k, v in start.items() if k in p0})
        # yields from a larger sample: rescale to this one
        if 'ntot' in start and start['ntot'] > 0:
            for k in p0:
                if k.startswith('N'):
                    p0[k] = start[k] * ntot / start['ntot']
    p0.update(model.cfg.get('fixed') or {})
    fixed = set(fixed) | set(model.cfg.get('fixed') or {})
    names = model.par_names

    def nll(*vals):
        mu = model.expected(dict(zip(names, vals)))
        mu = np.maximum(mu, 1e-12)
        return 2 * float(np.sum(mu - counts * np.log(mu)))   # -2 ln L (up to a constant)

    m = Minuit(nll, *[p0[k] for k in names], name=names)
    m.errordef = Minuit.LEAST_SQUARES   # nll is -2 ln L: errordef 1
    lim = {'s': (0.7, 1.3), 'sigmaRel': (0.003, 0.3), 'alpha': (0.2, 10.), 'n': (1.01, 50.), 'Nbkg': (0, None),
           'N1': (0, None), 'r21': (0., 5.), 'r01': (0., 1.)}
    for i in range(1, model.nbkg + 1):
        lim['c%d' % i] = (-2., 2.)
    lim.update(limits or {})
    for k, v in lim.items():
        if k in names:
            m.limits[k] = v
    for k in fixed:
        m.fixed[k] = True
    m.strategy = 1
    m.migrad()
    m.hesse()
    vals = {k: float(m.values[k]) for k in names}
    errs = {k: float(m.errors[k]) for k in names}
    at_limit = [k for k in ('s', 'sigmaRel', 'alpha', 'n') if not m.fixed[k] and m.limits[k][0] is not None
                and min(abs(vals[k] - m.limits[k][0]), abs(vals[k] - m.limits[k][1])) < 1e-3 * abs(m.limits[k][1] - m.limits[k][0])]
    mu = model.expected(vals)
    ok = mu > 0
    chi2 = float(np.sum((counts[ok] - mu[ok]) ** 2 / mu[ok]))
    nfree = len(names) - len([k for k in names if m.fixed[k]])
    return {
        'values': vals, 'errors': errs, 's': vals['s'], 's_err': errs['s'],
        'n_sig': sum(model.yields(vals).values()), 'yields': model.yields(vals), 'n': int(ntot), 'ntot': float(ntot),
        'valid': bool(m.valid and m.fmin.has_accurate_covar and not at_limit), 'at_limit': at_limit, 'chi2': chi2, 'ndf': int(ok.sum() - nfree),
        'nll': float(m.fval), 'counts': counts, 'expected': mu, 'model': model,
    }


# -------------------------------------------------------------------------------------------- binning

def bin_index(df, binning):
    """Bin numbers of the candidates for an N-dimensional binning.

    binning: list of {'name', 'expr' (pandas expression), 'edges'}. Returns an (n_cand, N) int array,
    -1 outside the edges.
    """
    idx = []
    for b in binning:
        v = np.asarray(df.eval(b['expr']), dtype=float)
        e = np.asarray(b['edges'], dtype=float)
        i = np.searchsorted(e, v, side='right') - 1
        i[(v < e[0]) | (v >= e[-1]) | ~np.isfinite(v)] = -1
        idx.append(i)
    return np.stack(idx, axis=1) if idx else np.zeros((len(df), 0), dtype=int)


def all_bins(binning):
    return list(itertools.product(*[range(len(b['edges']) - 1) for b in binning]))


# ---------------------------------------------------------------------------------------------- map

class ScaleMap:
    """Photon energy scale s in bins of N variables, with JSON I/O and a clamped multilinear lookup.

    JSON: {"variables": [{"name", "expr", "edges"}], "scale": nested lists (N-dim), "error" (statistical),
           "syst" (systematic), "status", "inclusive": {"s", "s_err"}, "info": {...}}.
    status: 0 good fit; 1 fit not valid; 2 too few candidates. Bins with status != 0 hold the scale of their
    reference fit (conv_scale.py: the |eta| slice, or the inclusive fit).
    """

    def __init__(self, variables, scale, error, status, syst=None, inclusive=None, info=None):
        self.variables = variables
        self.scale = np.asarray(scale, dtype=float)
        self.error = np.asarray(error, dtype=float)
        self.syst = np.zeros_like(self.scale) if syst is None else np.asarray(syst, dtype=float)
        self.status = np.asarray(status, dtype=int)
        self.inclusive = inclusive or {}
        self.info = info or {}

    def to_json(self, path):
        d = {'variables': self.variables, 'scale': self.scale.tolist(), 'error': self.error.tolist(),
             'syst': self.syst.tolist(),
             'status': self.status.tolist(), 'inclusive': self.inclusive, 'info': self.info,
             'definition': 'peak scale of the converted-photon energy (E_reco = s * E_true at the peak); '
                           'correct the photon momentum by 1/s. status: 0 fit ok, 1 fit not valid, '
                           '2 too few candidates (scale of the reference fit filled in).'}
        with open(path, 'w') as f:
            json.dump(d, f, indent=1)

    @classmethod
    def from_json(cls, path):
        with open(path) as f:
            d = json.load(f)
        return cls(d['variables'], d['scale'], d['error'], d['status'], d.get('syst'), d.get('inclusive'), d.get('info'))

    def centers(self):
        return [0.5 * (np.asarray(v['edges'][1:]) + np.asarray(v['edges'][:-1])) for v in self.variables]

    def lookup(self, *values, interpolate=True):
        """s at the given variable values (arrays, one per variable, in the order of the map).

        interpolate: multilinear between bin centres, constant beyond the outer centres (clamped);
        otherwise the value of the bin (values outside the edges take the outer bin).
        """
        vals = [np.atleast_1d(np.asarray(v, dtype=float)) for v in values]
        if len(vals) != len(self.variables):
            raise ValueError('expected %d variables' % len(self.variables))
        if not interpolate:
            idx = tuple(np.clip(np.searchsorted(np.asarray(v['edges']), x, side='right') - 1, 0, len(v['edges']) - 2)
                        for v, x in zip(self.variables, vals))
            return self.scale[idx]
        from scipy.interpolate import RegularGridInterpolator
        cs = self.centers()
        # dimensions with one bin: duplicate the value so the interpolator has two points
        grid, table = [], self.scale
        for d, c in enumerate(cs):
            if len(c) == 1:
                c = np.array([c[0] - 1., c[0] + 1.])
                table = np.concatenate([table, table], axis=d)
            grid.append(c)
        interp = RegularGridInterpolator(grid, table, method='linear')
        pts = np.stack([np.clip(x, g[0], g[-1]) for x, g in zip(vals, grid)], axis=-1)
        return interp(pts)

    def lookup_df(self, df, interpolate=True):
        return self.lookup(*[np.asarray(df.eval(v['expr']), dtype=float) for v in self.variables],
                           interpolate=interpolate)


# ------------------------------------------------------------------------------------------ MC truth

M_BS = 5.36693
Y_BS = M_BS ** 2 - M_JPSI ** 2   # B_s -> J/psi gamma: peak of m(mu mu gamma)^2 - m(mu mu)^2


def y_from_vectors(dimu_pt, dimu_eta, dimu_phi, dimu_mass, pho_pt, pho_eta, pho_phi):
    """m(mu mu gamma)^2 - m(mu mu)^2 = 2 p(mu mu).p(gamma) for a massless photon [GeV^2]."""
    a = [np.asarray(v, dtype=float) for v in (dimu_pt, dimu_eta, dimu_phi, dimu_mass, pho_pt, pho_eta, pho_phi)]
    dpt, deta, dphi, dm, gpt, geta, gphi = a
    dpx, dpy, dpz = dpt * np.cos(dphi), dpt * np.sin(dphi), dpt * np.sinh(deta)
    de = np.sqrt(dpx ** 2 + dpy ** 2 + dpz ** 2 + dm ** 2)
    gpx, gpy, gpz = gpt * np.cos(gphi), gpt * np.sin(gphi), gpt * np.sinh(geta)
    ge = gpt * np.cosh(geta)
    return 2 * (de * ge - dpx * gpx - dpy * gpy - dpz * gpz)


def fit_peak(x, rng=(0.6, 1.3), bins=140, fixed=None):
    """Crystal Ball fit (no background) of a response-like distribution, e.g. E_reco/E_true or y/Y_true.

    The peak position 'mean' has the same definition as s of fit() (CB mean, tail parameters as in `fixed`,
    default n = 2 as DEFAULT_MODEL). Returns dict with values, errors, valid, chi2, ndf.
    """
    from iminuit import Minuit

    fixed = {'n': 2.0} if fixed is None else fixed
    x = np.asarray(x, dtype=float)
    counts, edges = np.histogram(x, bins=bins, range=rng)
    k = 8
    w = np.diff(edges)
    grid = edges[:-1, None] + w[:, None] * (np.arange(k) + 0.5) / k
    dx = (w / k)[:, None]
    ntot = counts.sum()
    med = np.median(x[(x > rng[0]) & (x < rng[1])]) if ntot else 1.
    names = ['mean', 'sigma', 'alpha', 'n', 'N']

    def expected(p):
        d = crystal_ball(grid, p[0], p[1], p[2], p[3])
        t = (d * dx).sum()
        return p[4] * (d * dx).sum(axis=1) / t if t > 0 else np.zeros(len(counts))

    def nll(*p):
        mu = np.maximum(expected(p), 1e-12)
        return 2 * float(np.sum(mu - counts * np.log(mu)))

    p0 = {'mean': med, 'sigma': 0.03, 'alpha': 1.0, 'n': 2.0, 'N': float(ntot)}
    p0.update(fixed)
    m = Minuit(nll, *[p0[k] for k in names], name=names)
    m.errordef = Minuit.LEAST_SQUARES
    for k, v in {'mean': rng, 'sigma': (1e-3, 0.5), 'alpha': (0.2, 10.), 'n': (1.01, 50.), 'N': (0, None)}.items():
        m.limits[k] = v
    for k in fixed:
        m.fixed[k] = True
    m.migrad()
    m.hesse()
    vals = {k: float(m.values[k]) for k in names}
    mu = expected([vals[k] for k in names])
    ok = mu > 0
    return {'values': vals, 'errors': {k: float(m.errors[k]) for k in names}, 'n': int(ntot),
            'valid': bool(m.valid and m.fmin.has_accurate_covar),
            'chi2': float(np.sum((counts[ok] - mu[ok]) ** 2 / mu[ok])), 'ndf': int(ok.sum() - (5 - len(fixed))),
            'counts': counts, 'edges': edges, 'expected': mu}
