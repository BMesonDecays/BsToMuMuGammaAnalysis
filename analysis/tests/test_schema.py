import awkward as ak
import numpy as np

from bmmg_ana.schema import MISSING, build_table, needed_branches


def toy_events():
    """Two events: event 0 has 2 muons, 1 photon, 1 dimuon, 2 mmg candidates; event 1 has no candidate."""
    return ak.Array({
        'evt_run': np.array([1, 1], dtype=np.uint32),
        'evt_event': np.array([10, 11], dtype=np.uint64),
        'gen_B_mass': np.array([5.37, 5.37], dtype=np.float32),
        'gen_pho_pt': [[3.0], []],                      # jagged gen branch: must not be copied
        'mu_pt': [[5.0, 4.0], [7.0]],
        'mu_isSoft': [[1, 0], [1]],
        'pho_pt': [[2.5], []],
        'dimu_mass': [[3.1], []],
        'mmg_mass': [[5.3, 5.5], []],
        'mmg_vtxProb': [[0.5, 1e-5], []],
        'mmg_fitOK': [[1, 1], []],
        'mmg_mu1': [[0, 1], []],
        'mmg_mu2': [[1, 0], []],
        'mmg_pho': [[0, -1], []],                      # second candidate: missing photon
        'mmg_dimu': [[0, 0], []],
    })


def test_build_table_gather_and_broadcast():
    df = build_table(toy_events(), 'mmg')
    assert len(df) == 2
    assert np.allclose(df['mass'], [5.3, 5.5])
    assert list(df['mu1_pt']) == [5.0, 4.0] and list(df['mu2_pt']) == [4.0, 5.0]
    assert list(df['mu1_isSoft']) == [1, 0]
    assert df['pho_pt'].iloc[0] == 2.5 and df['pho_pt'].iloc[1] == MISSING
    assert np.allclose(df['dimu_mass'], [3.1, 3.1])
    assert list(df['evt_event']) == [10, 10] and list(df['cand_idx']) == [0, 1] and list(df['n_cand']) == [2, 2]
    assert 'gen_B_mass' in df.columns and 'gen_pho_pt' not in df.columns
    assert df['mu1_pt'].dtype == toy_events()['mu_pt'].layout.content.dtype   # type of the input branch kept
    assert df['mu1_isSoft'].dtype.kind == 'i'


def test_preselection_and_drop():
    df = build_table(toy_events(), 'mmg', preselection='vtxProb > 1e-3', drop=['gen_*', 'evt_event'])
    assert len(df) == 1 and df['cand_idx'].iloc[0] == 0
    assert 'gen_B_mass' not in df.columns
    assert 'evt_event' in df.columns   # event ids are protected


def test_absent_channel_and_needed_branches():
    ev = toy_events()
    assert build_table(ev, 'mmkk') is None
    br = needed_branches(ev.fields, ['mmg', 'mmkk'])
    assert 'mmg_mass' in br and 'mu_pt' in br and 'pho_pt' in br and 'evt_event' in br


def test_missing_leg_with_empty_collection_and_bad_index():
    import pytest
    ev = ak.Array({'evt_event': np.array([1, 2], dtype=np.uint64),
                   'mu_pt': [[5.0, 4.0], [6.0, 3.0]], 'pho_pt': [[], [2.0]], 'dimu_mass': [[3.1], [3.0]],
                   'mmg_mass': [[5.3], [5.4]], 'mmg_mu1': [[0], [0]], 'mmg_mu2': [[1], [1]],
                   'mmg_pho': [[-1], [0]], 'mmg_dimu': [[0], [0]]})
    df = build_table(ev, 'mmg')        # event 1: no photon at all, leg index -1
    assert list(df['pho_pt']) == [MISSING, 2.0] and df['pho_pt'].dtype == np.float64
    bad = ak.with_field(ev, ak.Array([[0], [3]]), 'mmg_mu1')
    with pytest.raises(ValueError, match='points beyond mu_pt .* event 2'):
        build_table(bad, 'mmg')
