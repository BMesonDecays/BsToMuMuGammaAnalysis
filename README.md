# Ntuple production for the B → μμγ analysis

CMSSW ntuplizer for B_s → μμγ (and related radiative channels). The photon enters the kinematic fit through its ECAL cluster.

## Contributors
- Muhammad Alibordi
- Szymon Sławiński

## Setup
The photon kinematic fit needs modified `RecoVertex/KinematicFit*` and `RecoEcal/EgammaCoreTools` packages.
```bash
scram project CMSSW_14_1_1
cd CMSSW_14_1_1/src
cmsenv
git cms-init
git remote add -f sslawins https://github.com/sslawins/cmssw.git
git checkout photonKinematicFit_v2
git cms-addpkg RecoVertex/KinematicFit
git cms-addpkg RecoVertex/KinematicFitPrimitives
git cms-addpkg RecoEcal/EgammaCoreTools

git clone https://github.com/mumuhamm/BsToMuMuGammaAnalysis.git
cd BsToMuMuGammaAnalysis && git checkout sslawins_dev1 && cd ..
scram b -j8
scram b runtests     # unit tests of the analysis library
```

## Running
```bash
cd BsToMuMuGammaAnalysis/RadiativeAnalysis/test
cmsRun runNtuplizer_cfg.py maxEvents=1000                  # MC AOD test file (test_files.yaml, sample bsmmg_aod)
cmsRun runNtuplizer_cfg.py testSample=data_aod maxEvents=1000    # data AOD test file
cmsRun runNtuplizer_cfg.py testSample=bsmmg_miniaod maxEvents=1000   # MC MiniAOD test file (isMC, dataFormat from the sample)
cmsRun runNtuplizer_cfg.py inputFiles=file:in.root isMC=0 lumiMask=Cert.json outputFile=out.root
cmsRun runNtuplizer_cfg.py testSample=data_aod skim=1             # store only events with a valid B candidate (smaller ntuples)
```
- The options are described at the top of `runNtuplizer_cfg.py`.
- **Legacy AOD samples without endcap ECAL rechits** (e.g. the private zero-bias, no-pileup B_s → μμγ MC of 2025-07-09 without a generator photon-pt cut, test sample `bsmmg_legacy_aod`): `cmsRun runNtuplizer_legacy_cfg.py [options]`. It runs `runNtuplizer_cfg.py` with the same options and ntuple format, removes the endcap reco photons (|SC η| < 1.479; `mmg`/`mmgg` with barrel photons only), passes an empty endcap rechit collection (`EmptyEcalRecHitCollectionProducer`, `plugins/RadiativeAnalysis_legacy.cc`) and produces rho from `particleFlow` for the photon MVA. Converted photons (`mmcg`, `chic`) work in the whole tracker acceptance. The sample has no generator run info (RunInfo xsec = −1).
- The test input samples are listed in `RadiativeAnalysis/test/test_files.yaml`: each has a short name (the `testSample` value) and the attributes `isMC`, `dataFormat` (AOD / MINIAOD), `files` (globs allowed) and `description`; `python3 RadiativeAnalysis/python/testFiles.py` prints them. Besides B_s → μμγ and data there are MC samples (AOD and MiniAOD) of B_s → J/ψγ, φγ, J/ψπ⁰, J/ψη, J/ψη′ and B_d → μμγ.
- The ntuple content is documented in **[NTUPLE.md](NTUPLE.md)**.

### Grid submission (CRAB)
All tasks (data and MC, AOD and MiniAOD, any number of datasets) are described in a YAML task file and submitted with one script. Each submission goes through the CRAB library API in its own process:
```bash
cd BsToMuMuGammaAnalysis/RadiativeAnalysis/crab
source /cvmfs/cms.cern.ch/common/crab-setup.sh
voms-proxy-init -voms cms -valid 192:00
python3 submitCrab.py tasks_example.yaml --dry-run          # print the generated CRAB configurations
python3 submitCrab.py tasks_example.yaml                    # submit every task (existing project directories are skipped)
python3 submitCrab.py tasks_example.yaml --tasks 'Parking*' # only some tasks
python3 submitCrab.py tasks_example.yaml --command status   # also: resubmit, kill
python3 submitCrab.py tasks_example.yaml --command status -v # full CRAB output instead of one line per task
```
- By default every task prints one line (submitted / job states / failure reason with the end of the CRAB output); the full CRAB output is kept in `crab_projects/logs/<task>.<command>.log`. `-v`/`--verbose` prints the full output (and the full configurations with `--dry-run`).
- `tasks_example.yaml` documents the keys: dataset(s) or file list, `isMC`, `dataFormat`, lumi mask, splitting, output site, and the `runNtuplizer_cfg.py` options (`skim`, `globalTag`, `channels`, …). The channels are `mmg`, `mmcg`, `mmgg`, `mmkk` and `chic` (χ_c → J/ψ γ with a converted photon, the calibration sample of the conversion energy scale); default: all.
- Legacy AOD samples without endcap ECAL rechits: task key `legacy: true` runs `runNtuplizer_legacy_cfg.py` instead of `runNtuplizer_cfg.py` (AOD only; example in `tasks_example.yaml`). The files of a private sample on `/eos/cms` go in a `fileList` (`makeFileList.py`).
- `isMC` and `dataFormat` default to the dataset's data tier and must agree with it.
- `makeFileList.py` writes the LFN list for private samples.

## Layout
| path | content |
|---|---|
| `RadiativeAnalysis/plugins/RadiativeAnalysis.cc` | the ntuplizer (EDAnalyzer), which orchestrates the library components. Channels: μμγ (ECAL photon), μμγ (conversion), μμγγ, μμKK (normalisation) |
| `RadiativeAnalysis/interface`, `src` | library (namespace `bmmg`): `MMGVertexFit` (μμγ kinematic fit), object selection, `DimuonBuilder`, `CandidateBuilders`, `CandidateObservables`, `TriggerMatcher`, `GenChain` (gen truth), `BranchSet`/`NtupleWriter` |
| `RadiativeAnalysis/plugins/MMGVertexFitExample.cc`, `test/MMGVertexFitExample.py` | standalone validation of the photon kinematic fit (pulls, resolution) |
| `RadiativeAnalysis/test` | configs, `test_files.yaml`, unit tests, `compareNtuples.py` (regression check), `validateMMGFit.py` (fit validation), `checkDecayModes.py` (consistency check per MC decay mode) |
| `RadiativeAnalysis/devel` | not compiled: custom low-energy photon producer (work in progress), old debugging analyzers |
| `run3mvaid` | Run-3 soft-muon MVA (XGBoost) |
| `analysis` | Python analysis of the ntuples (no CMSSW needed): conversion of the CRAB output to candidate-level Parquet tables, plots, cut flows, k-fold BDT and MLP (TensorFlow), BDT tuning and mass-sculpting check, working-point scan with a Run 3 projection and a binned-fit significance. See [analysis/README.md](analysis/README.md) |

## Documentation
- [NTUPLE.md](NTUPLE.md): branches of the ntuple.
- [plan.md](plan.md): development plan and its status.
- [report.md](report.md): log of the changes, validations and decisions.
- [structure.md](structure.md): structure, processing flow and cuts of the analysis code.
- [structure_old.md](structure_old.md): description of the code before the restructuring. The old ntuplizer (`RadiativeAnalysisLegacy`) was removed in plan step P7.4; it is still in the git history (commit `bde3123`).

## MC production
Private AOD MC production is done in a separate repository: https://github.com/mumuhamm/PrivateMCProduction, branch `devel_GPExtrapolationPhaseI`. `RadiativeAnalysis/test/fragmentRequest.sh` downloads generator fragments.
