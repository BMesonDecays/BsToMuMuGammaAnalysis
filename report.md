# Progress report

Entries are newest first. Each entry gives: the plan item(s) (see `plan.md`), what was done, the build result, the validation result, and open issues.

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
