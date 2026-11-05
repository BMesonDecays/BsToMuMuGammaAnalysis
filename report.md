# Progress report

Entries are newest first. Each entry gives: the plan item(s) (see `plan.md`), what was done, the build result, the validation result, and open issues.

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
