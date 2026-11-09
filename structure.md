# Structure and flow of the analysis code

This document describes the current `RadiativeAnalysis` ntuplizer: its code layout, the processing flow of one event, and **every cut applied to objects and candidates**. The branches of the output are documented in [NTUPLE.md](NTUPLE.md). The code before the restructuring is described in [structure_old.md](structure_old.md).

---

## 1. Repository layout

```
CMSSW_14_1_1/src/
├── RecoVertex/KinematicFit, KinematicFitPrimitives   modified CMSSW (photon measured by its ECAL cluster)
├── RecoEcal/EgammaCoreTools                          modified CMSSW (cluster position covariance covariancesXYZ)
└── BsToMuMuGammaAnalysis/
    ├── run3mvaid/                   Run-3 soft-muon MVA (XGBoost), library
    └── RadiativeAnalysis/
        ├── interface/ + src/        library BsToMuMuGammaAnalysisRadiativeAnalysis (namespace bmmg)
        ├── plugins/                 EDM modules: RadiativeAnalysis, MMGVertexFitExample, BMMGCustomPhotonProducer
        ├── python/                  testFiles.py (reads test/test_files.txt)
        ├── test/                    configs, unit tests, validation and regression scripts, test_files.txt
        ├── crab/                    crabConfig.py, makeFileList.py
        └── devel/                   not compiled: old debugging analyzers
```

### 1.1 Library components (`interface/`, `src/`)
| file | content |
|---|---|
| `Constants.h` | PDG masses (2024) and dimuon mass windows: the only place these numbers are defined |
| `Candidates.h` | plain data structs passed between the components: `SelectedMuon`, `SelectedPhoton`, `SelectedConversion`, `SelectedTrack`, `PVTrack`, `DimuonCandidate`, `FlightInfo`, `BCandidate` |
| `ObjectSelection.{h,cc}` | preselection of muons, photons, conversions and kaon tracks; PV-track lists for AOD and MiniAOD |
| `DimuonBuilder.{h,cc}` | opposite-sign muon pairs with a Kalman vertex fit and dimuon observables |
| `MMGVertexFit.{h,cc}` | kinematic fit: vertex fit of the daughters, then a pointing constraint to the PV. Photons enter through their ECAL cluster; conversions as an e⁺e⁻ refit; tracks with a mass hypothesis |
| `CandidateBuilders.{h,cc}` | B candidates of the four channels (μμγ, μμγ conversion, μμγγ, μμKK): PV choice, fit, observables |
| `CandidateObservables.{h,cc}` | pure functions: PV quality and choice, flight distance and ct with errors, angles, isolation, track compatibility, track DCA |
| `TriggerMatcher.{h,cc}` | HLT path decisions and muon–trigger-object matching (AOD and MiniAOD) |
| `GenChain.{h,cc}` | `GenDecayChain`: generator decay of the signal B, decay-mode classification, truth matching |
| `BranchSet.h`, `NtupleWriter.{h,cc}` | ntuple: each variable is booked once; trees `Events` and `RunInfo` |

### 1.2 EDM modules (`plugins/`)
| module | purpose |
|---|---|
| `RadiativeAnalysis` | the ntuplizer (§2) |
| `MMGVertexFitExample` | standalone validation of the photon kinematic fit on gen-matched μ, μ, γ (pulls, resolutions) |
| `BMMGCustomPhotonProducer` | work in progress: photons with lower thresholds (a clone of the standard `PhotonProducer`) |

### 1.3 Configurations (`test/`)
| file | purpose |
|---|---|
| `runNtuplizer_cfg.py` | driver config: MC/data, AOD/MiniAOD (collection names per format), photon ID (VID), channel selection |
| `MMGVertexFitExample.py` | config of the fit validation module |
| `customPhotons_cfg.py` | custom photon producer + ntuplizer on its photons |
| `test_files.txt` | test input samples (used by the configs through `python/testFiles.py`) |
| `test_bmmgLibrary.cc` | catch2 unit tests (`scram b runtests`) |
| `compareNtuples.py`, `validateMMGFit.py`, `checkDecayModes.py` | regression check between two ntuples; fit validation (mass and ct resolution, pulls); per-decay-mode consistency check (gen classification, candidates) |

---

## 2. Processing flow

### 2.1 Job level
```
cmsRun runNtuplizer_cfg.py
  ├─ (photon ID)  egmPhotonIDSequence: VID on gedPhotons / slimmedPhotons → MVA score and MVA wp90 ValueMaps
  └─ RadiativeAnalysis
       beginJob            NtupleWriter books the trees (TFileService directory "bmmg/")
       beginRun            TriggerMatcher reads the HLT menu (HLTConfigProvider)
       analyze             per event, §2.2
       endLuminosityBlock  MC: merges GenFilterInfo (filter efficiency) over all lumi blocks
       endRun              MC: inverse-variance mean of the generator cross section over the runs
       endJob              one RunInfo entry: xsec, filter efficiency, nEvents, nEventsStored, lumi equivalent
```

### 2.2 One event (`RadiativeAnalysis::analyze`)
**No event-level selection by default:** every processed event gets one entry in `Events`. With the skim switch (`skim=1`, §3.9) only events with a valid B candidate are written.

```
1. event ids, beam spot, primary vertices                 → evt_*, bs_*, pv_* (first PV of the collection)
2. PV-track list (isolation, track compatibility)
      AOD: tracks of each PV fit;  MiniAOD: packed candidates / lost tracks used in the PV fit
3. MC: pile-up, GenDecayChain (signal B, decay mode)       → evt_nPU, gen_*
4. muon MVA for all muons (run3mvaid)
5. object preselection (§3.1, §3.2)                        → selected muons, selected photons
6. trigger: path decisions, muon matching (§3.6)           → trg_*, mu_trg_*
7. dimuons from the selected muons (§3.3)
8. write muons, photons, dimuons (with gen matches in MC)  → mu_*, pho_*, dimu_*
9. B candidates per enabled channel (§3.4, §3.5):
      mmg   dimuon × photon                 → mmg_*
      mmgg  dimuon × photon pair            → mmgg_*
      mmcg  conversions (§3.2) → dimuon × conversion   → conv_*, mmcg_*      (AOD only)
      mmkk  kaon tracks (§3.2) → J/ψ dimuon × K⁺K⁻      → trk_*, mmkk_*
10. skim (if enabled, §3.9): no valid candidate → the event is not written
    (with the skim, step 9 is skipped already when no dimuon has a valid vertex)
11. fill the event
```

The muon, photon, conversion and track collections are written **before** the candidates. Each candidate's truth flag (`<p>genMatched`) is the AND of the gen matches of its legs.

### 2.3 Candidate building (common to all channels: `CandidateBuilders`)
For every combination of a dimuon and the other legs:
1. **Pre-fit four-vector:** the sum of the reconstructed momenta. A muon has the PDG muon mass; a photon its energy; a conversion its refitted momentum; a kaon track the kaon mass.
2. **Cuts on the combination:** the leg cuts and the pre-fit B mass window (§3.4).
3. **PV choice:** the primary vertex closest to the line through the dimuon vertex along the pre-fit B momentum (lowest DCA). `<p>pv` is its index.
4. **Kinematic fit** (`MMGVertexFit::fitParticles`):
   - a vertex fit of all daughters (`KinematicParticleVertexFitter`); then
   - a pointing constraint of the B to the chosen PV (`PointingKinematicConstraint`).

   An ECAL photon is measured by its cluster position (covariance from `covariancesXYZ`, scaled along e_φ and e_θ) and its energy (scaled energy error). A failed or diverging fit (§3.5) gives `fitOK = 0`, **but the candidate is kept**.
5. **Observables:**
   - mass and mass error;
   - flight distance and ct (3D and 2D, signed projection on the momentum, error from the full covariance of the fitted state and the PV);
   - cos α;
   - helicity, coplanarity and cos θ_ℓ;
   - isolation and max track compatibility.

   They use the fitted momenta, or the pre-fit ones when the fit failed.

---

## 3. Cuts

There is no event selection, and **no MVA, ID or trigger requirement** is applied by default. Those quantities are stored and the analysis cuts are applied offline. The cuts below are loose preselections (configurable parameter in brackets, default value given) and technical requirements of the reconstruction.

### 3.1 Muons (`selectMuons`)
| requirement | value |
|---|---|
| type | global **or** tracker muon |
| inner track | must exist |
| pt | > 2.0 GeV (`muonMinPt`) |
| \|η\| | < 2.4 (`muonMaxAbsEta`) |
| Run-3 soft-muon MVA | ≥ −99, i.e. no cut (`muonMinMVA`); the legacy analysis used 0.83 |

### 3.2 Photons, conversions, kaon tracks
**Photons** (`selectPhotons`; `gedPhotons` in AOD, `slimmedPhotons` in MiniAOD)
| requirement | value |
|---|---|
| supercluster | must exist |
| E_SC | > 1.0 GeV (`photonMinSCEnergy`) |
| \|η_SC\| | < 2.5 (`photonMaxAbsSCEta`); EB and EE both kept |
| photon MVA | ≥ −99, i.e. no cut (`photonMinMVA`); the legacy analysis used −0.07 |
| for the fit only | the supercluster must have rechits (`pho_hasClusterHits`; can fail in MiniAOD) |

In practice the photon collections have their own reconstruction thresholds. `gedPhotons` start at a few GeV; `slimmedPhotons` at about 10 GeV.

**Conversions** (`selectConversions`; `allConversions`; channel `mmcg`, AOD only)
| requirement | value |
|---|---|
| number of tracks | = 2, tracks available in the event |
| charges | opposite |
| conversion vertex | valid, χ² probability > 1e-6 (`conversionMinVertexProb`) |
| pair momentum pt | > 0.5 GeV (`conversionMinPt`) |
| e⁺e⁻ refit | vertex fit with a zero-mass constraint; if it fails the conversion is kept (`conv_fitOK = 0`) but cannot be used in a B fit |

**Kaon tracks** (`selectKaonTracks`; `generalTracks` in AOD, pseudo-tracks of `packedPFCandidates` + `lostTracks` in MiniAOD; channel `mmkk`)
| requirement | value |
|---|---|
| track quality | high purity |
| pt | > 0.7 GeV (`kaonMinPt`) |
| \|η\| | < 2.5 |
| not a muon | not the inner track of a selected muon (ΔR < 5·10⁻³ and \|Δpt/pt\| < 1 %, same charge) |

### 3.3 Dimuons (`buildDimuons`)
| requirement | value |
|---|---|
| muons | two selected muons |
| charges | opposite |
| segments | the muons must not share muon segments (`muon::overlap`) |
| vertex | a Kalman vertex fit is attempted; pairs with an invalid vertex are kept in `dimu_*` (`dimu_vtxValid = 0`) but **not used for B candidates** |

Mass-region flag `dimu_resonance` (no cut): J/ψ 2.9–3.3 GeV, ψ(2S) 3.55–3.8 GeV, φ 0.97–1.07 GeV, otherwise non-resonant.

### 3.4 B candidates
| channel | legs | cuts on the combination |
|---|---|---|
| `mmg` (B → μμγ, ECAL photon) | dimuon + photon | dimuon vertex valid; pre-fit m(μμγ) in [4.0, 7.0] GeV (`minPrefitMass`, `maxPrefitMass`) |
| `mmcg` (B → μμγ, conversion) | dimuon + conversion | dimuon vertex valid; pre-fit m(μμγ) in [4.0, 7.0] GeV |
| `mmgg` (B → μμγγ, e.g. J/ψη) | dimuon + photon pair | dimuon vertex valid; m(γγ) in [0.05, 1.1] GeV (`diphotonMinMass`, `diphotonMaxMass`); pre-fit m(μμγγ) in [4.0, 7.0] GeV |
| `mmkk` (B_s → J/ψφ, normalisation) | dimuon + K⁺K⁻ | dimuon vertex valid; dimuon in the J/ψ window (`kkJpsiOnly`); kaon ΔR to the dimuon < 1.5; kaon \|Δz\| to the dimuon vertex < 1 cm; kaons not within ΔR < 0.01 of a muon; opposite-charge kaons; m(KK) in [0.99, 1.05] GeV (`kkMinMass`, `kkMaxMass`); pre-fit m(μμKK) in [4.9, 5.9] GeV (`kkMinBMass`, `kkMaxBMass`) |

All channels also need a primary vertex for the PV choice; there is always at least one in practice. Each channel can be switched off (`doMuMuGamma`, `doMuMuConvGamma`, `doMuMuGammaGamma`, `doMuMuKK`; driver option `channels=`).

### 3.5 Kinematic fit (`MMGVertexFit`): when a fit counts as failed
A candidate gets `fitOK = 0`, and stays in the ntuple, if:
- the photon has no cluster rechits (ECAL photon channels), or the muons have no inner track;
- the vertex fit or the pointing-constraint fit returns an invalid tree, or throws a `VertexException`;
- the decay vertex is invalid;
- the result diverged: non-finite χ², non-finite or non-positive mass, **mass > 100 GeV**, or **|p| > 10 TeV**.

No cut is made on the fit χ² or probability. **Recommended offline:** `<p>vtxProb > 1e-3`. Badly converged fits (χ² ≫ ndf) can differ slightly between runs (see report.md).

### 3.6 Trigger
- **Paths** (`triggerPaths`): `HLT_DoubleMu4_3_Bs`, `_3_Jpsi`, `_3_LowMass`, `_LowMass_Displaced`, `_3_Photon4_BsToMMG`, `_3_Displaced_Photon4_BsToMMG`, `_JpsiTrkTrk_Displaced`, `_Jpsi_NoVertexing`, `_Jpsi_Displaced` (the version suffix `_v*` is resolved per menu).
- **Decision** `trg_<path>`: 1/0, or −1 if the path is not in the menu. **No trigger requirement is applied.**
- **Muon matching** `mu_trg_<path>`:
  - AOD: ΔR < 0.1 (`triggerMatchDeltaR`) to an HLT muon object of **every** terminal filter of the path that stores muon objects. e/γ legs and ignored or inverted modules are skipped.
  - MiniAOD: ΔR < 0.1 to an HLT muon object associated with the last filter of the path.

### 3.7 Generator truth (MC)
- **Signal B:** the B_s or B_d with exactly two muons among its descendants (oldest copy, to handle oscillations). Open-charm hadrons and τ are not followed when the muons are counted, so the other b hadron of the event is not taken through semileptonic cascades (B → D μν, D → μX). If there is no such B and no b hadron at all in the event, the first π⁰ is used (π⁰-gun samples).
- **Decay mode:** classified from the B daughters. Extra photons are treated as FSR; for example B_s → μμγ needs ≥ 1 photon.
- **MiniAOD:** `prunedGenParticles` drops most final-state photons, e.g. those from π⁰/η/η′ → γγ. The final-state particles are taken from `packedGenParticles`; those not kept in the pruned collection are attached to their pruned mother when the decay is walked.
- **Object matching:** the closest **final-state** gen particle of the same type (status 1 `genParticles` in AOD, `packedGenParticles` in MiniAOD), with \|Δpt/pt\| < 0.5:
  - muons ΔR < 0.05;
  - photons and conversions ΔR < 0.1;
  - kaons ΔR < 0.05.

  `<obj>_genFromSignalB` is set if the matched particle descends from the signal B.
- **Candidates:** `<p>genMatched` = the muons and all legs match particles that descend from the signal B. Partially reconstructed decays are matched as well, e.g. an `mmg` candidate with one photon of J/ψ η.

### 3.8 Isolation and track compatibility (observables, no cut)
- **Isolation:** pt(B) / (pt(B) + Σ pt). The sum runs over the charged tracks of the candidate's PV with pt > 0.9 GeV and ΔR(B) < 0.7, excluding the muon tracks (ΔR < 0.01) and the candidate's own tracks (kaons, conversion tracks).
- **Max track compatibility:** the highest Kalman vertex probability of the two muon tracks plus one other track of the candidate's PV (pt > 0.9 GeV, not a muon or candidate track).

---

### 3.9 Event skim (optional, off by default)
- Switch: `skim=1` in `runNtuplizer_cfg.py` (`BMMG_SKIM=1` for CRAB); module parameter `onlyEventsWithCandidates`.
- An event is written if at least one candidate of any enabled channel (`mmg`, `mmcg`, `mmgg`, `mmkk`) has a successful fit (§3.5) and, if `skimMinVtxProb` / `minCandidateVtxProb` ≥ 0, a vertex probability above it (default −1: any successful fit).
- Events without a dimuon with a valid vertex are rejected before the candidates are built.
- `RunInfo.nEvents` still counts all processed events; `RunInfo.nEventsStored` the written ones.

## 4. Data formats
| | AOD | MiniAOD |
|---|---|---|
| muons / photons | `muons` / `gedPhotons` | `slimmedMuons` / `slimmedPhotons` |
| primary vertices | `offlinePrimaryVertices` | `offlineSlimmedPrimaryVertices` |
| PV tracks | tracks of the PV fit | packed candidates / lost tracks with `fromPV == PVUsedInFit` |
| kaon tracks | `generalTracks` | pseudo-tracks of packed candidates + lost tracks |
| conversions | `allConversions` | not available (tracks not stored): `mmcg` is switched off |
| ECAL rechits | `reducedEcalRecHitsEB/EE` | `reducedEgamma:reducedEB/EERecHits` |
| trigger objects | `hltTriggerSummaryAOD` | `slimmedPatTrigger` |
| gen / pile-up | `genParticles` / `addPileupInfo` | `prunedGenParticles` + `packedGenParticles` / `slimmedAddPileupInfo` |

The driver config chooses the collections from `dataFormat`. It is set by hand or detected from the test-sample name.
