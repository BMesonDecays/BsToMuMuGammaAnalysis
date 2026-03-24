#!/usr/bin/bash

tag='JpsiGEndMarch'

for run in E I ; do
	for i in {0..7} ; do
		crab submit --config='crabConfigData.py' General.requestName=${tag}vol1_dset${i} \
		General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/2024${run}_Golden.json \
		Data.unitsPerJob=200 JobType.maxJobRuntimeMin=600

		crab submit --config='crabConfigData.py' General.requestName=${tag}vol2_dset${i} \
		General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v2/MINIAOD \
		Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/2024${run}_Golden.json \
		Data.unitsPerJob=200 JobType.maxJobRuntimeMin=600
	done
done
