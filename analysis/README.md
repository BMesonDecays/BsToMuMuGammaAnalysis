# Analysis of the bmmg ntuples

Python analysis of the CRAB output of `RadiativeAnalysis` (see `../NTUPLE.md`). It runs on a local PC and needs neither CMSSW nor ROOT.

## Why two stages
A CRAB production is thousands of small ROOT files. Each has a few hundred to a few thousand events, with 287 branches stored in one basket each.

Reading them costs about 85 ms per file from a local SSD even for a handful of branches, and about 0.6 s for all branches. That cost is per-file overhead, not data volume. Reading the raw files again in every analysis iteration would spend minutes on overhead alone.

Instead:
1. **`convert.py`, once per production.** It turns the event-level ntuples (vectors of candidates and objects) into flat **candidate-level Parquet tables**: one row per B candidate, with the muons, photon(s), tracks and dimuon of the candidate copied into the row. It runs in parallel over the files and is incremental.
2. **`bmmg_ana` + `scripts/`.** Plots, cuts, BDT training and the working-point scan run on those tables in memory (pandas, numpy, xgboost). Loading a few columns of every candidate takes seconds.

Measured on the first production (2022C ParkingDoubleMuonLowMass0/1 and the B_s → μμγ MC: 1051 files, 3.6 GB, read from EOS on lxplus with 16 processes):

| step | time |
|---|---|
| conversion | 40 s → 1.5 GB of Parquet (lz4; zstd is smaller) |
| `plot_basic.py` | 6 s |
| `train_bdt.py` (124k candidates, 5 folds) | 4 s |
| `scan_wp.py` | 2 s |

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
python3 scripts/scan_wp.py    config/analysis.yaml        # S, B and figures of merit vs BDT threshold

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
| `significance` | `scan`: S, B, S/√B, Asimov Z and Punzi for all thresholds at once; `sideband_factor` (flat or exponential fit to the sidebands) |
| `plotting` | shape comparisons, data vs MC with ratio and blinding, ROC, overtraining, importance, scan (mplhep CMS style) |
| `config` | reads `config/analysis.yaml` and builds the samples (signal MC, data, data sidebands) |

## Analysis choices in `config/analysis.yaml` (starting values, to be decided)
- **Blinding:** data with `mass` in `blind` is never shown.
- **Samples:** background = data in the `sidebands`; signal = signal MC with `genMatched == 1`.
- **Background in the signal window:** the sideband yield above the BDT threshold times a flat or exponential factor. The exponential is fitted to the sideband mass after the preselection, before the BDT cut.
- **Normalisation:** set `lumi_pb` (brilcalc) to normalise the signal MC. Without it, S is the number of MC candidates: S/√B and Z are then not meaningful, but Punzi is, since it needs only efficiencies.
- **BDT inputs:** don't use the mass or variables strongly correlated with it, since the background is taken from the sidebands.
- **Best candidate:** `best_candidate` (e.g. `vtxProb`) keeps one candidate per event. By default all candidates are kept.
