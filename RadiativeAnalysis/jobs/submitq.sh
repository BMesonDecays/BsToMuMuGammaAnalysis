#!/usr/bin/bash

tag='JpsiGtestAOD'
run='24C'

i=0
crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} \
General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/AOD \
Data.lumiMask=/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/20${run}_Golden.json \
Data.unitsPerJob=4
