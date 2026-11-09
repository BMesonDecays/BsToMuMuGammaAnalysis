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
cmsRun runNtuplizer_cfg.py maxEvents=1000                  # MC AOD test file (test_files.txt, sample 0)
cmsRun runNtuplizer_cfg.py testSample=15 maxEvents=1000    # data AOD test file
cmsRun runNtuplizer_cfg.py testSample=1 maxEvents=1000     # MC MiniAOD test file (dataFormat from the sample name)
cmsRun runNtuplizer_cfg.py inputFiles=file:in.root isMC=0 lumiMask=Cert.json outputFile=out.root
cmsRun runNtuplizer_cfg.py testSample=15 skim=1             # store only events with a valid B candidate (smaller ntuples)
```
- The options are described at the top of `runNtuplizer_cfg.py`.
- The test input files are listed in `RadiativeAnalysis/test/test_files.txt`; `python3 RadiativeAnalysis/python/testFiles.py` prints them with their `testSample` index. Besides B_s → μμγ and data there are MC samples (AOD and MiniAOD) of B_s → J/ψγ, φγ, J/ψπ⁰, J/ψη, J/ψη′ and B_d → μμγ.
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
```
- `tasks_example.yaml` documents the keys: dataset(s) or file list, `isMC`, `dataFormat`, lumi mask, splitting, output site, and the `runNtuplizer_cfg.py` options (`skim`, `globalTag`, `channels`, …).
- `isMC` and `dataFormat` default to the dataset's data tier and must agree with it.
- `makeFileList.py` writes the LFN list for private samples.

## Layout
| path | content |
|---|---|
| `RadiativeAnalysis/plugins/RadiativeAnalysis.cc` | the ntuplizer (EDAnalyzer), which orchestrates the library components. Channels: μμγ (ECAL photon), μμγ (conversion), μμγγ, μμKK (normalisation) |
| `RadiativeAnalysis/interface`, `src` | library (namespace `bmmg`): `MMGVertexFit` (μμγ kinematic fit), object selection, `DimuonBuilder`, `CandidateBuilders`, `CandidateObservables`, `TriggerMatcher`, `GenChain` (gen truth), `BranchSet`/`NtupleWriter` |
| `RadiativeAnalysis/plugins/MMGVertexFitExample.cc`, `test/MMGVertexFitExample.py` | standalone validation of the photon kinematic fit (pulls, resolution) |
| `RadiativeAnalysis/test` | configs, `test_files.txt`, unit tests, `compareNtuples.py` (regression check), `validateMMGFit.py` (fit validation), `checkDecayModes.py` (consistency check per MC decay mode) |
| `RadiativeAnalysis/devel` | not compiled: custom low-energy photon producer (work in progress), old debugging analyzers |
| `run3mvaid` | Run-3 soft-muon MVA (XGBoost) |

## Documentation
- [NTUPLE.md](NTUPLE.md): branches of the ntuple.
- [plan.md](plan.md): development plan and its status.
- [report.md](report.md): log of the changes, validations and decisions.
- [structure.md](structure.md): structure, processing flow and cuts of the analysis code.
- [structure_old.md](structure_old.md): description of the code before the restructuring. The old ntuplizer (`RadiativeAnalysisLegacy`) was removed in plan step P7.4; it is still in the git history (commit `bde3123`).

## MC production
Private AOD MC production is done in a separate repository: https://github.com/mumuhamm/PrivateMCProduction, branch `devel_GPExtrapolationPhaseI`. `RadiativeAnalysis/test/fragmentRequest.sh` downloads generator fragments.
