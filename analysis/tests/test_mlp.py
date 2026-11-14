import numpy as np
import pandas as pd
import pytest

mlp = pytest.importorskip('bmmg_ana.mlp')   # needs TensorFlow

SMALL = {'hidden': [16, 16], 'epochs': 15, 'batch_size': 256, 'learning_rate': 3e-3}


def toy(n, shift, seed, first_event):
    rng = np.random.default_rng(seed)
    return pd.DataFrame({'x': rng.normal(shift, 1, n), 'y': rng.normal(0, 1, n),
                         'evt_event': np.arange(first_event, first_event + n, dtype=np.uint64)})


def test_kfold_out_of_fold_scores_match_predict(tmp_path):
    sig, bkg = toy(2000, 1.5, 1, 0), toy(2000, -1.5, 2, 10**6)
    models, info = mlp.train_kfold(sig, bkg, ['x', 'y'], k=4, params=SMALL, early_stopping=0)
    oof = info['oof']
    assert len(models) == 4 and oof['score'].notna().all()
    # the separation is learnt
    from sklearn.metrics import roc_auc_score
    assert roc_auc_score(oof['label'], oof['score']) > 0.95   # two Gaussians 3 sigma apart: AUC ~ 0.98
    # x separates, y doesn't
    assert info['importance'].index[0] == 'x' and info['importance']['x'] > 0.1
    assert abs(info['importance']['y']) < 0.02
    assert list(info['ks']['n_epochs']) == [15] * 4
    # predict() reproduces the out-of-fold scores (same fold -> same model)
    both = pd.concat([sig, bkg], ignore_index=True)
    assert np.allclose(mlp.predict(models, both, ['x', 'y']), oof['score'].to_numpy(), atol=1e-5)
    # save / load round trip
    mlp.save(models, ['x', 'y'], str(tmp_path))
    m2, feats = mlp.load(str(tmp_path))
    assert feats == ['x', 'y'] and np.allclose(mlp.predict(m2, both, feats), oof['score'].to_numpy(), atol=1e-5)


def test_early_stopping_and_history():
    sig, bkg = toy(2000, 1.5, 1, 0), toy(2000, -1.5, 2, 10**6)
    sig['evt_event'] //= 2
    bkg['evt_event'] //= 2
    models, info = mlp.train_kfold(sig, bkg, ['x', 'y'], k=3, params={**SMALL, 'epochs': 100}, early_stopping=3)
    ks = info['ks']
    assert (ks['n_epochs'] <= ks['epochs_run']).all() and (ks['epochs_run'] < 100).all()
    assert all(len(h['val_loss']) == len(h['loss']) for h in info['history'])


def test_preprocessor():
    df = pd.DataFrame({'a': [1., 10., 100., -999., np.nan], 'b': [0., 1., 2., 3., np.inf]})
    p = mlp.Preprocessor(['a', 'b'], log={'a': 0}, clip=1.0).fit(df)
    # missing marker and non-finite values are replaced by the median of the valid (log-)values
    assert np.isclose(p.median[0], np.log(10.)) and np.isclose(p.median[1], 1.5)
    x = p.transform(df)
    assert x.dtype == np.float32 and np.isfinite(x).all() and np.abs(x).max() <= 1.0
    assert x[3, 0] == x[4, 0] == x[1, 0]          # filled with the median
    p2 = mlp.Preprocessor.from_dict(p.to_dict())
    assert np.array_equal(p2.transform(df), x)
    with pytest.raises(ValueError):
        mlp.Preprocessor(['a'], log={'c': 1})
