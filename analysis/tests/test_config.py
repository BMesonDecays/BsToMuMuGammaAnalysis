import pandas as pd
import pytest

from bmmg_ana import config

MASS = {'column': 'mass', 'signal_window': [5.15, 5.60], 'sidebands': [[4.5, 5.0], [5.75, 6.5]], 'blind': [5.0, 5.75]}


def test_windows_validated():
    config.check_windows(MASS)
    with pytest.raises(ValueError, match='overlaps'):
        config.check_windows({**MASS, 'sidebands': [[4.5, 5.1], [5.75, 6.5]]})
    with pytest.raises(ValueError, match='inside the blind'):
        config.check_windows({**MASS, 'signal_window': [4.9, 5.6]})


def test_blind_query():
    df = pd.DataFrame({'mass': [4.9, 5.0, 5.3, 5.75, 6.0]})
    assert list(df.query(config.blind_query({'mass': MASS}))['mass']) == [4.9, 5.75, 6.0]


def test_signal_candidate_policy(tmp_path):
    """Best candidate chosen among all preselected candidates, then the truth label (as in data)."""
    d = tmp_path / 'sig' / 'mmg'
    d.mkdir(parents=True)
    pd.DataFrame({'evt_run': [1, 1, 1], 'evt_lumi': [1, 1, 1], 'evt_event': [7, 7, 8], 'cand_idx': [0, 1, 0],
                  'mass': [5.3, 5.4, 5.35], 'vtxProb': [0.9, 0.5, 0.4], 'genMatched': [0, 1, 1],
                  'pt': [10., 11., 12.]}).to_parquet(d / 'part-00000.parquet')
    cfg = {'parquet': str(tmp_path), 'channel': 'mmg', 'derived': {}, 'preselection': {'p': 'pt > 5'},
           'best_candidate': 'vtxProb', 'mass': {**MASS}, 'bdt': {'features': ['pt']},
           'samples': {'signal': {'datasets': ['sig'], 'query': 'genMatched == 1'}}}
    sig = config.load_samples(cfg, which=('signal',))['signal']
    # event 7: the combinatorial candidate wins the ranking, so the event gives no signal candidate
    assert list(sig['evt_event']) == [8] and 'vtxProb' in sig.columns


def test_load_combine_config(tmp_path):
    import yaml
    from bmmg_ana import config
    base = {'parquet': '~/a', 'output': '~/b', 'channel': 'mmg', 'lumi_pb': 1.0,
            'mass': {'column': 'mass', 'signal_window': [5.15, 5.6], 'sidebands': [[4.5, 5.0], [5.75, 6.5]],
                     'blind': [5.0, 5.75]}}
    (tmp_path / 'analysis.yaml').write_text(yaml.safe_dump(base))
    cc = {'analysis': 'analysis.yaml', 'override': {'parquet': '/data/parquet'},
          'lumi': {'data_golden_pb': 34650, 'data_recorded_pb': 38000, 'target_recorded_pb': 326000},
          'bdt_cuts': [0.9]}
    (tmp_path / 'combine.yaml').write_text(yaml.safe_dump(cc))
    cfg = config.load_combine_config(str(tmp_path / 'combine.yaml'))
    assert cfg['parquet'] == '/data/parquet' and cfg['channel'] == 'mmg'
    assert abs(cfg['combine']['data_factor'] - 326 / 38) < 1e-12
    assert abs(cfg['lumi_pb'] - 34650 * 326 / 38) < 1e-6
    assert cfg['combine']['bdt_cuts'] == [0.9]
