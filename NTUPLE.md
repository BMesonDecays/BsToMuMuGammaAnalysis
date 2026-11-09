# Ntuple of the `RadiativeAnalysis` module

Produced by `RadiativeAnalysis/test/runNtuplizer_cfg.py` from AOD or MiniAOD; the branches are the same for both. The MiniAOD differences are listed at the end. The output file (TFileService) contains the directory `bmmg/` with two trees:

| tree | entries | content |
|---|---|---|
| `bmmg/Events` | one per processed event (with `skim=1`: only events with a valid B candidate) | event-level scalars and collections stored as `std::vector` |
| `bmmg/RunInfo` | one per job | cross section, filter efficiency, number of processed and stored events |

**Skim** (`skim=1` in `runNtuplizer_cfg.py`, module parameter `onlyEventsWithCandidates`): an event is written only if at least one candidate of an enabled channel has a successful fit (`<p>fitOK = 1`) and, if `skimMinVtxProb` (`minCandidateVtxProb`) ≥ 0, `<p>vtxProb` above it. The stored events are unchanged; all their candidates are kept, including failed fits. Off by default.

Everything is stored, with no best-candidate choice:
- **Every** preselected muon, photon and dimuon.
- **Every** μμγ candidate in the pre-fit mass window.

Candidates point to their objects through indices. For example `mmg_mu1[i]` is an index into the `mu_*` vectors of the same event, and `mmg_pho[i]` is an index into `pho_*`. Missing values are −999 (float) or −1 (int). `MC` marks branches that only exist when `isMC=True`.

Units: GeV, cm, ns.

## Event: `evt_*`
| branch | type | description |
|---|---|---|
| `evt_run`, `evt_lumi`, `evt_event` | uint, uint, uint64 | event identification |
| `evt_nPV`, `evt_nGoodPV` | int | primary vertices, all / good (valid, not fake, ndof>4, \|z\|<24, ρ<2) |
| `evt_nPU`, `evt_nTruePU` (MC) | int, float | pile-up interactions (in-time) and true number of interactions |

## Beam spot `bs_*` and event primary vertex `pv_*`
- `bs_x/y/z`, `bs_xErr/yErr/zErr`, `bs_sigmaZ`, `bs_dxdz`, `bs_dydz`, `bs_widthX/Y`: the beam spot.
- `pv_x/y/z`, `pv_xErr/yErr/zErr`, `pv_chi2`, `pv_ndof`, `pv_nTracks`: the **first** vertex of `offlinePrimaryVertices`, i.e. the highest Σpt². The PV used for each candidate is in `mmg_pv*`.

## Trigger: `trg_*` and `mu_trg_*`
- `trg_<path>` (int): decision of `HLT_<path>_v*`. It is −1 if the path is not in the menu of the event.
- `mu_trg_<path>` (vector<int>, per muon): 1 if the muon matches the HLT muon objects of the path, i.e. ΔR < `triggerMatchDeltaR` (0.1) to a muon object of **every** terminal filter that stores muon objects. e/γ legs are skipped.

Paths (parameter `triggerPaths`): `DoubleMu4_3_Bs`, `DoubleMu4_3_Jpsi`, `DoubleMu4_3_LowMass`, `DoubleMu4_LowMass_Displaced`, `DoubleMu4_3_Photon4_BsToMMG`, `DoubleMu4_3_Displaced_Photon4_BsToMMG`, `DoubleMu4_JpsiTrkTrk_Displaced`, `DoubleMu4_Jpsi_NoVertexing`, `DoubleMu4_Jpsi_Displaced`.

## Muons: `mu_*` (vectors)
Preselection: global or tracker muon with an inner track, pt > `muonMinPt` (2 GeV), |η| < `muonMaxAbsEta` (2.4), MVA ≥ `muonMinMVA` (no cut by default).

| branch | description |
|---|---|
| `mu_index` | index in the `muons` collection |
| `mu_pt`, `mu_eta`, `mu_phi`, `mu_charge` | kinematics |
| `mu_mva` | Run-3 soft-muon MVA (`run3mvaid`, XGBoost); the legacy analysis cut was 0.83 |
| `mu_isGlobal`, `mu_isTracker`, `mu_isPF`, `mu_isLoose`, `mu_isMedium`, `mu_isSoft`, `mu_isGoodTMOneStationTight` | ID flags |
| `mu_highPurity`, `mu_trkLayers`, `mu_pixLayers`, `mu_validHits`, `mu_trkNormChi2`, `mu_validFraction` | inner track quality |
| `mu_dxyBS`, `mu_dxyBSErr`, `mu_dzBS` | inner-track impact parameters w.r.t. the beam spot |
| `mu_genPdgId`, `mu_genMotherPdgId`, `mu_genDR` (MC) | closest final-state gen muon (ΔR < 0.05, \|Δpt/pt\| < 0.5) |
| `mu_genFromSignalB` (MC) | the matched gen muon descends from the signal B of the event |

## Photons: `pho_*` (vectors)
Preselection: `gedPhotons` with a supercluster, E_SC > `photonMinSCEnergy` (1 GeV), \|η_SC\| < `photonMaxAbsSCEta` (2.5), MVA ≥ `photonMinMVA` (no cut by default).

| branch | description |
|---|---|
| `pho_index` | index in the photon collection |
| `pho_pt`, `pho_eta`, `pho_phi`, `pho_energy`, `pho_energyErr` | kinematics, energy uncertainty of the candidate |
| `pho_fitEnergyErr` | energy uncertainty used in the kinematic fit (scaled, see `energyErrorScaleEB/EE`) |
| `pho_mva` | photon MVA (`PhotonMVAEstimatorRunIIIWinter22v1`), −99 if not available |
| `pho_tightId` | MVA ID working point 90% (`mvaPhoID-RunIIIWinter22-v1-wp90`), −1 if not available |
| `pho_isEB`, `pho_isEE`, `pho_hasPixelSeed`, `pho_hasConversionTracks` | flags |
| `pho_hasClusterHits` | the supercluster has rechits (needed for the fit and for `pho_seedTime`) |
| `pho_scEnergy`, `pho_scRawEnergy`, `pho_scEta`, `pho_scPhi`, `pho_scEtaWidth`, `pho_scPhiWidth` | supercluster |
| `pho_r9`, `pho_sigmaIetaIeta`, `pho_sigmaIetaIphi`, `pho_sigmaIphiIphi`, `pho_hOverE` | full5x5 shower shapes, H/E |
| `pho_seedTime` | time of the seed crystal [ns] |
| `pho_genPdgId`, `pho_genMotherPdgId`, `pho_genDR`, `pho_genFromSignalB` (MC) | closest final-state gen photon (ΔR < 0.1, \|Δpt/pt\| < 0.5) |

## Dimuons: `dimu_*` (vectors)
Every opposite-sign pair of selected muons that do not share muon segments.

| branch | description |
|---|---|
| `dimu_mu1`, `dimu_mu2` | indices into `mu_*`; `mu1` is the positive muon |
| `dimu_mass`, `dimu_pt`, `dimu_eta`, `dimu_phi`, `dimu_deltaR` | kinematics from the muon momenta |
| `dimu_resonance` | 0 non-resonant, 1 J/ψ (2.9–3.3), 2 ψ(2S) (3.55–3.8), 3 φ (0.97–1.07) |
| `dimu_vtxValid`, `dimu_vtxProb`, `dimu_vtxChi2`, `dimu_vtxNdof`, `dimu_vtxX/Y/Z` | Kalman vertex of the two inner tracks |
| `dimu_lxyBS`, `dimu_lxyBSErr` | transverse displacement of the vertex from the beam spot (beam slope included; the error includes the BS width) |
| `dimu_cosAlphaBS` | cos of the transverse angle between the displacement and the dimuon momentum |
| `dimu_dca` | transverse distance of closest approach of the two tracks |
| `dimu_isCowboy` | 1 if Δφ(μ⁺, μ⁻) > 0 |

## Conversions: `conv_*` (vectors, if `doMuMuConvGamma`)
From `allConversions`: two opposite-charge tracks, a valid vertex with probability > `conversionMinVertexProb` (1e-6), pair pt > `conversionMinPt` (0.5 GeV). The e⁺e⁻ pair is refitted with a zero-mass constraint.
- `conv_index`: index in the collection.
- `conv_fitOK`: 1 if the zero-mass refit succeeded.
- `conv_pt`, `conv_eta`, `conv_phi`: from the refitted photon, or from the pair momentum if the refit failed.
- `conv_vtxX/Y/Z`, `conv_vtxRho`, `conv_vtxProb`: the conversion vertex.
- `conv_trkPlusPt`, `conv_trkMinusPt`: the two tracks.
- `conv_genPdgId`, `conv_genMotherPdgId`, `conv_genDR`, `conv_genFromSignalB` (MC): gen photon match (ΔR < 0.1, \|Δpt/pt\| < 0.5).

## Kaon tracks: `trk_*` (vectors, if `doMuMuKK`)
Only the tracks used in at least one `mmkk_` candidate.
- `trk_index`: index in `generalTracks`.
- `trk_pt`, `trk_eta`, `trk_phi`, `trk_charge`.
- `trk_dxyBS`, `trk_dzBS`, `trk_normChi2`, `trk_validHits`, `trk_pixLayers`.
- `trk_genPdgId`, `trk_genMotherPdgId`, `trk_genDR`, `trk_genFromSignalB` (MC): gen kaon match (ΔR < 0.05, \|Δpt/pt\| < 0.5).

## B candidates: `mmg_*`, `mmcg_*`, `mmgg_*`, `mmkk_*` (vectors)
Every dimuon with a valid vertex combined with the legs of the channel, if the pre-fit mass is in the window. Each channel can be switched off (`doMuMuGamma`, `doMuMuConvGamma`, `doMuMuGammaGamma`, `doMuMuKK`).

| prefix | channel | legs (index branches) | pre-fit B window | fit daughters |
|---|---|---|---|---|
| `mmg_` | B → μμγ, ECAL photon | `pho` → `pho_*` | 4–7 GeV | μ⁺, μ⁻, γ (cluster) |
| `mmcg_` | B → μμγ, converted photon | `conv` → `conv_*` | 4–7 GeV | μ⁺, μ⁻, γ (e⁺e⁻ refit) |
| `mmgg_` | B → μμγγ (J/ψ η, J/ψ π⁰, …) | `pho1`, `pho2` → `pho_*` | 4–7 GeV; m(γγ) in 0.05–1.1 | μ⁺, μ⁻, γ, γ (clusters) |
| `mmkk_` | B_s → J/ψ φ (normalisation) | `trk1` (K⁺), `trk2` (K⁻) → `trk_*` | 4.9–5.9 GeV; J/ψ dimuons only; m(KK) in 0.99–1.05 | μ⁺, μ⁻, K⁺, K⁻ |

Kaon candidates are high-purity tracks with pt > 0.7 GeV, ΔR < 1.5 to the dimuon and \|Δz\| < 1 cm to the dimuon vertex.

**PV choice:** `<p>pv` is the vertex closest to the line through the dimuon vertex along the pre-fit B momentum (lowest DCA).

**Kinematic fit:** `MMGVertexFit::fitParticles`, done in two steps:
1. A vertex fit of all daughters. ECAL photons are measured by their cluster position and energy.
2. A pointing constraint to `<p>pv`.

Fits that fail, or that diverge (non-finite χ², mass > 100 GeV, |p| > 10 TeV), give `<p>fitOK = 0` with −999 for every fit quantity.

| branch (`<p>` = prefix) | description |
|---|---|
| `<p>dimu`, `<p>mu1`, `<p>mu2`, `<p>pv`, leg branches | indices: dimuon, μ⁺, μ⁻ (into `mu_*`), PV (into the PV collection), legs (see the table) |
| `<p>pvX/Y/Z` | position of the chosen PV |
| `<p>prefitMass`, `<p>prefitPt`, `<p>prefitEta`, `<p>prefitPhi` | B from the reconstructed momenta |
| `<p>xMass` (two-leg channels) | pre-fit mass of the non-dimuon system: m(γγ) or m(KK) |
| `<p>fitOK` | the fit succeeded |
| `<p>mass`, `<p>massErr`, `<p>pt`, `<p>eta`, `<p>phi` | fitted B |
| `<p>fitDimuonMass` | dimuon mass from the fitted muons. For J/ψ channels use `<p>mass − <p>fitDimuonMass + 3.0969` (J/ψ-constrained mass approximation; there is no mass constraint in the fit) |
| `<p>vtxProb`, `<p>vtxChi2`, `<p>vtxNdof` | of the fitted vertex (including the pointing constraint); a cut like `vtxProb > 1e-3` removes badly fitted candidates |
| `<p>svX/Y/Z`, `<p>svXErr/YErr/ZErr` | fitted decay vertex |
| `<p>mu1Fit*`, `<p>mu2Fit*`, `<p><leg>Fit*` (Pt, Eta, Phi) | fitted daughters |
| `<p>l3d`, `<p>l3dErr`, `<p>lxy`, `<p>lxyErr` | PV–SV distance in 3D and transverse, with errors |
| `<p>cosAlpha3D`, `<p>cosAlpha2D` | cos of the angle between (SV − PV) and the fitted momentum |
| `<p>ct3D`, `<p>ct2D` | ct = m (d·p)/\|p\|², and transverse m (d_T·p_T)/p_T², with d = SV − PV (signed) and m = m(B_s) PDG. For B_d multiply by m(B_d)/m(B_s). |
| `<p>ct3DErr`, `<p>ct2DErr` | propagated from the full covariance of the fitted state (SV position, momentum and correlations) and the PV covariance |
| `<p>helicity` | cos θ of the non-dimuon system (γ, γγ, KK) in the B rest frame w.r.t. the B direction |
| `<p>coplanarity` | angle between the (μ⁺, μ⁻) plane and the (X, B) plane |
| `<p>cosThetaL` | cos θ of μ⁺ in the dimuon rest frame w.r.t. the dimuon direction |
| `<p>isolation` | pt(B) / (pt(B) + Σ pt of charged PV tracks with pt > 0.9 GeV within ΔR < 0.7, excluding the tracks of the candidate) |
| `<p>maxTrackCompatibility` | highest Kalman vertex probability of the two muons plus one other PV track (pt > 0.9, not a track of the candidate) |
| `<p>genMatched` (MC) | the muons and all legs match gen particles that descend from the signal B. Partially reconstructed decays also count (e.g. `mmg` with one photon of J/ψ η: its mass is below the B mass). A π⁰ whose two photons are merged in one cluster is usually not matched (photon energy ≫ gen photon) |

## Generator: `gen_*` (MC)
From `GenDecayChain`, using the B (B_s or B_d) with two muons among its descendants (open-charm hadrons and τ are not followed). In MiniAOD the final-state particles, in particular the photons of π⁰/η/η′ decays that `prunedGenParticles` drops, come from `packedGenParticles`.
- **Status:**
  - `gen_valid`: a B was found.
  - `gen_decayMode`: the `DecayMode` enum in `GenChain.h` (1 B_s→μμγ, 10 B_d→μμγ, 2/3 J/ψγ, 7–9 / 12–14 J/ψ π⁰/η/η′, …). Extra photons are treated as FSR.
  - `gen_B_pdgId`, `gen_isSignal` (μμγ modes), `gen_is3Body`, `gen_is4Body`, `gen_hasJpsi`.
  - `gen_neutralMesonPdgId` (π⁰/η/η′, 0 if none), `gen_vectorMesonPdgId` (φ/K*, 0 if none).
- **B:** `gen_B_pt/eta/phi/mass`, `gen_B_ct2D`, `gen_B_ct3D`, `gen_B_lxy`, production vertex `gen_pvX/Y/Z`, decay vertex `gen_svX/Y/Z`.
- **Muons:** `gen_muPlus_*`, `gen_muMinus_*` (pt, eta, phi), `gen_dimuonMass`.
- **Angles and masses:** `gen_helicity`, `gen_coplanarity`, `gen_cosThetaL`, `gen_triBodyMass` (3-body decays); `gen_diphotonMass` (two leading photons), `gen_fourBodyMass`, `gen_helicity4`, `gen_coplanarity4` (4-body decays). −999 if not applicable.
- **Photons:** `gen_pho_pt/eta/phi/origin/motherPdgId` (vectors, all photons of the decay ordered by pt). The origin codes are in `PhotonOrigin` in `GenChain.h`: 1 from B, 2 from resonance, 3 from neutral meson, 4 FSR, …

## RunInfo
- `xsec`, `xsecErr`: generator cross section [pb], an inverse-variance weighted mean over the runs.
- `filterEff`, `filterEffErr`: filter efficiency, merged over all luminosity blocks.
- `nEvents`: processed events (all of them, also with the skim). Use it for the normalisation.
- `nEventsStored`: entries written to `Events` (= `nEvents` without the skim).
- `lumiEquivalent`, `lumiEquivalentErr`: nEvents / (xsec · filterEff) [pb⁻¹].
- Data: −1, except `nEvents`.

## Differences from the legacy ntuple (`RadiativeAnalysisLegacy`, tree `BMMGTree`, removed in P7.4)
- **Candidates:**
  - All candidates are stored. The legacy ntuple kept one "best" muon pair, chosen by vertex probability after an MVA > 0.83 cut and a 100 µm displacement cut, plus one photon closest in ΔR (MVA > −0.07).
  - The pre-fit window is 4–7 GeV instead of 4.5–6.0.
  - EE photons are included.
- **Branches:**
  - The suffix scheme (`_mmrecog_`, …) is replaced by prefixes and index links.
  - The leading-two photon and PF-candidate blocks and the supercluster/rechit dump are not stored; `pho_*` covers the selected photons.
  - Channels: the legacy suffixes `_mmconvg_`, `_mmrecogg_` and `_mmkk_` are now `mmcg_`, `mmgg_` and `mmkk_`.
  - μμKK uses only J/ψ dimuons.
  - The conversion fit uses the e⁺e⁻ pair refitted with a zero-mass constraint as one photon particle. There is no J/ψ mass constraint (use `fitDimuonMass`).
- **Event PV:** `pv_*` is the first vertex (highest Σpt²); the legacy ntuple used the highest scalar Σpt.
- **ct:** the legacy `BsCt3D` was m·|SV−PV|/|p| (unsigned, not projected), with an error that ignored the momentum uncertainty. The pull 68% width on signal MC goes from about 1.7 to 1.27.

## MiniAOD input (`dataFormat=MINIAOD`)
The same ntuple is written; the inputs differ:

| | AOD | MiniAOD |
|---|---|---|
| muons, photons | `muons`, `gedPhotons` | `slimmedMuons`, `slimmedPhotons` (pat objects, through `edm::View`) |
| primary vertices | `offlinePrimaryVertices` | `offlineSlimmedPrimaryVertices` |
| PV tracks (isolation, track compatibility) | tracks of the PV fit (`vertex.tracks()`) | `packedPFCandidates` + `lostTracks` with track details and `fromPV == PVUsedInFit` |
| kaon tracks (`trk_*`, `trk_index`) | `generalTracks` (index in that collection) | pseudo-tracks of the charged `packedPFCandidates` + `lostTracks` (index in that list) |
| conversions (`conv_*`) | `allConversions` | `reducedEgamma:reducedConversions` (only conversions associated to photons) |
| ECAL rechits | `reducedEcalRecHitsEB/EE` | `reducedEgamma:reducedEB/EERecHits` (only around e/γ objects: `pho_hasClusterHits` can be 0) |
| trigger matching | `hltTriggerSummaryAOD`: muon objects of every terminal muon filter of the path | `slimmedPatTrigger`: HLT muon objects associated with the last filter of the path |
| gen, pile-up | `genParticles`, `addPileupInfo` | `prunedGenParticles` + `packedGenParticles` (final state), `slimmedAddPileupInfo` |
