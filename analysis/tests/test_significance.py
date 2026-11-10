import numpy as np
import pytest

from bmmg_ana import significance as sg


def test_cumulative_above_matches_brute_force():
    rng = np.random.default_rng(1)
    s, w = rng.random(1000), rng.random(1000)
    t = np.linspace(-0.1, 1.1, 57)
    brute = [w[s >= x].sum() for x in t]
    assert np.allclose(sg.cumulative_above(s, w, t), brute)
    assert np.allclose(sg.cumulative_above(s, None, t), [(s >= x).sum() for x in t])


def test_scan_by_hand():
    sig, bkg = np.array([0.9, 0.8, 0.2]), np.array([0.85, 0.1, 0.1, 0.1])
    res = sg.scan(sig, [2., 2., 2.], bkg, bkg_factor=0.5, thresholds=[0.0, 0.5], punzi_a=2, min_sideband=1)
    r0, r1 = res.iloc[0], res.iloc[1]
    assert (r0.S, r0.B, r0.n_sideband) == (6., 2., 4)
    assert (r1.S, r1.B, r1.n_sideband) == (4., 0.5, 1)
    assert r1.eff_sig == pytest.approx(4 / 6) and r1.punzi == pytest.approx((4 / 6) / (1 + np.sqrt(0.5)))
    assert r1.s_over_sqrt_b == pytest.approx(4 / np.sqrt(0.5))
    # min_sideband: too few sideband candidates -> NaN
    res = sg.scan(sig, None, bkg, 0.5, thresholds=[0.5], min_sideband=2)
    assert np.isnan(res['punzi'].iloc[0])


def test_asimov_limit():
    # s << b: Z -> s / sqrt(b)
    assert sg.asimov_z(1., 1e4) == pytest.approx(1 / 100, rel=1e-3)


def test_sideband_factor():
    sw, sb = [5.2, 5.5], [[4.5, 5.0], [5.8, 6.3]]
    assert sg.sideband_factor(sw, sb, 'flat') == pytest.approx(0.3)
    rng = np.random.default_rng(2)
    m = 4.5 + rng.exponential(1 / 1.5, 200000)            # slope 1.5 / GeV
    m = m[((m >= 4.5) & (m < 5.0)) | ((m >= 5.8) & (m < 6.3))]
    assert sg.exp_slope(m, sb) == pytest.approx(1.5, rel=0.02)
    f = sg.sideband_factor(sw, sb, 'exp', m)
    lam = 1.5
    expected = (np.exp(-lam * 5.2) - np.exp(-lam * 5.5)) / sum(np.exp(-lam * lo) - np.exp(-lam * hi) for lo, hi in sb)
    assert f == pytest.approx(expected, rel=0.02)


def test_invalid_inputs_rejected():
    with pytest.raises(ValueError, match='not finite'):
        sg.cumulative_above([0.2, np.nan], None, [0.8])     # the NaN used to count as passing
    with pytest.raises(ValueError, match='not finite'):
        sg.scan([0.5], [np.inf], [0.1], 1.0)
    with pytest.raises(ValueError, match='empty'):
        sg.scan([], None, [0.1], 1.0)
    with pytest.raises(ValueError, match='hi must be above lo'):
        sg.sideband_factor([5.6, 5.15], [[4.5, 5.0]], 'flat')
