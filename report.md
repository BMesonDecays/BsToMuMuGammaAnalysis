# Progress report

Entries are newest first. Each entry gives: the plan item(s) (see `plan.md`), what was done, the build result, the validation result, and open issues.

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
