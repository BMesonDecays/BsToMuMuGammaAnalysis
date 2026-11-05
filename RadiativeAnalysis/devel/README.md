# devel/: work in progress, NOT compiled

- `MVAAnalyzer.cc`, `MVAAnalyzer.py`: standalone check of the muon MVA on gen-matched Bs→μμγ muons.
  - It builds `MuonMVAID` (and loads the XGBoost model) every event.
  - Moved here in P3.8; the MVA score is now stored per muon in the ntuple (`mu_mva`).
- `ReadSoftMvaRun3ValueProducer.cc`: EDAnalyzer that prints `userFloat("run3muonmva")` of `pat::Muon`s. No config uses it.

The custom low-energy photon producer (`BMMGCustomPhotonProducer`) is compiled again since Phase 9, with the config `test/customPhotons_cfg.py`; see report.md. The old, broken `customPhotonConfig*.py` were removed.
