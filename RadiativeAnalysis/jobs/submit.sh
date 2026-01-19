#!/usr/bin/bash

tag='FL_dR_resub'
run='24C'


for i in {4,7} ; do
	crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} \
	General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
	Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
	Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/FL_dR_3perJob/24C${i}/notFinishedLumis.json
done
