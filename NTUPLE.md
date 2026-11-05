# Ntuple of the `RadiativeAnalysis` module

Produced by `RadiativeAnalysis/test/runNtuplizer_cfg.py`. The output file (TFileService) contains the directory `bmmg/` with two trees:

| tree | entries | content |
|---|---|---|
| `bmmg/Events` | one per processed event | event-level scalars and collections stored as `std::vector` |
| `bmmg/RunInfo` | one per job | cross section, filter efficiency, number of processed events |

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
| `pho_tightId` | cut-based tight ID (RunIIIWinter22 V1), −1 if not available |
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

## μμγ candidates: `mmg_*` (vectors)
Every dimuon with a valid vertex × every photon, with a pre-fit mass in [`mmgMinPrefitMass`, `mmgMaxPrefitMass`] = [4, 7] GeV.

**PV choice:** `mmg_pv` is the vertex closest to the line through the dimuon vertex along the pre-fit μμγ momentum (lowest DCA).

**Kinematic fit:** `MMGVertexFit`, done in two steps:
1. A vertex fit of μ⁺, μ⁻ and γ. The photon is measured by its ECAL cluster position and energy.
2. A pointing constraint to `mmg_pv`.

| branch | description |
|---|---|
| `mmg_dimu`, `mmg_mu1`, `mmg_mu2`, `mmg_pho`, `mmg_pv` | indices: dimuon, μ⁺, μ⁻ (into `mu_*`), photon (into `pho_*`), PV (into the PV collection) |
| `mmg_pvX/Y/Z` | position of the chosen PV |
| `mmg_prefitMass`, `mmg_prefitPt`, `mmg_prefitEta`, `mmg_prefitPhi` | sum of the reconstructed momenta |
| `mmg_fitOK` | 1 if the fit succeeded; if 0, all fit quantities are −999 |
| `mmg_mass`, `mmg_massErr`, `mmg_pt`, `mmg_eta`, `mmg_phi` | fitted B |
| `mmg_vtxProb`, `mmg_vtxChi2`, `mmg_vtxNdof` | of the fitted vertex (including the pointing constraint). **Diverging fits have `vtxProb` ≈ 0; cut on it (e.g. > 1e-3).** |
| `mmg_svX/Y/Z`, `mmg_svXErr/YErr/ZErr` | fitted decay vertex |
| `mmg_mu1Fit*`, `mmg_mu2Fit*`, `mmg_phoFit*` (Pt, Eta, Phi) | fitted daughters |
| `mmg_l3d`, `mmg_l3dErr`, `mmg_lxy`, `mmg_lxyErr` | PV–SV distance in 3D and transverse, with errors (SV and PV covariances) |
| `mmg_cosAlpha3D`, `mmg_cosAlpha2D` | cos of the angle between (SV − PV) and the fitted momentum |
| `mmg_ct3D`, `mmg_ct2D` | ct = m (d·p)/\|p\|², and the transverse version m (d_T·p_T)/p_T², with d = SV − PV (signed) and m = m(B_s) PDG. For B_d multiply by m(B_d)/m(B_s). |
| `mmg_ct3DErr`, `mmg_ct2DErr` | propagated from the full covariance of the fitted state (SV position, momentum and their correlations) and the PV covariance |
| `mmg_helicity` | cos θ of the photon in the B rest frame w.r.t. the B direction (fitted momenta; pre-fit if the fit failed) |
| `mmg_coplanarity` | angle between the (μ⁺, μ⁻) plane and the (γ, B) plane |
| `mmg_cosThetaL` | cos θ of μ⁺ in the dimuon rest frame w.r.t. the dimuon direction |
| `mmg_isolation` | pt(B) / (pt(B) + Σ pt of charged PV tracks with pt > 0.9 GeV within ΔR < 0.7, excluding the muons) |
| `mmg_maxTrackCompatibility` | highest Kalman vertex probability of the two muons plus one other PV track (pt > 0.9) |
| `mmg_genMatched` (MC) | μ⁺, μ⁻ and γ all match gen particles that descend from the signal B |

## Generator: `gen_*` (MC)
From `GenDecayChain`, using the B (B_s or B_d) with two muons among its descendants.
- **Status:**
  - `gen_valid`: a B was found.
  - `gen_decayMode`: the `DecayMode` enum in `GenChain.h` (1 B_s→μμγ, 10 B_d→μμγ, 2/3 J/ψγ, 7–9 / 12–14 J/ψ π⁰/η/η′, …). Extra photons are treated as FSR.
  - `gen_B_pdgId`, `gen_isSignal` (μμγ modes), `gen_is3Body`, `gen_is4Body`, `gen_hasJpsi`.
- **B:** `gen_B_pt/eta/phi/mass`, `gen_B_ct2D`, `gen_B_ct3D`, `gen_B_lxy`, production vertex `gen_pvX/Y/Z`, decay vertex `gen_svX/Y/Z`.
- **Muons:** `gen_muPlus_*`, `gen_muMinus_*` (pt, eta, phi), `gen_dimuonMass`.
- **Angles and masses:** `gen_helicity`, `gen_coplanarity`, `gen_cosThetaL`, `gen_triBodyMass` (3-body decays).
- **Photons:** `gen_pho_pt/eta/phi/origin/motherPdgId` (vectors, all photons of the decay ordered by pt). The origin codes are in `PhotonOrigin` in `GenChain.h`: 1 from B, 2 from resonance, 3 from neutral meson, 4 FSR, …

## RunInfo
- `xsec`, `xsecErr`: generator cross section [pb], an inverse-variance weighted mean over the runs.
- `filterEff`, `filterEffErr`: filter efficiency, merged over all luminosity blocks.
- `nEvents`: processed events.
- `lumiEquivalent`, `lumiEquivalentErr`: nEvents / (xsec · filterEff) [pb⁻¹].
- Data: −1, except `nEvents`.

## Differences from the legacy ntuple (`RadiativeAnalysisLegacy`, tree `BMMGTree`)
- **Candidates:**
  - All candidates are stored. The legacy ntuple kept one "best" muon pair, chosen by vertex probability after an MVA > 0.83 cut and a 100 µm displacement cut, plus one photon closest in ΔR (MVA > −0.07).
  - The pre-fit window is 4–7 GeV instead of 4.5–6.0.
  - EE photons are included.
- **Branches:**
  - The suffix scheme (`_mmrecog_`, …) is replaced by prefixes and index links.
  - The leading-two photon and PF-candidate blocks and the supercluster/rechit dump are not stored; `pho_*` covers the selected photons.
  - The conversion, μμγγ and μμKK channels are not in the new ntuple yet (plan.md Phase 7); use the legacy module for them.
- **Event PV:** `pv_*` is the first vertex (highest Σpt²); the legacy ntuple used the highest scalar Σpt.
- **ct:** the legacy `BsCt3D` was m·|SV−PV|/|p| (unsigned, not projected), with an error that ignored the momentum uncertainty. The pull 68% width on signal MC goes from about 1.7 to 1.27.
