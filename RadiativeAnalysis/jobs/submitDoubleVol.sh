#!/usr/bin/bash

tag='JpsiMassG'
run='24I'

vol=1
for i in {0..7} ; do
    crab submit --config='crabConfigData.py' General.requestName=${tag}_vol${vol}_dset${i} \
	General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
	Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v${vol}/MINIAOD \
	Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/20${run}_Golden.json \
    Data.unitsPerJob=15 JobType.maxJobRuntimeMin=60
done

vol=2
for i in {0..7} ; do
    crab submit --config='crabConfigData.py' General.requestName=${tag}_vol${vol}_dset${i} \
	General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
	Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v${vol}/MINIAOD \
	Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/20${run}_Golden.json \
    Data.unitsPerJob=15 JobType.maxJobRuntimeMin=60
done