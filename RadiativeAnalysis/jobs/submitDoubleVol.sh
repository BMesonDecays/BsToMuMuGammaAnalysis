#!/usr/bin/bash

tag='JpsiGMidApril'

for run in E I ; do
	for i in {0..7} ; do
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i}_vol1 \
		General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/2024${run}_Golden.json \
		Data.unitsPerJob=200 JobType.maxJobRuntimeMin=480

		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i}_vol2 \
		General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v2/MINIAOD \
		Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/2024${run}_Golden.json \
		Data.unitsPerJob=200 JobType.maxJobRuntimeMin=480
	done
done
