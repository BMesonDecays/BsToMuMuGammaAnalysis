# devel/ — work in progress, NOT compiled

scram does not build this directory. The files here were moved out of the build because they don't compile or are not used yet.

- `BMMGCustomPhotonProducer.{h,cc}`: custom low-energy photon producer (plan.md Phase 9).
  - It doesn't compile against CMSSW_14_1_1, because the `PhotonIsolationCalculator` and `PhotonMIPHaloTagger` constructors changed and `mipCalculate` was renamed to `MIPcalculate`.
  - To revive it, move the header back to `interface/` and the source to `plugins/`, then fix the include path.
- `customPhotonConfig.py`, `customPhotonConfig_cfi.py`: configs for the producer. Both are known to be broken (see plan.md P9.2).
- `MVAAnalyzer.cc`, `MVAAnalyzer.py`: standalone check of the muon MVA on gen-matched Bs→μμγ muons.
  - It builds `MuonMVAID` (and loads the XGBoost model) every event.
  - Moved here in P3.8; the MVA score is now stored per muon in the ntuple.
- `ReadSoftMvaRun3ValueProducer.cc`: EDAnalyzer that prints `userFloat("run3muonmva")` of `pat::Muon`s. No config uses it.
