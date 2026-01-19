#!/usr/bin/bash

tag='FL_dR_3perJob'

for run in {'24C','24D'} ; do
	for i in {0..7} ; do
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/20${run}_Golden.json
	done
done
