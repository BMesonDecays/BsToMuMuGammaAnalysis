"""k-fold BDT (XGBoost) for signal / background separation.

The fold of a candidate is evt_event % k. Model i is trained on the other folds and applied to
fold i, so every candidate of the training samples gets an unbiased (out-of-fold) score, and any
other sample is scored consistently with predict().
"""
import json
import os

import numpy as np
import pandas as pd
import xgboost as xgb
from scipy.stats import ks_2samp

DEFAULT_PARAMS = dict(n_estimators=400, max_depth=4, learning_rate=0.05, subsample=0.8,
                      colsample_bytree=0.8, min_child_weight=5, tree_method='hist')


def folds(df, k):
    return (df['evt_event'].to_numpy().astype(np.uint64) % np.uint64(k)).astype(int)


def train_kfold(sig, bkg, features, k=5, params=None, n_jobs=-1, early_stopping=20):
    """Train k models. sig, bkg: DataFrames with the features and evt_event.

    The two classes get equal total weight. With early_stopping > 0, the events with
    (evt_event // k) % 10 == 0 of the training folds (10%) are the validation set that stops adding trees.
    Returns (models, info): info has 'oof' (DataFrame of the sig+bkg rows with label, fold, score),
    'ks' (per fold, train vs test score KS p-values) and 'importance' (mean gain per feature).
    """
    params = {**DEFAULT_PARAMS, **(params or {})}
    data = pd.concat([sig[features].assign(label=1, evt_event=sig['evt_event'].to_numpy()),
                      bkg[features].assign(label=0, evt_event=bkg['evt_event'].to_numpy())], ignore_index=True)
    data['fold'] = folds(data, k)
    w = np.where(data['label'] == 1, 1.0 / max(1, len(sig)), 1.0 / max(1, len(bkg))) * len(data) / 2
    data['score'] = np.nan
    models, ks, gains = [], [], []
    for i in range(k):
        tr = (data['fold'] != i).to_numpy()
        te = ~tr
        idx_tr = np.where(tr)[0]
        fit_kw = {}
        p = dict(params)
        if early_stopping:
            # 10% of the events (not candidates) of the training folds: candidates of one event stay together
            ev = data['evt_event'].to_numpy()[idx_tr].astype(np.uint64)
            val = (ev // np.uint64(k)) % np.uint64(10) == np.uint64(0)
            idx_val, idx_tr = idx_tr[val], idx_tr[~val]
            p['early_stopping_rounds'] = early_stopping
            fit_kw['eval_set'] = [(data.loc[idx_val, features], data.loc[idx_val, 'label'])]
            fit_kw['sample_weight_eval_set'] = [w[idx_val]]
            fit_kw['verbose'] = False
        model = xgb.XGBClassifier(**p, n_jobs=n_jobs, eval_metric='logloss')
        model.fit(data.loc[idx_tr, features], data.loc[idx_tr, 'label'], sample_weight=w[idx_tr], **fit_kw)
        models.append(model)
        data.loc[te, 'score'] = model.predict_proba(data.loc[te, features])[:, 1]
        train_score = model.predict_proba(data.loc[idx_tr, features])[:, 1]
        lab_tr = data.loc[idx_tr, 'label'].to_numpy()
        lab_te = data.loc[te, 'label'].to_numpy()
        test_score = data.loc[te, 'score'].to_numpy()
        ks.append({'fold': i,
                   'ks_p_sig': ks_2samp(train_score[lab_tr == 1], test_score[lab_te == 1]).pvalue,
                   'ks_p_bkg': ks_2samp(train_score[lab_tr == 0], test_score[lab_te == 0]).pvalue,
                   'n_trees': model.best_iteration + 1 if early_stopping else p['n_estimators']})
        g = model.get_booster().get_score(importance_type='gain')
        gains.append([g.get(f, 0.) for f in features])
    importance = pd.Series(np.mean(gains, axis=0), index=features).sort_values(ascending=False)
    return models, {'oof': data[['label', 'fold', 'score', 'evt_event']], 'ks': pd.DataFrame(ks),
                    'importance': importance}


def predict(models, df, features):
    """Score of every row with the model of its fold (consistent with the out-of-fold training scores)."""
    k = len(models)
    f = folds(df, k)
    out = np.full(len(df), np.nan)
    for i, m in enumerate(models):
        sel = f == i
        if sel.any():
            out[sel] = m.predict_proba(df.loc[sel, features])[:, 1]
    return out


def save(models, features, outdir, extra=None):
    os.makedirs(outdir, exist_ok=True)
    for i, m in enumerate(models):
        m.save_model(os.path.join(outdir, 'model_fold%d.json' % i))
    with open(os.path.join(outdir, 'bdt.json'), 'w') as f:
        json.dump({'features': list(features), 'k': len(models), **(extra or {})}, f, indent=1)


def load(outdir):
    """(models, features) saved by save()."""
    with open(os.path.join(outdir, 'bdt.json')) as f:
        info = json.load(f)
    models = []
    for i in range(info['k']):
        m = xgb.XGBClassifier()
        m.load_model(os.path.join(outdir, 'model_fold%d.json' % i))
        models.append(m)
    return models, info['features']
