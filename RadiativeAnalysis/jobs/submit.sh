#!/usr/bin/bash

tag='JpsiGStartApril'
run='23D'

for i in {0..7} ; do
	for j in {1..2} ; do
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i}_vol${j} \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v${j}/MINIAOD \
		Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions23/PromptReco/Cert_Collisions2023_eraD_369803_370790_Golden.json \
		Data.unitsPerJob=200 JobType.maxJobRuntimeMin=480
	done
done
