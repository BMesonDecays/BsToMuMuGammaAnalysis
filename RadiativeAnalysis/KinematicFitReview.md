# Review: `TrippleObjectVertexFitRecoPhoton` with the modified KinematicFit

Scope: [src/KinematicConstrainedFit.cc](src/KinematicConstrainedFit.cc), `TrippleObjectVertexFitRecoPhoton`,
read together with the locally modified `RecoVertex/KinematicFit` and `RecoVertex/KinematicFitPrimitives`
(compared with CMSSW_14_1_1).

## What the modification does

Only particles built with a `reco::Photon*` (and a covariance) take the new path in
`ParticleKinematicLinearizedTrackState`. That path treats the photon as a **calorimeter measurement**:
the y and z of `caloPosition()` at x = x_calo, plus the energy and its uncertainty. It doesn't use a
perigee track. This code lives only in the linearised-track-state layer, which
`KinematicParticleVertexFitter` uses.

Two more facts matter:

- `KinematicConstrainedVertexFitter` never uses `LinearizedTrackState`, so it can't see the photon model.
- `TransientTrackKinematicParticle::refittedParticle()` doesn't pass the photon pointer on. Particles in
  the fit-1 tree therefore have `getPhoton() == nullptr`.

## Why there are two fits

- **Fit 1** (`KinematicParticleVertexFitter`, no constraints) is where the photon's ECAL information
  actually enters. It turns the calorimeter measurement into a 7-parameter kinematic state with a
  covariance at the common μμγ vertex.
- **Fit 2** (`KinematicConstrainedVertexFitter` with pointing to the PV, plus the dimuon mass constraint
  for J/ψ, ψ(2S) or φ) rebuilds each child from fit 1's output. The photon is rebuilt with
  `pFactory.particle(phoTTGlobal, ...)` and no photon pointer, so it is an ordinary neutral pseudo-track.
  All of its calorimeter information comes from fit 1.

So two steps aren't just a convergence trick. They're the only way to get the photon measurement model
into a mass- and pointing-constrained fit. `bhadron`, `bVertex`, `vtxprob_Bhadron` and `mass_Bhadron`
all come from fit 2.

## Points about the current code

| Point | Status |
|---|---|
| Fit 1 is "mainly a seed for convergence" | **Wrong in this setup.** Fit 1 is essential, as explained above. |
| Use `fit(..., initPoint)` with the original particles instead of rebuilding them | **Not viable.** The constrained fitter would ignore the photon model and use the raw `photonTT` state. |
| Vertex information counted twice, so χ²/prob from fit 2 isn't a proper p-value | **True.** It's a structural consequence of the design. Treat `vtxprob_Bhadron` as a ranking or cut variable. |
| Correlations between particles dropped when rebuilding children (only each 6×6 block is kept) | **True.** |
| Mass sigma 0.001 in fit 2 vs 1e-10 / 1e-6 in fit 1 | **Minor inconsistency.** Taking the mass from the refitted state (comment at line 292) is harmless, because the input mass sigma was tiny. |

### Child ordering (comment at line 273)

- `ConstrainedTreeBuilder` is unmodified and adds the children in input order, so the tree children are
  μ(`muonTT[0]`), μ(`muonTT[1]`), γ.
- The order matters: `TwoTrackMassKinematicConstraint` applies the mass constraint to the **first two**
  particles. If the photon ended up in slot 0 or 1, the fit would run and give nonsense without any error.
- Suggested hardening:
  1. Check the `bool` returned by `movePointerToTheFirstChild()` and `movePointerToTheNextChild()`.
  2. Add a charge check (each muon ±1, the photon 0):
     ```cpp
     if (std::abs(mu1Global->currentState().particleCharge()) != 1 ||
         std::abs(mu2Global->currentState().particleCharge()) != 1 ||
         phoGlobal->currentState().particleCharge() != 0) {
         edm::LogWarning("TrippleObjectVertexFit") << "Unexpected child ordering in vertex fit tree.";
         return false;
     }
     ```
  3. Optionally use `fullVertexFitTree->finalStateParticles()` (a vector in the same order) and fold the
     three nearly identical rebuild blocks into one helper.

## Issues in the modified KinematicFitPrimitives

1. **The photon parametrisation breaks when px ≈ 0.** `computePhotonJacobians()` divides by `px`
   everywhere because it measures y and z at fixed x = x_calo. Photons near φ ≈ ±90° make the Jacobians
   blow up, which can cause fit failures or bias in those φ regions. Check the fit-1 success rate against
   photon φ. One fix is to pick the measurement plane dynamically: for barrel photons, measure in φ and z
   at the calorimeter radius.
2. **The energy error has two different sources.** `predictedStateWeight()` uses
   `getCorrectedEnergyError(reco::Photon::ecal_photons)`, but `predictedStateError()` uses
   `superCluster()->correctedEnergyUncertainty()`. A weight and an error that aren't inverses of each
   other can make the χ² inconsistent. Use one source in both.
