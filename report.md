# Progress report

Entries are newest first.

## Status overview (2026-11-06), for review
- **Done:** Phases 0–8 of `plan.md` and most of Phase 9. The code builds without warnings and the unit tests pass. All steps are committed on `sslawins_dev1` (`008bbc6` … `47cc319`) and **nothing is pushed**.
- **Changes from the original code:**
  - The ntuplizer is a new modular `RadiativeAnalysis` module: a `bmmg` library in `src/` plus a thin EDAnalyzer.
  - **Channels:** μμγ (ECAL photon, MMGVertexFit), μμγ (conversion), μμγγ, μμKK.
  - **Ntuple:** all candidates as vectors; documented in `NTUPLE.md`.
  - **Formats:** AOD and MiniAOD.
  - **Configs:** one driver config (`test/runNtuplizer_cfg.py`) and one CRAB config.
  - The legacy module was removed in P7.4 (still in the git history at `bde3123`).
- **Points to look at first:**
  1. **Physics decisions** (each in the entry where it was made):
     - no MVA cuts at ntuple level;
     - pre-fit window 4–7 GeV;
     - EE photons included;
     - FSR handling in the gen classification;
     - trigger matching to HLT muon objects only;
     - μμKK uses J/ψ dimuons only;
     - the ct definition (signed projection, full-covariance error);
     - fits with mass > 100 GeV count as failed.
  2. **Known limitation:** run-to-run differences in fits that don't converge (χ² ≫ ndf). valgrind finds no memory error; cut on `vtxProb`.
  3. **Not validated:**
     - μμKK on B_s→J/ψφ MC (no sample available);
     - μμγγ: few gen-matched candidates on the J/ψπ⁰/η/η′ MC (2 / 47 / 8, 2026-11-09 entry): the soft photons are rarely reconstructed;
     - the CRAB config was not submitted.
  4. **MiniAOD:** no conversion channel, and few low-pt photons (`slimmedPhotons` threshold).
  5. **Open:** low-energy photon cores for `BMMGCustomPhotonProducer` (Phase 9 research).
  6. **Not committed:** the reference ntuples in `RadiativeAnalysis/test/reference/` (git-ignored, local only).
 Each entry gives: the plan item(s) (see `plan.md`), what was done, the build result, the validation result, and open issues.

---

## 2026-11-10: Fixes from the external review (`review.md`)
- **Request:** read `review.md` (review of the code at `184dc8b`), apply the fixes where there is a possible improvement, one commit per fix.
- **Triage:**
  - Applied: the source-level defects listed below.
  - Not applied: findings that are physics choices or need studies on data and MC (strict truth label, exclusive gen classification, frame of the angles, trigger-leg matching, sideband closure in score slices, normalisation and luminosity, fiducial region and q² binning). These are listed under "Open" at the end of this entry.
- **Fix 1, conversion bookkeeping** (`analysis/convert.py`, `bmmg_ana/io.py`):
  - **Problem:** a file whose second channel failed had already contributed its RunInfo and its first channel's rows, but it was listed as bad and retried at the next run, so both were counted twice. The reader also used every `part-*.parquet` on disk, including parts of a chunk whose manifest update never happened.
  - **Fix:**
    - A file contributes all or nothing (`convert_file`).
    - `io.parts` uses only the parts listed in the manifest (with rows for the channel).
    - The manifest stores the conversion settings (channels, preselection, `drop_columns`) and a fingerprint. A re-run with different settings is refused. Manifests from before this change are accepted with a warning.
    - A stale part file with the number of a new empty part is removed.
  - **Tests:** a new test covers a file failing in the second channel (no rows, no nEvents), an orphan part, and changed settings. It fails on the old code. All 11 tests pass.
  - **Real production:** an incremental re-run on the scratch Parquet converted 3 new CRAB files. The rows loaded through `io.load` equal the `meta.json` rows for all 3 datasets.
- **Fix 2, blinding at the analysis boundary** (`bmmg_ana/config.py`, `scripts/plot_basic.py`):
  - **Problem:** the first row of the data cut flow counted all candidates, including the blind window. `plot_basic.py` also printed the total data count next to the sideband count, so their difference could give the blinded yield.
  - **Fix:**
    - `load_samples` returns data only outside the blind window. The best-candidate choice is made before the blinding, as it will be after unblinding, so the ranking variable must not be the mass.
    - The data cut flow loads with the blind query, so no row includes the window.
    - `load_config` checks that the sidebands don't overlap the blind window and that the signal window lies inside it.
  - **Tests:** a new `tests/test_config.py`; 13 tests pass. On the production the data cut flow now starts at 90531 candidates outside the window.
- **Fix 3, input validation of the scan** (`bmmg_ana/significance.py`):
  - **Problem:** NaN scores sorted above every threshold. The review's probe, scores `[0.2, NaN]` with threshold 0.8, counted one passing candidate.
  - **Fix:**
    - `cumulative_above` and `scan` raise on non-finite scores or weights, on a length mismatch, and on empty samples.
    - The window widths require hi > lo.
  - **Tests:** a new test; 14 pass.
- **Fix 4, leg gather in `schema.build_table`:**
  - **Problem:** a missing leg (index −1) was replaced by index 0, which raises IndexError when the object list of the event is empty (e.g. no photon). An index beyond the object list failed without saying where.
  - **Fix:**
    - Missing legs are masked (`ak.mask`) and filled with −999.
    - An index beyond the list raises a ValueError naming the branch and the event number.
    - Leg and candidate vectors of different lengths raise.
  - **Also fixed:** `convert.py` crashed writing the manifest when the first finished chunk of a dataset had no rows, because the dataset directory did not exist yet. The new test exposed this depending on the completion order.
  - **Validation:** on a data and an MC file the new tables equal the old ones in all channels, with the same dtypes. 15 tests pass, 3 repeated runs.
- **Fix 5, same candidate policy for signal and data** (`bmmg_ana/config.py`):
  - **Problem:**
    - The signal truth query was applied before the best-candidate choice. A true candidate could therefore be kept even where a combinatorial candidate would win the ranking, as in data, which overestimates the signal efficiency.
    - A `best_candidate` column that was neither a feature nor in `columns` was not loaded.
  - **Fix:** signal MC is preselected and ranked first, then the truth query is applied. The ranking column is always loaded.
  - **Tests:** a new test, which fails on the old code; 16 pass. No effect with the current config, where `best_candidate: null`.
- **Fix 6, early-stopping split grouped by event** (`bmmg_ana/bdt.py`):
  - **Problem:** the 10% validation split for early stopping was random per candidate, so candidates of one event could be in both the training and the validation set.
  - **Fix:** the validation set is the events with `(evt_event // k) % 10 == 0`. It is deterministic and independent of the fold, which uses `evt_event % k`.
  - **Validation:** 17 tests pass. On the production the out-of-fold AUC is 0.957, unchanged, and the Punzi optimum is at 0.956 (before: 0.949).
  - **Note:** with all candidates kept, the row-based KS p-values still overstate the independent information. Use them as a qualitative check.
- **Fix 7, PV tracks in MiniAOD** (`src/ObjectSelection.cc`, `NTUPLE.md`, `structure.md`):
  - **Problem:**
    - `PackedCandidate::fromPV()` without an argument tests PV 0 only. The track list therefore kept only the tracks of PV 0, while their stored key was the actual PV.
    - A candidate assigned to another PV had isolation = 1 and `maxTrackCompatibility` = 0, exactly. Both are BDT features.
  - **Fix:**
    - MiniAOD: `pvAssociationQuality() == UsedInFitTight`, i.e. weight > 0.5 in the fit of the associated PV (`PATPackedCandidateProducer`).
    - AOD, for the same definition: only tracks with `trackWeight > 0.5` (before: all tracks of the vertex).
  - **Validation:** 2000 events each of MC AOD (sample 0), MC MiniAOD (1) and data MiniAOD (16), old vs new build. Fitted candidates:

    | sample, channel, PV | n | iso = 1 old → new | maxTrkComp = 0 old → new |
    |---|---|---|---|
    | MC MiniAOD μμγ, PV > 0 | 121 | 1.000 → 0.496 | 1.000 → 0.380 |
    | MC AOD μμγ, PV > 0 (reference) | 155 | 0.510 → 0.535 | 0.355 → 0.387 |
    | data MiniAOD μμKK, PV > 0 | 134 | 1.000 → 0.463 | 1.000 → 0.157 |
    | MC MiniAOD μμγ, PV = 0 | 195 | unchanged | unchanged |

    MiniAOD now agrees with AOD. In AOD the weight cut changes the values slightly. The μμKK AOD count changes from 67 to 65, consistent with the known fit non-determinism; the isolation does not enter the fit.
  - **Consequence:** in the existing CRAB production (MiniAOD), isolation and track compatibility are wrong for 38% of the MC μμγ candidates and 37% of the data μμKK candidates (those with PV ≠ 0). **The production must be redone** before these features are used in the BDT.

---

## 2026-11-10: Analysis framework for the ntuples (`analysis/`)
- **Request:** plan and set up the analysis of the CRAB ntuples on a local PC (SSD, 32 GB, 16 cores): histograms, cuts, a BDT for the signal selection, and the BDT working point from the signal significance. The production is thousands of small files.
- **Measured on the first production** (`/eos/user/s/sslawins/bmmg_ntuples`: 2 × ~515 2022C Parking files + 22 MC files):
  - A file has about 600–1300 events in 287 branches, each one ZLIB-1 basket.
  - Reading costs 85 ms per file from local disk for 11 branches (195–270 ms from EOS) and 570 ms for all branches. That is per-file overhead, not data volume.
  - Size by block: `mmkk_` 42%, `mu_` 13%, `dimu_` 10%, `mmg_` 10%.
- **Decision:** two stages in the scientific-Python stack (uproot, awkward, pyarrow, pandas, xgboost, hist, mplhep). Neither CMSSW nor ROOT is needed.
  1. **`analysis/convert.py`:** converts once into candidate-level Parquet tables.
     - One row per candidate. The leg objects are gathered through the index branches (`mu1_*`, `pho_*`, `dimu_*`, `trk1_*`, …); the event scalars are broadcast.
     - Loose preselection, parallel over files, incremental (manifest), unreadable files retried; RunInfo sums in `meta.json`.
  2. **`analysis/bmmg_ana`:** `io`, `selection`, `weights`, `bdt`, `significance`, `plotting`, `config`. Scripts `plot_basic.py`, `train_bdt.py`, `scan_wp.py`, configured by `config/analysis.yaml`; an example notebook.
     - k-fold XGBoost: fold = `evt_event % k`, out-of-fold scores, train/test KS, importance.
     - Vectorised scan over all thresholds: S/√B, Asimov Z, Punzi; background from the sidebands with a flat or exponential factor.
- **Results on the first production** (lxplus, LCG_107 view, 16 processes, input from EOS):
  - **Conversion:** 1051 files in 40 s → 1.5 GB of Parquet (lz4: the LCG pyarrow has no zstd; conda's has).
  - **Cross-check:** the Parquet row counts equal the preselected candidates counted directly in the raw files, for every dataset and channel (`mmg` 57340 / 57740 / 131610, `mmgg`, `mmkk`). `meta.json` `nEvents` equals the RunInfo sums (49.8M + 49.8M data events, 1.17M MC).
  - **Spot checks:** values gathered through the indices equal the raw values (MC and data, all channels).
  - **Plots** (`plot_basic.py`, 6 s): cut flows for data (blinded) and signal; shapes of signal vs sidebands; blinded data mass.
  - **BDT** (`train_bdt.py`, 4 s; 96k signal, 28k sideband candidates, 14 features): out-of-fold AUC 0.957, train/test KS p-values 0.01–0.93, `pho_mva` and the flight significance most important.
  - **Scan** (`scan_wp.py`, 2 s): Punzi optimum at score 0.949 (signal efficiency 45%, sideband efficiency 1%).
    - `lumi_pb` is not set, so S is a raw MC count, and S/√B and Z are not physical yet.
  - **Tests:** `pytest analysis/tests`, 10 tests: gather/broadcast/missing legs, preselection, the converter end to end with a corrupt file and an incremental re-run, k-fold consistency of `predict` and save/load, the scan against hand values, the exponential sideband fit, cut flow and best candidate.
- **Open (analysis choices, starting values in `config/analysis.yaml`):**
  - mass windows and blinding, the preselection, the BDT features;
  - `lumi_pb` and the signal normalisation (the generator σ in the ntuple is the SoftQCD σ: 5.7e8 pb × filter efficiency 1.7e-3);
  - the background model.
- `scram b` is unaffected: the analysis scripts are not executable, so SCRAM does not install them.

## 2026-11-09: CRAB submission for many datasets from a YAML task file
- **Requests:**
  1. The data type (AOD/MiniAOD) must be a job parameter.
  2. Submit tasks on many datasets with one script, following the CRAB library API (twiki `CRABClientLibraryAPI`).
  3. Set the parameters in a config file, not on the command line.
- **Problem found in the existing tasks** (`crab_projects/crab_Parking2022C`, `crab_BsTo2MuG`, MiniAOD datasets submitted with the old `crabConfig.py`):
  - `dataFormat` was not passed. Without `inputFiles`, `runNtuplizer_cfg.py` takes the format from the default test sample (AOD), so the jobs ran the AOD configuration on MiniAOD.
  - The Parking jobs fail with `ProductNotFound`. The MC task will fail the same way.
- **New `crab/submitCrab.py` + `crab/tasks_example.yaml`** (replaces the environment-variable `crabConfig.py`):
  - **Task file (YAML):** `defaults` plus a list of `tasks`.
    - A task has a `dataset`, a list of `datasets` (one task each, names from the dataset names) or a `fileList` (private LFNs).
    - It can set `isMC`, `dataFormat`, `lumiMask`, `runRange`, splitting, resources, output site/LFN, `tag` and `options` (any `runNtuplizer_cfg.py` option: `skim`, `skimMinVtxProb`, `globalTag`, `channels`, …).
  - **Data type:** `isMC` and `dataFormat` default to the data tier of the dataset (…SIM = MC, AOD/MINIAOD); explicit values must agree with it. They are always passed to cmsRun.
  - **Checks before anything is submitted:** unknown keys (typos), malformed dataset names, options the script sets itself, request-name length and characters, duplicate names; a warning for data without a lumi mask.
  - **Submission:** `crabCommand('submit', config=…)`, each task in its own `multiprocessing.Process`, as in the twiki. CRAB imports the CMSSW configuration once per Python session, so different `pyCfgParams` per task need separate processes.
    - Tasks whose project directory exists are skipped; there is a summary of submitted, skipped and failed tasks.
    - `--dry-run` prints the configurations without contacting CRAB; `--tasks` selects tasks by glob; `--command status|resubmit|kill` runs over the same tasks.
  - **Paths:** the pset path is absolute, so the script runs from any directory. A relative `workArea` is relative to the task file. `crab_projects/` is git-ignored.
- **Tests:**
  - `--dry-run` of the example file: the expected configurations, with `dataFormat=MINIAOD` and the right `isMC`.
  - Validation errors: AOD on a MINIAOD dataset, `isMC` on data, a key typo, a reserved option, duplicate names, a `fileList` without `dataFormat`.
  - The generated `pyCfgParams` were run locally with cmsRun on the MiniAOD test files (300 events each): data with skim 300 → 10 stored; MC without skim 300 → 300.
  - `--command status` reaches CRAB with the right project directories. The live query was not possible in this session: no grid proxy.
  - **Nothing was submitted.**
- **Output:** every task (submit, dry-run, status, resubmit, kill) starts with a banner: `[i/n] COMMAND request name`, plus the dataset.
- **To do (user):** kill the two old tasks and resubmit with `submitCrab.py`. Choose the global tags: the example uses `130X_mcRun3_2022_realistic_v5` for Run3Summer22 MC, while the config default `auto:run3_mc_FULL` is the 2024 MC GT in CMSSW_14_1_1.

## 2026-11-09: Event skim, store only events with a valid B candidate
- **Request:** Run-3 data ntuples must stay small. Add a switch that keeps only the events with valid B candidates.
- **Module parameters:**
  - `onlyEventsWithCandidates` (default `False`).
  - `minCandidateVtxProb` (default −1).
  - An event is written if at least one candidate of an enabled channel has a successful fit and, if the threshold is ≥ 0, a vertex probability above it.
  - With the skim on, events without a dimuon with a valid vertex return before the candidates are built.
- **Config and CRAB:** `runNtuplizer_cfg.py` options `skim=0/1` and `skimMinVtxProb`; CRAB variables `BMMG_SKIM=1` and `BMMG_SKIMMINVTXPROB`.
- **RunInfo:** new `nEventsStored`. `nEvents` still counts all processed events (normalisation).
- **Validation, 2000 events of the 2022C data AOD test file:**

  | | stored events | file size | time |
  |---|---|---|---|
  | no skim | 2000 | 3.25 MB | 43 s |
  | `skim=1` | 725 | 1.50 MB | 46 s |
  | `skim=1 skimMinVtxProb=1e-3` | 110 | 0.33 MB | 43 s |

  - The run time is dominated by input reading and the photon ID, so the skim doesn't save time here.
  - **Selected events vs the unskimmed ntuple:** the selection is reproduced except for events whose only "valid" candidate is a non-converging fit (vtxProb ≈ 0, χ² up to 10⁷; 9 events, and 2 at the 1e-3 threshold, one of them a borderline `mmcg` fit with prob 0.004). These flip because of the known run-to-run non-determinism.
  - **Stored events vs the same events unskimmed:** candidate values differ at the same rate as two identical unskimmed runs (160 of 2000 events differ there, nearly all non-converging `mmcg`/`mmkk` fits; 3 with a candidate of vtxProb > 1e-3, all `mmcg`). Nothing else changes.
  - **MC (1000 events, `skim=1`):** AOD 343 events stored (167 with a gen-matched fitted `mmg`), MiniAOD 170 (146).
- **Docs:** NTUPLE.md, structure.md (§2.2 and new §3.9), README, config and CRAB headers.

## 2026-11-09: Photon ID: MVA working point instead of the cut-based ID (user change)
- **User change:** VID is set up with `mvaPhotonID_Winter22_122X_V1_cff` instead of the cut-based RunIIIWinter22 V1 ID. `pho_tightId` now stores `mvaPhoID-RunIIIWinter22-v1-wp90` (default of `photonTightId` in `fillDescriptions`).
- **Fix to that change:** the edit also forced `DataFormat.AOD` in `switchOnVIDPhotonIdProducer` and removed the `egmPhotonIDs.physicsObjectSrc` setting. MiniAOD jobs then stopped (`ProductNotFound: gedPhotons`). Both lines are restored: format-dependent setup, and both producers read `process.bmmg.photons`.
- **Note:** `photonMVAValueMapProducer` keeps its `slimmedPhotons` default in AOD mode (`switchOnVIDPhotonIdProducer` changes only `egmPhotonIDs`), which looks like a CMSSW inconsistency; the config sets it explicitly.
- **Validation (200 events each):** AOD and MiniAOD run. `pho_tightId` ∈ {0, 1} for all photons; 47% (AOD) and 60% (MiniAOD) pass wp90; every photon with MVA score > 0.5 passes.
- **Docs:** NTUPLE.md (`pho_tightId`), structure.md, config header.

## 2026-11-09: Photon ID inputs follow the ntuplizer photons (comments.md, item 1)
- **Comment:** the photon ID was configured with an explicit input collection, a leftover from when the `photons` collection was used. The VID producer should pick the right collection on its own.
- **Check:**
  - `switchOnVIDPhotonIdProducer` does set `egmPhotonIDs` to `gedPhotons` (AOD) or `slimmedPhotons` (MiniAOD) by itself.
  - `photonMVAValueMapProducer` always defaults to `slimmedPhotons`. Without an explicit setting, an AOD job stops with `ProductNotFound` (tested).
  - The config already used `gedPhotons` / `slimmedPhotons` (no remaining use of `photons`).
- **Change (`runNtuplizer_cfg.py`):** the VID block moved after the ntuplizer. Both producers now take their input from `process.bmmg.photons`, so the ID value maps always belong to the collection the ntuplizer reads. A comment explains why the MVA producer needs it.
- **Validation:** 300 events each of AOD and MiniAOD are identical to the previous output (apart from the known `mmkk`/`mmcg` non-converging-fit noise). `pho_mva` is filled for all photons.

## 2026-11-09: Check on MC samples of other decay modes; MiniAOD gen fix
- **Samples:** the new `test_files.txt` entries (indices 3–14). B_s → J/ψγ, φγ (φ → μμ), J/ψπ⁰, J/ψη, J/ψη′ and B_d → μμγ, each in AOD and MiniAOD, plus B_s → μμγ (0, 1) as baseline. 3000 events each (J/ψη′ AOD: all 771 events). All 14 jobs ran without errors.
  - The data samples moved to indices 15 (AOD) and 16 (MiniAOD); README and the config header are updated.
- **New tool:** `test/checkDecayModes.py` prints the gen summary and the per-channel candidate summary of an ntuple.
- **Bugs found and fixed (`GenDecayChain`):**
  1. **MiniAOD, π⁰/η/η′ modes:** `prunedGenParticles` drops most photons from π⁰/η/η′ → γγ (e.g. 99 of 303 π⁰ keep their photons). Only about 20% of the events got the right `gen_decayMode`; the rest got 0. The photon gen matching also failed.
     - **Fix:** the analyzer also reads `packedGenParticles` (new parameter `packedGenParticles`, MiniAOD only). Packed particles that are not in the pruned collection (`lastPrunedRef()` not a status-1 copy) are attached to their pruned mother when the decay is walked. The reco–gen matching uses the packed final state. `GenDecayChain` now works on `reco::Candidate*`.
  2. **Wrong B in ~0.3–0.5% of events (AOD and MiniAOD):** the first B with two muons among its descendants was taken. That can be the other b hadron of the event through semileptonic cascades (B → D μν, D → μX).
     - **Fix:** the muon count does not follow open-charm hadrons and τ.
     - **Remaining:** ≤ 0.1% of events where the other b hadron genuinely decays to J/ψ(→μμ)X.
- **New gen branches:** `gen_neutralMesonPdgId`, `gen_vectorMesonPdgId`, `gen_diphotonMass`, `gen_fourBodyMass`, `gen_helicity4`, `gen_coplanarity4`. They were computed but not written.
- **Regression (before/after the fix, same events):**
  - Only gen branches, `mu/pho/conv_gen*` and `<p>genMatched` change, in the events with a corrected B (AOD: 6–13 of 3000). On MiniAOD the matching now uses the packed final state: `mu_genDR` changes at the 1e-5 level (packed precision), and a few photon matches change.
  - Fit quantities also change for some candidates. This is the known non-determinism of non-converging fits (P7.4 / P8 entries):
    - `mmg`: 0.3–0.6% of the candidates differ, almost all with vtxProb < 1e-3;
    - `mmcg`: **4–5%** differ, 0–4 per sample with vtxProb > 1e-3;
    - `mmkk`: about 15 of 3000 events.
    - The `mmcg` rate is higher than the ~1% measured in P7.4; it is open (see below).
- **Results after the fix:** the gen-matched rows use candidates with fitOK and vtxProb > 1e-3; s68 = half of the 16–84% interval.

  | sample | gen_decayMode correct (AOD / MiniAOD) | gen m(μμ) / m(γγ) | mmg fitted − gen B mass, med / s68 [MeV] (AOD) | ct pull s68 (AOD) |
  |---|---|---|---|---|
  | B_s→μμγ | 100% / 99.9% | continuum | +3 / 125 | 1.33 |
  | B_s→J/ψγ | 99.97% / 100% | 3.097 | +8 / 112 (J/ψ-corrected: s68 99) | 1.28 |
  | B_s→φγ | 99.9% / 99.9% | 1.019 | −5 / 162 | 1.36 |
  | B_d→μμγ | 99.97% / 100% | continuum, m_B = 5.2795 | +14 / 122 | 1.46 (median +0.25: ct uses m(B_s), see NTUPLE.md) |
  | B_s→J/ψπ⁰ | 99.9% / 99.97% | 0.135 | −98 / 228 (one cluster = merged π⁰) | 1.63 |
  | B_s→J/ψη | 100% / 100% | 0.548 | −442 / 341 (one photon missing) | 2.56 |
  | B_s→J/ψη′ | 100% / 99.97% | 0.958 | −587 / 284 (one photon missing) | 2.65 |

- **Observations, consistent with expectations:**
  - **Gen:** the B species, B mass, dimuon resonance mass and m(γγ) are right in every sample. The 4-body modes are classified as 4-body; J/ψγ and φγ as 3-body. `gen_isSignal` is 1 only for B_s/B_d → μμγ (by design).
  - **Muons:** both signal muons are reconstructed in 78–84% of the events in all samples.
  - **Photons:** ≥ 1 signal photon is reconstructed in 22–33% of the events (AOD), 18–28% (MiniAOD, `slimmedPhotons` threshold). ≥ 2 signal photons only in 0.2% (π⁰), 2–3% (η) and 3–4% (η′).
    - So `mmgg` has very few candidates: 47 matched on 3000 J/ψη AOD events, 8 on 771 J/ψη′, 2 on J/ψπ⁰. This is the low-energy photon reconstruction of Phase 9.
  - **mmg on the 3-body modes:** the fitted mass is unbiased (|median| ≤ 14 MeV) with 110–160 MeV resolution (φγ is wider: harder spectrum). The highest-vtxProb candidate is the gen-matched one in 83–87% (AOD) and 98% (MiniAOD) of the events with a matched candidate.
  - **mmg on the 4-body modes** (`genMatched` means all legs come from the signal B, so partial reconstruction counts):
    - J/ψη, J/ψη′: the mass is shifted down by the missing photon, and the ct pull is biased because of the missing momentum.
    - J/ψπ⁰: the single "photon" is mostly the merged π⁰ cluster, so the mass is near m(B_s).
  - **Photon energy response:** reco pt / gen pt of the matched photon is about 1.00 in B_s→μμγ and J/ψγ, 1.03–1.11 in J/ψη/η′ and 1.3–2.7 in J/ψπ⁰. The other photon of the meson overlaps with the cluster.
    - Because of this the `mmgg` m(γγ) of J/ψη is high (median 0.66 instead of 0.548, n = 65), and the J/ψη `mmgg` pre-fit mass is +0.38 GeV.
    - For J/ψη′ m(γγ) = 0.94–0.98, close to 0.958.
  - **mmcg (AOD):** about 47% of the candidate fits succeed (the conversion refit succeeds for about 53% of conversions, P7). The fitted mass is 60–135 MeV low (electron energy loss).
  - **mmkk:** no gen-matched candidates in these samples (none of them has kaons), as expected. There are 0.4–0.5 random J/ψ + track-pair candidates per event in the J/ψ samples.
  - **ct:** the pulls (s68 1.28–1.46 for the 3-body modes) agree with the B_s→μμγ validation (P1.6).
- **Open:**
  - The `mmcg` non-determinism rate (~4% of candidates, a few with vtxProb > 1e-3).
  - No J/ψφ (μμKK) sample to validate `mmkk`.
- **Build:** clean; unit tests pass.

---

## 2026-11-06: Phase 9, the custom photon producer compiles and runs (P9.1, P9.2 partial)
- **CMSSW_14_1_1 API fixes in `BMMGCustomPhotonProducer`:**
  - `PhotonIsolationCalculator` and `PhotonMIPHaloTagger` are default-constructed and configured with `setup(...)` in the constructor body; the tagger member is no longer `const`.
  - `mipCalculate` → `MIPcalculate(&photon, evt, es, mipVar)`.
- **Bugs fixed:**
  - `hits` and `preselCutValues` are reset for each supercluster.
  - The EB/EE rechit collections are no longer copied every event (pointers to the products).
  - `PhotonCollection_` was not actually a bug: an empty `std::string` means the default instance label.
- **`fillDescriptions`:** set to `setUnknown()` (no validation) while the module is in development. Its parameters are those of the standard `PhotonProducer`.
- **Configuration:**
  - New `test/customPhotons_cfg.py`: the standard `photons_cfi` retyped to `BMMGCustomPhotonProducer`. A recursive RECO→AOD mapping of the inputs is applied to every nested PSet (rechits, HBHE).
  - Inputs: `photonCore`, an SC Eₜ threshold option (`minSCEt`, default 1 GeV), and the ntuplizer run on the custom photons.
  - The old `customPhotonConfig*.py` were removed (they used parameter and collection names that don't exist).
  - The module is compiled again (plugin library, with the RecoEgamma dependencies in the BuildFile).
- **Result** (300 MC events, compared with gedPhotons):
  - The producer runs and reproduces standard photons: 0.41 photons/event, 0.233 signal photons/event (the same as gedPhotons), 56 gen-matched μμγ candidates (the same).
  - **The lowest photon pt is still 9.3 GeV with `minSCEt=1`**, because the input `photonCore` is built from superclusters with a ~10 GeV threshold.
- **Remaining for the low-energy photon (research):**
  - Build photon cores from lower-threshold superclusters, e.g. `particleFlowSuperClusterECAL` (available in AOD), with a photon-core producer.
  - Then study purity and energy scale at low Eₜ, and the MMGVertexFit error scales for those photons.

---

## 2026-11-05: Phase 8, MINIAOD support (P8.1–P8.4); non-determinism conclusion
**Non-determinism, conclusion of the investigation (from the previous entry):**
- valgrind memcheck on 600 events (μμKK only), which contain the affected entries 456 and 520, finds **no uninitialised or invalid memory access** in RecoVertex, the vertex fitters or the `bmmg` library. The only reports are ROOT's known `TStorage::UpdateIsOnHeap` false positive.
- The remaining run-to-run differences affect only fits that do not converge (χ² ≫ ndf); converged fits are bit-identical. The likely cause is numerical behaviour of the iterative KinematicFit that depends on memory layout.
- **Decision:** accepted as a known limitation; documented in `NTUPLE.md` through the vtxProb recommendation.

**MINIAOD support** (one module, `dataFormat` = `AOD` | `MINIAOD`):
- **P8.1:**
  - Muons and photons are read through `edm::View<reco::Muon>`/`edm::View<reco::Photon>` (pat objects in MINIAOD). Photon MVA and ID are looked up with `ptrAt`.
  - `MuonMVAID::produce` accepts muon pointers; the segment-match information is taken from the pat copy, which avoids a second copy.
- **P8.2:**
  - New common `PVTrack` list for isolation and track compatibility. AOD: the tracks of each PV fit. MINIAOD: `packedPFCandidates` + `lostTracks` with track details and `fromPV == PVUsedInFit`.
  - Kaon candidates in MINIAOD are the pseudo-tracks of the charged packed candidates and lost tracks. Muon tracks are excluded by matching (ΔR < 5e-3, |Δpt/pt| < 1%) instead of by reference.
  - Rechits come from `reducedEgamma`.
  - **Conversions:** in MINIAOD the `reducedConversions` point to tracks that are not stored (this was the job failure). `selectConversions` now skips conversions with unavailable tracks, and the driver switches the `mmcg` channel off for MINIAOD with a message.
- **P8.3:** `TriggerMatcher::evaluate` has a MINIAOD overload. It uses `slimmedPatTrigger` (`unpackPathNames`); a muon matches a path if it is within ΔR < 0.1 of an HLT muon object associated with the path's last filter. The muon MVA runs on `pat::Muon`.
- **Driver config:**
  - `dataFormat` option, MINIAOD auto-detected from the test sample name. A dictionary of collection names per format; VID runs in MiniAOD mode on `slimmedPhotons`.
  - New options `channels=` (list of channels) and `trackCompatibility=`.
- **P8.4 validation:**
  - **AOD unchanged:** compared with the previous version, everything outside the non-deterministic fit branches is identical (muons, photons, dimuons, trigger).
  - **MC MINIAOD** (`BsTo2MuG_miniaod.root`, 32,884 events, 2.5 min) against MC AOD (2000 events). The files contain no common events, so distributions are compared:

    | | AOD | MINIAOD |
    |---|---|---|
    | μ / dimuon / photon per event | 2.35 / 1.39 / 0.64 | 2.12 / 1.12 / 0.25 (`slimmedPhotons` only keep photons above ~10 GeV) |
    | photon-path trigger fired / matched μ per fired event | 0.233 / 1.99 | 0.234 / 1.99 |
    | `mmg` fitted mass 68% half-width (gen-matched, vtxProb > 1e-3) | 0.124 (pre-fit 0.150) | 0.117 (pre-fit 0.155), N=3664 |
    | ct3D pull 68% half-width | 1.27 | 1.32 |
    | isolation mean / max track compatibility mean | 0.922 / 0.164 | 0.950 / 0.097 (different track inputs) |
    | photons with cluster hits | 100% | 98.7% |

  - **Data MINIAOD** (6416 events, about 30 s): 287 branches; 2127 J/ψ dimuons; 0.19 μμKK candidates per event; `DoubleMu4_3_LowMass` fired in 6182 events with 12,385 matched muons. Only 0.09 photons per event (the `slimmedPhotons` threshold), so very few μμγ candidates: **for low-pt photons MINIAOD is limited**.
- **Unit tests:** pass.

---

## 2026-11-05: P7.4 legacy removed; TransientTrack lifetime fix; non-determinism investigation (in progress)
- **P7.4:** removed `RadiativeAnalysisLegacy` and every class only it used:
  - `RadiativeRootTree`, `Tripple/Tetra/ReferenceModeratorVertex`, `KinematicConstrainedFit`, `ReferenceResonance`, `DecayChainVariables`, `BeamSpotAndVertex`;
  - `MuonSelector`, `PhotonSelector`, `RecoPhotons`, `ParticleFlowCandidate`, `SCRecHitAccumulator`, `BaselineMatching`, `VariableDefinitions`, `MuonClosestApproachCalculator`, `GlobalIncludes.h`;
  - the legacy config.

  The last commit with the legacy code is `bde3123`. The plugin library is now `BsToMuMuGammaAnalysisRadiativeAnalysisPlugins`: only `RadiativeAnalysis` and `MMGVertexFitExample`, with a pruned BuildFile. A clean build has no warnings.
- **`MMGVertexFitExample`:** the null check on the matched photon is added (a compiler warning, not a reachable bug), and the unused variables are removed.
- **Bug found: dangling `TransientTrack` pointers in the kinematic particles.**
  - `KinematicParticleFactoryFromTransientTrack::particle()` keeps a raw pointer to the `TransientTrack` it is given.
  - `MMGVertexFit::muonParticle()` (already in the original `sslawins_miniaod` version) and `photonParticle()` used local `TransientTrack`s, so every particle pointed to a destroyed object.
  - `selectConversions` moved the conversion tracks after building the particle.
  - **Fix:**
    - `muonParticle(const TransientTrack&)` now takes a track owned by the caller (`SelectedMuon::track`).
    - Particles are built only after the object vectors are final.
    - Photon `TransientTrack`s are owned by `PhotonParticle`/`Result` (`fitPhotonTracks`).
    - The lifetime rule is documented in `MMGVertexFit.h`.
- **Driver config:** new options `channels=mmg,mmcg,mmgg,mmkk` and `trackCompatibility=0/1`.
- **Non-determinism, still open:**
  - Two identical runs of the same binary (2000 MC events) still differ in a few candidates: 1 `mmg`, about 13 `mmcg`, about 8 `mmkk` (`mmkk` alone, without the track-compatibility fits: 8 of 145 candidates).
  - **Only badly converged fits are affected** (χ² from hundreds to 10⁴, e.g. a μμKK mass of 6.88 vs 2.95 GeV for χ² ≈ 1.4·10⁴). Well-converged candidates are identical.
  - A single affected event processed on its own is reproducible (3 identical runs), so the result depends on the job history: uninitialised memory or state carried between events.
  - A valgrind memcheck run is in progress (next entry).
  - **Practical impact:** small, because these candidates fail any reasonable vtxProb cut. They also explain the event-330 conversion variable of the legacy module (P1.3b, P3).

---

## 2026-11-05: Phase 7, the other channels in the new module (P7.1–P7.3)
- **`MMGVertexFit` generalised:**
  - `fitParticles(particles, pv)`: vertex fit followed by the pointing constraint, for any set of daughters.
  - `photonParticle()`: ECAL photon (cluster measurement).
  - `conversionParticle(e⁺, e⁻)`: e⁺e⁻ vertex fit with a zero-mass constraint.
  - `trackParticle(track, mass)`.
  - `fitCandidate` for N photons.
  - `Result` owns all photon copies and covariances (`fitPhotons`, `fitPhotonCovariances`).
  - The muon mass comes from `Constants.h`.
- **Divergence check (decision):** fits with mass > 100 GeV or |p| > 10 TeV now count as failed. On the filtered sample the ntuple contained masses up to ~10¹⁴ GeV with vtxProb ≈ 0.
- **Candidates:**
  - `MMGCandidate` became the generic `BCandidate` (dimuon + `legs` indices + `xP4`).
  - `MuMuGammaBuilder` became `CandidateBuilders` with `buildMuMuGamma`, `buildMuMuConvGamma`, `buildMuMuGammaGamma` and `buildMuMuKK`. They share the PV choice, the fit and the observables (`Finisher`).
  - Isolation and track compatibility exclude the candidate's own tracks (kaons, conversion tracks).
  - The `(dimuon, PV)` track compatibility is cached when there are no extra tracks.
- **Selection:**
  - `selectConversions`, from `allConversions`: 2 opposite-charge tracks, a valid vertex with prob > 1e-6, pt > 0.5 GeV, plus the zero-mass refit. The legacy module used the onia producer on `allConversions`; reading the collection directly removes that producer from the config.
  - `selectKaonTracks`: high-purity `generalTracks` with pt > 0.7 GeV that aren't muon inner tracks.
- **Ntuple:** `NtupleWriter` books and fills each channel from a channel table (prefix, leg names, leg type):
  - Channels: `mmg_`, `mmcg_`, `mmgg_`, `mmkk_`.
  - New collections: `conv_*`, and `trk_*` (only the kaon tracks used).
  - New candidate branches: `<p>fitDimuonMass` (J/ψ-constrained mass approximation: mass − fitDimuonMass + m_J/ψ) and `<p>xMass` (m(γγ), m(KK)).
  - **Changed:** the `mmg_` daughter branches are now named after the leg (`mmg_phoFit*`, unchanged); `mmg_fitDimuonMass` is new.
  - `NTUPLE.md` is updated.
- **Config:** `doMuMuGamma`, `doMuMuConvGamma`, `doMuMuGammaGamma`, `doMuMuKK` (all on), plus windows and cuts (see `fillDescriptions`).
  - **Decision:** μμKK uses only J/ψ dimuons (`kkJpsiOnly`), with m(KK) in 0.99–1.05 and a pre-fit B window of 4.9–5.9 GeV. The kaon tracks need ΔR < 1.5 to the dimuon and |Δz| < 1 cm to its vertex.
  - **Decision:** μμγγ uses m(γγ) in 0.05–1.1 (π⁰, η, η′). There is no J/ψ mass constraint in the fits; use `fitDimuonMass`.
  - New driver option **`photonID=0`** skips the photon VID, for samples without `fixedGridRhoFastjetAll`. The same option was added to the legacy config.
- **Validation:**
  - **MC B_s→μμγ** (2000 events; 409 branches; 39 s):

    | channel | candidates | fitted | gen-matched | resolution (gen-matched, vtxProb > 1e-3) |
    |---|---|---|---|---|
    | `mmg` | 508 | 395 | 387 | fit 68% half-width **0.124 GeV** vs pre-fit 0.150 |
    | `mmcg` | 1182 | 551 | 189 (2.2 conversions/event, 53% refit OK) | N=53: fit 0.128 vs pre-fit 0.126; the median shifts from −0.065 to −0.049 GeV (electron energy loss) |
    | `mmgg`, `mmkk` | 1, 145 | — | 0 | as expected on this sample |

  - **Filtered B_s→μμγ sample** (36,281 events; P4/P5 code, before the divergence check): **16,406** gen-matched fitted `mmg` candidates. The legacy ntuple had 728 in the same events (P1.6). With vtxProb > 1e-3 (13,104):
    - mass core σ 73.5 MeV (pre-fit 84.0), RMS 0.196 (pre-fit 0.227);
    - ct3D pull core 1.19, RMS 1.71.
  - **Data** (whole 2022C test file, 5721 events): `mmg` 453 candidates (188 fitted), `mmcg` 3326 (1794), `mmgg` 2, `mmkk` 2608 (2577). The J/ψ-corrected μμKK mass shows no clear B_s peak with this statistics.
  - **μμγγ on official B_s→J/ψπ⁰ MC** (Run3Summer22EE AODSIM over xrootd, 3000 events): the channel runs, but only **4 events have a gen-matched candidate**, because the π⁰ photons are mostly below the gedPhoton reconstruction threshold. A meaningful validation needs a J/ψη sample with full AOD content or the low-energy photon of Phase 9.
  - The private J/ψη samples on `/eos/cms/.../privateMC_ForBsMMGAnalysis` lack `fixedGridRhoFastjetAll` and `reducedEcalRecHitsEE`, so neither the new nor the legacy module can run on them.
- **Not validated:** the μμKK normalisation channel on a B_s→J/ψφ MC sample; there was none in the test list.

---

## 2026-11-05: Phase 6, configuration, CRAB, README (P6.3–P6.5)
- **P6.3:** removed the stale configs `makeTree_BsMuMuGamma_DATA_AOD.py`, `..._MC_AOD_Test.py` and `..._MC_MINIAOD.py`; none of them could run.
  - The new driver `test/runNtuplizer_cfg.py` covers MC and data. It was tested on the MC and data AOD test files (P4/P5 entry).
  - The legacy config `makeTreeLegacy_BsMuMuGamma_MC_AOD.py` is kept, together with the legacy module.
- **P6.4:** `crab/`, `crab_mc/` and `crab_data/` are replaced by **one** `crab/crabConfig.py`:
  - It runs `runNtuplizer_cfg.py`. The sample comes from environment variables: `BMMG_NAME` and `BMMG_DATASET` or `BMMG_FILELIST`; optionally `BMMG_ISMC`, `BMMG_LUMIMASK`, `BMMG_GLOBALTAG`, `BMMG_UNITS`, `BMMG_OUTLFN`, `BMMG_SITE`.
  - Request names are distinct (taken from `BMMG_NAME`). The output goes to the submitter's area (`getUsername()`), no longer to another user's.
  - Data from DAS uses LumiBased splitting, with the lumi mask applied by CRAB.
  - `crab/makeFileList.py` takes a glob pattern and an output name, which fixes the file-list name mismatch.
  - Only a syntax check was possible here; it **has not been submitted**.
- **P6.5:** `README.md` rewritten: setup including the modified CMSSW packages, how to run, layout, documentation links.

---

## 2026-11-05: Phase 4, unit tests
- `test/test_bmmgLibrary.cc` (catch2, `scram b runtests`) tests:
  - `flightInfo`: distances, cosα, the sign and value of ct3D/ct2D, and the full-covariance error ≥ the position-only error.
  - `lowestDCAVertex`, including the empty PV collection.
  - The helicity, coplanarity and lepton angles.
  - The dimuon resonance flags.
  - `BranchSet`: double booking, unbooked access and wrong-type access throw; reset; fill.
- **Result:** all tests pass (27 assertions in 5 test cases).
- The unused CMSSW template tests (`test_catch2_*.cc.disabled`) are removed.

---

## 2026-11-05: Phase 4 + 5 (+ P6.1/P6.2), the new `RadiativeAnalysis` module and ntuple
**New library code** (`src/` + `interface/`, namespace `bmmg`):

| component | file | replaces |
|---|---|---|
| constants (PDG 2024 masses, dimuon mass windows) | `Constants.h` | 3 copies of `MassLimits`, `nominal*Mass`, `ReferenceResonance::Constants` |
| data structs | `Candidates.h` | `DecayChainVariables` |
| PV choice, flight distance / ct, angles, isolation, track compatibility, DCA | `CandidateObservables.{h,cc}` | `VariableDefinitions`, `BeamSpotAndVertex` statics, angle code in the analyzer, `MuonClosestApproachCalculator` |
| muon / photon preselection | `ObjectSelection.{h,cc}` | `MuonSelector`, `PhotonSelector` (reco photons) |
| dimuons | `DimuonBuilder.{h,cc}` | the 3 copied dimuon blocks |
| μμγ candidates | `MuMuGammaBuilder.{h,cc}` | `TrippleObjectVertex` (reco-photon part) |
| trigger bits + muon matching | `TriggerMatcher.{h,cc}` | trigger code in the analyzer |
| ntuple storage | `BranchSet.h` | each variable declared once instead of 4 hand-written lists |
| ntuple | `NtupleWriter.{h,cc}` | `RadiativeRootTree` |
| gen truth | `GenChain` + new `fromSignalB()` | `fillMCInfo`, analyzer matching code |

- **The module** (`plugins/RadiativeAnalysis.cc`, about 300 lines):
  - It only orchestrates the components.
  - It uses TFileService (`usesResource`) and a complete `fillDescriptions`, which auto-generates `radiativeAnalysis_cfi`.
  - No raw `new`; MC-only products are consumed only when `isMC`.
- **Driver config** `test/runNtuplizer_cfg.py`:
  - Takes `inputFiles` or a `testSample` from `test_files.txt`.
  - `isMC` is detected from the sample name; the GlobalTag defaults depend on MC/data; `lumiMask` is supported.
  - It runs the photon VID on gedPhotons, then the analyzer.
- **Ntuple:** `bmmg/Events` + `bmmg/RunInfo`, 209 branches (MC) / 162 (data). All muons, photons, dimuons and μμγ candidates are stored as vectors with index links. **Documented in `NTUPLE.md`**, including the differences from the legacy ntuple.

**Decisions:**
- **Preselection:**
  - **No MVA cuts at ntuple level**; the scores are stored. Muon pt > 2 GeV, |η| < 2.4. Photon E_SC > 1 GeV, |η_SC| < 2.5.
  - μμγ pre-fit window 4–7 GeV (sidebands included).
  - All of these are configurable.
- **Candidates:**
  - Every dimuon with a valid vertex × every photon. A candidate is kept even if its fit fails (`mmg_fitOK=0`).
  - The dimuon resonance flag uses J/ψ / ψ(2S) / φ windows. K* is dropped (no K*→μμ).
- **ct definition and error:**
  - ct = m(B_s)·(d·p)/|p|², signed projection; same for ct2D.
  - The error is propagated from the full 6×6 covariance of the fitted state (SV position, momentum and their correlations) plus the PV covariance.
  - With only position errors the pull width was about 1.7; with the full covariance it is 1.27.
- **Lxy error:** `dimu_lxyBSErr` includes the beam-spot covariance. The legacy error had only the vertex error.
- **Event PV:** `pv_*` is the first vertex (highest Σpt², the CMS default). The PV for each candidate is in `mmg_pv*`.
- **Dropped:** the leading-two-photon / PF-candidate blocks and the full SC rechit dump (P5.4). A per-photon seed time is kept.
- **Phase 3 for the legacy code:** not done. The legacy module stays frozen as a reference until Phase 7 and is then deleted.

**Validation:**
- **MC**, `BsTo2MuG_aod.root`, 2000 events (`reference_new_P45_mc2000.root`):
  - Per event: 2.35 muons, 0.64 photons, 1.39 dimuons, 0.25 μμγ candidates.
  - 508 candidates; 78% fitted; **387 gen-matched candidates in 386 events**. The legacy ntuple had 68 fitted candidates in the same events.
  - Fitted mass − gen mass, gen-matched with `vtxProb > 1e-3` (N=272): median 0.000, 68% half-width **0.124 GeV**; pre-fit: median −0.007, half-width 0.150.
  - ct3D pull: median 0.02, 68% half-width **1.27**, 2.6% with |pull| > 4. ct3D residual width 20 µm, mean error 18 µm.
  - Diverging fits (8 of 396) have `vtxProb ≈ 0` and absurd masses; `NTUPLE.md` recommends a vtxProb cut.
  - 76% of the selected muons and 37% of the selected photons come from the signal B.
  - Trigger: the photon path fired in 466 events, with 926 matched muons.
  - `RunInfo` agrees with the legacy values (xsec 5.754e8 pb, filter efficiency 0.001663).
- **Data**, `testSample=3` (2022C ParkingDoubleMuonLowMass0 AOD), 2000 events in 36 s (`reference_new_P45_data2000.root`):
  - Runs without errors; no gen branches; `RunInfo` is −1 except nEvents.
  - 664 J/ψ dimuons. `DoubleMu4_3_LowMass` fired in 1919 events with 3856 matched muons.
  - 48% of the μμγ candidates have a successful fit (background).
- **Legacy module:** unchanged (it shares only `MMGVertexFit`/`GenChain` with the new code, plus the added `fromSignalB`).

---

## 2026-11-05: Phase 4 step 1, legacy module and library layout
- **Renames:**
  - The analyzer is now **`RadiativeAnalysisLegacy`** (`plugins/RadiativeAnalysisLegacy.cc`, `interface/RadiativeAnalysisLegacy.h`).
  - Its config is `test/makeTreeLegacy_BsMuMuGamma_MC_AOD.py`, previously `makeTree_BsMuMuGamma_MC_AOD.py`. **The CRAB configs still point to the old name; they are fixed in Phase 6.**
- **New package library** `BsToMuMuGammaAnalysisRadiativeAnalysis`: `RadiativeAnalysis/BuildFile.xml` + `src/`.
  - `MMGVertexFit.cc` and `GenChain.cc` moved there, so both analyzers link them.
  - New code goes there in namespace `bmmg`.
  - `plugins/` now holds the EDM modules and the legacy helper classes.
- **Validation:** the legacy output is identical to `reference_P2b_2000.root` (555/555 branches, with the known non-deterministic branch ignored).

---

## 2026-11-05: Phase 3 starts; the restructuring strategy (P3.2, P3.8)
- **Decision, how Phases 3–5 are done:**
  - Most of Phase 3 (ownership, TFileService, `fillDescriptions`, constants, includes) would patch code that Phase 4/5 replace anyway.
  - The current analyzer will become **`RadiativeAnalysisLegacy`**: still compiled and still producing the old ntuple, with all channels (conversions, μμγγ, μμKK) until Phase 7 ports them.
  - A **new `RadiativeAnalysis`** is built from the Phase 4 components (library in `src/`, namespace `bmmg`) and writes the Phase 5 ntuple. It is written to the Phase 3 standards from the start.
  - Benefit: the old and new ntuples can be compared on the same events.
- **P3.2 (`run3mvaid`):**
  - `MuonMVAID.h` no longer has `using namespace std` or the global `MatchPair`/`LorentzVector` typedefs; they moved into the `.cc`, so nothing leaks into the analyzer headers.
  - `XGBooster` owns its handle: a destructor with `XGBoosterFree`, no copies, move constructor and assignment. `MuonMVAID` builds the boosters with `emplace_back`.
  - `XGBooster::set` throws on an unknown feature name; before, it silently wrote feature 0.
- **P3.8:** `MVAAnalyzer` (+ config) and `ReadSoftMvaRun3ValueProducer` moved to `devel/`.
- **Note:** `MMGVertexFit::Result::particle->lastConstraint()` points to a constraint local to the fit and must not be used (documented in the header). The same holds for the conversion pre-fit in `PhotonSelector`.
- **Build:** OK.
- **Validation:** identical to `reference_P2b_2000.root` except one known non-deterministic branch:
  - **Known issue:** `VertexfitBsCtErr3D_mmconvg` differs at the 1e-5 level **between two runs of the same binary** (checked twice on 400 events).
  - All other branches, including every MMGVertexFit `_mmrecog` branch, are reproducible, so the source is in the legacy conversion fit path (`KinematicConstrainedFit` + `PhotonSelector` conversion refit). I checked that the modified RecoVertex particle initialises its photon pointers.
  - That path is replaced in Phase 7. Until then, regression checks use `--ignore VertexfitBsCtErr3D_mmconvg`.

---

## 2026-11-05: Phase 2, batch 2 of bug fixes (P2.12–P2.15) — Phase 2 complete
New reference: `test/reference/reference_P2b_2000.root`.
- **P2.12 (`ReferenceResonance`):**
  - `calculateMassFromComponents` no longer shadows `totalVec`; it always returned 0 before.
  - **NonResonant** (flag 4) is now set for dimuons with both muons above 3 GeV that are outside all resonance windows. Before, its window was 0, so it was never set, and the dimuon kinematics (`dcv.dimuonMass/Eta/Phi/Pt`) were filled **only for resonant pairs**.
  - `KinematicConstrainedFit` treats NonResonant like None (pointing constraint only, no mass constraint). Without this, the conversion fits would have failed for these pairs.
  - The μμKK kaon ΔR cut uses the dimuon direction from the muons, not the resonance eta/phi (which were −9999 for non-resonant pairs).
  - `ReferenceModeratorVertex` no longer writes the μμKK mass into `fittedBmassConvertedPhoton` (copy-paste error). `HadronMass_fromVertexFitConPhoton` is now never filled and will be removed in Phase 5.
- **P2.13:**
  - `nominalPsiMass` set to 3.686097 (it held the J/ψ mass).
  - `PsiPDGMass` is read from the config.
  - The `KinematicConstrainedFit` result members are initialised.
- **P2.14 (trigger):**
  - The bit for `HLT_DoubleMu4_3_LowMass` is renamed `triggerbit_HLT_DoubleMu4_3_LowMass`, and a new bit `triggerbit_HLT_DoubleMu4_3_Jpsi` is added.
  - The list of paths is now a member (`triggersOfInterest_`).
  - With `verbose=True`, `beginRun` prints the filters used for the matching. For the test file:
    - The `Photon4_BsToMMG` paths end with a μμγ mass filter **and** the e/γ filter `hltEG4R9Id50b80eFilter`.
    - `Jpsi_NoVertexing` included a `-hltL1f...` module whose result the path ignores.
  - The old logic required a muon to match an object of **every** terminal filter, so muons could never match the photon paths.
  - **Fix (decision for open question 4):**
    - Muons are matched (ΔR < 0.1) only to HLT muon objects (`filterIds == TriggerMuon`).
    - Filters without muon objects are not legs.
    - Ignored or negated modules (`-`/`!` prefix) are skipped.
- **P2.15:** the track collection has its own `tracks` parameter, set in the MC config; `pfCandTag` is no longer read.
- **Validation** (`reference_P2a_2000.root` → this version; with `--map` for the renamed LowMass bit), 523 of 555 branches identical:
  - Dimuon kinematics (`DiMuon*_beffit`, `DiMuon_ResonanceType`, `LxyOverPt`) are filled in 417 more events, about 538 instead of 120. NoBound branches: 336.
  - μμKK candidates in 76 events (before: almost none).
  - Trigger matches in 2000 events:

    | path | before | after |
    |---|---|---|
    | `Photon4_BsToMMG` (path fired in 466 events) | 31 matched muons | **974** |
    | `Displaced_Photon4_BsToMMG` | 23 | **716** |
    | `Jpsi_NoVertexing` | 0 | **229** |
    | `DoubleMu4_3_Bs` | 43 | 43 (unchanged) |
- **Build:** OK.

**Phase 2 summary:** all items are done except P2.3, which is deferred to P4.1. Still open (not bugs): open question 2, the photon exclusion `pt > 50 || !isEB` in the leading-photon block. That block will be replaced by per-selected-photon vectors in Phase 5, so it is left as is.

---

## 2026-11-05: Phase 2, batch 1 of bug fixes (P2.1, 2.2, 2.4–2.11, 2.16–2.20)
All of these change the content of the ntuple on purpose. New reference: `test/reference/reference_P2a_2000.root`.

**Corrections to the plan's diagnosis (found while fixing):**
- **P2.1:** the `photon*_[2]` and `pfCand*_[2]` arrays do **not** overflow, because `RecoPhotons`/`ParticleFlowCandidate` return at most 2 objects. The real bug was an **index mismatch**: the helpers sort by pt, but the analyzer took the EB/pt exclusion, tight ID, MVA (photons) and the pt<0.5 check (PF) from `(*collection)[i]` of the *unsorted* collection, i.e. from a different object.
- **New bug:** 17 `bool` branches (`Mu1/Mu2/DiMuon_isMatched`, `Photon_isMatched`, `Topology_*`, `IsSignalMatched`, `IsMuonsFromJpsi`, `PFECAL_RecHit_Is*`, …) were reset to `-9999999`, which converts to **true**. Events without matching therefore looked "matched". In the P1 reference this gave `Mu1_isMatched` 0.986 and `Topology_isMatched` 0.866.
- **New bug:** the analyzer copied `View<GenParticle>` into a local vector before building `GenDecayChain`. `mother()`/`daughter()` point into the original product, so pointer comparisons (ancestry) could never work. `GenDecayChain` now gets the original `std::vector<reco::GenParticle>` product (new token `genParticlesVecTok`).

**Changes:**
- **P2.1:**
  - `RecoPhotons`/`ParticleFlowCandidate` return the original `index` and use `partial_sort` on indices instead of copying and sorting whole collections (the PF collection has thousands of entries).
  - The analyzer uses `photonVar[i].index` for the exclusion, tight ID and MVA. The wrong-object PF pt check is removed.
  - Dead code in `ParticleFlowCandidate` removed.
- **P2.2:** `scBrem` and `refphotonscBrem` are guarded against a zero eta width (the analyzer now uses `photonVar.scBrem`). In `VariableDefinitions::calculateCt2DError/3DError` the zero check now comes before the division.
- **P2.4 (`BeamSpotAndVertex`):**
  - All `pv_*` (ntracks, covariance, ndof, ρ, position) come from the same selected vertex (highest scalar Σpt; unchanged choice). Before, ntracks came from `front()` and covariance/ndof/ρ from the last vertex in the loop.
  - `bs_dx/dy/dz` are filled (x0/y0/z0 errors).
  - `LowestDCAIndex` returns −1 only for an empty PV collection, uses a min search instead of a sort, and every caller checks the result.
  - `NSelectedVertices` and `pv_cutmultiplicity` are initialised.
- **P2.5 + P2.18 (`GenChain`):**
  - Null check on `mother()`.
  - Photons are sorted by pt **before** the angular variables are computed.
  - The π0-gun fallback is used only if the event has no b hadron.
  - Decay classification accepts extra FSR photons (`nGam >= 1` / `>= 2`). **Decision for open question 3:** extra photons are treated as FSR.
  - Bd 4-body modes get their own codes: `kBd_Jpsi_Pi0/Eta/Etap` = 12/13/14, appended so the existing codes are unchanged.
  - `processDaughters` has the `!muPlus_` guard.
  - `matchToGen` uses final-state particles only (status 1).
  - `matchTopology` requires the matched gen μ/γ to **descend from the signal B** (`isAncestor`) instead of a hardcoded grandmother 531. It works for direct and cascade decays and for Bd. `decayModeMatched` covers the Bd and J/ψγ modes.
- **P2.6:** mu2 pixel hits come from `muTrack2` (all three builders).
- **P2.7:** the photon acceptance is `|η_SC| < 2.5` (it was `isEB && η < −2.5`, which never cut anything). **Decision:** the endcap veto in the μμγ path is removed, because it was a workaround for the old fit and MMGVertexFit is validated for EE (P1.6). Result: 19 more fitted candidates in 2000 events (49 → 68). The 49 existing ones are unchanged.
- **P2.8 (`PhotonSelector`):** the MVA score is taken from the list parallel to the passing photons. The conversion index is recorded only for successful fits. The `MassKinematicConstraint` leak is fixed with `unique_ptr`. The log category is renamed.
- **P2.9:** Tripple now writes `VertexfitBs{CtErrMPV,CtMPVRefit,CtErr3DRefit,Lxy,LxyErr}_mmconvg_` instead of `_mmconvgg_`. Tetra fills the J/ψ branches only for J/ψ.
- **P2.10:**
  - `BsVtxProb_mmrecog` is the probability of the fitted μμγ vertex; the dimuon vertex probability is kept only as the quality cut.
  - `BsCt2DBS_mmrecog` uses the fitted B vertex.
  - The `AlgebraicMatrix31` objects are indexed `(i,0)`.
  - The MPV error uses the same PV as the position (closest-z).
  - `abs` → `std::abs`.
  - For μμKK, `BsVtxProb_mmkk` was already overwritten by the fit probability, and `BsCt2DBS_mmkk` uses the 4-track Kalman vertex, which is a real B vertex, so they are kept.
- **P2.11:**
  - Cowboy = Δφ(μ⁺, μ⁻) > 0, wrapped to (−π, π], and set for every pair.
  - `isPFMuon` no longer overwrites the tracker/global category.
  - `muon::overlap` is checked first.
- **P2.16:** MC events without a gen B are kept; the `Gen*` branches keep their defaults.
- **P2.17:**
  - The run-level variables are in a new **`RunInfo`** tree (1 entry) instead of a fake extra event row.
  - The filter efficiency is merged over all lumi blocks (`GenFilterInfo::mergeProduct`); for 2000 events it is now 0.001663 instead of the last lumi's 0.001631.
  - The cross section is an inverse-variance weighted mean over runs (`GenRunInfoProduct` has no merge).
  - Missing gen products give a warning only in MC.
  - `genXsec_` and `lumiEquivalent_` are initialised.
- **P2.19:** topology matching uses the selected photon.
- **P2.20:** `photonTightID` is booked as `/O`.
- **P2.3:** deferred to P4.1 (see plan.md).

**Validation** (`reference_P1_fit_2000.root` → this version, 2000 events): 417 of 555 branches are identical. Every difference matches one of the fixes above:
- **Gen matching** (bool defaults, ancestry):
  - `Mu1_isMatched` 0.986 → 0.316 per event (631 of the 659 events with a muon pair).
  - `Topology_isMatched` is **41%** among events with a selected photon: 91% of the muon legs match, but only about 51% of the selected photons come from the B.
  - `IsMuonsFromJpsi` is 0 on this μμγ sample, as it should be.
- **PV branches:** `PVndof` mean 3.7 → 137, now the real PV.
- **Photon and PF branches** (index fix): 67 events differ for photons; the PF branches are now filled for 1598 events.
- **Fit:** `_mmrecog` fit branches gain the 19 EE candidates.
- **Gen classification:** 3 events changed decay mode (FSR).
- **Build:** OK.

---

## 2026-11-05: Phase 1, validation on the large filtered sample (P1.6)
- **New tool:** `test/validateMMGFit.py`. Mass and ct3D residuals and pulls against gen from the ntuple (optionally `--matched`, i.e. `Topology_isMatched`). Mean, RMS and core Gaussian width of every `MMGVertexFitExample` histogram. PNG plots.
- **Input:** the first 20 files of the filtered sample (`testSample=2 testMaxFiles=20`), **36,281 events**. Code as of P1.5: gedPhotons, MMGVertexFit, EB photons only in the analyzer.
- **Analyzer ntuple** (2133 events with a fitted μμγ candidate; gen-matched topology: 728):

  | | pre-fit 4-vector | MMGVertexFit |
  |---|---|---|
  | m − m_gen core σ, all (N=2133) | 75.6 ± 3.3 MeV | **59.9 ± 2.6 MeV** |
  | m − m_gen core σ, gen-matched (N=728) | 73.3 ± 5.6 MeV | 70.7 ± 5.4 MeV |
  | fraction with \|m − m_gen\| > 0.1 / 0.2 / 0.3 / 0.5 GeV | 44% / 24% / 14% / 4.1% | **32% / 14% / 7.7% / 2.8%** |
  | m − m_gen RMS | 0.203 | 0.271 |

  The fit improves the core and lowers the tail fraction at every threshold. The larger RMS comes from a small number of far outliers (only 20 of 2133 candidates are more than 0.3 GeV off after the fit while within 0.1 GeV before it). The fit χ²/probability is not stored per candidate yet (P5.2 adds it), so these outliers can't be cut on yet.
  - ct3D − ct_gen: core σ 17 µm, but RMS 411 µm.
  - ct3D error: mean 15.6 µm.
  - **ct3D pull: core σ 1.40, RMS 5.4.** The ct error is too small by about 40% in the core, and the definition has large tails. `calculateCt3D` uses the unsigned PV–SV distance instead of the projection on the B direction, and the error ignores correlations. This is planned for P4.7 (`CandidateObservables`).
- **`MMGVertexFitExample`** (gen-matched μ, μ, γ, gen PV, EB+EE):

  | quantity | N | core mean | core σ |
  |---|---|---|---|
  | fitted B mass | 14264 | 5.374 | 0.0716 |
  | B mass without fit | 15687 | 5.376 | 0.0857 |
  | mass residual / pull | 14264 | +0.007 / +0.17 | 0.0717 / 1.08 |
  | vertex pull x / y / z | 14264 | ~0 | 1.08 / 1.10 / 1.05 |
  | cluster pull φ / θ | 17940 | ~0 | 0.82 / 0.91 |
  | photon energy pull | 18181 | +0.32 | 0.87 |
  | χ²/ndf (mean); vertex prob (mean) | 14264 | 1.68; 0.379 | — |

  The vertex and mass pulls have core width ≈ 1, so the error scales are consistent. The cluster pulls (0.82 / 0.91) and the energy pull (0.87, mean +0.32) are a bit narrower than 1. They are inputs whose scales were tuned on another sample, so re-tuning is possible but not needed now.
- **Conclusion:** MMGVertexFit is an improvement over both the pre-fit mass and the old fit (P1.3b: RMS 0.162 vs 0.217 on the same candidates). Phase 1 is complete.
- **Note for later:** the old combined job (analyzer + example started in one shell) failed because of a broken trailing `ls`; the example was rerun separately. Run jobs separately.

---

## 2026-11-05: Phase 1, deprecate KinematicConstrainedFit (P1.5)
- `KinematicConstrainedFit` is marked deprecated in its header. It is kept only for μμ+conversion, μμγγ and μμKK until Phase 7.
- Removed the unused methods `doFit` and `TrippleObjectVertexFitRecoPhoton` (replaced by MMGVertexFit), together with their declarations, about 200 lines.
- The old dev1 `MMGVertexFit` code was already replaced in P1.1.
- **Build:** OK.
- **Validation:** 561/561 branches identical to `reference_P1_fit_2000.root`, including the event-330 conversion value. That 1e-5 shift seen in P1.3b is therefore reproducible and depends on the code, not random.

---

## 2026-11-05: Phase 1, MMGVertexFit in the analyzer (P1.3b + P1.4)
- **P1.4:**
  - `RadiativeAnalysis` owns an `MMGVertexFit mmgVertexFit_`, configured by the new parameters `clusterCovScalePhiEB/EE`, `clusterCovScaleThetaEB/EE`, `energyErrorScaleEB/EE`.
  - In `makeTree_BsMuMuGamma_MC_AOD.py` they are set to the gedPhotons values from `MMGVertexFitExample.py`: EB 0.16 / 0.165 / 1.2, EE 0.20 / 0.24 / 1.3.
- **P1.3b:**
  - In the reco-photon path of `TrippleObjectVertex`, `KinematicConstrainedFit::TrippleObjectVertexFitRecoPhoton` is replaced by `MMGVertexFit::fitCandidate`.
  - Muon kinematic particles are built once per muon pair. The pointing constraint uses the lowest-DCA PV, chosen as before from the dimuon Kalman vertex and the pre-fit μμγ momentum.
  - `TrippleObjectVertexObservables` takes `const MMGVertexFit&`.
  - Removed: the leaked `new TMatrixD` per photon, and the photon `FreeTrajectoryState` with its 1e6 placeholder covariance.
  - Everything after the fit (ct, ct errors, isolation, compatibility, PV–SV distances) is unchanged and uses the new fitted B and vertex.
- **Behaviour differences on purpose:**
  1. The old fit dropped the photon cluster model in its final constrained refit; the new one keeps it.
  2. The old fit added a J/ψ/φ/K* mass constraint on the dimuon when it was flagged as a resonance; MMGVertexFit has no mass constraint. This doesn't matter for non-resonant Bs→μμγ, but it is needed later for J/ψγ / J/ψη (Phase 7).
  3. The endcap veto (`photon.isEE() continue`) is kept for now, so the change is only the fit. The new fit is validated for EE in the example, so the veto can probably go (P2.7).
- **Build:** OK. No new warnings.
- **Validation** against `reference_P1_ged_2000.root` (2000 events):
  - 550 of 561 branches are identical. The 10 `_mmrecog` fit-derived branches differ in exactly the 49 events with a fit: mass, ct2D/3D, ct errors, Dist2D/3D and their errors, isolation.
  - The same 49 events have a fitted candidate before and after.
  - 1 unexpected difference: `VertexfitBsCtErr3D_mmconvg` in event 330, 0.00211144 → 0.00211146 (relative 1e-5). The conversion fit doesn't use the changed code, so this is a numerical side effect, maybe from state in the fit framework or the uninitialised `KinematicConstrainedFit` members (P2.13). To be checked when the conversion path is reworked.
- **Physics comparison** on the 49 candidates, against the gen B mass and gen ct3D:

  | | mass − gen: mean | RMS | median abs |
  |---|---|---|---|
  | pre-fit 4-vector | −0.081 | 0.206 | 0.071 |
  | old fit | −0.040 | 0.217 | 0.110 |
  | **new fit** | −0.066 | **0.162** | **0.071** |

  ct3D − gen: old fit mean +0.0039, RMS 0.0109; new fit mean +0.0036, RMS 0.0108 (gen mean 0.048). The **ct3D pull RMS is 5.3 (old) and 4.4 (new)**: `VariableDefinitions::calculateCt3DError` clearly underestimates the error (mean 0.0016 against a spread of 0.011). This goes to P1.6 and P4.7.
- **New reference for later steps:** `test/reference/reference_P1_fit_2000.root` (this version).

---

## 2026-11-05: Phase 1, switch to gedPhotons (P1.3a)
- **Config:** in `makeTree_BsMuMuGamma_MC_AOD.py`, `PhotonTag`, `photonMVAValueMapProducer.src` and `egmPhotonIDs.physicsObjectSrc` now use `gedPhotons` instead of `photons` (user decision: the MMGVertexFit error scales are tuned on gedPhotons).
- **New reference:** `test/reference/reference_P1_ged_2000.root` (same 2000 events). **Later Phase 1 steps compare against this one.**
- **Comparison with P0:**
  - 476 of 561 branches are identical: all muon, dimuon, trigger, PV/BS, μμKK, gen-level and PF branches.
  - The 85 that differ are all photon-dependent: photon variables, the μμγ/μμγγ pre-fit and fit branches, photon MVA and gen match, and topology matching.
  - No "missing photon MVA" warnings, so the VID ValueMaps match the collection.

  | quantity | P0 (`photons`) | P1 (`gedPhotons`) |
  |---|---|---|
  | photons per event (leading 2) | 0.435 | 0.674 |
  | events with a selected-photon MVA (`GammaMVAScore_mmrecog`) | 90 | 203 |
  | events with pre-fit μμγ mass (`Bsmass_recommg`) | 77 | 89 |
  | events with fitted μμγ mass (`VertexfitBsMass_mmrecog`) | 30 (mean 5.376) | 49 (mean 5.327) |
  | `Photon_isMatched[0]` | 0.995 | 0.951 |
- **Note:** the topology-matching fractions drop (`Topology_isMatched` 0.904 → 0.866) because they use the two leading *raw* photons, not the selected one (bug P2.19). With more photons in the collection, the leading ones are more often not the signal photon.

---

## 2026-11-05: Phase 1, MMGVertexFit API for the analyzer (P1.2)
- **New API:**
  - `static std::optional<RefCountedKinematicParticle> muonParticle(const reco::Muon&, const TransientTrackBuilder*)`: built once per muon and event.
  - `std::optional<Result> fitCandidate(mu1, mu2, const reco::Photon&, lazyTools, ttBuilder, const GlobalPoint& pv) const`: fits one candidate with an explicit PV for the pointing constraint.
  - `fit(...)` (used by the example) is now a thin wrapper that loops over the combinations with `primaryVertices[0]`.
- **`Result` gains:**
  - `daughters`: the fitted μ1, μ2 and γ.
  - `mu1Index`, `mu2Index`, `photonIndex`: set by `fit()`.
  - `fitPhoton`, `fitPhotonCovariance` (`shared_ptr`): the fitted photon particle keeps raw pointers to the photon copy and its covariance. Before, they were destroyed when `fit()` returned while the results were still in use. Now they live as long as the Result.
- **Pitfall found:** calling `KinematicTree::finalStateParticles()` **before** the pointing-constraint fit changed the result of 1 of the 290 fits. The daughters are therefore read after the constraint fit, which only changes the top particle.
- **Build:** OK.
- **Validation:** `MMGVertexFitExample` on 2000 AOD events gives histograms bin-for-bin identical to P1.1. The example is deterministic: two identical runs gave identical output.

---

## 2026-11-05: Phase 1, port MMGVertexFit (P1.1)
- **Port:** `git checkout sslawins_miniaod --` `interface/MMGVertexFit.h`, `plugins/MMGVertexFit.cc`, `plugins/MMGVertexFitExample.cc`, `test/MMGVertexFitExample.py`. The old dev1 prototype is replaced. Nothing else used the old API.
- **Example config:**
  - Uses VarParsing: `dataFormat` (AOD/MINIAOD), `maxEvents`, `outputFile`, `inputFiles`.
  - Without `inputFiles` it takes the AOD or MINIAOD test file from `test_files.txt` (`testSample`, `testMaxFiles`).
  - The hardcoded `ls` of an EOS directory is removed.
- **Build:** OK against `../RecoVertex` (`photonKinematicFit_v2`). The new warnings are 3 unused variables plus a maybe-uninitialized `bestMatchedMuon`, all in `MMGVertexFitExample.cc`.
- **Validation:** `cmsRun MMGVertexFitExample.py maxEvents=2000` on `BsTo2MuG_aod.root` (gedPhotons, gen PV, gen-matched μμγ) took 32 s:

  | quantity | N | mean | RMS |
  |---|---|---|---|
  | fitted B mass | 290 | 5.381 | 0.150 |
  | B mass without fit | 332 | 5.391 | 0.158 |
  | mass residual / pull | 290 | −0.004 / 0.02 | 0.140 / 1.43 |
  | vertex pull x / y / z | 290 | 0.15 / 0.09 / −0.04 | 1.39 / 1.43 / 1.41 |
  | cluster pull φ / θ | 378 | −0.11 / −0.03 | 1.88 / 1.43 |
  | photon energy pull | 382 | 0.17 | 1.32 |
  | χ²/ndf; vertex prob | 290 | 1.75; 0.36 | — |

  Pull RMS values are above 1 because of non-Gaussian tails; the error scales were tuned on the core widths. For comparison, the current analyzer fills a fitted μμγ mass in only 30 of the same 2000 events.
- **Decision (user):** use **gedPhotons** in the analyzer, because the error scales were tuned on them. This is applied in P1.3.

---

## 2026-11-05: Phase 0, reference ntuple and regression tool (P0.5)
- **Input handling:**
  - New `RadiativeAnalysis/python/testFiles.py` parses `test/test_files.txt` (a header line ending in `:`, then paths or globs). `testFiles(i)` returns the `file:` list of sample `i`, or of a sample matched by a description substring.
  - `makeTree_BsMuMuGamma_MC_AOD.py`: the hardcoded list of 21 xrootd files is replaced by `inputFiles=...`. When that option is empty, the config uses `testSample=<index>` (default 0, the AOD test file) and `testMaxFiles=<n>`.
- **Regression tool, `test/compareNtuples.py`** (uproot/awkward, available in the CMSSW python):
  - Compares the common branches of two ntuples event by event (`--rtol`, `--entries`, `--ignore`, `--map old:new` for renamed branches).
  - Lists the branches found in only one file. The exit code is 0 when everything agrees.
- **Reference:** `RadiativeAnalysis/test/reference/reference_P0_BsTo2MuG_aod_2000.root`, the first 2000 events of `/eos/user/s/sslawins/testing_mc/BsTo2MuG_aod.root`, plus its log. The directory is git-ignored.
  - Command: `cmsRun makeTree_BsMuMuGamma_MC_AOD.py nEvents=2000 outputFile=.../reference_P0_BsTo2MuG_aod_2000.root`.
  - Runtime is about 1 min; it is dominated by EOS I/O.
- **Determinism check:** a separate 100-event run against the reference (`--entries 100`) gives 561/562 branches identical. The exception is **`photonTightID`**, which holds garbage that changes from run to run. It is declared `bool photonTightID_[2]` but booked as `"photonTightID[2]/I"`, so ROOT reads 8 bytes from a 2-byte array. **Added to plan.md as P2.20.** Until that fix, regression checks use `--ignore photonTightID`.
- **How to run the regression check after each change:**
  ```
  cmsRun RadiativeAnalysis/test/makeTree_BsMuMuGamma_MC_AOD.py nEvents=2000 outputFile=new.root
  python3 RadiativeAnalysis/test/compareNtuples.py RadiativeAnalysis/test/reference/reference_P0_BsTo2MuG_aod_2000.root new.root --entries 2000 --only-diff --ignore photonTightID
  ```
  `--entries 2000` skips the fake row that `endJob` adds (the tree has 2001 entries).
- **Baseline content of the reference (2000 events of Bs→μμγ MC):**
  - `vertexTypeFlag`: unset in 1341 events, **3 in 659, never 1 or 2**. The μμKK builder runs whenever exactly 2 muons are selected and overwrites the μμγ flag (plan P4).
  - `DiMuonM_beffit` is filled in 120 events, `Bsmass_recommg` in 77, and `VertexfitBsMass_mmrecog` in 30 (mean 5.376).
  - `HadronMass_fromVertexFitRecoPhoton` is never filled (the DecayChainVariables field is never written).
  - Every μμγγ (`_mmrecogg`) mass is empty.
  - `GenIsSignal` = 0.9965.
- **Open issues:** the tree has N+1 entries because of the fake `endJob` row (P2.17). The reference must be regenerated whenever an intentional change alters existing branches; each new reference is recorded here.

---

## 2026-11-05: Phase 0, build fixed (P0.1–P0.4)
- **P0.1, initial build** (`scram b -k -j8`): only `BMMGCustomPhotonProducer.cc` fails, with 3 errors; the plugin link failure follows from them. The causes are CMSSW_14_1_1 API changes:
  - `PhotonIsolationCalculator(pset, flagsEB, flagsEE, sevEB, sevEE, iC)` constructor no longer exists.
  - `PhotonMIPHaloTagger(pset, iC)` constructor no longer exists.
  - `mipCalculate` was renamed to `MIPcalculate`.
- **P0.2:**
  - Moved `BMMGCustomPhotonProducer.{h,cc}` and `customPhotonConfig{,_cfi}.py` to `RadiativeAnalysis/devel/`, which scram does not compile.
  - Added `devel/README.md` explaining how to revive it (plan Phase 9).
- **P0.3:** Deleted `interface/SPRecHitAccumulator.h`: unused, with a syntax error at line 43.
- **P0.4, BuildFiles:**
  - `plugins/BuildFile.xml` now lists every package included by the code explicitly (GeneratorProducts, HepMCCandidate, ParticleFlowCandidate, CaloGeometry, VertexTools, VertexPrimitives, HLTReco, BeamSpot, …) instead of relying on transitive dependencies.
  - Removed the dependencies only the photon producer needed: RecoEgamma/*, CaloTowers, HcalAlgos, EcalLaserCorrection, EcalObjects, HLTfilters.
  - `run3mvaid/BuildFile.xml`: added FWCore/Framework, FWCore/ParameterSet, FWCore/Utilities, DataFormats/{Math,MuonReco,PatCandidates,Scouting}.
  - `test/BuildFile.xml`: the catch2 target is commented out, because its sources are `.disabled`.
- **Build:** a clean package build (`scram b clean; scram b -j8`) succeeds with 0 errors and 10 warnings:
  - 9 unused variables: `ParticleFlowCandidate.cc:86`, `MMGVertexFitExample.cc:173,176,178`, `PhotonSelector.cc:170,255`, `RadiativeAnalysis.cc:246,1201,1202`.
  - 1 maybe-uninitialized `bestMatchedMuon` in the old `MMGVertexFitExample.cc`.

  These are left for Phases 1 and 3.
- **Validation:** none yet. The reference ntuple is P0.5.

---

## 2026-11-05: Code review and planning
- **Done:**
  - Reviewed the whole `RadiativeAnalysis` package, `run3mvaid`, the configs, and the `MMGVertexFit` rework on `sslawins_miniaod`.
  - Wrote `structure_old.md` (current structure) and `plan.md` (prioritised task list, Phases 0–9).
- **Decisions recorded with the user:**
  - Analyzer stays AOD-only for now; MMGVertexFit gets ported.
  - The ntuple may be redesigned.
  - All candidates are stored as vectors.
  - Bs→μμγ (ECAL photon) comes first.
  - Default PV is lowest DCA.
  - The muon displacement cut is removed.
- **Build:** not run yet (P0.1 is next).
- **Validation:** none yet.
- **Open issues:** the open questions in `plan.md` (photon collection `photons` vs `gedPhotons` for the MMGVertexFit error scales; photon pt/EB exclusion; FSR in the gen classification; trigger-matching logic; a data test file).
