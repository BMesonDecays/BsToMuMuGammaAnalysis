"""k-fold multilayer perceptron (TensorFlow / Keras) for signal / background separation.

The analogue of bdt.py, with the same folds (evt_event % k), class weights, early-stopping split,
out-of-fold scores and the same predict / save / load interface, so scan_wp.py can use either.

Unlike XGBoost, a neural network needs its inputs prepared. Per fold, from the training folds only:
  1. x -> log(x + offset) for the features in params['log'] ({feature: offset}), for long tails;
  2. non-finite values and the missing-value marker (-999) -> the training median;
  3. standardisation to mean 0 and standard deviation 1, then clipping to +-params['clip'].
The preprocessing constants are saved with the models (mlp.json), so the saved Keras models are plain
Sequential models with no custom layers.

TensorFlow uses the first GPU if there is one; set CUDA_VISIBLE_DEVICES='' before importing this
module to run on the CPU.
"""
import json
import os

os.environ.setdefault('TF_CPP_MIN_LOG_LEVEL', '2')

import numpy as np  # noqa: E402
import pandas as pd  # noqa: E402
import tensorflow as tf  # noqa: E402
from scipy.stats import ks_2samp  # noqa: E402
from sklearn.metrics import roc_auc_score  # noqa: E402

from .bdt import folds  # noqa: E402

keras = tf.keras

MISSING = -999.
DEFAULT_PARAMS = dict(hidden=[64, 64, 64], activation='swish', dropout=0.1, l2=1e-5, batch_norm=False,
                      learning_rate=1e-3, batch_size=1024, epochs=200, lr_patience=5, lr_factor=0.5,
                      log={}, clip=5.0, seed=1)

for _gpu in tf.config.list_physical_devices('GPU'):
    try:   # take GPU memory as needed instead of all of it at the start
        tf.config.experimental.set_memory_growth(_gpu, True)
    except RuntimeError:   # already initialised
        pass


def devices():
    """Names of the GPUs TensorFlow sees (empty: training runs on the CPU)."""
    return [d.name for d in tf.config.list_physical_devices('GPU')]


class Preprocessor:
    """log transform, fill of missing / non-finite values, standardisation and clipping (see module doc)."""

    def __init__(self, features, log=None, clip=5.0, median=None, mean=None, std=None):
        self.features = list(features)
        self.log = {f: float(o) for f, o in (log or {}).items() if f in self.features}
        unknown = set(log or {}) - set(self.features)
        if unknown:
            raise ValueError('log transform requested for features not in the feature list: %s' % sorted(unknown))
        self.clip = clip
        self.median, self.mean, self.std = (None if v is None else np.asarray(v, dtype=float)
                                            for v in (median, mean, std))

    def _transform_raw(self, df):
        x = df[self.features].to_numpy(dtype=float, copy=True)
        x[x == MISSING] = np.nan
        for j, f in enumerate(self.features):
            if f in self.log:
                with np.errstate(divide='ignore', invalid='ignore'):
                    x[:, j] = np.log(x[:, j] + self.log[f])   # x + offset <= 0 -> nan/-inf -> filled
        x[~np.isfinite(x)] = np.nan
        return x

    def fit(self, df):
        x = self._transform_raw(df)
        self.median = np.nanmedian(x, axis=0)
        self.median = np.where(np.isfinite(self.median), self.median, 0.)   # all missing
        x = np.where(np.isnan(x), self.median, x)
        self.mean = x.mean(axis=0)
        std = x.std(axis=0)
        self.std = np.where(std > 0, std, 1.)
        return self

    def transform(self, df):
        x = self._transform_raw(df)
        x = np.where(np.isnan(x), self.median, x)
        x = (x - self.mean) / self.std
        if self.clip:
            x = np.clip(x, -self.clip, self.clip)
        return x.astype(np.float32)

    def to_dict(self):
        return {'features': self.features, 'log': self.log, 'clip': self.clip,
                'median': self.median.tolist(), 'mean': self.mean.tolist(), 'std': self.std.tolist()}

    @classmethod
    def from_dict(cls, d):
        return cls(d['features'], d['log'], d['clip'], d['median'], d['mean'], d['std'])


class FoldModel:
    """The Keras model of one fold with its preprocessing. predict_proba(df) -> signal probability."""

    def __init__(self, model, prep):
        self.model, self.prep = model, prep

    def predict_proba(self, df, batch_size=8192):
        if len(df) == 0:
            return np.empty(0)
        x = self.prep.transform(df)
        return self.model.predict(x, batch_size=batch_size, verbose=0)[:, 0].astype(float)


def build_model(n_inputs, params):
    """Sequential MLP: hidden Dense layers (optionally batch-normalised, with dropout), sigmoid output."""
    reg = keras.regularizers.l2(params['l2']) if params.get('l2') else None
    layers = [keras.Input(shape=(n_inputs,))]
    for n in params['hidden']:
        layers.append(keras.layers.Dense(n, kernel_regularizer=reg))
        if params.get('batch_norm'):
            layers.append(keras.layers.BatchNormalization())
        layers.append(keras.layers.Activation(params['activation']))
        if params.get('dropout'):
            layers.append(keras.layers.Dropout(params['dropout']))
    layers.append(keras.layers.Dense(1, activation='sigmoid'))
    model = keras.Sequential(layers)
    model.compile(optimizer=keras.optimizers.Adam(learning_rate=params['learning_rate']),
                  loss='binary_crossentropy', weighted_metrics=[keras.metrics.AUC(name='auc')])
    return model


def _dataset(x, y, w, batch_size, shuffle, seed):
    ds = tf.data.Dataset.from_tensor_slices((x, y.astype(np.float32), w.astype(np.float32)))
    if shuffle:
        ds = ds.shuffle(len(x), seed=seed, reshuffle_each_iteration=True)
    return ds.batch(batch_size).prefetch(tf.data.AUTOTUNE)


def permutation_importance(fold_model, df, labels, features, seed=0):
    """Decrease of the AUC when one feature is shuffled (on df, normally the test fold)."""
    rng = np.random.default_rng(seed)
    base = roc_auc_score(labels, fold_model.predict_proba(df))
    out = []
    for f in features:
        shuffled = df.copy()
        shuffled[f] = rng.permutation(shuffled[f].to_numpy())
        out.append(base - roc_auc_score(labels, fold_model.predict_proba(shuffled)))
    return out


def train_kfold(sig, bkg, features, k=5, params=None, early_stopping=10, verbose=0):
    """Train k MLPs. sig, bkg: DataFrames with the features and evt_event.

    As in bdt.train_kfold: the two classes get equal total weight; with early_stopping > 0 (patience
    in epochs), the events with (evt_event // k) % 10 == 0 of the training folds (10%) are the
    validation set that stops the training, and the weights of the best epoch are kept.
    Returns (models, info): models are FoldModel; info has 'oof' (DataFrame of the sig+bkg rows with
    label, fold, score), 'ks' (per fold, train vs test score KS p-values and the epochs used),
    'importance' (mean permutation importance per feature: AUC decrease on the test fold) and
    'history' (per fold, the loss and validation loss per epoch).
    """
    params = {**DEFAULT_PARAMS, **(params or {})}
    data = pd.concat([sig[features].assign(label=1, evt_event=sig['evt_event'].to_numpy()),
                      bkg[features].assign(label=0, evt_event=bkg['evt_event'].to_numpy())], ignore_index=True)
    data['fold'] = folds(data, k)
    w = np.where(data['label'] == 1, 1.0 / max(1, len(sig)), 1.0 / max(1, len(bkg))) * len(data) / 2
    data['score'] = np.nan
    labels = data['label'].to_numpy()
    models, ks, imps, history = [], [], [], []
    for i in range(k):
        keras.utils.set_random_seed(params['seed'] + i)
        tr = (data['fold'] != i).to_numpy()
        te = ~tr
        idx_tr = np.where(tr)[0]
        idx_val = np.empty(0, dtype=int)
        if early_stopping:
            # 10% of the events (not candidates) of the training folds: candidates of one event stay together
            ev = data['evt_event'].to_numpy()[idx_tr].astype(np.uint64)
            val = (ev // np.uint64(k)) % np.uint64(10) == np.uint64(0)
            idx_val, idx_tr = idx_tr[val], idx_tr[~val]
        prep = Preprocessor(features, params['log'], params['clip']).fit(data.iloc[idx_tr])
        x_tr = prep.transform(data.iloc[idx_tr])
        model = build_model(len(features), params)
        callbacks, val_ds = [], None
        if early_stopping:
            val_ds = _dataset(prep.transform(data.iloc[idx_val]), labels[idx_val], w[idx_val],
                              params['batch_size'], False, None)
            callbacks.append(keras.callbacks.EarlyStopping(monitor='val_loss', patience=early_stopping,
                                                           restore_best_weights=True))
            if params.get('lr_patience'):
                callbacks.append(keras.callbacks.ReduceLROnPlateau(monitor='val_loss', factor=params['lr_factor'],
                                                                   patience=params['lr_patience']))
        hist = model.fit(_dataset(x_tr, labels[idx_tr], w[idx_tr], params['batch_size'], True, params['seed'] + i),
                         validation_data=val_ds, epochs=params['epochs'], callbacks=callbacks, verbose=verbose,
                         shuffle=False)   # the dataset is shuffled by _dataset
        fm = FoldModel(model, prep)
        models.append(fm)
        data.loc[te, 'score'] = fm.predict_proba(data.loc[te, features])
        train_score = fm.predict_proba(data.iloc[idx_tr][features])
        lab_tr = labels[idx_tr]
        lab_te = labels[te]
        test_score = data.loc[te, 'score'].to_numpy()
        h = hist.history
        best_epoch = int(np.argmin(h['val_loss'])) + 1 if early_stopping else len(h['loss'])
        ks.append({'fold': i,
                   'ks_p_sig': ks_2samp(train_score[lab_tr == 1], test_score[lab_te == 1]).pvalue,
                   'ks_p_bkg': ks_2samp(train_score[lab_tr == 0], test_score[lab_te == 0]).pvalue,
                   'n_epochs': best_epoch, 'epochs_run': len(h['loss'])})
        history.append({'loss': h['loss'], 'val_loss': h.get('val_loss', [])})
        imps.append(permutation_importance(fm, data.loc[te, features], lab_te, features, seed=params['seed'] + i))
    importance = pd.Series(np.mean(imps, axis=0), index=features).sort_values(ascending=False)
    return models, {'oof': data[['label', 'fold', 'score', 'evt_event']], 'ks': pd.DataFrame(ks),
                    'importance': importance, 'history': history}


def predict(models, df, features):
    """Score of every row with the model of its fold (consistent with the out-of-fold training scores)."""
    k = len(models)
    f = folds(df, k)
    out = np.full(len(df), np.nan)
    for i, m in enumerate(models):
        sel = f == i
        if sel.any():
            out[sel] = m.predict_proba(df.loc[sel, features])
    return out


def save(models, features, outdir, extra=None):
    os.makedirs(outdir, exist_ok=True)
    for i, m in enumerate(models):
        m.model.save(os.path.join(outdir, 'model_fold%d.keras' % i))
    with open(os.path.join(outdir, 'mlp.json'), 'w') as f:
        json.dump({'features': list(features), 'k': len(models),
                   'preprocessing': [m.prep.to_dict() for m in models], **(extra or {})}, f, indent=1)


def load(outdir):
    """(models, features) saved by save()."""
    with open(os.path.join(outdir, 'mlp.json')) as f:
        info = json.load(f)
    models = [FoldModel(keras.models.load_model(os.path.join(outdir, 'model_fold%d.keras' % i)),
                        Preprocessor.from_dict(info['preprocessing'][i])) for i in range(info['k'])]
    return models, info['features']
