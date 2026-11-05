# Improvement plan for BsToMuMuGammaAnalysis / RadiativeAnalysis

The current structure is described in `structure_old.md`; progress is logged in `report.md`.

## Working rules
- Work on `sslawins_dev1`. **Commit regularly**: one commit per plan item or small group of related fixes. The commit message references the item (e.g. `[P2.3] fix mu2 pixel hits`).
- After every item, add an entry to **`report.md`**: date, what was done, build result, validation result, and open issues. Commit it together with the change.
- After every step: `scram b -j8`, then run on the reference file and compare with the reference ntuple (P0.5). Variables that are not supposed to change must stay identical.
- Test inputs are listed in `RadiativeAnalysis/test/test_files.txt`, maintained by the user.

## Decisions already taken
| Topic | Decision |
|---|---|
| Data format | Port MMGVertexFit now, keep the analyzer **AOD-only**; MINIAOD support later (Phase 8) |
| Ntuple | Free to redesign; downstream scripts will be adapted |
| Candidates | Store **all** candidates as vector branches; best-candidate selection offline |
| Channels, first iteration | **Bs→μμγ with an ECAL photon** only; conversions, μμγγ (η) and μμKK later (Phase 7) |
| Default PV | The PV with the **smallest impact parameter** of the B candidate (lowest DCA) |
| Muon displacement cut (100 µm from the BS) | **Removed** |

## Open questions (to be decided by the user when the relevant step comes)
1. ~~Photon collection~~ **Decided: gedPhotons** (2026-11-05).
2. The photon exclusion `pt > 50 GeV || !isEB` in the analyzer: keep, make configurable, or drop?
3. ~~FSR in the gen classification~~ **Decided in P2.18:** extra photons are treated as FSR (`nGam >= 1` / `>= 2`).
4. ~~Trigger-object matching~~ **Resolved in P2.14:** muons are matched (ΔR < 0.1) only to HLT muon objects of the terminal filters; filters without muon objects (e/γ legs) are skipped.
5. A data test file for `test_files.txt` (needed in Phase 6).

---

## Phase 0: Make it build, and a baseline
- [x] **P0.1** Run `scram b -j8` and record every compile error and warning in `report.md`.
- [x] **P0.2** Move `BMMGCustomPhotonProducer.{h,cc}` and `test/customPhotonConfig*.py` to `RadiativeAnalysis/devel/` (not compiled).
- [x] **P0.3** Delete `interface/SPRecHitAccumulator.h`: it is unused and has a syntax error at line 43.
- [x] **P0.4** Fix the BuildFiles:
  - In `plugins/BuildFile.xml`, add `SimDataFormats/GeneratorProducts`, `DataFormats/HepMCCandidate`, `DataFormats/ParticleFlowCandidate`, `Geometry/CaloGeometry`, `RecoVertex/VertexTools` and `HLTrigger/HLTcore`.
  - In `run3mvaid/BuildFile.xml`, add `FWCore/ParameterSet` and `DataFormats/PatCandidates`.
  - Disable the empty catch2 target in `test/BuildFile.xml`.
  - Apply whatever other minimal fixes P0.1 shows are needed.
- [x] **P0.5** Make the reference ntuple:
  - `makeTree_BsMuMuGamma_MC_AOD.py` reads its input from `test_files.txt` (via a `inputFiles` option); the hardcoded list is removed.
  - Run on `/eos/user/s/sslawins/testing_mc/BsTo2MuG_aod.root` and keep the output as the regression reference.
  - Write a small comparison script (`test/compareNtuples.py`, PyROOT) that compares the branches common to two ntuples.

## Phase 1: Port the new kinematic fit (`MMGVertexFit` from `sslawins_miniaod`)
- [x] **P1.1** `git checkout sslawins_miniaod -- RadiativeAnalysis/interface/MMGVertexFit.h RadiativeAnalysis/plugins/MMGVertexFit.cc RadiativeAnalysis/plugins/MMGVertexFitExample.cc RadiativeAnalysis/test/MMGVertexFitExample.py`. Build against `../RecoVertex` (`photonKinematicFit_v2`) and run the example on the AOD test file.
- [x] **P1.2** Extend the API for use in the analyzer, keeping the existing `fit()` as a wrapper so the example keeps working:
  - Add `std::optional<Result> fitCandidate(const RefCountedKinematicParticle& mu1, const RefCountedKinematicParticle& mu2, const reco::Photon& photon, const EcalClusterLazyTools&, const GlobalPoint& pv)`, or an equivalent.
  - Add a helper that builds the muon kinematic particles once per event.
  - Add the indices (mu1, mu2, photon) to `Result`.
  - Pass the PV explicitly instead of `primaryVertices[0]`.
- [x] **P1.3** Replace `KinematicConstrainedFit::TrippleObjectVertexFitRecoPhoton` in the reco-photon path of `TrippleObjectVertex` with `MMGVertexFit`.
  - This also removes the leaked `new TMatrixD` (`TrippleObjectVertex.cc:634`), the 1e6 placeholder covariance, and the problem that the old constrained refit dropped the photon cluster model.
  - The fitted B and vertex then feed the existing ct, isolation and compatibility code.
  - The PV used in the pointing constraint is the lowest-DCA PV. The fit has to be done before the B direction is known, so take the PV with the lowest DCA to the pre-fit μμγ direction from the dimuon vertex, and record that choice in `report.md`.
- [x] **P1.4** Add the ErrorScale factors as parameters of `RadiativeAnalysis`: `clusterCovScalePhiEB/EE`, `clusterCovScaleThetaEB/EE`, `energyErrorScaleEB/EE`, with defaults from `MMGVertexFitExample.py`.
- [x] **P1.5** Mark `KinematicConstrainedFit` as deprecated. It stays only for conversions, μμγγ and μμKK until Phase 7. Delete the old `MMGVertexFit` code paths and `doFit`.
- [x] **P1.6** Validate on the large filtered sample: mass resolution, vertex pulls and ct, compared with the reference ntuple and with `MMGVertexFitExample`. Put the plots and numbers in `report.md`.

## Phase 2: Obvious local bug fixes
Each item is a small change, and they are committed in small groups.

**Out-of-bounds writes and crashes**
- [x] **P2.1** `photon*_[2]` arrays are written with the full photon index (`RadiativeAnalysis.cc:768-846`), and `pfCand*_[2]` with the full PF index (`:908-958`). Both corrupt memory. Use a separate counter and stop at 2; Phase 5 replaces them with vectors anyway.
- [x] **P2.2** `photonSCBrem` divides by `scEtaWidth`, which can be 0. `VariableDefinitions.cc:61,92` divide by `transmom` before the zero check.
- [x] **P2.3** (new module: `event.get()` throws a clear exception for missing required products; optional products, i.e. VID maps and trigger summary, are checked) (deferred to P4.1: an invalid handle already throws a clear `ProductNotFound` exception when dereferenced, so there is no crash; P4.1 adds explicit checks) Check `getByToken` handles for validity before use (muons, photons, tracks, conversions, PF candidates, SC, rechits, trigger results).
- [x] **P2.4** `BeamSpotAndVertex`:
  - `vertex.front()` and `vertex[VtxIndex]` are used before the empty and `-99` checks.
  - `LowestDCAIndex`, `ClosestZindex` and `CosThetaindex` can return -1, and the callers index `PVs[-1]`. They should return `std::optional`.
  - `pv_covariance`, `pv_ndof` and `pv_rho` come from the last vertex in the loop, not the selected one.
  - `bs_dx`, `bs_dy` and `bs_dz` are never filled.
- [x] **P2.5** `GenChain.cc:53`: `mother()` is dereferenced without a null check.

**Wrong values**
- [x] **P2.6** `pp2 = muTrack1->hitPattern()` should use muTrack2: `TrippleObjectVertex.cc:200`, `TetraObjectVertex.cc:156`, `ReferenceModeratorVertex.cc:169`.
- [x] **P2.7** Photon η cuts without `fabs` (`TrippleObjectVertex.cc:615,617`, TetraObjectVertex too). The `isEE()` veto at `TrippleObjectVertex.cc:661` comes after the fit inputs are built; move it before.
- [x] **P2.8** `PhotonSelector`:
  - The MVA score is taken as `photonMVAIDs[idx]` with `idx` indexing `passingPhotons` (`:71,129-130`).
  - `convIndex` and `convPhotons` get out of step after a failed conversion fit (`:164/223`, `:249/316`).
  - Fix both by carrying the original indices.
- [x] **P2.9** `TrippleObjectVertex` writes `_mmconvgg_` instead of `_mmconvg_` (`:466,526,544,556-557`). `TetraObjectVertex.cc:48-51` fills the J/ψ branches for every resonance flag.
- [x] **P2.10** Fields with the wrong source:
  - `BsVtxProb_mmrecog_` and `BsCt2DBS_mmrecog_` are taken from the dimuon Kalman vertex instead of the B vertex (`TrippleObjectVertex.cc:652-685`, and the same in `ReferenceModeratorVertex.cc:286`).
  - The PV position (closest-z) is combined with the error of the highest-pT PV (`TrippleObjectVertex.cc:448/457`).
  - `AlgebraicMatrix31` is indexed as `(0,i)` (`:447-455,506-514`).
- [x] **P2.11** Dimuon block:
  - The `isPFMuon()` category overwrites the computed category.
  - `isCowboy` is never reset; use `atan2`.
  - `muon::overlap` is checked after the expensive computations; move it first.
- [x] **P2.12** `ReferenceResonance`:
  - `calculateMassFromComponents` shadows a variable and always returns 0.
  - The `NonResonant` window is 0, so the flag is never set.
  - `ReferenceModeratorVertex.cc:197` uses resonance eta/phi = -9999 when no resonance was found.
- [x] **P2.13** Constants:
  - `nominalPsiMass = 3.096916` should be 3.6861.
  - `PsiPDGMass` is never read from the config.
  - `KinematicConstrainedFit::vtxprob_Bhadron` and `mass_Bhadron` are uninitialised.
  - `genXsec_`, `genXsecErr_`, `lumiEquivalent_*` and `NSelectedVertices` are uninitialised.
- [x] **P2.14** Trigger:
  - `HLT_DoubleMu4_3_LowMass` is stored in `triggerbit_HLT_DoubleMu4_LowMass_`.
  - `HLT_DoubleMu4_3_Jpsi` is in the list but has no bit.
  - Check the "all terminal filters" matching against the menu (open question 4).
- [x] **P2.15** The track collection is read from the parameter `pfCandTag`; add its own `tracks` parameter.

**MC and bookkeeping**
- [x] **P2.16** `if (!G.isValid) return;` (`RadiativeAnalysis.cc:982`) drops MC events from the ntuple. Fill the event with gen flags set to invalid.
- [x] **P2.17** `endJob` calls `fill()` a second time, creating a fake event row. Write the run-level info into a separate `RunInfo` tree. `filterEff_` and `genXsec_` must combine all lumis and runs (weighted), not keep the last one.
- [x] **P2.18** `GenChain`:
  - Angular variables are computed before `sortPhotonsByPt()`.
  - The π0-gun fallback takes the first π0 in the event.
  - Bd 4-body modes are labelled `kBs_*`.
  - `matchTopology` requires a grandmother of 531, which is wrong for direct B→μμγ.
  - `processDaughters` has no `!muPlus_` guard.
  - `matchToGen` has no final-state/status requirement.
- [x] **P2.19** Topology matching uses `(*photons)[0..1]` instead of the selected photons (`RadiativeAnalysis.cc:1095-1100`).
- [x] **P2.20** `bool photonTightID_[2]` is booked as `"photonTightID[2]/I"` (`RadiativeRootTree.cc:519`), so ROOT reads 8 bytes from a 2-byte array: garbage, and the values change from run to run. Book it as `/O`, or store it as int. It was found by the P0.5 determinism check.

## Phase 3: Ownership, hygiene, conventions
- [x] **P3.1** (for the new `RadiativeAnalysis` code; the legacy module is not changed further and is deleted in Phase 7) Replace every `new` without `delete`:
  - The 5 per-event `new vector<>` in `analyze` (`:579-623`).
  - Constraints in `KinematicConstrainedFit` and `PhotonSelector`.
  - `TMatrixD` covariances.
  - `RadiativeRootTree` and `MuonMVAID` become `std::unique_ptr` members.
- [x] **P3.2** `XGBooster`: add a destructor with `XGBoosterFree`, delete copy, allow move. `MuonMVAID`: no `using namespace std` in the header.
- [x] **P3.3** (for the new `RadiativeAnalysis` code; the legacy module is not changed further and is deleted in Phase 7) Switch output to `TFileService` and call `usesResource(TFileService::kSharedResource)`.
- [x] **P3.4** (for the new `RadiativeAnalysis` code; the legacy module is not changed further and is deleted in Phase 7) Write a real `fillDescriptions` with every parameter and its default. Remove unused parameters and tokens: Jet, Electron, OOT photons, OOT SC, `MuonPtCut`, `phoEnergyCorrMap`, `StoreDeDxInfo`, `TestVerbose`, and the copied `PackedTriggerPrescales`.
- [x] **P3.5** (for the new `RadiativeAnalysis` code; the legacy module is not changed further and is deleted in Phase 7) Create a single `interface/Constants.h` (namespace `bmmg`) with PDG masses and fit sigmas. Remove the copies (`MassLimits` ×3, `MuMass`, `nominal*Mass`, `kaonmass`…) and the three different Bs masses (5.36677, 5.3699, 5.36689).
- [x] **P3.6** (for the new `RadiativeAnalysis` code; the legacy module is not changed further and is deleted in Phase 7) Remove `GlobalIncludes.h`: each file includes what it uses. Remove duplicated and unused includes and `using namespace` in headers.
- [x] **P3.7** (dead code is not carried into the new code; the legacy classes are deleted in Phase 7) Remove dead code:
  - Declared-but-undefined analyzer methods: `MCmatching*`, `reVertex*`, `CalculateCtErr*`, `sel*Muon`, `MuonChargeCone*`, `FindMuon*`, `LookForMother*`.
  - `listOfBmesonIds`, `doFit`, `BaselineMatching`, unused counters, commented-out blocks.
  - The ~300 never-filled `DecayChainVariables` fields.
- [x] **P3.8** Move `MVAAnalyzer` and `ReadSoftMvaRun3ValueProducer` to `devel/`, or fix them as tests. `MVAAnalyzer` builds `MuonMVAID` every event.

## Phase 4: Restructure (core of the plan) — done (new `RadiativeAnalysis`, see report.md)
The goal is a thin analyzer that orchestrates small single-purpose components. Move gradually, one component per commit, comparing with the reference ntuple each time.

**Target layout** (CMSSW conventions):
```
RadiativeAnalysis/
  interface/  + src/   library (own BuildFile.xml), unit-testable
  plugins/             EDM modules only (RadiativeAnalysis, MMGVertexFitExample)
  python/              radiativeAnalysis_cfi.py (defaults), collections per data format
  test/                run configs, catch2 tests, compareNtuples.py, test_files.txt
  devel/               non-compiled work in progress (custom photon producer)
```

**Components**
| # | Component | Replaces | Responsibility |
|---|---|---|---|
| P4.1 | `EventInputs` | handle retrieval in `analyze` | Gets all handles and ES products once, with validity checks |
| P4.2 | `PrimaryVertexTools` | `BeamSpotAndVertex` | BS/PV summary. PV choice for a candidate, **default lowest DCA**, with alternatives as optional variables. Returns `optional` |
| P4.3 | `MuonSelector` | current class | **All** muons that pass ID/MVA (indices + MVA score). No displacement cut |
| P4.4 | `DimuonBuilder` | 3 copied dimuon blocks | All OS pairs → `DimuonCandidate` (Kalman vertex, prob, Lxy, cosα, DCA, cowboy, resonance flag, muon indices) |
| P4.5 | `PhotonSelector` | current class | All photons that pass the cuts (indices + MVA + tight ID); conversions later |
| P4.6 | `MMGVertexFit` | `KinematicConstrainedFit` | The fit engine; extension point for Phase 7 |
| P4.7 | `CandidateObservables` | `VariableDefinitions` + ct code in the builders | Pure functions: ct2D/3D and their errors, Lxy, PV–SV distances, isolation, max compatibility, helicity, coplanarity |
| P4.8 | `MuMuGammaBuilder` | `TrippleObjectVertex` (reco-photon part) | dimuons × photons → fit → observables → `vector<MMGCandidate>`, with indices of mu1, mu2, photon, dimuon |
| P4.9 | `TriggerMatcher` | trigger code in `analyze` | Path bits, per-muon matches |
| P4.10 | `GenTruth` | `GenChain` + `fillMCInfo` | Event-level truth, per-object and per-candidate truth match |
| P4.11 | `NtupleWriter` | `RadiativeRootTree` | Phase 5 |
| P4.12 | `RadiativeAnalysis` | 1400-line analyzer | ~150 lines: call the components, hand the results to the writer |

Rules for the components:
- Plain structs in and out.
- No component writes into the tree directly; only `NtupleWriter` does.
- Constants come from `Constants.h`.

## Phase 5: Ntuple redesign
- [x] **P5.1** `BranchSet` helper: a variable is declared **once** (name, storage, default/reset) and creates its branch. This removes the four hand-written lists (members, `Branch`, `resetEntries`, `SetBranchAddress`). `readTree`/`setBranchAddresses` are removed; reading belongs to the offline code.
- [x] **P5.2** Collections of vector branches with a consistent `prefix_name` scheme:
  - `evt_*` (ids, PU), `bs_*`, `pv_*`, `trg_*`.
  - `mu_*` (selected muons: kinematics, ID, MVA, trigger match, hits).
  - `pho_*` (selected photons: kinematics, shower shapes, MVA, tight ID, SC/rechit timing).
  - `dimu_*`.
  - `mmg_*` (all fitted candidates: pre-fit and fitted kinematics, vertex prob, ct2D/3D + errors, Lxy, isolation, compatibility, helicity, coplanarity, chosen PV index, links `mmg_mu1_idx`, `mmg_mu2_idx`, `mmg_pho_idx`, `mmg_dimu_idx`, truth match).
  - `gen_*`.
- [x] **P5.3** `RunInfo` tree with xsec, filter efficiency, nEvents and lumi equivalent.
- [x] **P5.4** (decision: the leading-two photon / PF-candidate blocks and the full rechit dump are not carried over; per selected photon the seed time `pho_seedTime` is stored. They can be added back behind flags if needed) Heavy optional blocks behind config flags: `storePFCandidates`, `storeRecHits`. Fix `SCRecHitAccumulator` to fill per-hit vectors and use `recHits.find()`.
- [x] **P5.5** Document every branch in `NTUPLE.md`, and give downstream scripts a mapping from old to new names.

## Phase 6: Configuration and job submission
- [x] **P6.1** `python/radiativeAnalysis_cfi.py` with all defaults, generated from or matching `fillDescriptions`.
- [x] **P6.2** One driver `test/runNtuplizer_cfg.py`:
  - VarParsing options: `isMC`, `inputFiles` (or a key from `test_files.txt`), `maxEvents`, `outputFile`, `globalTag`.
  - Collections from a dictionary keyed by data format, as in `MMGVertexFitExample.py`.
  - Data uses a data GlobalTag and a JSON lumi mask.
- [x] **P6.3** Remove the stale `makeTree_*` configs: DATA_AOD, MC_AOD_Test, MC_MINIAOD.
- [x] **P6.4** One parametrised CRAB config replacing `crab/`, `crab_mc/` and `crab_data/`. This fixes the shared `requestName`, the file-list name case and the foreign `outLFNDirBase`.
- [x] **P6.5** Update `README.md`: setup with the RecoVertex/RecoEcal branch, how to run, the documentation files.

## Phase 7: Other channels on the new framework
- [x] **P7.1** Bs→μμγ with a converted photon (γ→e⁺e⁻):
  - Extend `MMGVertexFit` with a photon built from a track pair, i.e. an e⁺e⁻ sub-fit with a mass/collinearity constraint.
  - Add `MuMuConvGammaBuilder`; reuse `DimuonBuilder` and `CandidateObservables`.
- [x] **P7.2** Bs→J/ψη(γγ), μμγγ: extend `MMGVertexFit` to N photons, with an optional J/ψ mass constraint on the dimuon and an optional η mass window.
- [x] **P7.3** Bs→J/ψφ (μμKK) normalisation channel with the same builder pattern.
- [x] **P7.4** Remove `KinematicConstrainedFit`, `TrippleObjectVertex`, `TetraObjectVertex`, `ReferenceModeratorVertex`, `ReferenceResonance` (reduced to a resonance flag in `DimuonBuilder`) and `DecayChainVariables`.

## Phase 8: MINIAOD support
- [x] **P8.1** Read `edm::View<reco::Muon>` and `edm::View<reco::Photon>` and pass pointers on (MMGVertexFit already supports `pat::` objects). Skip photons where `hasClusterHits()` is false.
- [x] **P8.2** Tracks and isolation from `packedPFCandidates`/`lostTracks`; rechits from `reducedEgamma`.
- [x] **P8.3** Trigger via `TriggerObjectStandAlone` (`unpackPathNames`); muon MVA on `pat::Muon`.
- [x] **P8.4** Validate on the MINIAOD file from `test_files.txt` against the AOD output of the same events.

## Phase 9: Custom low-energy photon (low priority)
- [ ] **P9.1** Revive `BMMGCustomPhotonProducer` from `devel/`:
  - `PhotonCollection_` is never set.
  - `hits` and `preselCutValues` are not reset per SC.
  - The full rechit collections are copied every event.
  - `fillDescriptions` is incomplete.
  - `minSCEt` of 5/15 GeV contradicts the low-energy goal.
- [ ] **P9.2** Fix `customPhotonConfig_cfi.py`: collections that don't exist in AOD, and the EE severity copied from EB. Hook the producer into the driver config as an option.
