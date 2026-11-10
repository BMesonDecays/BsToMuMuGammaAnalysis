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
