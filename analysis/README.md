# Analysis of the bmmg ntuples

Python analysis of the CRAB output of `RadiativeAnalysis` (see `../NTUPLE.md`). It runs on a local PC and needs neither CMSSW nor ROOT.

## Why two stages
A CRAB production is thousands of small ROOT files. Each has a few hundred to a few thousand events, with 287 branches stored in one basket each.

Reading them costs about 85 ms per file from a local SSD even for a handful of branches, and about 0.6 s for all branches. That cost is per-file overhead, not data volume. Reading the raw files again in every analysis iteration would spend minutes on overhead alone.

Instead:
1. **`convert.py`, once per production.** It turns the event-level ntuples (vectors of candidates and objects) into flat **candidate-level Parquet tables**: one row per B candidate, with the muons, photon(s), tracks and dimuon of the candidate copied into the row. It runs in parallel over the files and is incremental.
2. **`bmmg_ana` + `scripts/`.** Plots, cuts, BDT and MLP training and the working-point scan run on those tables in memory (pandas, numpy, xgboost, tensorflow). Loading a few columns of every candidate takes seconds.

Measured on the first production (2022C ParkingDoubleMuonLowMass0/1 and the B_s → μμγ MC: 1051 files, 3.6 GB, read from EOS on lxplus with 16 processes):

| step | time |
|---|---|
| conversion | 40 s → 1.5 GB of Parquet (lz4; zstd is smaller) |
| `plot_basic.py` | 6 s |
| `train_bdt.py` (124k candidates, 5 folds) | 4 s |
| `scan_wp.py` | 2 s |

On the 2022 Parking production (local PC, RTX A500 laptop GPU, 61k signal and 373k sideband candidates, 3 folds): `train_bdt.py` 1 min (5000 trees, GPU), `train_mlp.py` 2.7 min (GPU, ~75 epochs per fold), `scan_wp.py` 9 s.
With the tuned BDT (37 features, 5 folds, 61k signal and 318k sideband candidates): `train_bdt.py` 43 s, `scan_wp.py` with the shape fit 7 s, `check_sculpting.py` 40 s, `tune_bdt.py` 0.5–2 min per trial (GPU).

## Setup
```bash
mamba env create -f environment.yml && conda activate bmmg
# or, on lxplus:  source /cvmfs/sft.cern.ch/lcg/views/LCG_107/x86_64-el9-gcc13-opt/setup.sh
```

## Workflow
```bash
# 1. copy the CRAB output to the fast disk (from lxplus / CERNBox)
rsync -a lxplus.cern.ch:/eos/user/s/sslawins/bmmg_ntuples/ ~/bmmg/ntuples/

# 2. convert (re-run after new CRAB outputs: only new files are converted)
python3 convert.py config/convert.yaml                    # all datasets
python3 convert.py config/convert.yaml --datasets Parking2022C_0 --workers 8

# 3. analyse
python3 scripts/plot_basic.py config/analysis.yaml        # cut flows, shapes, blinded mass
python3 scripts/train_bdt.py  config/analysis.yaml        # k-fold BDT, ROC, overtraining, importance
python3 scripts/train_mlp.py  config/analysis.yaml        # k-fold MLP (TensorFlow, GPU): ROC, overtraining, importance, loss curves
python3 scripts/scan_wp.py    config/analysis.yaml        # S, B and figures of merit vs BDT threshold
python3 scripts/scan_wp.py    config/analysis.yaml --classifier mlp   # the same for the MLP
python3 scripts/tune_bdt.py   config/analysis.yaml --trials 40        # random search of the BDT hyperparameters
python3 scripts/check_sculpting.py config/analysis.yaml               # can the BDT find the mass? (run after feature changes)

python3 -m pytest tests                                   # unit tests
```
Interactive use: `notebooks/example.ipynb`.

## Candidate tables (`<output>/<dataset>/<channel>/part-NNNNN.parquet`)
For channel `mmg` (the other channels are analogous):

| columns | content |
|---|---|
| `mass`, `vtxProb`, `lxy`, `ct3D`, `genMatched`, … | the candidate branches `mmg_*`, without the prefix |
| `mu1`, `mu2`, `pho`, `dimu` | the leg indices (as in the ntuple) |
| `mu1_pt`, `mu2_mva`, `pho_mva`, `pho_r9`, `dimu_mass`, … | the objects of the legs: `<leg>_<var>` (`trk1_*`, `trk2_*` for `mmkk`; `pho1_*`, `pho2_*` for `mmgg`; `conv_*` for `mmcg`) |
| `evt_*`, `bs_*`, `pv_*`, `trg_*`, `gen_*` | event-level scalars, repeated for every candidate of the event |
| `cand_idx`, `n_cand` | index of the candidate in its event, and number of candidates of the channel in the event |

- Missing objects are −999, as in the ntuple.
- `config/convert.yaml` applies a loose preselection (`fitOK == 1 and vtxProb > 1e-3`) and can drop columns.
- `<output>/<dataset>/meta.json`: summed RunInfo, i.e. `nEvents` (all processed events, also with the ntuple skim), `nEventsStored`, generator `xsec` and `filterEff`; plus `nFiles` and the rows per channel.
- `manifest.json`: the input files of every part. A file that can't be read is reported and retried at the next run. Delete a dataset's directory to convert it from scratch.

## Library (`bmmg_ana/`)
| module | content |
|---|---|
| `io.load(base, datasets, channel, cols, query, derived)` | DataFrame of one channel. Loads only the requested columns plus those used by `query` / `derived`. Adds a `dataset` column. |
| `selection` | `apply`, `cutflow` (candidates, events, efficiencies), `best_candidate(df, key)`, `in_windows` |
| `weights.mc_scale(meta, lumi_pb, …)` | σ · ε_filter · L / N_processed (with overrides of σ, ε and the BR) |
| `bdt` | `train_kfold` (fold = `evt_event % k`; out-of-fold scores; train/test KS; importance), `predict`, `save`, `load` |
| `mlp` | the same interface for a Keras multilayer perceptron: same folds, class weights and early-stopping split; per-fold input preprocessing (`Preprocessor`); permutation importance; loss history. Models in `<output>/mlp/` (`model_fold<i>.keras` + `mlp.json` with the preprocessing) |
| `significance` | `scan`: S, B, S/√B, Asimov Z and Punzi for all thresholds at once; `sideband_factor` (flat or exponential fit to the sidebands); `binned_asimov`: expected Z of a binned mass fit in score categories; `optimise_categories`: category boundaries, with an unbiased half-sample estimate |
| `plotting` | shape comparisons, data vs MC with ratio and blinding, ROC, overtraining, importance, scan (mplhep CMS style) |
| `config` | reads `config/analysis.yaml` and builds the samples (signal MC, data, data sidebands) |
| `convscale` | converted-photon energy scale: χ_c fit model and `fit` (iminuit), `pt_est`, `bin_index`, `ScaleMap` (JSON map, lookup), `corrected_mass`; MC truth: `y_from_vectors`, `fit_peak` |

## Analysis choices in `config/analysis.yaml` (starting values, to be decided)
- **Blinding:** data with `mass` in `blind` is never shown.
- **Samples:** background = data in the `sidebands`; signal = signal MC with `genMatched == 1`.
- **Background in the signal window:** the sideband yield above the BDT threshold times a flat or exponential factor. The exponential is fitted to the sideband mass after the preselection, before the BDT cut.
- **Normalisation:** set `lumi_pb` (brilcalc) to normalise the signal MC. Without it, S is the number of MC candidates: S/√B and Z are then not meaningful, but Punzi is, since it needs only efficiencies.
- **BDT inputs:** don't use the mass or variables strongly correlated with it, since the background is taken from the sidebands.
  - `scripts/check_sculpting.py` trains without the inner 0.2 GeV of the sidebands and compares the BDT efficiency there with the adjacent strips. A ratio above 1 means the BDT has learnt the mass.
  - `dR_mumu_pho` and `dR_mu_pho_min` are correlated with the mass (ρ ≈ 0.3 and 0.2 in the sidebands). With the standard sidebands the test passes. Trained on wider sidebands (4.0–5.0, 5.75–7.0 GeV), the BDT learnt the mass from them (ratio 1.2–1.4).
- **Projection and significance:** `significance.projection_lumi_pb` scales S and B to another luminosity (320 fb⁻¹, Run 3). `significance.shape` adds the expected significance of a binned fit of the mass in the blind window, in up to 3 BDT-score categories:
  - The background in each category is its sideband count times an exponential, whose slope is fitted to the top 1% of sideband scores.
  - The category boundaries are chosen on a grid of sideband efficiencies. The 'unbiased' Z chooses them on one half of the sideband events and evaluates on the other.
  - This is an Asimov estimate with a known background shape: no systematic uncertainties, and no uncertainty on the background shape or normalisation.
- **MLP:** the `mlp` block uses the BDT features (YAML anchor `*features`). A neural network needs prepared inputs; per fold, from the training folds only:
  1. `log` features: x → log(x + offset), for the long tails of the flight significances, the pts and `dimu_dca`;
  2. −999 and non-finite values (e.g. `lxySig` = 0/0) → the training median;
  3. standardisation, then clipping to ±`clip`.

  Hidden layers, activation, dropout, L2, batch normalisation, learning rate, batch size and the early-stopping patience (epochs) are in `params`. TensorFlow uses the GPU if it sees one (`--cpu` hides it). The importance is the AUC decrease on the test fold when a feature is shuffled, not the XGBoost gain, so the two importance plots are not directly comparable.
- **Best candidate:** `best_candidate` (e.g. `vtxProb`) keeps one candidate per event. By default all candidates are kept.

## Converted-photon energy scale (χ_c → J/ψ γ)
The energy of the converted photons (`conv_*`, the e⁺e⁻ pair refitted with the colinearity constraint, used by `mmcg`) is low by 1–5% at the peak. The bremsstrahlung tail is not part of this correction. The scale is measured in data with χ_c1,2 → J/ψ γ (the ntuple block `chic_`).
```bash
python3 convert.py config/convert_chic.yaml                  # chic tables -> ~/bmmg/parquet_chic
python3 scripts/conv_scale.py config/conv_scale.yaml         # data map -> ~/bmmg/results/conv_scale/conv_scale.json
python3 convert.py config/convert_convmc.yaml                # MC truth check: gen-matched mmcg of B_s -> J/psi gamma
python3 scripts/conv_scale_mc.py config/conv_scale_mc.yaml   # MC map from E_reco/E_true, y-method check, data/MC
```
- **Fit variable:** y = m(μμγ)² − m(μμ)² = 2 p_μμ·p_γ. It is linear in the photon energy, so the χ_c1/χ_c2 peaks sit at s·(m_χc² − m_J/ψ²).
- **Model:** Crystal Ball peaks (tail on the low side; n fixed to 2) plus a Chebyshev background, in a binned likelihood fit. The scale s is the CB mean, i.e. the peak. The correction is p_γ → p_γ / s.
- **Binning:**
  - The map is binned in `pt_est` = conv_pt · Y₁ / y and |η|. For a χ_c1 candidate `pt_est` is the true photon pt, so the binning does not select on the response.
  - Binning in the reconstructed pt biases s upwards at low pt, through migration and the 0.5 GeV conversion threshold.
  - The map starts at `pt_est` = 1 GeV: below that, the threshold cuts into the fit range.
- **Shape parameters:**
  - The tail α comes from one reference fit per |η| slice (`shape_from`); it is 0.9–1.0 in the barrel and smaller forward.
  - N(χ_c2)/N(χ_c1) comes from the barrel (`r21_from`) and is fixed everywhere. In the endcaps the two peaks merge and a free ratio is degenerate with s.
- **Systematics:** the model variations in the config (background order, χ_c0, fit range, n, ±20% on the ratio). Per bin, the largest deviation is the systematic error.
- **Output:**
  - `conv_scale.json`: the `ScaleMap`, with variables, edges, `scale`, `error` (stat), `syst` and `status` (0 ok, 1 fit not valid, 2 too few candidates; bins with status ≠ 0 hold the scale of their |η| slice).
  - Fit plots per bin, the summary plot, a CSV of all fits.
  - The closure: the map applied to every candidate, then refits in the finer cross-check bins (pt, reconstructed pt, |η|, conversion radius).
- **Applying the map:** the correction uses the value of the candidate's bin (`interpolate: false`). For `mmcg`, `corrected_mass(m, m_μμ, s)` gives √(m_μμ² + (m² − m_μμ²)/s). It is exact for the unfitted mass and approximate for the fitted one; the ntuplizer will apply 1/s to the photon before the fit.
- **Range:** the χ_c photons cover about 1–5 GeV, the signal photons 3–40 GeV. Above the last edge the lookup returns the last bin. The MC (`conv_scale_mc.py`) gives the pt dependence beyond the data range.
