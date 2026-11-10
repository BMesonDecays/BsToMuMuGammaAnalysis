import numpy as np
import pandas as pd

from bmmg_ana import bdt, selection


def toy(n, shift, seed, first_event):
    rng = np.random.default_rng(seed)
    return pd.DataFrame({'x': rng.normal(shift, 1, n), 'y': rng.normal(0, 1, n),
                         'evt_event': np.arange(first_event, first_event + n, dtype=np.uint64)})


def test_kfold_out_of_fold_scores_match_predict(tmp_path):
    sig, bkg = toy(2000, 1.5, 1, 0), toy(2000, -1.5, 2, 10**6)
    models, info = bdt.train_kfold(sig, bkg, ['x', 'y'], k=4, params={'n_estimators': 30}, n_jobs=2, early_stopping=0)
    oof = info['oof']
    assert len(models) == 4 and oof['score'].notna().all()
    # the separation is learnt
    from sklearn.metrics import roc_auc_score
    assert roc_auc_score(oof['label'], oof['score']) > 0.95   # two Gaussians 3 sigma apart: AUC ~ 0.98
    # predict() reproduces the out-of-fold scores (same fold -> same model)
    both = pd.concat([sig, bkg], ignore_index=True)
    assert np.allclose(bdt.predict(models, both, ['x', 'y']), oof['score'].to_numpy(), atol=1e-6)
    # save / load round trip
    bdt.save(models, ['x', 'y'], str(tmp_path))
    m2, feats = bdt.load(str(tmp_path))
    assert feats == ['x', 'y'] and np.allclose(bdt.predict(m2, both, feats), oof['score'].to_numpy(), atol=1e-6)


def test_best_candidate_and_cutflow():
    df = pd.DataFrame({'evt_run': [1, 1, 1], 'evt_lumi': [1, 1, 1], 'evt_event': [5, 5, 6],
                       'vtxProb': [0.1, 0.7, 0.3], 'pt': [10., 3., 20.]})
    best = selection.best_candidate(df, 'vtxProb')
    assert list(best['vtxProb']) == [0.7, 0.3]
    cf = selection.cutflow(df, {'pt': 'pt > 5', 'prob': 'vtxProb > 0.2'})
    assert list(cf['candidates']) == [3, 2, 1] and list(cf['events']) == [2, 2, 1]
    assert selection.in_windows(np.array([4.6, 5.3, 6.0]), [[4.5, 5.0], [5.8, 6.5]]).tolist() == [True, False, True]
