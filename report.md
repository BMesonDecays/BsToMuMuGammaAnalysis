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

## 2026-11-23: v3 tables and the new variables in the mmg BDT
- **Conversion** (`python3 convert.py config/convert_v3.yaml`, LCG_107/108, lz4 compression since these pyarrow builds lack zstd):
  - output `/eos/user/s/sslawins/bmmg/parquet_v3`, channels mmg, mmcg, mmgg, same cut as v1 (`fitOK == 1 and vtxProb > 1e-3`);
  - MC: 7 datasets in 22 s. Data: 3705 files in 634 s (24 workers), no unreadable file. mmg 3.04M candidates (2.4 GB), mmcg 28.5M (15 GB), mmgg 30k;
  - the CRAB production is not complete: ParkingDoubleMuonLowMass0 has 407 of ~476 files. The data hold 96.69% of the processed events of v1, so `analysis_v3.yaml` uses `lumi_pb: 33503` and `combine_v3_mmg.yaml` both luminosities × 0.9669. A re-run of the conversion adds the missing files (incremental); then update the luminosities.
- **Configuration:** `load_config` now supports a base configuration (`base:`, recursive merge, `null` removes an entry). `analysis_v3.yaml` (base `analysis.yaml`) adds derived columns of the new variables: daughter isolations without the B products (`muIsoRel_max/min`, `muIsoCh_max`, `muIsoNeu_max`, `phoIsoRel`, …), `nMuInPV`, `logPvSumPt`.
- **GPU:** `LCG_108_cuda` has XGBoost with CUDA (RTX 2000 Ada): a 5-fold training takes 30–40 s instead of ~20 min. In LCG_108, `hist` axis edges and centres are wrong (boost-histogram 1.3.2; contents are right): fixed in `plotting.data_vs_mc` and `combine_hists.py` (TH1D written from numpy).
- **Feature study** (`scripts/feature_study.py config/analysis_v3.yaml config/features_v3.yaml`): 60,882 signal and 307,388 sideband candidates; out-of-fold signal efficiency at sideband efficiency 2e-4 (mean of 2–3 seeds, spread ≈ 0.005):

  | set | eff@2e-4 | eff@1e-3 | AUC |
  |---|---|---|---|
  | baseline (v1 features) | 0.319 | 0.526 | 0.9905 |
  | `pho_mva` → `phoMvaNoMu` | 0.335 | 0.547 | 0.9914 |
  | + `mu1InPV`, `mu2InPV` | 0.317 | 0.524 | 0.9905 |
  | + relative daughter isolations | 0.369 | 0.571 | 0.9922 |
  | + absolute daughter isolations | 0.384 | 0.583 | 0.9922 |
  | + PV activity (`pvNTracks`, `logPvSumPt`, `pvVecSumPt`) | 0.328 | 0.551 | 0.9916 |
  | + standard PF isolations | 0.356 | 0.562 | 0.9921 |
  | all of the above except the PV flags, + abs. isolations (`r2_abs`) | 0.402 | 0.611 | 0.9935 |
  | `r2_abs` + standard PF isolations (`r2_abs_pf`, **chosen**) | 0.429 | 0.623 | 0.9939 |

  - The daughter isolations without the B products are the strongest new input; the photon MVA without the muons helps a little, the muon-PV flags not at all (their information is presumably already in the impact-parameter and flight variables).
  - Mass sculpting (`check_sculpting.py`, inner/adjacent sideband strips): baseline 0.99 ± 0.04 / 0.93 ± 0.12 / 1.10 ± 0.23 at background efficiencies 1e-2 / 1e-3 / 3e-4; `r2_abs_pf` 0.90 ± 0.04 / 0.89 ± 0.11 / 0.76 ± 0.18. No sign of the BDT favouring masses near the signal window.
  - `analysis_v3.yaml` uses `r2_abs_pf` (52 features).

## 2026-11-22: Preselection variants: narrower J/ψ and ψ(2S) vetoes, lower vertex-probability cut (`scripts/presel_variants.py`, v1 tables)
- **Dimuon resolution in data** (double Gaussian, after vertex and trigger cuts): J/ψ at 3.093 GeV, σ = 21 / 43 MeV (50/50; 19 / 34 MeV for |η| < 1, 35 / 60 MeV above); ψ(2S) σ = 24 / 45 MeV; φ σ = 9 / 19 MeV.
- **Method:** the candidates are loaded once with the loosest selection, each variant is a query. Scores come from the Combine-study BDT (trained on the baseline; out-of-fold, folds by event number). Per BDT threshold: S in the signal window, B = sidebands × exponential factor fitted at that threshold × 326/38, Asimov Z with known background, at least 10 raw sideband candidates.
- **Results** (best Z over the thresholds; at the best threshold, 0.9982, all variants with the old J/ψ low edge have the same 22 sideband candidates):

  | variant | S presel | sidebands | Z@0.99 | Z@0.995 | Z@0.998 | Z best |
  |---|---|---|---|---|---|---|
  | baseline (J/ψ 2.9–3.3, ψ(2S) 3.55–3.8, vtxProb > 0.01) | 65.2 | 318,030 | 1.29 | 1.66 | 2.10 | 2.35 |
  | vtxProb > 3e-3 | 68.9 | 348,724 | 1.31 | 1.67 | 2.13 | 2.39 |
  | vtxProb > 1e-3 | 71.3 | 371,862 | 1.31 | 1.67 | 2.15 | 2.41 |
  | J/ψ 2.90–3.20 | 68.3 | 330,030 | 1.30 | 1.69 | 2.13 | 2.39 |
  | J/ψ 2.95–3.20 | 70.0 | 339,564 | 1.27 | 1.64 | 2.17 | 2.42 |
  | J/ψ 3.00–3.20 | 71.7 | 353,561 | 1.19 | 1.54 | 2.00 | 2.11 |
  | J/ψ 3.02–3.17 | 73.3 | 373,145 | 1.07 | 1.41 | 1.76 | 1.90 |
  | J/ψ 3.04–3.15 | 74.6 | 406,191 | 0.92 | 1.21 | 1.56 | 1.75 |
  | ψ(2S) 3.55–3.75 | 66.2 | 321,068 | 1.29 | 1.66 | 2.12 | 2.37 |
  | J/ψ 2.90–3.20, ψ(2S) 3.55–3.75, vtxProb > 1e-3 | 76.0 | 389,525 | 1.31 | 1.70 | 2.21 | 2.47 |

- **Conclusions:**
  - Opening the J/ψ veto on the low side adds background the BDT does not remove: real J/ψ (radiative tail) with a photon from b-hadron decays, which the BDT never saw in training. The high side (3.2–3.3) and the ψ(2S) high edge can be opened.
  - Lowering the vertex cut to the table floor (1e-3) gives +9% signal at the same background after a tight BDT cut.
  - The combination: +16% signal after the preselection, Z 2.35 → 2.47 (+5%) with the baseline BDT.
- **Open:** a BDT trained on the variant (the job was stopped before it finished); vtxProb < 1e-3 needs tables converted without the 1e-3 cut. To be redone on the v3 tables.

## 2026-11-22: Candidates removed before the analysis preselection (ntuple skim and conversion cut)
- **Two filters with vtxProb > 1e-3 in the v1 production:**
  - ntuplizer `skim=1`, `skimMinVtxProb=1e-3` (`tasks_all.yaml`): per event. An event is written if any candidate of any channel has a valid fit with vtxProb > 1e-3; all its candidates are written. `RunInfo.nEvents` counts all events, so the normalisation is right;
  - `convert.py` with `config/convert.yaml`: `preselection: "fitOK == 1 and vtxProb > 1e-3"`, per candidate (recorded in the v1 manifests). This is why no candidate in the tables is below 0.001.
- **Size of the loss:** the current ntuplizer without the skim (`skim=0`) on 10,000 events of `bsmmg_miniaod`. Gen-matched `mmg` candidates in [4.5, 6.5] GeV:

  | | candidates | fit failed | vtxProb ≤ 1e-3 | (1e-3, 1e-2] | > 1e-2 |
  |---|---|---|---|---|---|
  | all | 1383 | 0 | 18.0% | 7.6% | 74.4% |
  | + event trigger | 906 | 0 | 14.7% | 7.3% | 78.0% |
  | + window [5.15, 5.60] | 777 | 0 | 13.1% | 7.2% | 79.7% |

  Events with a gen-matched candidate: 1383, of which 1135 pass the skim and 1134 keep the candidate after the conversion cut. The conversion cut, not the skim, removes the candidates.
- **Consequences for `presel_impact.py` (2026-11-22 entry below):**
  - the rows from the vertex cut (vtxProb > 0.01) on, and so the 65 signal events after the preselection, are unchanged: a candidate with vtxProb > 0.01 always passes both filters;
  - the first two rows are low by 15–18%. Corrected: ~195 in [4.5, 6.5] without the trigger, ~130 with the event trigger (110.8 / 0.853), ~110 in the signal window with the trigger (95.7 / 0.869);
  - the vertex cut keeps 0.80 of the triggered signal in the window, not 0.91: it costs 20% of the signal;
  - with the event trigger, ~130 is close to the 141 of `PhotonCollectionStudy` (`slimmedPhotons`); the rest is the stricter `genMatched` (pt compatibility).
- The first row of `presel_impact.py` is now labelled `tables (vtxProb > 1e-3)`.

## 2026-11-22: Impact of the preselection on the number of candidates (`scripts/presel_impact.py`, `mmg`, v1 tables)
- **What was done:** `python3 scripts/presel_impact.py config/combine_mmg.yaml` (LCG_107, 12 s). Cumulative cut flow, each cut alone and N-1, for the gen-matched signal MC (Run 3 projection of the Combine study: 297 fb⁻¹ good luminosity, B = 5×10⁻⁹) and the data sidebands (× 326/38). B in the signal window = sidebands × 0.45/1.25. Tables in `<output>/presel/`.
- **Cumulative** (S in the signal window [5.15, 5.60]; 1453 signal decays pass the generator filter):

  | step | S window | S step eff. | sideband cand. | B step eff. | S/√B |
  |---|---|---|---|---|---|
  | ntuple | 137.1 | – | 1,116,601 | – | 0.074 |
  | event trigger (information) | 95.7 | 0.70 | 1,093,388 | 0.98 | 0.052 |
  | vertex prob > 0.01 | 87.4 | 0.91 | 958,484 | 0.88 | 0.051 |
  | resonance veto | 67.0 | 0.77 | 399,149 | 0.42 | 0.060 |
  | ρ/ω veto | 65.2 | 0.97 | 372,878 | 0.93 | 0.061 |
  | both muons trigger-matched | 65.2 | 1.00 | 318,030 | 0.85 | 0.066 |
  | (extra) soft muons | 64.2 | 0.98 | 299,868 | 0.94 | 0.067 |
  | (extra) muon pt > 4 / 3 | 58.7 | 0.91 | 247,590 | 0.83 | 0.067 |

- **N-1** (signal / sideband efficiency of each cut on top of the others): vertex 0.91 / 0.86; resonance veto 0.76 / 0.37; ρ/ω veto 0.97 / 0.93; trigger 0.70 / 0.84.
- **Conclusions:**
  - The trigger (30% of the signal) and the resonance veto (24%) cost the most. The trigger matching of the muons costs no signal on top of the event-level flag and removes 15% of the background.
  - Resonance veto, signal in the windows after vertex and trigger: J/ψ (2.9–3.3) 14.8%, ψ(2S) (3.55–3.8) 7.0%, φ (0.97–1.07) 1.6%. The signal m(μμ) peaks at 2–3 GeV after the trigger. A J/ψ window matched to the dimuon resolution could recover part of the 15% (not tried).
  - Without the trigger the ntuple has 160 gen-matched signal events in [4.5, 6.5] GeV (111 with the event trigger). `PhotonCollectionStudy` gave 141 with the trigger for `slimmedPhotons`: the ntuple `genMatched` also requires a pt compatibility of the reco and gen legs (`GenDecayChain::matchToGen`), and the photon must pass the ntuplizer object selection and the vertex fit.

## 2026-11-22: Signal yield per photon collection (`Study/`, `PhotonCollectionStudy`)
- **Question:** the v1 tables give about 75 signal events in Run 3 before the BDT (MiniAOD, `slimmedPhotons`, preselection), while an older study on the legacy MC with the AOD `photons` collection gave about 300.
- **What was done:**
  - `Study/plugins/PhotonCollectionStudy.cc`: per event, a gen B_s → μ⁺μ⁻γ (hardest photon if PHOTOS added more); `HLT_DoubleMu4_3_LowMass_v*` fired; both gen muons matched to reco muons (same charge, ΔR < 0.05); the gen photon matched to the closest reco photon (ΔR < 0.1). The μμγ mass is the plain sum of the reco 4-vectors. One module per photon collection, the rest is shared.
  - `Study/test/photonCollections_cfg.py` (`sample=legacy|new_aod|new_miniaod`), `Study/test/photonCollections.py` (yields and mass plot).
  - Samples:
    - new MC: 4 Run3Summer22 MINIAODSIM files and their 4 parent AODSIM files (the same 13,492 events, xrootd; file lists in `Study/test/files_new_*.txt`); σ = 5.746×10⁸ pb, filter eff. 1.701×10⁻³;
    - legacy MC: 40,000 events (130 files); σ = 5.822×10⁸ pb, filter eff. 0.1298.
  - Normalisation: B = 5×10⁻⁹, L = 297 fb⁻¹ (good-luminosity equivalent of 326 fb⁻¹ recorded, as in the Combine study).
- **Results** (expected events, raw MC counts in brackets):

  | sample | photons | trigger | + 2 μ matched | + γ matched | + m in [4.5, 6.5] |
  |---|---|---|---|---|---|
  | legacy AOD | `photons` | 2833 (1009) | 2814 (1002) | 149 (53) | 149 (53) |
  | legacy AOD | `gedPhotons` | 2833 (1009) | 2814 (1002) | 180 (64) | 174 (62) |
  | new AOD | `photons` | 727 (6752) | 722 (6703) | 177 (1647) | 173 (1603) |
  | new AOD | `gedPhotons` | 727 (6752) | 722 (6703) | 176 (1638) | 173 (1610) |
  | new MiniAOD | `slimmedPhotons` | 727 (6752) | 722 (6702) | 142 (1322) | 141 (1311) |

- **Conclusions:**
  - The photon reconstruction threshold is what costs the signal: no reco photon below 8.5–9 GeV in `photons`, below 10 GeV in `slimmedPhotons` (MiniAOD keep cut `pt > 10 && (pt > 14 || chargedHadronIso < 10)`); `gedPhotons` has a few lower-pt photons. Only 5–6% (legacy) and 20–25% (new MC, gen photon pt > 4 GeV) of the triggered events with matched muons have a matched photon.
  - `photons` and `gedPhotons` give the same yield (new AOD 177 vs 176; legacy 149 ± 20 vs 180 ± 22). `slimmedPhotons` loses 20% with respect to `gedPhotons`.
  - The legacy and new MC agree after the photon match (150–180 vs 176): the generator cuts of the new MC lose no reconstructable events.
  - No photon collection gives 300 events with this normalisation; the earlier estimate must differ in the normalisation (BR, luminosity, cross section) or the matching.
  - From ~140 (`slimmedPhotons`, no preselection) to ~75: the ntuple preselection.
- **Open:** the legacy statistics are small (53–64 matched events, ±13–15%).

## 2026-11-22: Expected B_s → μμγ significance in Run 3 (326 fb⁻¹) with Combine, per BDT cut (`mmg`, v1 tables)

**Result:** the expected (blind, Asimov) significance rises to **2.44σ at BDT > 0.998** (2.41σ at 0.999).
- Shape fit of the B mass in Combine v11.1.0, background slope and yield floating.
- B(B_s→μμγ) = 5×10⁻⁹ assumed; no systematics apart from the floating background.
- The expected 95% CL limit at that cut is r < 0.81 (0.57–1.18 at ±1σ), i.e. B < 4.1×10⁻⁹.
- Consistent with the earlier analytic estimate (2026-11-15: 1.9σ cut-and-count, 2.7–2.9σ with 3 categories and a known background shape): this is one category, and the background shape now floats.

**Inputs and normalisation:**
- **Tables:** v1 tables in `/eos/user/s/sslawins/parquet` (`Parking2022`, `BsToMuMuGamma_2022`), channel `mmg`.
- **Selection and BDT:** the preselection and BDT of `config/analysis.yaml` (37 features, tuned parameters, 5 folds).
- **Data:** 2022 = 34.65 fb⁻¹ golden, 38 fb⁻¹ recorded. Target: 326 fb⁻¹ recorded.
  - The sidebands are scaled by 326/38 = 8.58.
  - The signal MC is normalised to the equivalent good luminosity, 34.65 × 326/38 = 297 fb⁻¹, with the generator σ × filter efficiency of the sample and `br_scale` = 5×10⁻⁹.
- **Signal after the preselection:** 75.4 in [4.5, 6.5] GeV, 65.4 in the signal window [5.15, 5.60] GeV. This matches the earlier 7.6 at 34.65 fb⁻¹ × 297/34.65 = 65.

**Code** (commits 803ee04, 3ffa942, 3b41566, 686cc0c; README section "Expected significance in Run 3 with Combine"):
- **`config/combine_mmg.yaml` + `config.load_combine_config`:** the base `analysis.yaml` is used as it is. The file overrides the parquet and output directories and adds the luminosity projection, the BDT cuts (0, 0.9, 0.95, 0.98, 0.99, 0.995, 0.998, 0.999, 0.9995) and the fit range [4.5, 6.5] GeV with 25 MeV bins.
- **`scripts/combine_hists.py`** (LCG_107):
  - samples via `load_samples`; the blind window is never loaded;
  - k-fold BDT trained (or loaded) in `<output>/bdt`, out-of-fold scores;
  - per cut, `mmg_mass_hists.root` with `signal`, `data_sb` (× 8.58) and `data_sb_raw`, plus `yields.csv`.
- **`scripts/combine_fit.py`** (Combine area, CMSSW_14_1_0_pre4 + HiggsAnalysis-CombinedLimit v11.1.0 in `/eos/home-s/sslawins/combine/`), per cut:
  - double-sided Crystal Ball fitted to the signal MC, refitted at each cut, then fixed;
  - exponential fitted to the raw sidebands (binned Poisson likelihood in the sideband ranges);
  - workspace, with background slope and yield as `flatParam`, and a datacard;
  - `combine -M Significance -t -1 --expectSignal 1` and `-M AsymptoticLimits --run blind`;
  - `results.csv`, fit plots (blind window not drawn), `significance_vs_cut.png`.
- **`io.load` fix:** the v1 tables are zstd-compressed, and the pyarrow 15/20 of the LCG views (106–108) is built without zstd. `io.load` now falls back to polars (0.18 in LCG_107).
- **XGBoost:** LCG_107's is built without CUDA, so `device: cuda` runs on the CPU (warning only). The training took about 20 min on 32 cores.

**BDT:**
- Out-of-fold AUC 0.991.
- 60,893 signal and 318,030 sideband candidates; 1150–1240 trees per fold.
- At BDT > 0.995: signal efficiency 0.38, sideband efficiency 3.2×10⁻⁴ (2026-11-15: 0.33 at 2×10⁻⁴).
- Signal train/test KS p-values 10⁻¹³–10⁻²² (as before); all numbers use out-of-fold scores.

**Results:**
- S, B: in [4.5, 6.5] GeV / in [5.15, 5.60] GeV, at 326 fb⁻¹.
- sb raw: unscaled 2022 sideband candidates.
- σ: DSCB core width.
- λ: exponential slope [GeV⁻¹].
- Z(known B): binned Asimov with the background fixed.
- Z(Combine): background slope and yield float.
- r_exp: expected 95% CL limit on B/(5×10⁻⁹).

| BDT > | S | S win | B | B win | sb raw | σ [GeV] | λ | Z(known B) | **Z(Combine)** | r_exp (±1σ) | B limit |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | 75.4 | 65.4 | 4.43×10⁶ | 1.02×10⁶ | 318030 | 0.062 | −0.29 | 0.08 | (0.23)* | 30 (22–41) | 1.5×10⁻⁷ |
| 0.9 | 62.2 | 55.7 | 43764 | 9546 | 3218 | 0.060 | −1.02 ± 0.03 | 0.71 | 0.65 | 3.11 (2.26–4.34) | 1.6×10⁻⁸ |
| 0.95 | 56.8 | 51.4 | 23200 | 4957 | 1723 | 0.060 | −1.13 ± 0.04 | 0.91 | 0.83 | 2.42 (1.74–3.37) | 1.2×10⁻⁸ |
| 0.98 | 47.3 | 43.4 | 9015 | 1863 | 680 | 0.059 | −1.28 ± 0.07 | 1.27 | 1.15 | 1.73 (1.25–2.42) | 8.7×10⁻⁹ |
| 0.99 | 38.3 | 35.7 | 3693 | 748 | 281 | 0.061 | −1.36 ± 0.11 | 1.66 | 1.50 | 1.32 (0.95–1.86) | 6.6×10⁻⁹ |
| 0.995 | 28.7 | 27.1 | 1325 | 256 | 103 | 0.059 | −1.53 ± 0.19 | 2.16 | 1.95 | 1.02 (0.72–1.45) | 5.1×10⁻⁹ |
| **0.998** | 16.7 | 16.0 | 335 | 53.1 | 28 | 0.057 | −2.10 ± 0.48 | 2.75 | **2.44** | 0.81 (0.57–1.18) | 4.1×10⁻⁹ |
| 0.999 | 9.4 | 9.1 | 97 | 15.9 | 8 | 0.056 | −2.01 ± 0.87 | 2.73 | 2.41 | 0.84 (0.57–1.26) | 4.2×10⁻⁹ |
| 0.9995 | 4.4 | 4.3 | – | – | 1 | – | – | – | not fitted (< 3 sideband candidates) | | |

\* At cut 0 the expected ΔNLL is Z²/2 ≈ 0.003 on 4.4×10⁶ weighted events, below the precision of the minimisation: Combine's 0.23 (0.22 with the background frozen) is a numerical artefact. The known-background value 0.079 is robust (same with log1p and as Σs²/b). The cut is irrelevant for the result.

**Validation:**
- **Combine install:** a counting card with S = 10, B = 100 gives Z = 0.9840 (formula 0.984).
- **Asimov dataset** (`GenerateOnly -t -1`): 80 bins, 351.3 events = 334.6 B + 16.7 S. The model in the Combine workspace is identical to the fitted one (pdf values, parameters, yields).
- **Cost of the floating background** at BDT > 0.998: in the Combine workspace the yield is `shapeBkg_bkg_mmg__norm`.
  - both frozen: 2.761 (formula 2.759);
  - only the yield floating: 2.601;
  - slope and yield floating: 2.439.
  So the slope, constrained only by the 28 sideband candidates (λ = −2.1 ± 0.5), costs about 0.16σ and the yield another 0.16σ.
- **Fits:**
  - the signal DSCB describes the MC at every cut (σ_core 0.056–0.062 GeV; flat pulls);
  - the exponential describes the sidebands (deviance 45–62 for 48 degrees of freedom) at BDT ≥ 0.9; at the preselection level it does not (387/48), which is irrelevant for the result;
  - at tight cuts almost all sideband candidates are in the low sideband (0.998: 27 low, 1 high), so the slope is steep and comes mostly from below the peak.

**Caveats:**
1. **Background model:** a single exponential, extrapolated from a low sideband dominated by a steeply falling component (probably partially reconstructed B decays) under the peak. A different shape (e.g. a component ending near 5.2 GeV) changes B under the peak. The background-shape systematic (alternative functions, discrete profiling) is not included.
2. **No other systematics:** signal yield (BR, σ × filter efficiency of the MC, luminosity, efficiencies), signal shape (mass scale and resolution in data).
3. **Statistical precision:** at the best cuts the background comes from 8–28 sideband candidates of 2022. The choice of the best cut on the same candidates biases Z up slightly. The 0.998 and 0.999 results agree within that precision.
4. **Production and conditions:** v1 production (MiniAOD photons with p_T > 10 GeV, no photon-energy or conversion corrections); 2022 conditions and trigger for all of Run 3.
5. **One category:** combining several BDT-score categories in one datacard would gain, as in the analytic estimate (3 categories: +0.3–0.5σ).

---

## 2026-11-22: CRAB submission of the legacy MC (`submitCrab.py`, task key `legacy`)

**Changes** (commit bd1601b):
- **Task key `legacy: true`:** the pset is `test/runNtuplizer_legacy_cfg.py` instead of `runNtuplizer_cfg.py`. AOD only; `--dry-run` shows "(legacy cfg)".
- **`testMaxFiles=1`:** the script adds it, because without an input option the legacy configuration lists its default test sample. CRAB loads the configuration once at submission and sets the input files of each job, so one file is enough.
- **Input check in the legacy configuration:** it now looks for an input option in all of `sys.argv`. CRAB loads it with the configuration file, not `cmsRun`, as `argv[0]`.
- **Missing file list:** a `fileList` that does not exist is now an error at `--dry-run`.
- **Documentation:** an example task in `tasks_example.yaml`, and the README.

**Checks:**
- `--dry-run` of a legacy file-list task:
  - `psetName` = `runNtuplizer_legacy_cfg.py`;
  - `pyCfgParams` = isMC=1, dataFormat=AOD, outputFile, testMaxFiles=1, plus the task options;
  - whitelist T2_CH_CERN.
- The configuration loaded as CRAB does, with these parameters:
  - empty endcap rechits, `photonMaxAbsSCEta` 1.479, the rho producers in the task, the 2022 conversion scale map;
  - 1 s, 52 kB pickled.
- The other tasks (`tasks_example.yaml`) are unchanged.
- Not submitted.

---

## 2026-11-22: Ntuplizer for legacy AOD MC without endcap ECAL rechits (`runNtuplizer_legacy_cfg.py`)

**Sample.** `/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV_09_07_2025/`.
- Private B_s → μμγ AOD, CMSSW_12_4_11_patch3, zero bias, no pileup: 7401 files, 1.7 TB, about 310 events per file.
- It has **no generator cut on the photon pt**, unlike the samples used so far (cut at 4 GeV): in the test, 81% of the signal photons are below 4 GeV (median 2.0 GeV).
- Missing products: `reducedEcalRecHitsEE`, any rho (`fixedGridRho*`), and the run/lumi generator info (`GenRunInfoProduct`, `GenFilterInfo`).
- The main configuration fails: `EcalClusterLazyTools` gets both rechit collections in its constructor.

**Implementation** (commit 6a2a2c3). `test/runNtuplizer_legacy_cfg.py` executes `runNtuplizer_cfg.py` (same options, same ntuple format; default test sample `bsmmg_legacy_aod`, added to `test_files.yaml`) and then:
- removes the endcap reco photons: `photonMaxAbsSCEta = 1.479`, the barrel edge. Their cluster variables and the ECAL photon fits need the endcap rechits, so `mmg`/`mmgg` contain barrel photons only;
- passes an empty endcap rechit collection: `EmptyEcalRecHitCollectionProducer` in `plugins/RadiativeAnalysis_legacy.cc`. With no endcap photons it is never looked up;
- produces `fixedGridRhoAll` and `fixedGridRhoFastjetAll` from `particleFlow`, for the VID photon MVA and the photon MVA without the muons.

The ntuplizer module itself is unchanged. Conversions use only the tracker, so `mmcg` and `chic` cover the whole acceptance.

**Test** on 2 files (645 events, about 25 s):
- exit 0;
- 25 photons, none in the endcap (maximum |SC η| 1.440). `pho_mva` and `mmg_phoMvaNoMu` are computed for all of them (rho from PF);
- 255 conversions, 185 of them with |η| > 1.479. 45 come from the signal B, 26 of those in the endcaps;
- 5 `mmcg` candidates, 3 truth-matched; `conv_scale` 0.893–0.990;
- RunInfo: xsec and filter efficiency −1 (no generator products in the sample), with a warning per run.

**Note:** `crab/submitCrab.py` uses the main configuration and DAS datasets. The legacy sample is a list of EOS files and is meant to run locally or on the batch system with the legacy configuration.

---

## 2026-11-22: Converted-photon energy-scale correction in the ntuplizer (`mmcg`), with the 2022 χ_c map

**Implementation** (commits 88421ec, 9caac9c, 2f8834c):
- **`ConversionScaleMap`** (interface/, src/): reads the JSON `ScaleMap` of `analysis/scripts/conv_scale.py` with nlohmann/json (`<use name="json"/>`).
  - Only a 2D map in `pt_est` × `abs(conv_eta)` is accepted (either order). A map in other variables, such as the MC truth map in gen pt, or a scale outside 0.5–1.5 is refused.
  - The bin value is clamped to the outer bins, as `conv_scale.py` with `interpolate: false`; the filled high-pt bins are already in the JSON.
  - The map is in the true photon pt, so a reconstructed photon uses s at pt_reco / s(pt_reco), one iteration.
- **`MMGVertexFit::scaledParticle`:** copy of a standalone kinematic particle with the momentum and its covariance rows and columns scaled (relative errors unchanged).
- **`applyConversionScale`:** sets `SelectedConversion::energyScale`, `p4Corr = p4/s` and `photonCorr`.
  - Without a map they equal the uncorrected values.
  - The `mmcg` builder uses the corrected photon: prefit mass and mass window, B vertex fit, observables.
  - `conv_*` and `chic_*` stay uncorrected: the χ_c block is the calibration sample.
  - New branch `conv_scale` = s.
- **Configuration:** parameter `conversionScaleMap` (empty: no correction). cfg option `convScaleMap`:
  - `default`: `RadiativeAnalysis/data/conv_scale_2022.json`, the full-2022 map of the previous entry, applied to data and MC as in AN-2023/035;
  - `none`;
  - or a path.
- **Unit tests** (`testBmmgLibrary`, 54 assertions pass): lookup, clamping, the pt iteration, swapped variable order, refused maps.

**Validation** on the B_s → J/ψγ MiniAOD test file (35k events, channels mmcg,chic), with `convScaleMap=none` against `default`:
- **Unchanged:** `conv_pt` and `chic_mass` are identical.
- **Applied scale:** `conv_scale` is 1 with `none`, and 0.89–0.99 (median 0.952) with the map.
- **Prefit mass:** √(m_μμ² + (m² − m_μμ²)/s) to 4·10⁻⁵ GeV, for the 9925 candidates present in both.
- **Fitted photon pt:** rises by 1/s exactly (median 1.063). The mass error rises by 1.6%, because the momentum error scales with the momentum.
- **Gen-matched, fitted `mmcg` mass** (971 candidates):

  | | without correction | with the 2022 data map |
  |---|---|---|
  | prefit mass: mode / core mean [GeV] | 5.305 / 5.310 | 5.345 / 5.350 |
  | fitted mass: mode / core mean [GeV] | 5.315 / 5.315 | 5.375 / 5.371 |

  m_Bs = 5.367 GeV. The data map brings the MC peak to within 4 MeV of m_Bs, so the MC and data scales agree at the 0.1% level for these photons. This is a test file only; the MC map of `conv_scale_mc.py` needs the production.
- The number of `mmcg` candidates changes slightly (10236 → 9996), because the mass window uses the corrected prefit mass.

**Notes:**
- The map file is in the package `data/` directory, which CRAB ships in the sandbox. A new map (e.g. Run 3) is copied there or given with `convScaleMap=<path>`.
- The 2022 map is the default for every year until per-year maps exist.
- The local reference ntuples (`test/reference/`, `compareNtuples.py`) were made without the correction. The `mmcg` comparison now differs by design; use `convScaleMap=none` to compare.

---

## 2026-11-22: Converted-photon scale: pt binning up to 13 GeV; empty bins filled from the last measured pt bin; coarser N2/N1 binning (full 2022 data)

**Why.** The map had one pt_est bin from 3 GeV. AN-2023/035 (π⁰ → γγ, O(10⁶) π⁰ from all 2018 datasets) bins photon pt up to 17.2 GeV. A fine binning on the 2022 χ_c sample shows that s keeps falling above 3 GeV:
- barrel |η| < 0.8: 0.9836 at 3–3.5 GeV → 0.978 at 9–13 GeV;
- |η| 1.5–1.8: 0.944 → 0.924 at 5–6.5 GeV.

The open bin, dominated by 3–4 GeV photons, overestimated s for the harder photons: by about 0.5% in the barrel and up to 2% in the endcaps at about 10 GeV.

**Changes:**
- **`fill_along`** (bmmg_ana/convscale.py, 5b2ed07, with test): a bin without a good fit takes the scale and stat error of the last good bin below it along one variable. The step between the last two good bins is added to its systematic error.
  - Before, such bins held the |η| slice average, which at high pt is too high: the forward 8–100 GeV bins got 0.955 / 0.943 / 0.939 in the first fine-binning test, above the measured 6–8 GeV values.
  - Config `fill_from: pt` (c333b0c); the filled bins are listed in `info.filled` of the map.
- **`r21_per` with its own binning** (67313b5). N2/N1 per map pt bin is noisy at high pt: 0.09 ± 0.06 at 13–17 GeV, 0.32 ± 0.04 at 10–13 GeV. The ratio now has bins up to 6.5 GeV and one value above; the map bins take the value at their centre.
- **Config** (e33de0e):
  - pt_est edges [1, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6.5, 9, 13, 100];
  - N2/N1 edges [1, 1.5, …, 5, 6.5, 100];
  - `min_candidates` 2000 (was 3000; fits with about 2500 candidates in the fit range are stable, stat 0.7–1.1%).

**N2/N1 in the barrel per pt_est bin:** 0.485, 0.445, 0.400, 0.360, 0.334, 0.319, 0.294, 0.276 (5–6.5 GeV), 0.257 ± 0.013 (> 6.5 GeV).

**Map** (s ± stat ⊕ syst; "filled": no good fit, taken from the last measured pt bin):

| pt_est [GeV] | \|η\| 0–0.8 | 0.8–1.5 | 1.5–1.8 | 1.8–2.0 | 2.0–2.5 |
|---|---|---|---|---|---|
| 1–1.5 | 0.9899 ± 0.0009 | 0.9849 ± 0.0009 | 0.967 ± 0.003 | 0.952 ± 0.006 | 0.948 ± 0.006 |
| 1.5–2 | 0.9875 ± 0.0009 | 0.9820 ± 0.0008 | 0.962 ± 0.004 | 0.951 ± 0.008 | 0.945 ± 0.008 |
| 2–2.5 | 0.9857 ± 0.0009 | 0.9796 ± 0.0008 | 0.959 ± 0.003 | 0.941 ± 0.004 | 0.938 ± 0.006 |
| 2.5–3 | 0.9840 ± 0.0010 | 0.9771 ± 0.0009 | 0.949 ± 0.003 | 0.934 ± 0.003 | 0.933 ± 0.009 |
| 3–3.5 | 0.9836 ± 0.0010 | 0.9752 ± 0.0010 | 0.944 ± 0.003 | 0.927 ± 0.005 | 0.929 ± 0.004 |
| 3.5–4 | 0.9825 ± 0.0010 | 0.9745 ± 0.0010 | 0.941 ± 0.004 | 0.927 ± 0.010 | 0.920 ± 0.016 |
| 4–5 | 0.9822 ± 0.0011 | 0.9735 ± 0.0010 | 0.934 ± 0.004 | 0.921 ± 0.008 | 0.916 ± 0.011 |
| 5–6.5 | 0.9808 ± 0.0011 | 0.9705 ± 0.0014 | 0.924 ± 0.007 | 0.922 ± 0.010 | 0.913 ± 0.027 |
| 6.5–9 | 0.9790 ± 0.0014 | 0.9689 ± 0.0019 | 0.924 ± 0.012 (filled) | 0.922 ± 0.010 (filled) | 0.893 ± 0.014 |
| 9–13 | 0.9779 ± 0.0020 | 0.969 ± 0.003 | 0.924 ± 0.012 (filled) | 0.922 ± 0.010 (filled) | 0.893 ± 0.025 (filled) |
| > 13 | 0.978 ± 0.002 (filled) | 0.969 ± 0.004 (filled) | 0.924 ± 0.012 (filled) | 0.922 ± 0.010 (filled) | 0.893 ± 0.025 (filled) |

- **Closure:** unchanged; inclusive 0.9991, pt_est bins 0.997–1.000, |η| bins 0.994–1.002.
- **Reach of 2022 (35 fb⁻¹):**
  - barrel to 13 GeV (9–13 GeV: about 700 / 350 signal, stat 0.2% / 0.5%);
  - endcaps to 6.5 GeV (2.0–2.5 to 9 GeV).
  - Above, the filled values assume no further pt dependence. The added extrapolation step is a crude estimate: in |η| 1.8–2.0 the last two measured bins happen to agree (step 0.0007).
- **Projection for Run 3 (about 350 fb⁻¹, ×10).** The candidate counts in the fit range give:
  - forward 6.5–9 GeV: about 25k; 9–13 GeV: about 10k; > 13 GeV: about 3.5k;
  - barrel 13–17 GeV: about 8k / 3.6k; > 17 GeV: about 3k / 1.2k.
  - So the endcaps would be measured up to about 13 GeV and the barrel to about 17–20 GeV, with a finer high-pt binning. The code takes several datasets (`datasets: [...]` in both configs).
  - Before merging years, the scale should be compared per year or era. The conversion energy comes from the tracker, but material and alignment conditions can differ.
- The first fine-binning run crashed with a segmentation fault during plotting, after the map was printed. An identical rerun was fine (an intermittent fault in the LCG_107 environment; not reproduced).

---

## 2026-11-22: Converted-photon scale: tighter forward selection reduces the endcap systematic from 1.4–4% to 0.3–0.9% (full 2022 data)

Follows the previous entry (open item: the endcap systematic, dominated by `range_wide`).

**Cause.** In the forward region the χ_c1/χ_c2 peaks merge and S/B under the peak is about 0.6 (|η| 1.8–2.5). The CB tail and the background shape are then degenerate: in the inclusive forward fit α runs to its lower limit 0.2, and s moves with the fit range and the background order.

**Cut scan** on |η| 1.8–2.5, pt_est > 1 GeV (2.1M candidates). N2/N1 is fixed to 0.43 and α is free. Columns: signal kept, S/B in ±2σ of the merged peak, and the shift of s in three model variations.

| cut (in addition) | signal | S/B | Δs range_wide | Δs bkg3 | Δs range_narrow |
|---|---|---|---|---|---|
| none | 1.00 | 0.62 | −0.021 (α at limit) | −0.019 | −0.016 |
| `phoDCA < 0.2` | 0.78 | 0.79 | +0.008 | −0.005 | −0.003 |
| `phoDCA < 0.1` | 0.71 | 0.98 | +0.008 | −0.002 | −0.002 |
| wide π⁰ veto (`conv_flags` bit 16 unset) | 0.98 | 0.67 | −0.009 | −0.012 | −0.010 |
| `phoDCAxy < 0.1` | 0.73 | 0.60 | −0.001 | −0.006 | −0.004 |
| trk–PV compatible (bit 1) | 0.57 | 0.66 | +0.005 | −0.005 | −0.004 |
| high purity (bit 4) / `conv_fitProb > 0.01` / `phoFlight > 0` / ρ cuts | 0.56–0.85 | 0.51–0.56 | −0.01 to −0.04 | ≈ −0.01 | ≈ −0.01 |

- Bit 8 (narrow π⁰ window) removes 3% of the candidates with no effect.
- One conversion per dimuon, and `evt_nPV < 30` (S/B 0.76, but only 46% of the signal), don't help enough.

**Full maps** with four forward selections, applied only for |η| > 1.5, all on top of the previous selection:
- B: `phoDCA < 0.2` + π⁰ veto.
- C: `phoDCA < 0.1` + π⁰ veto.
- D: `phoDCA < 0.1`.
- E: C applied everywhere, as a check of the barrel.

| \|η\| | pt_est | nominal (old) | B | **C (new default)** | D | E |
|---|---|---|---|---|---|---|
| 1.5–1.8 | 1–1.5 | 0.973 ± 0.006 | 0.967 ± 0.003 | **0.967 ± 0.003** | 0.969 ± 0.004 | 0.965 ± 0.004 |
| | > 3 | 0.940 ± 0.003 | 0.937 ± 0.003 | **0.938 ± 0.003** | 0.940 ± 0.003 | 0.938 ± 0.003 |
| 1.8–2.0 | 1–1.5 | 0.962 ± 0.026 | 0.953 ± 0.017 | **0.952 ± 0.006** | 0.955 ± 0.009 | 0.952 ± 0.007 |
| | 2–3 | 0.948 ± 0.025 | 0.942 ± 0.016 | **0.939 ± 0.003** | 0.940 ± 0.006 | 0.939 ± 0.003 |
| | > 3 | 0.934 ± 0.040 | 0.928 ± 0.023 | **0.924 ± 0.006** | 0.925 ± 0.008 | 0.924 ± 0.006 |
| 2.0–2.5 | 1–1.5 | 0.975 ± 0.016 | 0.957 ± 0.009 | **0.949 ± 0.006** | 0.950 ± 0.007 | 0.947 ± 0.006 |
| | 2–3 | 0.945 ± 0.019 | 0.942 ± 0.009 | **0.936 ± 0.007** | 0.936 ± 0.007 | 0.936 ± 0.007 |
| | > 3 | 0.921 ± 0.040 | 0.921 ± 0.004 | **0.921 ± 0.009** | 0.920 ± 0.012 | 0.920 ± 0.010 |

(stat ⊕ syst)

- **Forward systematic:** with C it is 0.3–0.9% (was 1.4–4%). No single variation dominates: range_wide ≤ 0.9%, the ratio variations ±0.3%, n5 ≤ 0.4%.
- **Convergence:** the forward values converge as the purity increases (C, D and E agree within 0.3%). The old values were too high by up to 2.6% (|η| 2.0–2.5, pt_est 1–1.5), more than their quoted systematic: the loose-selection fit was biased by the tail–background degeneracy.
- **Bias check (E):** the same cuts in the barrel change s by ≤ 0.0011 (the barrel systematic is 0.0007–0.0010), so the cuts don't select on the photon energy.
- **Closure with C:** inclusive 0.9991.
  - Fine |η| bins above 1.5: 0.999 / 1.001 / 1.003 / 0.997 (was 0.997 / 0.991 / 0.995 / 0.996).
  - Barrel and pt bins unchanged.
- **Statistics:** 7.8M selected candidates (was 13.0M); forward reference fits 0.26–0.31M per |η| slice. Forward statistical errors are unchanged or smaller, because of the higher purity.

**Committed:** C as the default selection in `config/conv_scale.yaml` (6d624ef). `conv_flags % 32 < 16` assumes MiniAOD flags (−1 in AOD).

---

## 2026-11-22: Converted-photon scale: N(χ_c2)/N(χ_c1) per pt bin, |η| bin 1.5–2.0 split at 1.8 (full 2022 data)

Follows the previous entry (model problems 1 and the closure in |η| 1.8–2.0).

**Changes:**
- **N2/N1 per pt bin** (commit 974f6cc). New option `r21_per: [pt]`: one fit of the barrel (`r21_from`) with a free ratio per pt_est bin. Each map bin fixes the ratio of its pt_est bin.
  - The cross-check bins along pt_est take the value at their bin centre; the other cross-checks keep the whole-barrel value.
  - The r21_low/high variations scale the per-bin values.
  - The per-bin values are stored in `info.r21_bins` of `conv_scale.json`.
- **|η| binning** (commit a5b9bf1): [0, 0.8, 1.5, 1.8, 2.0, 2.5].

**N2/N1 in the barrel, per pt_est bin** (1–1.5 / 1.5–2 / 2–3 / > 3 GeV): 0.485 / 0.445 / 0.383 / 0.309 (± 0.003–0.008). The pt-inclusive value was 0.429.

**Map** (s ± stat ± syst; all bins status 0):

| pt_est [GeV] | \|η\| 0–0.8 | 0.8–1.5 | 1.5–1.8 | 1.8–2.0 | 2.0–2.5 |
|---|---|---|---|---|---|
| 1.0–1.5 | 0.9899 ± 0.0002 ± 0.0009 | 0.9849 ± 0.0005 ± 0.0008 | 0.973 ± 0.002 ± 0.006 | 0.962 ± 0.003 ± 0.026 | 0.975 ± 0.004 ± 0.016 |
| 1.5–2.0 | 0.9875 ± 0.0002 ± 0.0009 | 0.9820 ± 0.0003 ± 0.0007 | 0.964 ± 0.001 ± 0.009 | 0.960 ± 0.002 ± 0.021 | 0.959 ± 0.003 ± 0.014 |
| 2.0–3.0 | 0.9850 ± 0.0001 ± 0.0009 | 0.9786 ± 0.0002 ± 0.0007 | 0.956 ± 0.001 ± 0.008 | 0.948 ± 0.002 ± 0.025 | 0.945 ± 0.002 ± 0.019 |
| > 3.0 | 0.9824 ± 0.0001 ± 0.0010 | 0.9738 ± 0.0003 ± 0.0008 | 0.940 ± 0.002 ± 0.002 | 0.934 ± 0.002 ± 0.040 | 0.921 ± 0.003 ± 0.040 |

- **Barrel:** changes ≤ 0.0013 with respect to the fixed ratio. In pt > 3 GeV, |η| < 0.8 the χ² falls from 1534 to 719 / 84; the remainder is the single-CB core shape.
- **Endcap pt > 3 GeV:** +0.4% (2.0–2.5: 0.917 → 0.921) and +0.9% (1.5–2.0: 0.931 → 0.940 / 0.934 after the split).
- **Closure:** inclusive 0.9988 ± 0.0001; pt_est bins 0.997–0.999; conversion radius 0.991–1.001.
  - |η| 1.8–2.0: 0.991 ± 0.002 (was 0.988).
  - The remaining deviation comes from the closure fit itself: free α and the whole-barrel ratio, against the fixed α and per-pt ratio of the map. It is well within the systematic of that bin.
- **Endcap systematic (|η| > 1.8: 1.4–4%)** is almost entirely the `range_wide` variation (y up to 6.0 GeV²): −1.4% to −4.0%. `bkg3` and `range_narrow` give about −1%.
  - In |η| 1.5–1.8 all variations stay below 0.9%.
  - **Open:** the background shape under the merged peaks in |η| > 1.8. Options: a better background model, or a cleaner forward sample (conversion quality, `phoDCA`) to reduce the background.

---

## 2026-11-22: Converted-photon scale on the full 2022 χ_c sample: fit convergence fixed; first full map; χ_c2/χ_c1 ratio depends on pt

**Run:** user's tables `/eos/user/s/sslawins/bmmg/parquet_chic` (30 files, 8.6 GB). After the selection: 13.0M candidates, 55× the development sample. Full run with variations and plots: 56 s, 8 GB RSS.

**Fixes** (commits 16bd9ca, a64cf73):
- **Crash:** `RuntimeError: the r21_from fit (abs(conv_eta) < 1.5) is not valid` in the variation `bkg3`. The nominal inclusive fit, range_narrow and n15 were also invalid.
  - **Cause:** every failing fit had Migrad stop above its EDM target (edm 0.003–0.015, target 2·10⁻⁴). No parameter was at a limit; the covariance was accurate. With −2 ln L ≈ 10⁸ in absolute value the numerical derivatives lose precision.
  - **Fix:** the likelihood is now −2 ln(L/L_sat) (Baker–Cousins: same minimum and errors, of the order of χ²), and Migrad is called again when it stops above the target. All 18 inclusive and barrel fits (nominal and 8 variations) are now valid, with the same values.
  - A variation whose r21 reference fit fails now prints a warning and is skipped instead of stopping the run.
- **Closure fit:** from the generic start values (s = 0.97, α = 1) it fell into a false minimum (α = 10 at its limit, Hesse failed). It now starts from the inclusive fit with s = 1.

**Map** (s ± stat ± syst; all bins status 0):

| pt_est [GeV] | \|η\| 0–0.8 | 0.8–1.5 | 1.5–2.0 | 2.0–2.5 |
|---|---|---|---|---|
| 1.0–1.5 | 0.9900 ± 0.0002 ± 0.0010 | 0.9852 ± 0.0005 ± 0.0009 | 0.968 ± 0.002 ± 0.012 | 0.976 ± 0.004 ± 0.017 |
| 1.5–2.0 | 0.9876 ± 0.0002 ± 0.0009 | 0.9821 ± 0.0003 ± 0.0008 | 0.961 ± 0.001 ± 0.008 | 0.960 ± 0.003 ± 0.015 |
| 2.0–3.0 | 0.9849 ± 0.0001 ± 0.0009 | 0.9783 ± 0.0002 ± 0.0007 | 0.950 ± 0.001 ± 0.011 | 0.944 ± 0.002 ± 0.021 |
| > 3.0 | 0.9820 ± 0.0002 ± 0.0010 | 0.9725 ± 0.0003 ± 0.0009 | 0.931 ± 0.001 ± 0.021 | 0.917 ± 0.003 ± 0.044 |

- **Inclusive:** s = 0.9835 ± 0.0002, consistent with the development value 0.9830 ± 0.0011.
- **Barrel:** stable across all variations (systematic ≤ 0.1%).
- **Endcaps:** the systematic is dominated by `range_wide` and `bkg3` (1–4%). The χ_c peaks merge (σ/y = 4–7%), so s is degenerate with the background shape under the peak.
- **Closure:** after the bin-value correction, inclusive s = 0.9990 ± 0.0001.
  - pt_est bins: 0.996–1.000.
  - Fine |η| bins: 0.988–1.002. The worst is |η| 1.8–2.0 at 0.988: the 1.5–2.0 map bin averages over s = 0.957 (1.5–1.8) and 0.941 (1.8–2.0), so finer |η| bins are needed.
  - Conversion radius: 0.991–1.001.

**Model problems that appear only at this statistics:**
1. **N(χ_c2)/N(χ_c1) depends on pt.** Free in the barrel fits it is 0.49 / 0.46 / 0.39 / 0.31 in the four pt_est bins (|η| < 0.8; similar in 0.8–1.5). The fit fixes it everywhere to the pt-inclusive barrel value of 0.43, which is outside the ±20% variation at both ends.
   - The likely reasons are the production ratio σ(χ_c2)/σ(χ_c1) changing with pt, and pt_est = 0.895·pt_true for χ_c2.
   - **Barrel:** freeing it changes s by ≤ 0.0014.
   - **Endcaps:** the merged peak moves with the ratio. 0.43 instead of 0.31 lowers s by about 0.7% at pt > 3 GeV.
   - **Proposed:** take N2/N1 from the barrel per pt_est bin (`r21_from` per bin of the pt variable).
2. **The barrel peak shape is not a single Crystal Ball** at this statistics. In pt > 3 GeV, |η| < 0.8 the χ² is 1534/84, with ±10σ pulls at the χ_c1 top and on the χ_c2 peak. Part of this is the fixed N2/N1: freeing it gives χ²/ndf = 701/82. The rest is a core that is narrower with wider tails (the resolution varies within the bin). The effect on s is ≤ 0.1% (the variation spread).
3. **Cross-check bins below the map** (pt_est 0.6–0.85 GeV): fits invalid, as expected from the conversion threshold. They are not part of the map.

---

## 2026-11-21: Converted-photon energy scale from χ_c → J/ψ γ(conv) on the Parquet tables (`analysis/`); development on 100 files

Implements plan steps 2–3 of the 2026-11-21 χ_c / π⁰ entries. The map is meant to go into the ntuplizer later.

**Code** (`analysis/`, README section "Converted-photon energy scale"):
- `convert.py` knows the channel `chic`; `dimu_*` and `conv_*` are gathered into each row.
  - `config/convert_chic.yaml`: separate output `~/bmmg/parquet_chic`, preselection `q < 0.9`.
- `bmmg_ana/convscale.py`: fit model, fit (iminuit, added to `environment.yml`), `pt_est`, binning, `ScaleMap` (JSON, lookup), `corrected_mass`; MC truth helpers.
- `scripts/conv_scale.py` with `config/conv_scale.yaml`: the inclusive fit, reference fits per |η| slice, the map fits, model variations, 1D cross-checks, closure, JSON/CSV/plots.
- `scripts/conv_scale_mc.py` with `config/convert_convmc.yaml` / `config/conv_scale_mc.yaml`: the MC truth check on gen-matched `mmcg` of B_s → J/ψγ.
- `tests/test_convscale.py` (toy closure, fixed parameters, `pt_est`, `ScaleMap`) and a `chic` test in `tests/test_schema.py`. All 29 tests pass.

**Method:**
- **Fit variable:** y = m(μμγ)² − m(μμ)² = 2 p_μμ·p_γ, linear in the photon energy.
  - The χ_c1/χ_c2 peaks sit at s·(m_χc² − m_J/ψ²).
  - With the PDG J/ψ mass instead of m(μμ), i.e. Q(Q + 2m_J/ψ), the scale and the width are the same (0.983, σ/y = 0.028).
- **Model:** Crystal Ball peaks with the tail on the low side, plus a Chebyshev background (2nd order), in y ∈ [1.6, 5.2] GeV² (Q ≈ 0.25–0.75 GeV). Binned extended likelihood. The scale s is the CB mean, i.e. the peak.
- **Choices forced by the development data:**
  1. **CB n fixed to 2:** free, it runs to its lower limit (degenerate with the background). Inclusive s for n = 1.5 / 2 / 3 / 5 / 10: 0.9824 / 0.9830 / 0.9837 / 0.9843 / 0.9848. The variations use n = 1.5 and 5.
  2. **Binning in `pt_est` = conv_pt · Y₁ / y, not in the reconstructed pt.**
     - For a χ_c1 candidate, `pt_est` is the true photon pt, independent of the response, because y and conv_pt both scale with it.
     - Binning in conv_pt selects on the response. Migration across the edges and the 0.5 GeV conversion threshold give s = 1.02–1.03 at 0.5–0.7 GeV.
     - Below `pt_est` ≈ 1 GeV the threshold (y > 0.5 GeV · Y₁ / pt_est) cuts into the fit range, so the map starts at 1 GeV.
  3. **The tail α fixed per |η| slice** (`shape_from: [abseta]`), not to the inclusive value.
     - α is 0.97 / 0.90 / 0.55 for |η| 0–0.8 / 0.8–1.5 / 1.5–2.0.
     - With the inclusive α (0.57), the barrel bins came out 0.5% high, and the merged barrel sample (0.985) was below every one of its pt bins (0.987–0.992).
  4. **N(χ_c2)/N(χ_c1) from the barrel** (`r21_from: abs(conv_eta) < 1.5`, 0.396 ± 0.018), fixed everywhere.
     - In the endcaps σ/y is 5–10% and the peaks merge. A free ratio trades χ_c2 for χ_c1: s = 0.885 with N2/N1 = 3 is about s = 0.99 with χ_c1.
     - Variations: ±20% on the ratio.
- **Selection:** `conv_fitOK == 1`, `phoDCA < 0.5` cm, |m(μμ) − m_J/ψ| < 0.1 GeV, `dimu_vtxProb > 0.01`.
- **Systematics:** per bin, the largest deviation over the variations: background order 3, χ_c0 included, fit range 2.0–4.6 / 1.4–6.0 GeV², n = 1.5 / 5, ratio ±20%.

**Development sample:** 100 files of `ParkingDoubleMuonLowMass0` 2022C v2 (about 44M processed events), from EOS.
- Conversion: 24 s → 784k `chic` rows. The row count equals the raw `chic_q < 0.9` count, and 200 random candidates have the right `dimu_*`/`conv_*` values.
- 235k candidates selected. Inclusive fit: s = 0.9830 ± 0.0011, σ/y = 2.8%, N(χ_c1) = 13.1k, χ²/ndf = 87/82.
  - The earlier estimate from the 85k-event 2022C test file was 0.969 (+0.014 −0.016).
- All fits and plots take 9 s.

**Map** (s ± stat ± syst; `pt_est` [GeV] × |η|):

| pt_est | 0–0.8 | 0.8–1.5 | 1.5–2.0 | 2.0–2.5 |
|---|---|---|---|---|
| 1.0–1.5 | 0.988 ± 0.002 ± 0.001 | 0.982 ± 0.004 ± 0.001 | 0.959 ± 0.010 ± 0.008 | 0.93 ± 0.03 ± 0.05 |
| 1.5–2.0 | 0.987 ± 0.001 ± 0.001 | 0.979 ± 0.003 ± 0.001 | 0.970 ± 0.009 ± 0.004 | 0.93 ± 0.03 ± 0.06 |
| 2.0–3.0 | 0.985 ± 0.001 ± 0.001 | 0.981 ± 0.002 ± 0.001 | 0.953 ± 0.005 ± 0.005 | 0.92 ± 0.02 ± 0.01 |
| > 3.0 | 0.982 ± 0.001 ± 0.002 | 0.974 ± 0.002 ± 0.001 | 0.947 ± 0.006 ± 0.008 | status 2 (too few; inclusive value filled in) |

- The scale falls slowly with pt (barrel 0.988 → 0.982) and strongly with |η|: 1–2% in the barrel, 3–5% at 1.5–2.0, 7–8% above 2.0. This is the trend of the AN-2023/035 π⁰ map.
- **Conversion radius** (1D, free tail): 0.985 / 0.986 / 0.981 / 0.970 / 0.981 at ρ = 1.5–4 / 4–7 / 7–12 / 12–25 / 25–100 cm. After the pt × |η| correction this closes (1.002, 0.999, 0.996, –, 1.001), so the ρ dependence is mostly the |η| correlation.

**Closure** (the map applied per candidate with its bin value, refit in the finer cross-check bins, candidates inside the map):
- pt_est: 0.996–1.001. Reconstructed pt: 0.993–1.002. |η| < 1.2: 0.998–1.001.
- |η| 1.2–2.5: 0.994 / 1.010 / 0.993 / 0.977 / 1.05, within about 2σ of the 0.3–2.4% errors.
- The inclusive refit gives 0.995: one CB fitted to a mix of bins with different shapes. It is not a closure test.
- Multilinear interpolation between bin centres is available (`interpolate: true`). With the steep |η| dependence and coarse bins it is −0.4% off in closure, so bin values are the default.

**MC check** (`conv_scale_mc.py`, B_s → J/ψγ MiniAOD test file, 35k events, `channels=mmcg`):
- Only 952 truth-matched conversions, all with photon pt > 3 GeV.
- Inclusive peak of E_reco/E_true: 0.965 ± 0.003. The peak of y/(m_Bs² − m_J/ψ²) is 0.958 ± 0.004: the y method agrees with the photon response within 1.7σ.
- Per bin (≥ 100 conversions) only two bins can be fitted, so no data/MC comparison is possible yet. The script prints none where the MC has no fit.
- **Needed:** the B_s → J/ψγ MC production (`tasks_remaining_mc.yaml`) converted with `config/convert_convmc.yaml`.

**For the full 2022 data:**
- 744 files, 7.4 times this sample: statistical errors about 2.7 times smaller.
- The |η| 2.0–2.5 slice should then fit, together with the bin at pt > 3 GeV.
- Finer pt bins in the barrel will be possible, and ρ could become a third map variable.
- Run with `python3 convert.py config/convert_chic.yaml`, then `python3 scripts/conv_scale.py config/conv_scale.yaml`.

**Open:**
1. The data/MC comparison and the MC map. The map is needed for MC too, from its own truth.
2. The pt range: χ_c gives 1–5 GeV, the signal photons are at 3–40 GeV. The MC pt trend beyond 5 GeV decides how to extrapolate; the lookup is clamped at the last bin.
3. Ntuplizer implementation: read the JSON (FileInPath), scale the conversion momentum by 1/s before the `mmcg` fit, store the applied s.

## 2026-11-21: Signal photon energy response E_reco/E_gen: the excess comes from the reconstruction threshold and from other particles in the supercluster; the muon ECAL deposits are a small effect (study; no code change)

**Question:** reconstructed signal photon energies are on average above the generated ones, more in EE, also at high
energy. Is the cause the energy that the muons leave in the ECAL?

**Method:**
- **Sample:** the filtered B_s→μμγ AOD MC (`bsmmg_aod_filtered`, 114 files, 242.7k events), read in FWLite.
- **Gen photon:** the leading photon daughter of the B with two muon daughters.
- **Matching:** the `gedPhotons` photon closest in ΔR, with ΔR < 0.1. There is no cut on the pt ratio, so the
  matching does not bias the response. 146.7k photons are matched.
- **Response:** R = `energy()` / E_gen. `energy()` is the regression-corrected energy, i.e. `pho_energy` in the ntuple.
- **Muon deposit:** each gen muon is matched to a reco muon (ΔR < 0.05, same charge).
  - The muon's crossed crystal `calEnergy().ecal_id` is looked up in the SC `hitsAndFractions()`.
  - The deposit is estimated from that crystal's rechit energy × fraction, and from `calEnergy().em`.
  - These estimates also contain the photon's own shower energy in that crystal, so they are upper bounds. A MIP
    leaves about 0.25–0.3 GeV in one crystal.
- **Other particles:** the gen energy of the other stable photons, electrons and neutral hadrons (n, K⁰_L) in a
  window |Δη| < 0.05, |Δφ| < 0.3 around the gen photon (`othW`).
  - These particles come from the same pp interaction: B fragmentation and underlying event.
  - Pileup is not in `genParticles`.
  - B daughters other than the leading photon: none in this sample (no FSR photons among the B daughters).

**1. Reconstruction threshold (dominant at low pt).** The `gedPhotons` collection effectively starts at E_T ≈ 10 GeV.
Near the threshold only the photons that fluctuated up are reconstructed:

| gen pt [GeV] | reco efficiency | EB median R | EE median R | EE mean R |
|---|---|---|---|---|
| 8–10 | 0.48 | 1.032 | 1.081 | 1.112 |
| 10–12 | 0.89 | 1.005 | 1.042 | 1.065 |
| 12–15 | 0.96 | 0.999 | 1.021 | 1.032 |
| 15–20 | 0.98 | 1.000 | 1.015 | 1.018 |
| 20–30 | 0.98 | 1.001 | 1.013 | 1.018 |
| > 30 | 0.98 | 1.007 | 1.015 | 1.031 |

- The bias reaches higher pt in EE because the resolution there is about twice as wide (σ_eff 0.08–0.13 vs 0.04–0.05).
- Removing the photons with muon deposits or other particles hardly changes the bias (EE at 8–10 GeV: 1.081 → 1.077).
  It is a selection effect.

**2. Above the threshold (gen pt > 15 GeV).** EB is unbiased: median 1.001, mean 1.000. EE is high: median 1.014,
mean 1.020, with 5.7% of the photons at R > 1.2.

| gen pt > 15 | EB median | EB mean | EE median | EE mean |
|---|---|---|---|---|
| all | 1.001 | 1.000 | 1.014 | 1.020 |
| muon crystal in the SC (3.3% EB / 4.8% EE) | 1.021 | 1.059 | 1.036 | 1.069 |
| other gen e/γ/n⁰ > 2 GeV in the window (9.6% / 21%) | 1.019 | 1.066 | 1.035 | 1.058 |
| neither (other < 0.5 GeV, no muon crystal) | 0.999 | 0.990 | 1.009 | 1.009 |
| E / (E_gen + other gen E in the window) | 0.991 | 0.978 | 1.000 | 0.999 |

- **Muons:**
  - Only 3–5% of the signal photons have a muon crystal in their SC.
  - For those photons, with no other particles nearby, the median R rises by 1.2% (EB, 0.999 → 1.011) and
    1.6% (EE, 1.009 → 1.024).
  - Contribution to the mean R: **+0.2% (EB), +0.3% (EE)**.
  - The rechit estimate of the deposit (median 0.6 GeV EB, 3.3 GeV EE) is dominated by the photon shower in the same
    crystal. Subtracting it overcorrects (EE median of these photons: 0.979), so it is not usable as a correction.
- **Other particles in the SC window:**
  - 28% (EB) / 36% (EE) of the photons have more than 0.5 GeV of other gen energy in the window. This is mostly soft
    photons from π⁰ of the B fragmentation and the underlying event.
  - Contribution to the mean R: **+0.9% (EB), +1.0% (EE)**.
  - With this energy added to E_gen, the EE response closes: median 1.000, mean 0.999.
  - The forward region has more of it, because the same E_T means a larger E: on average 1.6 / 2.0 / 2.9 GeV at
    |η| 1.5–2.0 / 2.0–2.2 / 2.2–2.5.
- **Response vs muon distance:** R rises as the muon gets closer to the SC (EE median 1.008 at ΔR > 0.2, 1.032 at
  ΔR < 0.05). Most of this rise is not the muon:
  - Close muons mean a boosted B, with more fragmentation particles around the photon. The fraction with other gen
    energy > 0.5 GeV goes from 0.15–0.30 to 0.42–0.48.
  - Without other particles and muon crystals, the rise is +0.5% (EB) and +1.5% (EE) between ΔR 0.2–0.4 and < 0.05.
    This residual includes muon deposits next to the crystal looked up, and charged particles, which the window does
    not count.
- **Residual in EE without muons and other particles:** median 0.998 / 1.013 / 1.035 at |η| 1.48–2.0 / 2.0–2.2 /
  2.2–2.5.
  - It grows slightly with pileup: 1.006 at nPU < 30, 1.011 at nPU > 40.
  - It is probably pileup and charged hadrons (not in the window), plus possibly regression non-closure at |η| > 2.
    This sample cannot separate them.
  - EB shows no pileup dependence (0.9985–0.9992).
- **Converted photons** (`hasConversionTracks`): EE median 0.996 vs 1.018 for unconverted.

**Conclusions:**
- The muon ECAL deposits are a small part of the excess (+0.2–0.3% on the mean).
- The excess comes from:
  1. the reconstruction threshold at E_T ≈ 10 GeV: +3–8% for gen pt < 12 GeV, mostly in EE;
  2. overlap with other particles of the same collision: about +1%;
  3. in EE at |η| > 2, a further +1–3.5%, probably pileup.
- Points 2 and 3 are physical extra energy, not a bias in the regression for isolated photons. A correction would
  have to be candidate-dependent, e.g. using the neutral isolation.
- For the energy calibration and the kinematic fit:
  - the response should be measured in bins of gen (not reco) energy, above ~15 GeV;
  - the EE tails (R > 1.2: 5–6%) argue for |η| < 2.0 or a non-Gaussian error model.

Scripts (not committed): `escale.py` (FWLite, AOD), `ana*.py` in the session scratchpad.

## 2026-11-21: Ntuplizer: photon MVA without the candidate's muons (`<p><leg>MvaNoMu`, `<p><leg>TightIdNoMu`)

Implementation of the study below.

**Code:**
- New class `PhotonMVANoMuons` (`interface/PhotonMVANoMuons.h`, `src/PhotonMVANoMuons.cc`):
  - Loads the official weight files `RunIII_Winter22/PhoMVA_ID_{EB,EE}_V1.weights.root` with `createGBRForest`.
    The variable names are read from the files; an unknown name throws.
  - Rebuilds the inputs from the `reco::Photon`, removes the terms of the given muons, and returns `tanh(response)`.
  - Muon terms: inner-track pt in the two track isolations, with the official `PhotonTkIsolation` selection (exact);
    HCAL deposit at the muon's HCAL impact point in `hcalPFClusterIso` and `hcalOverEcal` (approximate).
- The plugin calls it after building the `mmg` and `mmgg` candidates, with the candidate's μ1 and μ2. New inputs:
  - `rho` (`fixedGridRhoAll`);
  - `photonMVANoMuons` (default true);
  - the two weight files and the two wp90 cuts.
- **Branches:**
  - `mmg_phoMvaNoMu`, `mmg_phoTightIdNoMu`;
  - `mmgg_pho1MvaNoMu`, `mmgg_pho1TightIdNoMu`, `mmgg_pho2MvaNoMu`, `mmgg_pho2TightIdNoMu`.

**Validation:** 5000 events each of the test files `bsmmg_miniaod`, `bsmmg_aod` and `data_miniaod`, no skim.
- **Closure:**
  - AOD: for photons with ΔR > 0.7 to both muons (119 candidates), `MvaNoMu` equals `pho_mva` to 6e-8.
  - Candidates with nothing to remove are bit-identical to `pho_mva`: 30% (MiniAOD MC), 42% (AOD), 45% (data).
- **Direction of the change:** for about 4% of the candidates the value decreases slightly (median −0.015). The
  MVA is not monotonic in its isolation inputs.
- **Gen-matched `mmg` candidates, wp90 efficiency:**

  | sample | official | `MvaNoMu` |
  |---|---|---|
  | MiniAOD | 0.590 | 0.759 |
  | AOD | 0.588 | 0.788 |

  - By ΔR(γ, μ), MiniAOD: < 0.2: 0.445 → 0.684; 0.2–0.3: 0.595 → 0.784; 0.3–0.4: 0.797 → 0.835; > 0.4 unchanged.
  - Non-matched candidates in MC are unchanged.
- **Data:** the data test file has only 20 `mmg` candidates, none passing either version. The effect on the
  background has to be measured on the B-mass sidebands of a production with the new branches.

## 2026-11-21: Photon MVA re-evaluated with the muons removed from its inputs: signal wp90 efficiency 0.60 → 0.77 (study; no code change)

Follow-up of the entry below.

**Method:**
- The Winter22 V1 MVA is recomputed in FWLite from the official weight files
  (`RunIII_Winter22/PhoMVA_ID_{EB,EE}_V1.weights.root`) and the 18 (EE) or 15 (EB) inputs of each `slimmedPhotons`
  photon. The output is `tanh(GBRForest response)`.
- **Closure:** the recomputed value equals the stored `PhotonMVAEstimatorRunIIIWinter22v1Values` to 5e-8.
- **Muon terms removed** (all `slimmedMuons` of the event):
  - **Track isolations:** the inner-track pt of each muon is subtracted with the official `PhotonTkIsolation`
    rules. `trkSumPtSolidConeDR04` uses ΔR < 0.4; `trkSumPtHollowConeDR03` uses 0.04 ≤ ΔR < 0.3. Both require
    |Δη| ≥ 0.015, |v_z − z_PV| < 0.2 cm and |d_xy(BS)| < 0.1 cm. The subtraction is exact: no photon gets a negative
    residual, in MC or in data.
  - **HCAL:** `calEnergy().had` of each muon is subtracted when its HCAL impact point (`calEnergy().hcal_position`)
    is within ΔR < 0.3 of the SC (E_T, `hcalPFClusterIso`, EE only) or within 0.15 (`hcalOverEcal`). This is an
    approximation: 7–9% of the signal photons would get a negative residual, which is set to 0.
- **Samples:** the MiniAOD signal test file (32.9k events, 6043 gen-matched signal photons) and the 2022C MiniAOD
  data test file (6.4k events, 604 photons).

**Signal photon wp90 efficiency:**

| | n | official | − muon tracks | − muon tracks and HCAL |
|---|---|---|---|---|
| all signal | 6043 | 0.595 | 0.755 | 0.772 |
| ΔR(γ, μ) < 0.2 | 2658 | 0.459 | 0.694 | 0.728 |
| 0.2–0.3 | 1716 | 0.594 | 0.776 | 0.784 |
| 0.3–0.4 | 1004 | 0.801 | 0.832 | 0.834 |
| > 0.4 | 651 | 0.837 | 0.837 | 0.837 |
| pt 10–15 GeV | 3543 | 0.685 | 0.820 | 0.829 |
| pt 15–20 GeV | 1383 | 0.534 | 0.716 | 0.736 |
| pt 20–30 GeV | 806 | 0.424 | 0.644 | 0.675 |
| pt > 30 GeV | 311 | 0.280 | 0.479 | 0.543 |
| EB / EE | | 0.544 / 0.701 | 0.755 / 0.754 | 0.773 / 0.772 |

- **Input medians of the signal photons, before → after:**
  - `trkSumPtSolidConeDR04` 6.67 → 1.47 GeV
  - `trkSumPtHollowConeDR03` 3.33 → 0.64 GeV
  - `hcalPFClusterIso` 3.59 → 1.81 GeV
  - H/E 0.054 → 0.026

  For comparison, the signal photons with ΔR(μ) > 0.4 have 1.16 / 0.43 / 2.41 GeV / 0.042.
- **Median signal MVA:** 0.20 → 0.62.
- **Remaining gap:** photons at ΔR < 0.2 and at high pt stay below the isolated photons.
  - Probable causes: muon energy inside the ECAL cluster (shower shapes, `ecalPFClusterIso`), and soft muons that PF
    reconstructs as charged hadrons (`chargedHadronIso`). Neither is corrected here.
  - Even the isolated signal photons reach only 0.84 rather than 0.90.

**Background proxy:**
- **Data photons:** 0.127 → 0.132 overall. Near a muon (ΔR < 0.4): 0.051 → 0.077, but only 117 photons (6 → 9).
  Away from the muons: unchanged.
- **Non-signal MC photons:** 0.162 → 0.163.
- The effect on the background of the analysis has to be measured with more data, e.g. the photons of `mmg`
  candidates in the B-mass sidebands.

**For the ntuplizer:**
- The muons to remove are the candidate's μ1 and μ2. The variable is therefore candidate-level, like the existing
  candidate isolations.
- Everything needed exists in both AOD and MiniAOD: muon inner tracks, `calEnergy()`, the photon inputs and the
  GBRForest.

## 2026-11-21: Photon MVA ID wp90 efficiency of the signal photons is 57%, not 90%: the signal muons are in the isolation cones (study; no code change)

**Question:** `pho_tightId` (`mvaPhoID-RunIIIWinter22-v1-wp90`) is documented as 90% efficient. For gen-matched
signal photons of the B_s → μμγ MC (`v1` production, MiniAOD, 129k photons) it is 57%. Does the ntuple assign
wrong IDs?

**The ID in the ntuple is correct:**
- `pho_tightId == (pho_mva > cut)` for all 144k photons, with cut 0.0440 (EB) / −0.2495 (EE); the EB/EE category
  agrees with |η_SC| < 1.479.
- 5000 events of the MiniAOD test file were re-run without the skim. For each of the 1248 photons, our recomputed MVA
  equals the official `slimmedPhotons` value `PhotonMVAEstimatorRunIIIWinter22v1Values` exactly (max difference 0).
  `pho_tightId` equals the embedded `mvaPhoID-RunIIIWinter22-v1-wp90` for 100% of the photons.

**The efficiency loss comes from the signal muons inside the isolation cones of the photon:**
- The MVA inputs include `trkSumPtSolidConeDR04` and `trkSumPtHollowConeDR03`, which sum all tracks, muons included.
  They also include `hcalPFClusterIso` and `hcalOverEcal`, which pick up the muon's energy deposit in the HCAL.
- 93.5% of the signal photons are within ΔR < 0.4 of a signal muon. All signal photons have pt > 10 GeV, so the
  pt range of the 90% target is not the issue.

| signal photons | n | wp90 efficiency |
|---|---|---|
| all | 129446 | 0.569 |
| ΔR(γ, nearest μ) < 0.2 | 59630 | 0.441 |
| 0.2–0.3 | 39586 | 0.565 |
| 0.3–0.4 | 21806 | 0.811 |
| 0.4–0.6 | 8355 | 0.868 |
| ΔR > 0.6, R9 > 0.94 | 41 | 0.90 ± 0.05 |
| pt 10–15 / 15–20 / 20–30 / > 30 GeV | | 0.67 / 0.50 / 0.42 / 0.33 |

The efficiency falls with photon pt because harder B decays are more collimated. Medians of the MVA inputs on the
MiniAOD test file:

| signal photons | `trkSumPtSolidConeDR04` | `trkSumPtHollowConeDR03` | `hcalPFClusterIso` | H/E |
|---|---|---|---|---|
| ΔR(μ) < 0.3 | 11.1 GeV | 7.6 GeV | 3.9 GeV | 0.067 |
| ΔR(μ) > 0.4 | 1.0 GeV | 0.4 GeV | 2.3 GeV | 0.027 |

The scalar sum of the muon pt in the cone is 15 GeV (ΔR < 0.4); the track isolations are dominated by the muons.

**Consequences:**
- `pho_tightId` must not be used as a selection for the signal photon. It removes about 43% of the signal, more at
  high photon pt.
- `pho_mva` (a BDT feature) carries the same muon dependence. It acts partly as a measure of the γ–μ opening angle,
  not as a pure photon-quality variable.
- **Options:**
  1. Cut on `pho_mva` at a threshold tuned on signal vs background.
  2. Re-evaluate the Winter22 MVA with the signal-muon contributions removed from the inputs: muon pt out of the
     track isolations, muon deposit out of the HCAL terms.
  3. Use the photon shower-shape variables directly in the analysis BDT.

## 2026-11-21: 2022 data production `v2` is empty: `channels` list passed as `str(list)` (fixed)

The production from `crab/tasks_2022.yaml` (48 tasks, 2022C–G, `bmmg_ntuples_v2`) has
`nEventsStored = 0` in every file checked. In a sample of 40 of 1689 files, 58.9 M events were
processed and none were stored. The files are 12 kB and contain only the 125 event-level branches.

- **Cause:** the task file gives `channels: [mmg, mmcg, mmgg, chic]` as a YAML list.
  `submitCrab.pyCfgParams` formatted it with `%s`, which produced `channels=['mmg', 'mmcg', 'mmgg', 'chic']`.
  `runNtuplizer_cfg.py` split this on commas into `['mmg'`, ` 'mmcg'`, … . None of these matched a channel
  name, so every channel was off and `skim=1` dropped every event. Nothing failed.
- **Fix:**
  - `submitCrab.py` joins list values with commas, e.g. `channels=mmg,mmcg,mmgg,chic`.
  - `runNtuplizer_cfg.py` raises an error for an unknown or empty channel list instead of silently
    disabling channels.
- **Check:** `edmConfigDump` with `channels=mmg,mmcg,mmgg,chic` enables exactly those four channels.
  The old string now raises a `ValueError`.
- **Action:** the `v2` tasks must be killed and resubmitted. The output and the CRAB project directories
  `crab_*_v2` must first be removed, or a new `tag` used.

## 2026-11-21: χ_c → J/ψ γ(conversion) calibration channel (`chic_`)

- **Purpose:** data sample for the energy scale of the converted photons (plan step 3 of the entry below), with photons closer to the signal p_T than the π⁰.
- **Change:**
  - `bmmg::buildChiC` (`CandidateBuilders`), `ChiCCandidate` (`Candidates.h`), `NtupleWriter::fillChiC`, block `chic_` (NTUPLE.md).
  - Every J/ψ dimuon (`dimu_resonance == 1`, valid vertex) × every conversion, Q = m(μμγ) − m(μμ) < `chicMaxQ` (1 GeV). No kinematic fit: dimuon momentum + `SelectedConversion::p4`, the photon that enters `mmcg`.
  - Photon pointing to the dimuon vertex: `chic_phoDCA`, `chic_phoDCAxy`, `chic_phoFlight`.
  - Parameters `doChiC` (cfi default false), `chicMaxQ`, `chicInSkim` (the skim keeps events with a χ_c candidate, default true).
  - The conversions are selected and stored if `doMuMuConvGamma` or `doChiC`.
  - `runNtuplizer_cfg.py`: channel `chic`, now in the default `channels=mmg,mmcg,mmgg,mmkk,chic`.
- **Regression** (`data_aod`, 5721 events, default channels without vs with `chic`): all object branches (`mu_`, `pho_`, `dimu_`, `conv_`, …) identical, plus the 9 `chic_` branches. The only differences are in fit branches of `mmcg` (82 events), `mmkk` (13) and `mmg` (3), with `vtxOnlyChi2` differing as well, i.e. the known run-to-run non-determinism of the fits.
- **Data test:** `ParkingDoubleMuonLowMass0_Run2022C-10Dec2022-v2`, `channels=chic`:
  - MiniAOD (84915 events, 28080 with a J/ψ): 1882 candidates (2.2% of the events). AOD (12811 events): 431.
  - Without a cut the χ_c peak is barely visible. With `chic_phoDCA < 0.5` cm (MiniAOD: 600 candidates), 0.38 < Q < 0.48 GeV has 108 candidates against 50 in 0.52–0.62 GeV.
  - **Fit** (MiniAOD, phoDCA < 0.5 cm, 0.25 < Q < 0.75 GeV): χ_c1 and χ_c2 Gaussians with a common width, positions from the PDG masses scaled by the photon energy scale s (m² − m_J/ψ² ∝ E_γ), linear background. Result: s = 0.969 (+0.014 −0.016), σ = 15.5 MeV, N(χ_c1) ≈ 30, N(χ_c2) ≈ 4. This agrees with the MC core scale (0.98) and with the size of the AN-2023/035 π⁰ corrections. AOD: too few (≈ 11 χ_c).
  - Rate: ≈ 4·10⁻⁴ fitted χ_c per event after the DCA cut, i.e. ~10³ χ_c per 2.5M parking events. A map in a few (p_T, |η|) bins at the 1% level needs O(10⁸) events, a fraction of the 2022 parking data.
- **Next:** χ_c from the full production; fit in (p_T, |η|) bins of the photon (m(μμγ) with the per-photon scale, χ_c1 + χ_c2 shapes from MC or double Crystal Ball); scale map; apply to `mmcg` (`mmcg_massCorr`); χ_c MC (or Bs → J/ψ γ-like samples) to compare data and MC scale.

## 2026-11-21: Converted photon energy scale: the π⁰ → γγ calibration of AN-2023/035 (B* → Bγ); plan

- **AN-2023/035 v13, App. B** (`src/AN2023_035_v13.pdf`): the converted-photon energy scale (PES) is calibrated in data with π⁰ → γγ, both photons converted.
  - **Reconstruction:** each e⁺e⁻ pair refitted to a common vertex with zero mass; the two photons fitted to a common vertex, probability > 1%; PV = smallest sum of squared distances to the extrapolated photon trajectories; photons refitted to the PV.
  - **Fit:** unbinned fits of m(γγ), double Crystal Ball (common mean, n and α; f = 0.75, n = 20, σ₂/σ₁ = 2.5 fixed unless the yield is large) + 2nd-order polynomial.
  - **Binning:** 31 × 17 = 527 bins of the average p_T (0.3–17.2 GeV) and |η| (0–2.2) of the two photons. About 3.2M π⁰ in all 2018 AOD data sets, Σχ²/ndf = 56686/57105.
  - **Correction:** s = 134.977 MeV / m_fit per bin (m ∝ √(E₁E₂), so m/m_PDG is the scale). It is interpolated per photon in (p_T, |η|) with `TGraph2DErrors::Interpolate` and applied only in invariant masses, (P_B + s·P_γ).M(); the stored photon p_T is not changed.
  - **Size (2018):** m/m_PDG for |η| < 0.2 goes from 0.998 (0.3 GeV) through 0.992 (1.2 GeV), 0.981 (5 GeV) and 0.976 (10 GeV) to 0.971 (17 GeV); for |η| = 0.8–1.0 from 0.993 to 0.958; for |η| > 1.5 from 0.97 to 0.86. No φ dependence.
  - **Validation:** corrected π⁰ mass within 1% for |η| < 1.4 (worse at 1.4–1.6). The data correction was applied to MC (too little MC for its own calibration); afterwards E_reco/E_gen in MC is consistent with 1.
- **Comparison with our signal photons** (colinearity fit, `Study/ConvFitStudy`, B_s → μμγ MC): the core of E_reco/E_gen (Gaussian fit around the mode) is 0.983 (AOD) / 0.981 (MiniAOD), width 3–4%, about 0.98–0.99 in each p_T bin from 3 to 40 GeV. This agrees with the size of the AN's π⁰ scale at these p_T. The median (0.93) and the mean (0.89–0.92) are lower because of the bremsstrahlung tail, which a scale calibration (the π⁰ peak position) does not correct.
- **Plan (not started):**
  1. A calibration channel in the ntuplizer: pairs of conversions (both from the conversion collection of the format), common-vertex fit, m(γγ) < 0.3 GeV (π⁰, and η up to 0.6 GeV as a cross-check), storing m, the p_T/η of both photons, the vertex probability and the PV. Run on the full 2022 ParkingDoubleMuonLowMass data (the AN needed O(10⁶) π⁰; our 6k-event test files are far too small: no visible peak with a naive pairing).
  2. Fits in (p_T, |η|) bins as in the AN; the scale map as a ROOT/JSON file; per-photon interpolation, applied to the `mmcg` mass, e.g. as `mmcg_massCorr`. The same map is used in MC, after checking it against E_reco/E_gen in MC.
  3. A second candle closer to our photon p_T (3–10 GeV, where the π⁰ statistics are smaller): χ_c1,2 → J/ψγ with a converted photon (`mmcg` with a J/ψ dimuon and a lower pre-fit mass window). Q = m(μμγ) − m(μμ) peaks at 414 / 459 MeV. Both are abundant in the low-mass parking data.
  4. The remaining low tail stays a resolution effect, handled by the mass error (σ_m correction) and the signal model.

## 2026-11-21: ECAL energy for the conversions from PF candidates (study; no change to the ntuplizer)

- **Goal:** recover the bremsstrahlung loss of the conversion electrons (track energy −7%, long low tail) with the calorimeter.
- **Available ECAL information:** AOD PF superclusters exist only above ≈ 4 GeV (barrel; min. E 4.1 GeV, endcap 9.7 GeV in the signal sample), basic clusters only inside them; MiniAOD keeps only the e/γ clusters (`reducedEgamma`). Usable in both formats: the PF candidates: ECAL energy linked to a charged candidate (AOD `PFCandidate::ecalEnergy()`, MiniAOD `caloFraction·(1 − hcalFraction)·E`, stored for charged candidates with pt > 0.5 GeV) and the PF photons (brem photons, excess ECAL energy).
- **Study:** `Study/ConvFitStudy` now stores, per conversion, the PF candidates within |Δη| < 0.2, |Δφ| < 0.8 of the pair (`pf_pdgId`, `pf_E`, `pf_ecal`, `pf_dEta`, `pf_dPhi`, `pf_leg` = matched to conversion track 1/2: same charge, |Δpt|/pt < 2%, ΔR < 0.02), the generated photon energy `genE` and `pairEta` (new parameters `pfCandidates`, untracked `runFits`). Full `bsmmg_aod` (8888 events, 1463 signal-B conversions) and `bsmmg_miniaod` (32884, 1176).
- **Results** (E/E_gen − 1, median / (q84 − q16)/2, fraction within ±5%):
  - Both legs matched to a PF candidate in 20% (AOD) / 45% (MiniAOD) of the conversions (the legs are PF charged hadrons, 96–98%); 2–3 unmatched charged PF candidates per conversion lie nearby, so a looser match would raise this, but it does not change the conclusion below.
  - On the conversions with both legs matched: track pair −10.2% / 16% / 0.35 (AOD), −6.0% / 9.9% / 0.45 (MiniAOD). ECAL of the legs alone: −31% / 31% and −28% / 27% (ECAL/track of the legs ≈ 0.77). ECAL of the legs + PF photons in a road (|Δη| < 0.05–0.10, |Δφ| < 0.3–0.8; 1.0–2.0 photons, 2.8–4.5 GeV): median −18% … +0% depending on the road, width 36–38%, only 6–10% within ±5%. max(track, ECAL + photons): bias ≈ 0 but width 18–27% and fewer within ±5% (0.20–0.31) than the tracks.
  - The ECAL does not see the lost energy: when the track energy is > 10% low (51% AOD, 38% MiniAOD of these conversions), the ECAL + road estimate is low as well (median 0.72–0.77 of E_gen, width 40%). Linear combinations p_trk + w (E_cal − p_trk) are worse than the tracks for every w > 0 (correlation of E_cal − p_trk with E_gen − p_trk: 0.47–0.49 AOD, 0.32–0.33 MiniAOD).
- **Conclusion:** the PF-level ECAL information of these 1–5 GeV electrons is too coarse to correct the brem loss (low-energy showers below the clustering thresholds, fragmented clusters of the curling electrons, PF hadron calibration of the energy linked to the legs). A calorimeter estimate would need a dedicated low-threshold clustering (cf. `BMMGCustomPhotonProducer`, plan.md Phase 9, AOD only). For now the best photon energy remains the track pair (colinearity fit), with the mass error corrected after the fit.

## 2026-11-21: `mmcg` fit with the pointing constraint inside the vertex fit (`mmcgPointingInVertexFit`, default false)

- **Idea:** with `KinematicConstrainedVertexFitter` + `MultiTrackPointingKinematicConstraint(pv)` the muons, the photon and the vertex are refitted together with the pointing constraint, so with a realistic photon error (`conversionMomentumError`) the fit might correct the photon energy lost by bremsstrahlung.
- **Change:** `MMGVertexFit::fitParticlesPointing` (stage 1, the plain vertex fit, still fills `vtxOnly*`); the common part of the two fits is `MMGVertexFit::resultFromTree`. Switch `mmcgPointingInVertexFit` (`CandidateConfig`), default false. Regression with the default: the same 44 `mmcg` fit branches differ in 116/8888 events, as between two identical runs (101/8888, known non-determinism); everything else identical.
- **Scan** (B_s → μμγ gen-matched `mmcg`, vtxProb > 10⁻³; mass − m_gen median / (q84 − q16)/2 [MeV], candidates within ±100 MeV, pull (q84 − q16)/2):

  | | AOD | MiniAOD |
  |---|---|---|
  | default fit, s = 0 | 459: −61 / 164, 259; 2.63 | 599: −88 / 171, 293; 3.34 |
  | pointing in the fit, s = 0 | 431: −66 / 161, 236; 2.48 | 560: −85 / 170, 280; 3.38 |
  | s = 0.10 | 445: −65 / 169, 216; 1.14 | 598: −101 / 178, 247; 1.28 |
  | s = 0.15 | 452: −65 / 216, 189; 1.18 | 606: −93 / 201, 244; 1.14 |
  | s = 0.20 | 456: −69 / 261, 166; 1.23 | 607: −90 / 228, 221; 1.14 |

  Fewer fits succeed (all fitted: 732 vs 836 AOD, 822 vs 909 MiniAOD at s = 0). As with the sequential fit, a larger s calibrates the pull but worsens the mass. The photon response after the fit stays at −5% to −6%. With s = 0.20 the fit moves the photon by 12–14% (rms-like), but the shift is only weakly correlated with the needed correction (0.22 AOD, 0.36 MiniAOD) and brings the photon closer to the generated one in only 39–44% of the candidates. Large flight-length significance (lxy/σ > 10) does not help.
- **Why: the pointing measures the photon energy poorly.** Per candidate, the photon scale k that makes μ⁺ + μ⁻ + kγ (reconstructed momenta) parallel to the flight direction gives the photon energy from pointing alone. With an unbiased decay vertex (the dimuon vertex) its spread is 34% (AOD) / 43% (MiniAOD), against 9% for the measured photon (after the vtxProb cut); with the true SV and the reconstructed PV 27%; only with both true vertices 2.5% (B mass 63–68 MeV). The flight length is short (median 2.6–2.8 mm) compared with the decay vertex resolution (~100 µm), and the photon is close to the B direction (median 0.1 rad): a 1% photon energy change turns the B direction by only ~0.4 mrad. (The fitted SV cannot be used for this test: the pointing constraint has already moved it along the biased momentum.) So the constraint adds noise rather than information about the brem loss.
- **Conclusion:** keep the default sequential fit with `conversionMomentumError = 0`; correct the mass error after the fit (σ_m'² = σ_m² + ((m² − m_μμ²)/2m)² s², s ≈ 0.15–0.20). Recovering the brem loss needs a calorimeter energy for the conversion.

## 2026-11-21: Inflated conversion photon momentum error in the B fit (`conversionMomentumError`, default 0)

- **Change:** new parameter `conversionMomentumError` (`ConversionCuts::momentumError`, passed to `MMGVertexFit::conversionParticle`): σ(p)/p added along the refitted photon momentum (C_p += s² p pᵀ, direction unchanged) before the photon enters the B fit. Default 0 (no change to the ntuple).
- **Scan** (B_s → μμγ gen-matched `mmcg`, `bsmmg_aod` 8888 / `bsmmg_miniaod` 32884 events; mass − m_gen(B) median / (q84 − q16)/2 [MeV], candidates within ±100 MeV, pull median / (q84 − q16)/2, median σ_m):

  | s | AOD all fitted | AOD vtxProb > 10⁻³ | MiniAOD all fitted | MiniAOD vtxProb > 10⁻³ |
  |---|---|---|---|---|
  | 0 | −96 / 292, 337; pull −1.85 / 5.11; σ_m 56 | −61 / 164, 259; pull −1.04 / 2.63 | −130 / 218, 367; pull −2.80 / 4.37; σ_m 47 | −88 / 171, 293; pull −2.13 / 3.34 |
  | 0.10 | −97 / 298, 293; pull −0.68 / 2.22; σ_m 146 | −64 / 181, 233; pull −0.43 / 1.22 | −117 / 232, 319; pull −0.80 / 1.68; σ_m 142 | −99 / 188, 260; pull −0.63 / 1.31 |
  | 0.15 | −107 / 328, 250; pull −0.59 / 1.99; σ_m 182 | −65 / 212, 200; pull −0.35 / 1.18 | −110 / 257, 313; pull −0.59 / 1.48; σ_m 182 | −94 / 204, 254; pull −0.49 / 1.16 |
  | 0.20 | −101 / 364, 232; pull −0.53 / 1.82; σ_m 208 | −70 / 253, 174; pull −0.37 / 1.22 | −103 / 307, 274; pull −0.49 / 1.44; σ_m 216 | −89 / 237, 232; pull −0.43 / 1.09 |

  The pull becomes calibrated (≈ 1.1–1.3 with vtxProb > 10⁻³ for s ≥ 0.10), but the resolution gets **worse** with s: candidates within ±100 MeV drop by 10–30%.
- **Why:** the fitted daughters do not change: the photon response after the B fit is −6.7% / −7.0% for every s (the fit does not correct the brem loss), the muons move by 0.2%, and the B mass computed from the fitted daughters is the same for all s (AOD −106 / 302 MeV, MiniAOD −129 / 218 MeV). Only the top particle changes: the pointing constraint (stage 2) acts on the 7-parameter B state and moves its mass through the mass–momentum correlation (as the zero-mass constraint did for the photon, 2026-11-20). The larger photon error increases that correlation, and the PV→SV direction is too poorly measured to determine the photon energy, so the mass picks up noise. (The fit hardly improves on the pre-fit mass for `mmcg` anyway: vtxProb > 10⁻³, AOD 164 vs 182 MeV, MiniAOD 171 vs 175 MeV.)
- **Conclusion:** keep `conversionMomentumError = 0` in the fit. To get a calibrated mass error without degrading the mass, add the photon term after the fit: σ_m'² = σ_m² + ((m² − m_μμ²)/2m)² s², with s ≈ 0.15–0.20 (estimate on the s = 0 ntuples: pull width ≈ 1.0–1.3); this can be done in the analysis from `mmcg_mass`, `mmcg_massErr`, `mmcg_fitDimuonMass`. The mass bias (median −60 to −130 MeV) and the low tail are the brem loss itself and need an energy correction or a calorimeter energy.

## 2026-11-21: Why `mmcg_massErr` is underestimated (study; no code change)

- **Symptom:** the `mmcg` mass pull (m − m_gen(B))/σ_m has median −1.9 / −2.8 and (q84 − q16)/2 = 5.1 / 4.4 (AOD / MiniAOD, B_s → μμγ, gen-matched, fitted). The `mmg` mass pull is fine (core 1.08, 2026-11-06), so the problem is the conversion photon.
- **It is all in the photon energy:** with the photon momentum rescaled to the generated one (m² − m_μμ² ∝ E_γ), the residual drops from 292 / 218 MeV to 70 / 44 MeV and the pull width to 1.13 / 0.99. Binned in the photon response r = p_T/p_T,gen − 1, the pull is −0.3 / −0.7 with width 1.2 / 1.4 for |r| < 5%, −3.3 / −4.3 for −20% < r < −5%, and −11 / −12 for r < −20%. Muons: response 0.2% / 0.1%, width 1.4–1.5%, not the cause.
- **Photon p_T error vs actual spread** (`Study/ConvFitStudy`, new branches `<variant>_ptErr` from the 7×7 covariance, `trkPt/PtErr/NHits/Chi2n` of the two tracks; 10k events each, signal-B conversions):

  | | AOD | MiniAOD |
  |---|---|---|
  | photon response, colinearity fit (median / (q84 − q16)/2) | −6.5% / 20.5% | −7.5% / 12.3% |
  | fraction with response < −20% | 21–35% (by ρ) | 9–26% |
  | assigned σ(p_T)/p_T, colinearity fit (median) | 3.0% | 1.8% |
  | assigned σ(p_T)/p_T of each track | 3.1% | 2.4% |
  | photon p_T pull, colinearity fit: median / width | −2.1 / 10.6 | −4.6 / 7.0 |
  | old vertex fit + zero mass (`cur`, only its converged 40%): assigned / pull width | 6.7% / 5.6 | 1.6% / 4.8 |

- **Cause: electron bremsstrahlung, not modelled in the track fit.** The conversion electrons radiate in the tracker material after the conversion point. `generalTracks` are Kalman-fitted with the pion hypothesis: their material effects are multiple scattering and mean ionisation loss, without bremsstrahlung. The fitted momentum is therefore biased low (≈ −7%) with a long Bethe–Heitler tail, while the error only reflects hit resolution and multiple scattering (2–3%). The observed spread is about 7× the assigned error in the core, plus the non-Gaussian low tail. Bias and width hardly depend on the conversion radius or the photon p_T.
- **Why the new fit made the pull worse (3.1 → 4–5):** the colinearity constraint combines the two tracks and reduces the photon error below the single-track error. The old fit converged only for 30–40% of the conversions, and its non-converged vertex left larger errors (AOD `vtxOnly`: 11.8% assigned). Its pull looked better because its errors were inflated by accident, not because it described the brem.
- **Effect of a realistic photon error** (estimate: σ_m² + (∂m/∂ln E_γ)²(s² − a²), with ∂m/∂ln E_γ = (m² − m_μμ²)/2m ≈ 1.8 GeV and a the assigned error): a relative photon error s = 15% (AOD 20%) gives a pull width ≈ 1.0–1.3 and σ_m ≈ 270–370 MeV; 10% gives 1.4–1.9. The low tail (|pull| > 3: 8–14% at s = 15%) stays, since the brem loss is not Gaussian.
- **Options, not implemented:** (1) inflate the photon momentum error in `conversionParticle` (a relative energy error of ~12–20% added in quadrature, like the ECAL error scale of `mmg`); with the pointing constraint the B fit can then partly correct the photon energy instead of pulling the muons and vertex, so the resolution may improve, not only the pull; (2) correct the −7% mean bias; (3) use a calorimeter energy for the conversion (supercluster or PF cluster matched to the pair), which contains the brem photons; (4) GSF refit of the electron tracks (needs RECO, not available in AOD/MiniAOD).

## 2026-11-21: Conversions refitted with a colinearity constraint; B mass resolution of `mmcg`

- **Change:** `MMGVertexFit::conversionParticle` fits the e⁺e⁻ tracks with `KinematicConstrainedVertexFitter` + `ColinearityKinematicConstraint(PhiTheta)` (as the reco `ConversionVertexFinder`), without the zero-mass constraint; the photon keeps the fitted mass (≈ 1 MeV). Reasons: the plain vertex fit does not converge for 58–74% of the signal conversions (2026-11-17), and the zero-mass constraint after a colinearity fit moves the momentum by many σ (2026-11-20). `conv_fitProb` is now the χ² probability of the colinearity fit.
- **Bug found on the way (separate commit):** the photon was the top particle of a local `KinematicTree` that was freed on return, and the B fit read that tree (`InputSort`, `FinalTreeBuilder`): ~1% of the `mmcg` fits changed (old fit with the fix vs without: 87/8888 AOD, 196/32884 MiniAOD events; resolution unchanged), and with the new fit cmsRun crashed. Keeping the tree alive is not enough: the B fit treats a particle with a tree as a composite and grafts the (modified) tree into the B tree, which loops forever when the photon is used for a second dimuon. The photon is now a standalone `VirtualKinematicParticle` with the fitted state, χ² and ndof.
- **Validation:** `bsmmg_aod` (8888 events) and `bsmmg_miniaod` (32884 events), `channels=mmcg`, old fit (with the bug fix) vs new. Signal-B conversions refitted: AOD 42.7% → 98.0%, MiniAOD 28.9% → 99.9%. Events with a fitted gen-matched candidate: AOD 2.5% → 5.2%, MiniAOD 0.78% → 2.8%.
- **B mass of the gen-matched candidates** (fitted mass − m(B_s); median, (q84 − q16)/2, IQR/1.349, fraction within ±100 MeV):

  | | AOD | MiniAOD |
  |---|---|---|
  | same candidates, fitted by both (350 / 252): old | −77 MeV, 266, 197, 0.46 | −86 MeV, 165, 147, 0.51 |
  | same candidates: new | −66 MeV, 242, 182, 0.51 | −84 MeV, 164, 162, 0.51 |
  | same, both vtxProb > 10⁻³ (207 / 168): old | −60 MeV, 129, 94, 0.59 | −55 MeV, 118, 98, 0.62 |
  | same, both vtxProb > 10⁻³: new | −47 MeV, 120, 88, 0.64 | −50 MeV, 123, 100, 0.62 |
  | fitted only by the new fit (486 / 657) | −156 MeV, 331, 287, 0.33 | −150 MeV, 234, 238, 0.36 |
  | ... with vtxProb > 10⁻³ (235 / 426) | −81 MeV, 191, 174, 0.49 | −113 MeV, 185, 183, 0.44 |
  | all usable (fitOK, vtxProb > 10⁻³): old | 237: −68 MeV, 155, 122, 0.55 | 177: −57 MeV, 135, 102, 0.60 |
  | all usable: new | 459: −62 MeV, 164, 144, 0.56 | 599: −88 MeV, 171, 158, 0.49 |

  - On the same candidates the resolution is equal (MiniAOD) or slightly better (AOD, where the zero-mass constraint biased the photon by −11%).
  - The recovered candidates are the harder conversions with more bremsstrahlung (lower and wider mass, as their input pair momentum); they widen the overall distribution, but the number of usable candidates within ±100 MeV grows ×2.0 (AOD, 130 → 257) and ×2.8 (MiniAOD, 106 → 293).
  - The mass pull (m − m(B_s))/σ_m has (q84 − q16)/2 ≈ 4–5 (old: 3.1): `mmcg_massErr` underestimates the error, more so with the new fit (median σ_m 56 MeV AOD, 47 MeV MiniAOD).
- **Non-determinism:** two identical AOD runs still differ in 101/8888 events for `mmcg` (and 2/3000 for `mmg`), all in the B fit (`conv_*` identical). This is the known run-to-run non-determinism of non-converging fits, not from the conversion photon.

## 2026-11-20: Conversion refit with a loose fallback vertex fit (`sergey` variant of `Study/ConvFitStudy`)

- **Why the zero-mass constraint degrades the fit:** the photon after the vertex fit is one 7-parameter state; `MassKinematicConstraint` moves the momentum only through its correlation with the mass, Δp ≈ −ρ σ_p (m/σ_m). After the vertex-only fit m ≈ 10 MeV with σ_m ≈ 9.5 MeV (pull ≈ 1), so the shift is small (median −0.2%). After the colinearity fit the mass is at the e⁺e⁻ minimum 2mₑ (1.06 MeV) with σ_m ≈ 6 keV (pull 70–500): m = 0 is unreachable for two massive electrons and the momentum is moved by many σ_p (median |Δp_T|/p_T = 57%). Hence no mass constraint after the colinearity fit.
- **Bug fixed:** the `sergey` variant went through the shared default vertex fit first and returned on its failure, so its fallback (`maxDistance` 3 cm, 10000 iterations) never ran and the variant was identical to `cur`. It now runs on its own (default fit, fallback on failure or exception, then the zero-mass constraint with σ = 10⁻⁶, constraint on the stack); new branches `sergey_fallback`, `sergey_vtxPt`.
- **Results** (10k events each of `bsmmg_aod` / `bsmmg_miniaod`):

  | | AOD all | AOD signal B | MiniAOD all | MiniAOD signal B |
  |---|---|---|---|---|
  | OK `cur` | 47% | 42% | 59% | 26% |
  | OK `sergey` | 94% | 89% | 98.5% | 90% |
  | OK `colin` | 98% | 97% | 99.9% | 100% |
  | fallback used / fallback OK | 53% / 88% | 58% / 82% | 41% / 96% | 74% / 86% |

  - Failures of `sergey` are all in the fallback vertex fit; the colinearity fit succeeds for 187 of the 191 signal conversions where `sergey` fails.
  - Fallback vertices: |ρ − ρ_reco| median 0.2–0.5 cm, but 84% quantile 0.75–1.4 cm (vs 0.2–0.7 cm with the default fit); χ² probability < 10⁻³ for 13–37%.
  - Signal p_T/p_T,gen − 1 on the conversions where both succeed: AOD `sergey` −8.8% / 20% (median / half-width), `colin` −5.7% / 20%, input pair −6.6% / 18%; MiniAOD −8.4% / 12.7%, −8.0% / 12.7%, −8.7% / 12.5%. On the fallback the mass constraint shifts p_T by a median 0 (84% within 10% AOD, 1% MiniAOD).
- **Conclusion:** the loose fallback recovers most fits (×2.1 AOD, ×3.4 MiniAOD on signal) without spoiling the momentum, but the colinearity fit is still better: ~8–10% more signal conversions, better vertex, and no extra AOD bias (−5.7% vs −8.8%). The recommendation stays the colinearity fit without mass constraint.

## 2026-11-20: Test samples in YAML, selected by name

- **What:** `RadiativeAnalysis/test/test_files.txt` is replaced by `test_files.yaml`. Each sample has a short name (the key) and the attributes `isMC`, `dataFormat` (AOD / MINIAOD), `files` (globs allowed) and an optional `description`. All 18 samples of the text file are converted, in the same order.
- **Selection:** `testSample` is now the sample name (string; default `bsmmg_aod`) instead of the index, e.g. `cmsRun runNtuplizer_cfg.py testSample=data_miniaod`. An unknown name raises an error listing the available names.
- **isMC / dataFormat:** taken from the sample attributes instead of being guessed from the description (`data…` / `…miniaod…`); the command-line `isMC` / `dataFormat` still override them. For `inputFiles` / `dataset` the default stays MC, AOD.
- **Fix:** `dataset=` used to fail with a NameError (`sampleName` undefined); it now uses the same defaults as `inputFiles`.
- **`python/testFiles.py`:** `testSamples()` → `{name: sample}` with validation (required/unknown keys, bool `isMC`, allowed `dataFormat`); `testSample(name)`; `testFiles(name, maxFiles)`. `python3 RadiativeAnalysis/python/testFiles.py` lists the samples. PyYAML is imported lazily (only when a test sample is used).
- **Other users:** `customPhotons_cfg.py` (`testSample`, default `bsmmg_aod`) and `MMGVertexFitExample.py` (default `bsmmg_aod` / `bsmmg_miniaod` from `dataFormat`) take names too. README and structure.md are updated.
- **Validation:** the config resolves the right files, isMC and format for `bsmmg_aod`, `data_miniaod`, `bdjpsipi0_miniaod` (glob, `testMaxFiles=3`) and explicit `inputFiles`; `cmsRun runNtuplizer_cfg.py testSample=data_miniaod maxEvents=20` runs.

## 2026-11-17: Why the e⁺e⁻ zero-mass refit of the conversions fails (study; no code change)
`MMGVertexFit::conversionParticle` does a `KinematicParticleVertexFitter` vertex fit of the two electrons, then a `MassKinematicConstraint(0)` on the parent. It fails for 57% (AOD) and 71% (MiniAOD) of the signal-B conversions.

**Method:** a scratch analyzer (`src/ConvFitStudy/Study`, not part of this repo) refits every conversion of the signal MC with several methods. Conversions are matched to a gen photon from the B (ΔR < 0.1, |Δpt|/pt < 0.5). It also replicates the loop of `SequentialVertexFitter<6>` with diagnostics; the replica agrees with the real fitter for 100% of the conversions.

**Cause:** the **vertex fit**, not the mass constraint (the constraint never fails), and not the starting state (starting the electrons at their PCA to the conversion vertex changes nothing).
- 99% of the failures stop at the maximum of 100 iterations without converging. The convergence criterion is a transverse shift of the vertex below 100 µm between iterations.
- Out of tracker bounds: 1%. Kalman update failure, NaN, propagation failure: 0.
- For the non-converging fits the vertex keeps jumping by a median of 3–5 cm (signal) per iteration. The smallest jump over all iterations is still ~2 mm. The last position stays close to the reconstructed conversion vertex (median 0.3–0.9 cm in ρ).
- The two electron tracks are nearly parallel at the conversion point (median Δcotθ ≈ 0.01), so the vertex is not constrained along their direction; the relinearised fit oscillates along it. The effect is stronger for harder conversions, so signal (pt ≈ 6 GeV) fails more often than the average conversion (pt ≈ 1 GeV).

**Alternatives** (signal-B conversions, 10k events; momentum resolution = half-width of the 16–84% range of pt/pt_gen − 1):

| method | OK AOD | OK MiniAOD | pt bias / resolution AOD | pt bias / resolution MiniAOD | χ² prob < 10⁻³ AOD / MiniAOD |
|---|---|---|---|---|---|
| input pair momentum (no fit) | – | – | −7.7% / 18% | −7.5% / 12% | – |
| current: vertex fit + zero mass | 42% | 26% | −10.8% / 22% | −2.6% / 7.8% | 23% / 11% |
| vertex fit only | 42% | 26% | +0.6% / 17% | −1.9% / 7.6% | 10% / 0% |
| `KinematicConstrainedVertexFitter` + `TwoTrackMassKinematicConstraint(0)` | 25% | 41% | −4.6% / 14% | −5.0% / 10% | 16% / 10% |
| **`KinematicConstrainedVertexFitter` + `ColinearityKinematicConstraint(PhiTheta)`** | **97%** | **100%** | −6.5% / 21% | −7.5% / 12% | 37% / 11% |
| colinearity fit, then `MassKinematicConstraint(0)` | 97% | 100% | −24% / 85% | −49% / 49% | 100% / 100% |

- **Colinearity fit:** the colinearity constraint (Δφ = Δθ = 0 at the vertex) is what the standard conversion reconstruction (`ConversionVertexFinder`) uses. It converges for 97–100% of the signal conversions. Its momentum resolution is that of the input pair, and on the conversions where the current fit works it is the same as the current fit. The fitted mass is ≈ 1.1 MeV (≈ 2 mₑ), negligible for the B mass.
- **Zero mass on top:** adding the zero-mass constraint after the colinearity fit is wrong. The parent mass is then known to ~10⁻⁸, so forcing it to 0 drags the momentum (χ² probability ≈ 0).
- **Current fit, AOD:** the zero-mass constraint biases pt by −11%. The vertex-only parent mass is large there (median 73 MeV), because `allConversions` contains many fake or badly measured pairs.
- **Momentum bias:** the −7% pt bias of the input pair (and of the colinearity fit) is the electrons' bremsstrahlung energy loss; the fit cannot recover it.

**Recommendation:** in `conversionParticle`, replace the vertex fit + zero-mass constraint with `KinematicConstrainedVertexFitter` + `ColinearityKinematicConstraint(PhiTheta)` and use the top particle (mass ≈ 2 mₑ) as the photon. Expected effect: signal conversion refit OK 42% → 97% (AOD), 26% → 100% (MiniAOD), i.e. ×2.3 / ×3.8 more fitted signal `mmcg` candidates. The B-fit mass resolution with these photons is still to be checked.

## 2026-11-16: Conversions (μμγ, `mmcg`) in MiniAOD from `oniaPhotonCandidates`
The conversion channel was switched off for MiniAOD, because the tracks of `reducedEgamma:reducedConversions` are not stored. It now reads `oniaPhotonCandidates:conversions` (`pat::CompositeCandidate` from `OniaPhotonConversionProducer`, kept in MiniAOD for BPH). These candidates carry full copies of the two tracks as userData `track0`/`track1`, so the same e⁺e⁻ zero-mass refit and B fit run as in AOD.

**Changes:**
- `SelectedConversion` no longer points to a `reco::Conversion`. It holds the vertex, its probability (AOD only), the pair momentum, the MiniAOD `flags`, and the refit χ² probability.
- New overload `selectConversions(std::vector<pat::CompositeCandidate>, ...)`; the plugin picks the token by `dataFormat`.
- In the 2022C PromptReco MiniAOD the product is **missing in about 11% of the events** (225/2000; 268/300 in a direct check of the file). This is now handled: no conversions in those events, a new event flag `evt_convAvailable`, and a warning with the count at the end of the job.
- New branches: `conv_flags` (MiniAOD), `conv_fitProb` (both formats). `conv_vtxProb` is −1 in MiniAOD (the vertex χ² is not stored there).
- `runNtuplizer_cfg.py`: MiniAOD `conversions = 'oniaPhotonCandidates:conversions'`; the MiniAOD switch-off of `mmcg` is removed.

**Validation:**
- **AOD regression (2000 events):** `conv_*` identical; differences only in the new branches and the known fit non-determinism.
- **Same events, AOD file:** running `OniaPhotonConversionProducer` on the AOD signal file, 73 of the 122 events with a signal-B conversion in `allConversions` keep one in `oniaPhotonCandidates` (60%), and all 73 are a subset of the 122.

| signal MC, μμγ (different files) | AOD (8888 events) | MiniAOD (20000 events) |
|---|---|---|
| conversions per event | 2.18 | 1.28 |
| events with a signal-B conversion | 6.4% | 3.6% |
| signal-B conversions per such event | 1.88 (duplicates sharing a track) | 1.00 |
| zero-mass refit OK, per signal-B conversion | 43% | 29% |
| events with a signal conversion: any refit OK | 48% | 29% |
| events with a fitted, gen-matched `mmcg` candidate | 2.5% | 0.80% |
| gen-matched fitted mass: median, 16–84% half-width | 5.289, 0.269 GeV | 5.309, 0.155 GeV |

- **Data MiniAOD (2022C, 2000 events):** 0.78 conversions per event (with the collection present), refit OK 64%, 0.20 `mmcg` candidates per event.

**Conclusions:**
- The MiniAOD yield of signal `mmcg` candidates is about 1/3 of AOD. Roughly half of the loss comes from the producer's preselection (ρ > 1.5 cm, highPurity + generalTracksOnly, compatible inner hits, no duplicates). The other half comes from the zero-mass refit, which fails more often on these conversions, and AOD gets a second chance through the duplicates.
- The MiniAOD candidates have a better mass resolution.
- **The zero-mass refit fails for more than half of the signal conversions in both formats.** This is the main efficiency loss of the channel and is worth a separate study (vertex fit or mass constraint; starting point for nearly parallel tracks).

## 2026-11-16: Candidate-level isolation of the daughters with the B products removed
**New branches** (all channels): `<p><d>IsoCh` and `<p><d>IsoNeu` for every daughter `<d>` = `mu1`, `mu2` and
the legs (`pho`; `pho1`, `pho2`; `conv`; `trk1`, `trk2`). Cone ΔR < 0.3 around the daughter.
- **Charged:** Σ pt of the tracks of the candidate's PV with pt > 0.5 GeV (the same PV track list as
  `<p>isolation` and `<p>pvSumPt`). All tracks of the candidate are removed: muons, conversion tracks, kaons.
- **Neutral:** Σ pt of the PF photons and neutral hadrons with pt > 0.5 GeV. The footprints of all photons of
  the candidate are removed (AOD: `particleBasedIsolation:gedPhotons` value map; MiniAOD:
  `pat::Photon::associatedPackedPFCandidates`, i.e. the footprint used by the standard photon isolation).
  Around muons and tracks, the Muon POG inner veto ΔR < 0.01 is applied.
- No pile-up correction (no PU charged sum). Limitation: for `mmcg` the ECAL deposits of the conversion
  electrons are not removed from the neutral sums.
- **Code:** `bmmg::daughterIsolation` (`CandidateObservables`); `NeutralParticle`, `SelectedPhoton::footprint`,
  `BCandidate::daughterIso` (`Candidates.h`); `neutralsFromPF`/`neutralsFromPacked` and `setPhotonFootprints`
  (`ObjectSelection`). The builders take the neutral list; the `Finisher` computes the isolations.
  New parameters, with defaults, so no configuration change: `isolationCone` (0.3), `isolationMinChargedPt` (0.5),
  `isolationMinNeutralPt` (0.5), `pfCandidates` (`particleFlow`, AOD), `photonPFFootprint` (AOD).
  Unit test "daughter isolation without the B products"; all 43 assertions pass.

**Tests:** cmsRun on 2000 events of samples 0, 1 and 16. Compared with the previous commit, only the new
branches are added; the other differences are the known fit non-determinism (PV choice, `<p>pvSumPt` etc.
unchanged). For gen-matched μμγ:

| | MC AOD | MC MiniAOD |
|---|---|---|
| μ: neutral, standard (PF nh+ph) − cleaned, photon inside the μ cone (median) | 13.1 GeV = 0.90 × pt(γ) | 13.2 GeV = 0.89 × pt(γ) |
| μ: same, photon outside the cone | 0.00 | 0.00 |
| μ1: `mu_trkIsoR03` − cleaned charged, other muon inside the cone (median / pt(μ2)) | 1.00 | 1.00 |
| μ1 neutral, median / 90%: standard → cleaned | 6.1 / 23.2 → 0.9 / 4.4 GeV | 3.8 / 21.4 → 0.9 / 4.7 GeV |

- The cleaned values remove exactly the B products: the photon's PF energy (~0.9 of the photon pt) and
  the other muon's track. Elsewhere they equal the standard sums; for μμKK the neutral sums equal the
  standard muon PF sums.
- The cleaned charged sums are 0 for more than half of the signal daughters. The standard PF charged sum
  has a median of 0.4–0.8 GeV; it has no pt threshold and uses PV 0.

## 2026-11-16: Muon and photon isolation; PV track sums and muon–PV association per candidate
**New branches:**
- **Muons:** `mu_pfIsoChargedR03`, `mu_pfIsoNeutralR03`, `mu_pfIsoPhotonR03`, `mu_pfIsoPUR03` (PF sums,
  ΔR < 0.3), `mu_pfRelIsoR03`, `mu_pfRelIsoR04` (Δβ-corrected relative isolation, Muon POG formula),
  `mu_trkIsoR03` (tracker isolation). From `reco::Muon`, so identical in AOD and MiniAOD; NaN if not valid.
- **Photons:** `pho_pfIsoCharged`, `pho_pfIsoNeutral`, `pho_pfIsoPhoton` (PF, ΔR < 0.3), `pho_trkIsoR03`
  (hollow cone), `pho_ecalPFClusterIso`, `pho_hcalPFClusterIso`. From `reco::Photon`.
- **Candidates** (all channels), from the tracks of the candidate's PV (the same list as `<p>isolation`:
  weight > 0.5 in the PV fit; MiniAOD `UsedInFitTight`):
  - `<p>pvNTracks`, `<p>pvSumPt` (Σ pt), `<p>pvVecSumPt` (\|Σ p⃗_T\|). **The candidate's own tracks are
    excluded** (muons, conversion tracks, kaons), so the sums describe the rest of the PV and do not
    depend on whether the B muons were fitted into it.
  - `<p>mu1InPV`, `<p>mu2InPV`: the muon's inner track is a track of that PV. Matching: same charge,
    ΔR < 5·10⁻³, \|Δpt\| < 1% (tolerances for the MiniAOD pseudo-tracks).
- Code: `bmmg::pvTrackSums` and `bmmg::sameTrack` (`CandidateObservables`), `PVTrackSums` in `BCandidate`,
  `NtupleWriter::fillMuonIsolation`. Unit test "PV track sums and muon association" (37 assertions pass).

**Tests:** cmsRun on 2000 events of samples 0 (MC AOD), 1 (MC MiniAOD), 16 (data MiniAOD). Compared with
the previous build, only the new branches are added. The other differences are the known fit
non-determinism (μμγ-conversion 16/2000 events, μμKK, 1–2 μμγ events).

| | MC AOD | MC MiniAOD | data MiniAOD |
|---|---|---|---|
| gen-matched μμγ: `mu1InPV` / `mu2InPV` | 0.14 / 0.13 | 0.13 / 0.17 | – |
| μμKK, all: `mu1InPV` / `mu2InPV` | 0.17 / 0.13 | 0.18 / 0.14 | 0.55 / 0.55 |
| gen-matched μμγ: median `pvNTracks` / `pvSumPt` / `pvVecSumPt` | 52 / 47 / 9.4 GeV | 52 / 49 / 9.6 GeV | – |
| signal muons: median `mu_pfRelIsoR03` | 0.19 | 0.19 | (all μ) 0.25 |

- **Behaviour:** displaced signal muons are rarely in the PV fit (~14%); the mostly prompt μμKK data
  candidates are in it 55% of the time. AOD and MiniAOD agree; no NaN; \|Σ p⃗_T\| ≤ Σ pt always.
- **Caveat (documented in NTUPLE.md):** the object isolations include the other B products. For
  gen-matched μμγ, ΔR(μ, μ) < 0.3 in ~40% of the candidates and the photon is within 0.3 of a muon in
  ~75%, so `mu_trkIsoR03`, `mu_pfIsoPhotonR03` and `pho_trkIsoR03` contain them; e.g. the median
  `pho_trkIsoR03` of signal photons is 3.0 GeV against 1.2 GeV for all photons (AOD). PF charged-hadron
  sums contain no muons. A candidate-level isolation with the B products removed is not implemented yet.

## 2026-11-15: BDT tuning, new features, Run 3 projection (320 fb⁻¹)
- **Request:** tune the BDT, add features if needed (e.g. ΔR between the dimuon and the photon); the goal is an expected significance of at least 3σ after the BDT cut, projected to the Run 3 luminosity of 320 fb⁻¹.
- **Result:** the expected significance went from 0.6–0.9σ to **2.7–2.9σ**, close to the 3σ goal but not robustly above it. The 2.7–2.9σ is for a binned mass fit in 3 BDT-score categories; a cut-and-count window gives 1.9σ.
  - **Assumptions:** B(B_s→μμγ) = 5×10⁻⁹ (`br_scale`); the MC normalised to 34.65 fb⁻¹ of 2022 data; 2022 conditions for all of Run 3.
  - **No systematic uncertainties.** The background shape and normalisation are taken as known.
- **Inputs:**
  - **Data:** `Parking2022` = ParkingDoubleMuonLowMass0–7, 2022C–G, 18901 files, all of 2022. The working-tree config pointed to `Parking2022C`, which does not exist; changed to `Parking2022`.
  - **Signal:** B_s→μμγ MC, `genMatched == 1`. 7.6 events in the signal window after the preselection at 34.65 fb⁻¹, 70 at 320 fb⁻¹.
- **Preselection** (`analysis.yaml`):
  - `vtxProb > 0.01`;
  - `dimu_resonance == 0` (J/ψ, ψ(2S), φ);
  - ρ/ω veto (0.70 < m_μμ < 0.85 GeV);
  - **both candidate muons trigger-matched** to `DoubleMu4_3_LowMass`. This keeps the same signal as the event-level flag (69% of the signal) and removes 15% of the sideband background. The OR with the other dimuon paths (`LowMass_Displaced`, `Photon4_BsToMMG`, `Bs`) adds no signal: in the MC, `DoubleMu4_3_LowMass` contains them.
  - Multiple candidates are negligible: 1.6% of the sideband events have more than one, and 0.01% of the signal events.
- **Method:**
  - k-fold out-of-fold scores (5 folds); S from the signal MC in [5.15, 5.60] GeV; B from the sideband candidates times the exponential sideband factor; both × 320/34.65.
  - **Unbiased Z:** with the tight cuts the optimum sits at 10–20 sideband candidates, and choosing the best threshold among many on the same candidates biases Z up. So the threshold (or the category boundaries) is chosen on one half of the sideband events (`(evt_event // 7) % 2`) and Z is evaluated on the other half, and vice versa.
  - **Ranking objective for the hyperparameters:** signal efficiency at a sideband efficiency of 2×10⁻⁴ (about 60 sideband candidates), which is less noisy than the best Z.
- **Steps** (Z at 320 fb⁻¹, unbiased; the first rows use fixed default-like parameters):

  | | features | eff_sig at eff_bkg 2e-4 | cut & count Z | binned fit, 3 categories Z |
  |---|---|---|---|---|
  | baseline: features of the config, default params | 13 | 0.13 | 0.63 | 1.25 |
  | + `ctSig` (decay-time significance) | 14 | | 0.78* | |
  | + muon IP significances, dimuon flight, pointing, `cosAlpha2D` | 20 | | 0.94* | |
  | + photon shower shapes, `pho_energyErr` | 26 | | 0.94 | |
  | + ΔR(μμ,γ), min ΔR(μ,γ), p_T(γ)/p_T(B), p_T(B), \|η(B)\| | 31 | | 1.30 | |
  | + helicity, \|cos θ_ℓ\| | 33 | 0.30 | 1.50 | |
  | + \|η_γ\|, p_T(μμ), σ_iφiφ, conversion flag (final set) | 37 | 0.31 | 1.70 | 2.66 |
  | + tuned hyperparameters (final) | 37 | 0.33 | 1.86–1.89 | 2.68–2.89 |

  \* best Z, not the unbiased one. The final row spans seeds and the repository run (`scan_wp.py`: cut & count 1.86; 3 categories 2.84, unbiased 2.68).
- **Features:** 37, defined as `derived` expressions in `analysis.yaml`. The ΔR use the fitted photon direction (`phoFitEta/Phi`).
  - Most important by gain: `ctSig`, `pho_mva`, `lxySig`, `l3dSig`, `cosAlpha3DVtxOnly`, `isolation`.
  - Removing the two ΔR features from the final set costs a lot: signal efficiency 0.33 → 0.23, Z (3 categories) 2.87 → 2.0.
  - Not used:
    - `evt_nPV`: it separates only through the MC pileup profile;
    - `massErr`/mass: correlated with the mass, and no gain;
    - `pho_seedTime`: no separation, timing likely mis-modelled;
    - the dimuon mass.
- **Hyperparameters:** `scripts/tune_bdt.py`, 20 random trials (stopped at 20 of 40: the best trials agree within noise).
  - Chosen: depth 6, learning rate 0.045, L2 λ = 31, γ = 1, subsample 0.92, colsample 0.79, min_child_weight 5.2, early stopping 50 rounds (about 1200 trees).
  - The 5000-tree, learning-rate-0.003 configuration of the earlier working tree is no better than the 600-tree default.
- **Mass sculpting** (`scripts/check_sculpting.py`): train without the inner 0.2 GeV of the sidebands; the BDT efficiency there is compared with the adjacent strips.
  - Final BDT: ratio 0.99 ± 0.04, 0.93 ± 0.12, 0.96 ± 0.20 at sideband efficiencies 10⁻², 10⁻³, 3×10⁻⁴.
  - **Rejected: training on wider sidebands** (4.0–5.0 and 5.75–7.0 GeV, 1.9× the background). It gave Z = 3.2–3.4, but the test shows sculpting: ratio 1.21 ± 0.04 at 10⁻², 1.3–1.4 at 10⁻³, and the far strips have half the efficiency. The BDT learnt the mass from the ΔR features: without them the sculpting disappears (ratio 0.84–0.88), but so does the gain.
- **Background after the BDT:**
  - Concentrated in the low sideband: low/high ratio 1.0 after the preselection, 5.5 at sideband efficiency 10⁻³.
  - Flat in mass within each sideband, i.e. a steeper exponential: λ from 0.29 to 1.6 GeV⁻¹.
  - A single exponential describes the sidebands (deviance 28–31 for 23 degrees of freedom at 10⁻² to 10⁻³). An exponential plus a constant converges to the same curve.
- **Binned fit estimate** (`significance.binned_asimov`, `optimise_categories`; `scan_wp.py` with `significance.shape`):
  - Asimov Z of a fit of the mass in [5.0, 5.75] GeV (25 MeV bins), in 1–3 BDT-score categories added in quadrature.
  - Background per category: its sideband count × an exponential with the slope of the top 1% of sideband scores. At least 10 sideband candidates per category.
  - It gains a factor 1.5 over the cut-and-count window because it uses the signal peak (σ ≈ 0.11 GeV).
  - Not used: an EB/EE split of the photons (fewer sideband candidates per category, lower Z).
- **Also tested:** averaging the scores of 4 models (2 parameter sets × 2 seeds) gave 2.76–2.99, within the seed spread; not used.
- **New code:**
  - `scripts/tune_bdt.py`, `scripts/check_sculpting.py`;
  - `significance.binned_asimov`, `exp_fraction`, `optimise_categories` (+2 tests);
  - `scan_wp.py`: `projection_lumi_pb` and the shape-fit section (`shape_categories.csv`).
  - `pytest analysis/tests`: 22 passed.
- **Caveats and open points:**
  1. **3σ is not robustly reached.** The estimate's own statistical uncertainty is about ±0.2: half-samples 2.5–3.1, seeds 2.68–2.89. Also: S ∝ B(B_s→μμγ), and Z ≈ S/√B here, so Z scales linearly with the assumed BR (5×10⁻⁹); the generator σ × filter efficiency of the MC is not cross-checked.
  2. **No systematics:** a real fit has the background-shape uncertainty, e.g. a partially reconstructed component below the B mass, which dominates the background after the BDT.
  3. **MC modelling of the inputs:** the BDT is trained on signal MC against data. The photon shower shapes, `pho_mva`, the conversion flag and the isolation must be validated in data versus MC, e.g. with B→J/ψ X with radiative photons, or the J/ψγ control channel (`analysis_JpsiG.yaml`). The angular variables (`helicity`, `cos θ_ℓ`) make the efficiency depend on the B_s→μμγ form-factor model of the MC.
  4. **Overtraining:** the train/test KS p-values of the signal are 10⁻¹³–10⁻²² (61k signal candidates; the background ones are 10⁻³–0.04). All estimates use out-of-fold scores, so they are unbiased, but more signal MC would help.
  5. **The biggest acceptance limit is not the BDT:** the photons in this MiniAOD production start at p_T = 10 GeV (`slimmedPhotons`). Lower-p_T photons (AOD, or a custom low-p_T photon collection, Phase 9 of `plan.md`) would add signal.
  6. The 2023–2025 trigger thresholds and pileup are assumed to be those of 2022.
- **Config:** `analysis.yaml` now has the preselection, derived features, BDT features and parameters (`device: cuda`) and the shape-fit settings above. The MLP block uses the same 37 features through the YAML anchor; it was not retrained.

---

## 2026-11-14: k-fold MLP (TensorFlow) as an alternative to the BDT
- **Request:** a neural-network classifier (multilayer perceptron) analogous to the BDT, with the same features and code structure, configured in `analysis.yaml`, trained with TensorFlow on the GPU.
- **New:**
  - `analysis/bmmg_ana/mlp.py`: `train_kfold`, `predict`, `save`, `load` with the interface of `bdt.py`.
    - Same folds (`evt_event % k`), equal class weights and event-grouped 10% early-stopping split as the BDT.
    - Keras `Sequential`: Dense hidden layers (optional batch normalisation, dropout, L2), sigmoid output, Adam, weighted binary cross-entropy. Early stopping on the validation loss keeps the best epoch; the learning rate is halved on plateaus.
    - **Input preprocessing** (`Preprocessor`), fitted on the training folds of each fold: log(x + offset) for the configured long-tailed features, −999 and non-finite values → training median, standardisation, clipping to ±5. Needed because `lxySig`/`l3dSig` contain NaN (XGBoost handles it, a network doesn't), and pt, flight significance and `dimu_dca` have tails over 2–3 orders of magnitude. The constants are saved in `mlp.json`, so the `.keras` models have no custom layers.
    - **Importance:** permutation importance (AUC decrease on the test fold when a feature is shuffled).
  - `analysis/scripts/train_mlp.py`: as `train_bdt.py`, plus a loss-curve plot; `--cpu` hides the GPU.
  - `scan_wp.py --classifier mlp`: the scan with the MLP models; results in `<output>/mlp/`.
  - `analysis.yaml`: `mlp` block; `features: *features` reuses the BDT list. `load_samples` also loads the MLP features.
  - `plotting`: axis labels of the overtraining, importance and scan plots are parameters; new `history`.
  - `environment.yml`: `tensorflow`.
- **Result** (2022 Parking + B_s → μμγ MC, with the uncommitted working-tree `analysis.yaml` and `bdt.py` (`Parking2022`, resonance veto + trigger preselection, 5000-tree BDT on CUDA): 61k signal, 373k sideband candidates, 13 features, 3 folds; RTX A500 laptop GPU):

  | | BDT (5000 trees, GPU) | MLP (3 × 64, GPU) |
  |---|---|---|
  | training time | 1.0 min | 2.7 min (59–70 epochs kept per fold) |
  | out-of-fold AUC | 0.9858 | 0.9850 |
  | train/test KS p-values (sig; bkg) | 0.000005–0.003; 0.009–0.14 | 0.21–0.54; 0.06–0.95 |
  | Punzi optimum (threshold, ε_sig, ε_bkg) | 0.0287 (0.9911, 40%, 0.11%) | 0.0281 (0.9928, 41%, 0.12%) |
  | S, B at the Punzi optimum | 3.03, 154 | 3.08, 168 |

  - The two classifiers perform the same within these numbers; the MLP shows no sign of overtraining, while the BDT signal KS p-values are small.
  - Most important MLP inputs: `pho_mva`, `lxySig`, `isolation`, `cosAlpha3DVtxOnly` (BDT gain: `lxySig`, `pho_mva`, `l3dSig`, `cosAlpha3DVtxOnly`).
  - The validation loss is below the training loss because dropout is active only in training.
- **Tests:** `tests/test_mlp.py` (3 tests, skipped without TensorFlow): AUC on Gaussian toys, out-of-fold scores equal `predict`, save/load round trip, permutation importance picks the separating variable, early stopping, preprocessing (median fill, log, clip, serialisation). `pytest analysis/tests`: 20 passed.
- **Open:** the MLP hyperparameters are not tuned (only this one configuration was trained); an MLP/BDT score correlation and an average of the two are not studied.

---
## 2026-11-13: PV choice with a signed DCA (study; not adopted)
**Proposal:** the PV is the one with the lowest DCA to the B line, whether it lies behind or ahead of the
SV. Sign the DCA (negative if the momentum points from the PV to the SV, i.e. t = (SV − PV)·p̂ > 0) and
choose the lowest value.

**Method:** temporary branches with all PV positions (not committed), cmsRun on 5000 events of samples 0
(MC AOD), 1 (MC MiniAOD), 16 (data MiniAOD). The rules were evaluated offline on the same line
(dimuon vertex + pre-fit momentum); the offline "lowest DCA" reproduces the stored `<p>pv` for 99.9–100%
of the candidates. Truth: the reco PV closest in z to `gen_pvZ`, for gen-matched μμγ candidates with a
reco PV within 500 µm of it (841 AOD, 698 MiniAOD).

| rule | correct PV, AOD | MiniAOD | AOD, gen ct < 100 µm |
|---|---|---|---|
| lowest DCA (current) | **0.982** | **0.977** | **0.987** |
| lowest signed DCA (literal proposal) | 0.024 | 0.014 | 0.051 |
| behind first, then lowest DCA (tol 0) | 0.954 | 0.966 | 0.853 |
| behind first, tolerance 100 µm | 0.966 | 0.970 | 0.917 |

- **The literal rule fails:** among the PVs behind the SV, the most negative signed DCA is the one with
  the *largest* distance, so it picks a far pile-up vertex.
- **"Behind first" is also worse than the current rule:**
  - The true PV is ahead of the SV (t < 0) for 2.9% (AOD) / 1.3% (MiniAOD) of the candidates. For most
    of them it is ahead by more than 50 µm (median 140–160 µm); these are mainly short decays. The rule
    then picks another PV, which is wrong.
  - The current rule's 31 failures (15 AOD, 16 MiniAOD): in 30 of them the wrongly chosen PV is
    **behind** the SV as well, a pile-up vertex next to the B line, a few 100 µm from the true one. So
    the sign cannot fix them.
  - μμKK and data: 2–8% (MC) and 11–32% (data) of the candidates would change PV.
- **Decision:** the PV choice is unchanged. The signed DCA does not improve it; the remaining ~2% of wrong
  PVs are pile-up vertices on the same side. Candidates for a better choice: DCA significance with the
  PV covariance, or the PV that the muon tracks are associated with (MiniAOD `pvAssociationQuality`, AOD
  track weights). The study script is not in the repository (it needs the temporary branches).

## 2026-11-11: `submitCrab.py`: one summary line per task, `--verbose` for the full output
- **Default output:** one line per task. The CRAB call runs in the per-task child process as before, with
  its stdout/stderr redirected (at file-descriptor level, so the CRAB loggers are included) to
  `<workArea>/logs/<task>.<command>.log`. The child sends the outcome back to the parent through a pipe.
  - `submit`: `submitted: <unique task name>`, or `skipped: ... exists`.
  - `status`: task status and job states, e.g. `SUBMITTED  120 jobs: finished 100 (83%), running 18, failed 2`,
    plus the task failure message if any; at the end, the job states summed over all tasks.
  - `resubmit` / `kill`: `OK`.
  - Failure: the error on one line (≤ 200 characters), the last 15 lines of the CRAB output and the log path.
  - `--dry-run`: dataset, data type, splitting and cmsRun options per task.
- **`-v` / `--verbose`:** the previous behaviour: banner per task, full CRAB output, full CRAB configurations with `--dry-run`.
- **Failure detection improved:** `crab status` returning `commandStatus: FAILED`, and `crab resubmit`
  returning nothing (e.g. status not yet available), now count as failures; before, only exceptions did.
- **Tests:** dry run in both modes on `tasks_all.yaml`. No grid proxy was available, so the status /
  resubmit paths were tested with a stub `CRABAPI` (success, exception, verbose); the real `crab status`
  call reaches CRAB and reports the missing proxy on one line. The real submit path was not run.

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
- **Fix 8, pointing constraint not satisfied after the fit** (`MMGVertexFit`, `NtupleWriter`, `NTUPLE.md`; CMSSW fork):
  - **Measured on the production:** the angle between the fitted B momentum and SV − PV, which the constraint should make ≈ 0, exceeds 1 mrad for 8% of the signal MC and 30% of the data μμγ candidates. In the tail it reaches π. The float32 resolution of `cosAlpha3D` is about 0.3 mrad.
  - **Cause 1 (fork, `src/RecoVertex`, branch `photonKinematicFit_v2`):**
    - The convergence test of `LagrangeParentParticleFitter` loops `i < vl.num_row()` over a 1-based CLHEP vector. It skips the last equation, here the second pointing equation.
    - Fixed to `<=` in both loops. Committed in the fork as `1a4776adc6b`, **not pushed**. The CRAB sandbox ships the local build.
  - **New branches** (all channels):
    - `<p>pointingAngle`: the residual after the constraint, computed with atan2, stable near 0 and π;
    - `<p>vtxOnlyChi2/Ndof/Prob`: stage 1, the vertex-only fit;
    - `<p>cosAlpha3DVtxOnly`: the unconstrained pointing.
  - **Validation:** 2000 events each of MC AOD, MC MiniAOD and data MiniAOD, fitted candidates:

    | sample, channel | angle > 1 mrad, before → after the fitter fix | > 10 mrad | > 1.5 rad |
    |---|---|---|---|
    | MC AOD μμγ, gen-matched | 10.6% → 3.8% | | |
    | MC AOD μμγ, all | 12.9% → 5.3% | 4.1% → 3.6% | 0.5% → 1.0% |
    | MC MiniAOD μμγ, gen-matched | 10.7% → 4.3% | | |
    | MC MiniAOD μμγ, all | 11.7% → 5.1% | 2.8% → 2.5% | 0.6% → 0.9% |
    | data MiniAOD μμKK | 49.6% → 36.2% | 35.2% → 31.3% | 10.0% → 10.4% |

    - Gen-matched MC MiniAOD mass in [4.5, 6.5]: σ 0.184 → 0.179 GeV.
    - Reading the stage-1 values does not change the fit: compared with the run before this change, all branches are identical except the μμKK fits of 1–5 events per 2000 and μμγ(conversion) in 42/2000, both from the known fit non-determinism.
  - **Remaining:** about 3% of the μμγ fits (and many μμKK fits with χ² ≫ ndf) end at the iteration cap or at an anti-parallel solution. They are now visible through `pointingAngle`.
  - **Recommendation for the analysis:** cut `pointingAngle < 1e-2`, and use `cosAlpha3DVtxOnly` instead of `cosAlpha3D` as a BDT feature. Both need the new production; the config is not changed yet.
- **Fix 9, photon measurement checks** (`MMGVertexFit`, `NtupleWriter`, plugin, `NTUPLE.md`):
  - **Problem:**
    - `EcalClusterLazyTools::SuperClusterSeedTime` dereferences the rechit lookup without checking for a miss.
    - `pho_energyErr` called `getCorrectedEnergyError` with the candidate P4 type, which throws if the type is undefined.
    - The fit accepted any photon with rechits, even with a degenerate cluster covariance or a non-positive energy or energy uncertainty.
  - **Fix:**
    - The seed time is computed with an explicit seed and rechit check (NaN on a miss).
    - `MMGVertexFit::energyType` (fallback `ecal_photons`) is public and also used for `pho_energyErr`.
    - New `MMGVertexFit::photonInputStatus`: the energy and its uncertainty must be finite and positive, and the cluster covariance projected on (e_φ, e_θ) must be finite, with positive variances and |correlation| < 1 − 5·10⁻⁷. Photons that fail are not fitted.
    - New branch `pho_fitInputStatus`.
  - **Validation** (2000 events each):
    - **Seed time:** 9 (MC MiniAOD) and 6 (data MiniAOD) photons had garbage seed times from the unchecked lookup; they are now NaN. All other values are unchanged.
    - **Status counts** (0 / no hits / energy / energy error / covariance):
      - MC AOD: 820 / 0 / 0 / 0 / 463. Mostly low-pt AOD photons: 64% below 2 GeV, 9% above 10 GeV.
      - MC MiniAOD: 473 / 9 / 0 / 0 / 44.
      - data MiniAOD: 165 / 6 / 0 / 0 / 5.
    - **Effect on the fit:** fitted μμγ candidates 394 → 379 (AOD) and 316 → 311 (MiniAOD). Most of those lost had vtxProb ≈ 0 or a mass far from the B. Gen-matched: 338 → 334 and 299 → 295, i.e. −1.2% of the signal, from photons whose covariance has zero variance in one direction.
    - The other branches are identical, except the known μμKK and μμγ(conversion) fit non-determinism.
  - **Option, not done:** keep these photons with a calibrated resolution floor, e.g. of the order of the crystal size / √12, instead of rejecting them. This would need a pull study.
- **Fix 10, defensive fixes** (`MMGVertexFit::fit`, `run3mvaid`):
  - **`MMGVertexFit::fit()`**, used only by `MMGVertexFitExample`: the muon TransientTracks were a local vector destroyed at return, while the returned particles point to them. They are now shared pointers owned by the results (`Result::ownedTracks`).
  - **Muon MVA:**
    - The inner, outer and global tracks are used only if their reference is set and available; otherwise the existing missing-value defaults apply.
    - Pulls with zero uncertainties give the missing value 9999, not inf or NaN. They are still computed in double, as before.
    - The `XGDMatrixCreateFromMat` status is checked (−999 on failure).
  - **Validation:** 2000 events each of the 3 samples. All muon branches (including `mu_mva`) and all object branches are identical to the previous run. Differences are only in fits with χ² ≈ 3·10⁵ (known non-determinism).
- **Fix 11, documentation** (`NTUPLE.md`):
  - `dimu_dca` is the 3D separation of the points found by `ClosestApproachInRPhi`, not a transverse DCA.
  - `pho_hOverE` is not full5x5.
  - The wp90 efficiency target of `pho_tightId` holds above 10 GeV only.
- **Open (not applied; physics choices or studies, see `review.md`):**
  1. **New CRAB production needed:** fixes 7–9 change the ntuple. The isolation and track compatibility of the current MiniAOD production are wrong for candidates with PV ≠ 0.
  2. **Strict truth label:** signed IDs, unique gen indices, photon origin. `genMatched` is the inclusive ancestry flag and is too loose for the training label and the efficiency.
  3. **Gen classification:** deterministic B choice with opposite-sign muons; status-1 muon copies; photon origin labels (hard, FSR, from π0/η).
  4. **Angles:** the B is from the pointing fit, the daughters from the vertex fit. The lepton axis and coplanarity are lab-frame. Also the J/ψ mass correction.
  5. **Trigger matching:** distinct HLT objects per leg, menu-dependent filters, trigger availability and prescales.
  6. **Working point:** sideband closure in BDT score slices, the low-sideband partially reconstructed component, the transfer-factor uncertainty, and a likelihood fit for the final sensitivity.
  7. **Normalisation:**
     - the luminosity of the effective trigger OR;
     - the provenance of the cross section and filter efficiency;
     - weighted MC;
     - a J/ψφ normalisation channel.
  8. **PV choice:** good-PV requirement, wrong-PV fraction, PV refit without the candidate tracks; the ct error ignores the PV dependence of the constrained fit.
  9. **Muon selection:** the `mu1_pt > 4 and mu2_pt > 3` cut is charge-ordered (mu1 = μ⁺). To model the trigger thresholds, use leading and subleading pt. Not changed because `analysis/config/analysis.yaml` has your uncommitted edits.
  10. **Fiducial region:** resonance vetoes and q² bins.
  11. **Photon covariance:** a resolution floor instead of rejecting one-crystal-wide clusters (fix 9).

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
