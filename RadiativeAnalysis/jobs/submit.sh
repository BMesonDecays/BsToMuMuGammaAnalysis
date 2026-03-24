#!/usr/bin/bash

tag='JpsiGEndMarch'

for run in B C D F G H ; do
	for i in {0..7} ; do
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} \
		General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/2024${run}_Golden.json \
		Data.unitsPerJob=200 JobType.maxJobRuntimeMin=600
	done
done