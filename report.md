# Progress report

Entries are newest first. Each entry gives: the plan item(s) (see `plan.md`), what was done, the build result, the validation result, and open issues.

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
