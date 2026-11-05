# Structure of the code (state of `sslawins_dev1` at commit `93fd923`, 2026-11-05)

This document describes the code **as it is now**, before the refactor described in `plan.md`. Line numbers refer to that commit. Paths are relative to the repository root `BsToMuMuGammaAnalysis/`.

---

## 1. Repository layout

```
CMSSW_14_1_1/src/
├── RecoEcal/EgammaCoreTools/            modified CMSSW package (git branch photonKinematicFit_v2)
├── RecoVertex/KinematicFit/             modified CMSSW package
├── RecoVertex/KinematicFitPrimitives/   modified CMSSW package
└── BsToMuMuGammaAnalysis/               this repository
    ├── README.md
    ├── run3mvaid/                       soft-muon MVA ID (XGBoost) – a library, not a plugin
    │   ├── interface/ MuonMVAID.h XGBooster.h CommonTools.h Candidate.h
    │   ├── src/       MuonMVAID.cc XGBooster.cc CommonTools.cc
    │   ├── data/      Run2022-20231030-1731-Event0.{model,features,params,impact}
    │   └── external-tools/ xgboost.xml rabit.xml   (not registered; xgboost.xml points to 0.80)
    └── RadiativeAnalysis/
        ├── interface/   headers for every class below
        ├── plugins/     ALL .cc files (helper classes and EDM modules) -> one plugin library
        ├── test/        cmsRun configs, crab helpers, disabled catch2 tests, test_files.txt
        ├── crab/ crab_mc/ crab_data/   CRAB submission configs
```

The `plugins/BuildFile.xml` builds a single EDM plugin library from every `.cc` in `plugins/`. There is no `src/` library, so the helper classes cannot be unit tested or reused from other packages.

### 1.1 Modified CMSSW packages (needed for photons in the kinematic fit)
These are set up as described in `README.md` (branch `photonKinematicFit_v2` of `sslawins/cmssw`):

| Package | Change |
|---|---|
| `RecoEcal/EgammaCoreTools` | `EcalClusterLazyTools::covariancesXYZ(cluster, w0=4.7)` → `EcalClusterTools::covariancesXYZ`: 3×3 spatial covariance of the cluster |
| `RecoVertex/KinematicFitPrimitives` | `KinematicParticleFactoryFromTransientTrack::particle(..., const reco::Photon* = nullptr, TMatrixD* cov = nullptr)`. `TransientTrackKinematicParticle` keeps **raw, non-owning** pointers to the photon and its cluster covariance (`getPhoton()`, `getPhotonCov()`). `ParticleKinematicLinearizedTrackState::computePhotonJacobians()` adds a measurement model: cluster position (2 coordinates) + energy (+mass) instead of the helix parameters |
| `RecoVertex/KinematicFit` | `FinalTreeBuilder` subtracts 2 ndf per photon |

---

## 2. EDM modules (classes with `DEFINE_FWK_MODULE`)

| Module | File | Status |
|---|---|---|
| `RadiativeAnalysis` | `plugins/RadiativeAnalysis.cc` | **Main ntuplizer**, `edm::one::EDAnalyzer` (SharedResources, WatchRuns, WatchLuminosityBlocks) |
| `MMGVertexFitExample` | `plugins/MMGVertexFitExample.cc` | Old prototype of the photon vertex fit, config `test/MMGVertexFitExample.py`. The reworked version is on `sslawins_miniaod` (see §7) |
| `MVAAnalyzer` | `plugins/MVAAnalyzer.cc` | Standalone check of the muon MVA on gen-matched muons (`test/MVAAnalyzer.py`). Not part of the ntuplization |
| `ReadSoftMvaRun3ValueProducer` | `plugins/ReadSoftMvaRun3ValueProducer.cc` | Despite its name an EDAnalyzer that prints `userFloat("run3muonmva")`. No config uses it |
| `BMMGCustomPhotonProducer` | `plugins/BMMGCustomPhotonProducer.cc` | Stream EDProducer cloned from CMSSW `PhotonProducer`, meant to build low-energy photons. **Does not work.** Its configs (`test/customPhotonConfig*.py`) use unknown parameters and collections that don't exist in AOD |

---

## 3. Data flow of `RadiativeAnalysis`

### 3.1 Constructor / job / run / lumi
- **Constructor** (`RadiativeAnalysis.cc:27-150`):
  - Reads about 50 parameters, among them the PDG masses, mass windows, MVA cuts and the `isMCstudy`/`isMINIAOD` flags.
  - Creates the consume tokens. The track collection is read from the parameter **`pfCandTag`** (`:94`).
  - Some members are hardcoded constants (`nominal*Mass`). Note `nominalPsiMass = 3.096916`, which is the J/ψ mass.
- **`beginJob`**: `new RadiativeRootTree` → `createTree(outputFile)` opens its own `TFile` (no TFileService); `new MuonMVAID(config)`.
- **`beginRun`**: `HLTConfigProvider::init`.
- **`endRun`**: reads `GenRunInfoProduct` → `genXsec_`, keeping the last run only.
- **`endLuminosityBlock`**: reads `GenFilterInfo` → `filterEff_`, overwritten every lumi.
- **`endJob`**:
  1. Computes the lumi equivalent.
  2. Stores it in the event tree and calls `fill()` **again**, which creates an extra fake event row.
  3. Writes the file and deletes the tree and the MVA objects.

### 3.2 `analyze()` step by step (`RadiativeAnalysis.cc:242-1139`)
1. **Reset and event IDs.** `resetEntries()`, then run, event and lumi numbers.
2. **Pile-up (MC only).** `PileupSummaryInfo` for BX = 0.
3. **Event setup and collections.**
   - ES products: B-field, CaloGeometry, `EcalClusterLazyTools` (EB/EE rechits), TransientTrackBuilder.
   - Collections: beamspot, PVs, `reco::Muon`, oniaPhotonCandidates conversions (`pat::CompositeCandidate`), `reco::Track` (generalTracks), PF superclusters (EB), EB rechits, `reco::Photon` (`photons`), `reco::PFCandidate`, and the photon MVA and tight-ID ValueMaps.
   - Only the beamspot and PV handles are checked for validity.
4. **`BeamSpotAndVertex::BSAndVtxObservables`.** BS position and width, PV multiplicity, and the PV position, error, ndof and ρ.
5. **Trigger bits** (`:356-410`). Substring match on 8 HLT paths (`..._v`).
6. **AOD trigger-object matching** (`:417-540`).
   - `getFiltersForPath()` takes the terminal EDFilters of each path from `HLTConfigProvider`.
   - A muon is matched to a path if it is within ΔR < 0.1 of an object in **every** one of those filters.
   - Result: per-muon `vector<int>` branches.
7. **MVA scores.**
   - Muon MVA: `MuonMVAID::produce(muons)`, XGBoost via `run3mvaid`.
   - Photon MVA: from the `photonMVAValueMapProducer` ValueMap.
8. **`MuonSelector::selectMuonPair`.** Opposite-sign global/tracker muons that pass the MVA cut, Kalman dimuon vertex, displacement > 100 µm from the BS. Keeps **one** pair, the one with the best vertex probability.
9. **`PhotonSelector`.**
   - `selectPhoton` and `selectPhotons` return the photon (or pair) closest in ΔR to the dimuon that passes the MVA cut.
   - `selectConvertedPhoton(s)` does the same for conversions.
10. **Vertex builders.** They run in sequence and **each overwrites** `DecayChainVariables decayVariables` and `vertexTypeFlag_`:
    - `TrippleObjectVertex` (μμγ: converted and reco photon), when ≥2 muons and ≥1 photon or conversion.
    - `TetraObjectVertex` (μμγγ), when ≥2 photons or conversions.
    - `ReferenceModeratorVertex` (μμKK, Bs→J/ψφ), when there are exactly 2 muons and ≥2 tracks.

    Each builder also writes many branches **directly** into `RadiativeRootTree`.
11. **Dimuon copy-out** (`:657-699`). About 40 dimuon fields go from `decayVariables` to the tree; whichever builder ran last wins.
12. **`SCRecHitAccumulator`.** EB supercluster and rechit timing variables. A single struct is returned, so only the last hit survives.
13. **Photon variables** (`:753-896`).
    - `RecoPhotons::PhotonObservables` gives the shower shapes of the 2 leading photons **in the event**, not of the selected photon.
    - Photons with pt > 50 or outside EB are excluded.
    - Pre-fit μμγ and μμγγ masses, helicity and coplanarity are computed from `decayVariables` muons and the photons, with Bs/Bd mass-window classification.
    - The `[2]` arrays are indexed by the full photon index.
14. **`ParticleFlowCandidate`** (`:898-962`). Variables of the PF candidates and their photon references. The `[2]` arrays are indexed by the full PF-candidate index.
15. **MC truth** (`:970-1136`).
    - The gen particles are copied into a vector, then `GenDecayChain` runs.
    - If no gen B is found it **returns before `fill()`**, so the event is lost.
    - Fills gen B, μ, γ, J/ψ, decay mode and angles.
    - Reco–gen matching of the selected muons and photons, topology match, and `fillMCInfo` (gen J/ψ).
16. **`bmmgRootTree_->fill()`.**

### 3.3 Dependency graph
```
RadiativeAnalysis ─┬─ RadiativeRootTree (TTree, ~560 branches)
                   ├─ MuonMVAID (run3mvaid) ── XGBooster, CommonTools
                   ├─ BeamSpotAndVertex
                   ├─ MuonSelector
                   ├─ PhotonSelector
                   ├─ TrippleObjectVertex ─┐
                   ├─ TetraObjectVertex ───┼─ KinematicConstrainedFit ── modified RecoVertex
                   ├─ ReferenceModeratorVertex ┘     ├─ ReferenceResonance
                   │        (all three) ─────────────┼─ MuonClosestApproachCalculator
                   │                                 ├─ VariableDefinitions (ct, isolation, compatibility)
                   │                                 ├─ BeamSpotAndVertex (PV choice)
                   │                                 └─ DecayChainVariables (return struct)
                   ├─ SCRecHitAccumulator
                   ├─ RecoPhotons
                   ├─ ParticleFlowCandidate
                   ├─ GenDecayChain (GenChain)
                   └─ GlobalIncludes.h (≈160 includes, ~20 duplicated)
Unused: SPRecHitAccumulator.h (does not compile), BaselineMatching, MMGVertexFit (old), Candidate.h
```

---

## 4. Helper classes

### 4.1 Selection
- **`MuonSelector`** (`MuonSelector.{h,cc}`)
  - API: `selectMuonPair(muons, ttBuilder, beamSpot, mvaScores, mvaCut)` → `pair<vector<reco::Muon>, vector<float>>`.
  - Returns the best opposite-sign pair by Kalman vertex probability.
  - Hardcoded displacement cut: `disp < 0.01` cm from the BS (x0, y0), ignoring the BS slope.
- **`PhotonSelector`** (`PhotonSelector.{h,cc}`)
  - `selectPhoton(s)`: MVA cut, then sorted by ΔR to the dimuon.
  - `selectConvertedPhoton(s)`: refits the conversion e⁺e⁻ with a mass constraint (leaked `new MassKinematicConstraint`), then sorted by ΔR. The two versions are about 80 lines of copied code.
  - Known index bugs: the MVA score is taken with the index into `passingPhotons`, and `convIndex` gets out of step with `convPhotons`.

### 4.2 Vertex builders (one per channel, ~70 % shared code)
All three share a ~150-line **copied dimuon block** that loops over muon pairs and computes:
- the resonance flag (`ReferenceResonance`), the Kalman dimuon vertex, the cowboy flag, cosα, DCA (`MuonClosestApproachCalculator`), the Mahalanobis distance and Lxy;
- the pixel/tracker hits and muon categories (bug: mu2 uses mu1's hit pattern).

- **`TrippleObjectVertex::TrippleObjectVertexObservables(muons, photons, PVs, lazyTools, conversions, bsInfo, bField, muMass, eMass, ttBuilder, tree*)`**, 749 lines.
  - **Converted-photon path:** `KinematicConstrainedFit::TrippleObjectVertexFitConvertedPhoton`, then ct (BS / highest-pT / closest-z / cos-θ PV, plus an "MPV" ct), helicity and coplanarity. Writes `*_mmconvg_` branches, but partly by mistake into `*_mmconvgg_`.
  - **Reco-photon path:**
    - Builds a neutral `FreeTrajectoryState` at the PV with a 1e6 diagonal covariance, then `lazyTools.covariancesXYZ(SC)` (leaked `new TMatrixD`).
    - Calls `KinematicConstrainedFit::TrippleObjectVertexFitRecoPhoton`.
    - Then ct/ct error (`VariableDefinitions`), isolation, max compatibility of the pair with other tracks, and 2D/3D PV–SV distance.
    - Writes `*_mmrecog_` branches.
- **`TetraObjectVertex`**: the same structure for μμγγ (two conversions, or two reco photons). Writes `*_mmconvgg_` and `*_mmrecogg_`.
- **`ReferenceModeratorVertex`**: K⁺K⁻ track pairs from `generalTracks`, ΔR-cleaned against the muons. 4-track Kalman fit, then `KinematicConstrainedFit::BsToJpsiPhiFit`. Writes `*_mmkk_`.

### 4.3 Fitting and vertex tools
- **`KinematicConstrainedFit`**: the fit engine used by all builders.
  - Reco-photon fit pattern:
    1. `KinematicParticleVertexFitter` on μ, μ, γ, with the photon carrying its pointer and cluster covariance (the cluster measurement model is active here).
    2. Every daughter is re-wrapped as a generic `FreeTrajectoryState` track with the refit covariance. **The photon pointer is dropped**, so the cluster model is not used in the next step.
    3. `KinematicConstrainedVertexFitter` with `MultiTrackPointingKinematicConstraint(PV)`, plus a `TwoTrackMassKinematicConstraint` when the dimuon is J/ψ, φ or K*.
  - Other methods: `BsToJpsiPhiFit`, `Tripple/TetraObjectVertexFitConvertedPhoton`, and `doFit` (unused).
  - The constraint-building switch is copied 3 times, and constraints are never deleted.
- **`ReferenceResonance`**: static resonance finder for the dimuon (J/ψ, ψ(2S), φ, K*, non-resonant; the non-resonant window is 0, so that flag is never set). Holds its own copy of the PDG masses.
- **`BeamSpotAndVertex`**
  - `BSAndVtxObservables(bs, PVs)` → struct with BS and PV info.
  - Static PV choosers: `ClosestZindex`, `CosThetaindex`, `LowestDCAIndex`. The last one can return -1, and callers don't check it.
- **`MuonClosestApproachCalculator`**: `calculateDCA(TSCP, TSCP)` → transverse DCA via `ClosestApproachInRPhi`.
- **`VariableDefinitions`**
  - `calculateCt2D/3D` and their errors (PV, KinematicVertex, KinematicParticle).
  - `isolation`, `maxCompatibility`.
  - Its own `MassLimits` with `BsPDGMass = 5.36677`.
- **`DecayChainVariables`** (header only): the return struct of the builders, with about 400 fields. About 300 of them are never written, because the builders write to the tree directly.

### 4.4 Object observables
- **`RecoPhotons::PhotonObservables(photons)`**: shower shapes and SC variables of the 2 leading photons.
- **`ParticleFlowCandidate::PFCandObservables(pfCands)`**: sorts all PF candidates, keeps the leading two, and reads their photon-ref variables.
- **`SCRecHitAccumulator::SCAndRecHitObservables(SCs, rechits, bsInfo, caloGeom)`**: rechit position, time, TOF and corrected time. It overwrites one struct per hit and searches the rechits linearly.
- **`SPRecHitAccumulator.h`**: unused, has a syntax error at line 43.
- **`BaselineMatching`**: muon-chamber matching helpers duplicated from `run3mvaid`. Included, never called.

### 4.5 MC truth
- **`GenDecayChain`** (`GenChain.{h,cc}`)
  - Built from `vector<reco::GenParticle>`. Finds the B with exactly 2 muon descendants and classifies `DecayMode`: μμγ, J/ψγ, J/ψπ0, J/ψη, …, or a π0-gun fallback.
  - Collects the gen photons and computes the helicity, coplanarity and cosθ_l angles.
  - Matching: `matchToGen`, `matchMuonToGen`, `matchPhotonToGen`, `matchTopology`.
  - Several bugs (see plan.md Phase 2): null `mother()`, angles computed before the photons are sorted, the π0 fallback, `nGam==1` vs FSR, Bd labelled as Bs.
- **`RadiativeAnalysis::fillMCInfo`**: gen J/ψ Lxy. Contains a large unused list of B hadron IDs.

### 4.6 Output: `RadiativeRootTree`
- A hand-written class (`.h` 890 lines, `.cc` 2127 lines). Every variable appears in **4 places**: the member, `Branch(...)` in `createTree`, `resetEntries`, and `SetBranchAddress` in `setBranchAddresses`.
- One tree `BMMGTree`, one row per event, **scalars**: one candidate per channel.
- Suffix convention:
  - `_mmrecog_` μμ + reco γ
  - `_mmconvg_` μμ + conversion
  - `_mmrecogg_` μμγγ
  - `_mmconvgg_` μμ + 2 conversions
  - `_mmkk_` μμKK
  - `_beffit_`/`_alone_` dimuon before the fit
  - `Gen*`, trigger, PF, photon and SC/rechit blocks
- Fixed `[2]` arrays for photons and PF candidates; `std::vector<int>*` for the trigger match.
- Run-level info (xsec, filter efficiency, lumi equivalent) is stored as event branches.

### 4.7 `run3mvaid`
- **`MuonMVAID(pset)`**: loads the XGBoost model(s) from `xgboost_models`/`xgboost_variable_names`. `produce(vector<reco::Muon>)` converts each muon to a `pat::Muon`, computes its features and returns the score (`run3muonmva`).
- **`XGBooster`**: wraps the xgboost C API, with no `XGBoosterFree` and no destructor.
- **`MuonMVAID.h`**: has `using namespace std` in the header, which leaks into `RadiativeAnalysis.h`.

---

## 5. Configuration

### 5.1 Parameters of `RadiativeAnalysis`
- **Flags**
  - `isMCstudy`: gates PU info and the gen block. `endRun`/`endLumi` always try to read the gen products and only warn on data.
  - `isMINIAOD`: only switches some trigger/prescale/iso-track tokens. The analyzer can't actually run on MINIAOD because every collection is an AOD type.
- **Collections**
  - `MuonTag` (`muons`), `PhotonTag` (`photons`, the non-GED collection), `pfCandTag` (`generalTracks`, read as `reco::Track`), `PFCandTag` (`particleFlow`).
  - `convertedPhotons` (`oniaPhotonCandidates:conversions`), `pfSupcluster`, `ecalrechitEB/EE/ES`, `primaryvertex`, `vertexBeamSpot`, `triggerbits`, `triggerSummary`, `PUInfo`, `genParticlesLabel`.
  - `mvaValuesMap`, `phoTightIDMap`: VID on `photons`.
- **Cuts and windows**: `muMVACut` (0.83), `photonMVACut` (-0.07), π0/η/η′ windows, J/ψ/ψ/φ windows, Bs/Bd mass windows before and after the fit, `JpsiPtCut`, `KaonTrackPtCut`, plus PDG masses.
- **Muon MVA**: `xgboost_models`, `xgboost_variable_names`.
- **Unused or never read**: `JetTag`, `ElectronTag`, `OOTPhotonTag`, `MuonPtCut`, `phoEnergyCorrMap`, `PsiPDGMass`, `StoreDeDxInfo` (read but unused), `verbose`, `TestVerbose`.
- **`fillDescriptions`**: calls `setUnknown()`, so there is no validation.

### 5.2 Configs in `RadiativeAnalysis/test`
| Config | State |
|---|---|
| `makeTree_BsMuMuGamma_MC_AOD.py` | **The only up-to-date one.** MC, AOD, `auto:run3_mc_FULL`. Path: `egmPhotonIDSequence * oniaPhotonCandidates * bmmgVertexAnalysis * genXSecAnalyzer`. Input: hardcoded list of Run3Summer22EE files |
| `makeTree_BsMuMuGamma_DATA_AOD.py` | Stale: many required parameters are missing, so construction throws; it uses an MC GlobalTag |
| `makeTree_BsMuMuGamma_MC_AOD_Test.py` | Stale (same parameter set as DATA) |
| `makeTree_BsMuMuGamma_MC_MINIAOD.py` | Stale; actually data (Run2023D MiniAOD) |
| `MMGVertexFitExample.py`, `MVAAnalyzer.py`, `genXSec_cfg.py` | Standalone tests |
| `customPhotonConfig*.py` | Broken (BMMGCustomPhotonProducer) |
| `test_files.txt` | List of test input files (AOD, MINIAOD, large filtered AOD sample) |

### 5.3 CRAB
- `crab/` and `crab_mc/` use `makeTree_..._MC_AOD.py`. `crab_data/` uses the broken DATA config.
- All three use the same `requestName`.
- `crab_mc` has a file-list case mismatch (`fileList` vs `FileList`) and an `outLFNDirBase` in another user's area.

---

## 6. Build status
- The package does not build as is. Known blockers:
  - `BMMGCustomPhotonProducer`.
  - Possibly missing BuildFile dependencies (the plugin BuildFile relies on transitive deps for GeneratorProducts, HepMCCandidate, ParticleFlowCandidate, CaloGeometry, VertexTools, HLTcore).
  - Exact errors are to be recorded in `report.md` (plan Phase 0).
- `test/BuildFile.xml` declares catch2 tests whose sources are `.disabled`.

---

## 7. Reworked fit on branch `sslawins_miniaod`: `MMGVertexFit`
`interface/MMGVertexFit.h`, `plugins/MMGVertexFit.cc`, `plugins/MMGVertexFitExample.cc`, `test/MMGVertexFitExample.py`:

- **`ErrorScale { clusterPhi, clusterTheta, energy }`**: scale factors (on sigma) for the cluster position along e_φ and e_θ and for the photon energy, separately for EB and EE. They are tuned on **gedPhotons**. Example config defaults: EB 0.16 / 0.165 / 1.2, EE 0.20 / 0.24 / 1.3.
- **`clusterCovariance(photon, lazyTools)`**: computes `S·covariancesXYZ·S`, with the scaling done perpendicular to the photon direction.
- **`hasClusterHits(photon)`**: false when the SC has no rechits (MiniAOD photons that fail the reducedEgamma selection).
- **`photonEnergyError(photon)`**: the scaled `getCorrectedEnergyError`.
- **`fit(vector<const reco::Muon*>, vector<const reco::Photon*>, lazyTools, PVs, ttBuilder)` → `vector<Result{particle, vertex}>`**, for all μμγ combinations:
  1. A Kalman `KinematicParticleVertexFitter` on μ, μ, γ, with the photon entering through its cluster position and energy.
  2. A `PointingKinematicConstraint(PV)` applied on the top particle by `KinematicParticleFitter`. It uses the full B covariance, including the daughter correlations.
  - Results with an invalid vertex, a non-finite χ² or a non-positive mass are dropped.
  - Photons are `clone()`d (so `pat::Photon` works) and kept alive together with their covariances (`unique_ptr`) for the duration of the fit.
  - Limitations for use in the analyzer: the PV is always `primaryVertices[0]`, and a `Result` does not record which muons and photon it came from.
- **`MMGVertexFitExample`**: gen-matches muons and photons, runs the fit, and makes mass/pull/vertex validation plots. It works for AOD and MINIAOD (selected by `dataFormat` in the config).
- The `sslawins_dev1` version of `MMGVertexFit` is an older prototype: it returns only the masses, leaks memory, and the example's EE token reads EB rechits.
