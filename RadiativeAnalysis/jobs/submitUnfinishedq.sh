#!/usr/bin/bash

tag='JpsiMassConstrained0_resubmit1'
prevTag='JpsiMassConstrained0_resubmit'
run='24B'

i=2

crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=50 \
General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}${i}/lumisToProcess.json \
Data.unitsPerJob=1
