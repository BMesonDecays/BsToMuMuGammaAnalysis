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
