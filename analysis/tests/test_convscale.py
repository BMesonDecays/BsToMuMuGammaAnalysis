import numpy as np
import pandas as pd
import pytest

from bmmg_ana import convscale as cs

pytest.importorskip('iminuit')


def sample(rng, n, pdf, lo, hi):
    """Inverse-CDF sampling of a 1D density on [lo, hi]."""
    x = np.linspace(lo, hi, 20001)
    c = np.cumsum(pdf(x))
    c /= c[-1]
    return np.interp(rng.random(n), c, x)


def toy(rng, s, n1=4000, r21=0.4, nbkg=30000, sigma_rel=0.03, alpha=1.0, n=2.0):
    lo, hi = cs.DEFAULT_MODEL['range']
    parts = []
    for j, nj in ((1, n1), (2, int(r21 * n1))):
        m = s * cs.Y_PEAK[j]
        parts.append(sample(rng, nj, lambda x: cs.crystal_ball(x, m, sigma_rel * m, alpha, n), lo, hi))
    parts.append(sample(rng, nbkg, lambda x: 1.2 - 0.15 * x, lo, hi))
    return np.concatenate(parts)


def test_fit_recovers_scale():
    rng = np.random.default_rng(1)
    pulls = []
    for s_true in (0.95, 0.98, 1.0):
        r = cs.fit(toy(rng, s_true))
        assert r['valid']
        assert abs(r['values']['r21'] - 0.4) < 0.1
        pulls.append((r['s'] - s_true) / r['s_err'])
    assert np.all(np.abs(pulls) < 3.5), pulls


def test_fixed_parameters():
    rng = np.random.default_rng(2)
    r = cs.fit(toy(rng, 0.97), {'fixed': {'n': 2.0, 'r21': 0.4}})
    assert r['values']['r21'] == 0.4 and r['values']['n'] == 2.0
    assert abs(r['s'] - 0.97) < 4 * r['s_err']


def test_pt_est_is_independent_of_the_response():
    # chi_c1: y and conv_pt scale together with the photon response r
    pt_true = np.array([1.2, 2.5, 4.0])
    for r in (0.8, 0.97, 1.05):
        assert np.allclose(cs.pt_est(r * pt_true, r * cs.Y_PEAK[1]), pt_true)


def test_corrected_mass():
    mmu = 3.1
    m = np.sqrt(mmu ** 2 + 0.98 * 2.734)
    assert np.isclose(cs.corrected_mass(m, mmu, 0.98) ** 2 - mmu ** 2, 2.734)


def test_scalemap_json_and_lookup(tmp_path):
    variables = [{'name': 'pt', 'expr': 'pt_est', 'edges': [1., 2., 4.]},
                 {'name': 'abseta', 'expr': 'abs(conv_eta)', 'edges': [0., 1.5, 2.5]}]
    scale = [[0.99, 0.95], [0.98, 0.93]]
    m = cs.ScaleMap(variables, scale, [[0.001] * 2] * 2, [[0, 0], [0, 2]], syst=[[0.002] * 2] * 2,
                    inclusive={'s': 0.98, 's_err': 0.001})
    p = tmp_path / 'map.json'
    m.to_json(p)
    m2 = cs.ScaleMap.from_json(p)
    assert np.allclose(m2.scale, scale) and m2.status[1, 1] == 2 and np.allclose(m2.syst, 0.002)
    # bin values, outside the edges clamped to the outer bins
    assert np.allclose(m2.lookup([1.5, 3., 0.5, 10.], [0.2, 2.0, 0.2, 3.0], interpolate=False),
                       [0.99, 0.93, 0.99, 0.93])
    # multilinear between bin centres (1.5, 3) x (0.75, 2), clamped outside
    assert np.isclose(m2.lookup([2.25], [0.75])[0], 0.985)
    assert np.isclose(m2.lookup([10.], [0.])[0], 0.98)
    df = pd.DataFrame({'pt_est': [1.5], 'conv_eta': [-2.0]})
    assert np.isclose(m2.lookup_df(df, interpolate=False)[0], 0.95)


def test_bin_index():
    df = pd.DataFrame({'a': [0.5, 1.5, 2.5, -1.0, np.nan]})
    idx = cs.bin_index(df, [{'name': 'a', 'expr': 'a', 'edges': [0, 1, 2]}])
    assert list(idx[:, 0]) == [0, 1, -1, -1, -1]


def test_fill_along():
    # 4 pt bins x 2 eta slices; slice 0: last two bins empty, slice 1: first bin empty, third failed
    scale = np.array([[0.99, 0.95], [0.98, 0.94], [0.97, 0.93], [0.96, 0.92]])
    error = np.full((4, 2), 0.001)
    syst = np.full((4, 2), 0.002)
    status = np.array([[0, 2], [0, 0], [2, 1], [2, 0]])
    s, e, y, fills = cs.fill_along(scale, error, syst, status, axis=0)
    # from the last good bin below, step of the last two good bins (0.01) added to the systematic
    assert np.allclose(s[2:, 0], 0.98) and np.allclose(y[2:, 0], np.hypot(0.002, 0.01))
    # one good bin below: no step
    assert np.isclose(s[2, 1], 0.94) and np.isclose(y[2, 1], 0.002)
    # no good bin below (first bin) and good bins are unchanged
    assert s[0, 1] == 0.95 and s[3, 1] == 0.92 and s[1, 0] == 0.98
    assert {tuple(f['bin']) for f in fills} == {(2, 0), (3, 0), (2, 1)}
