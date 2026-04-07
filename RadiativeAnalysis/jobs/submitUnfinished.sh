#!/usr/bin/bash

tag='JpsiGStartApril_resub3'
prevTag='JpsiGStartApril_resub2'

run='23D'

for i in {0..7} ; do
	for j in {1..2} ; do
		name='reports/'${run}'_'${prevTag}'_BsToJpsiGamma/crab_'${prevTag}'_dset'${i}'_vol'${j}'/'
		N=`ls -1 ./${name} | wc -l`
		if [ $N == 7 ] ; then
			crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i}_vol${j} JobType.priority=100 \
			General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
			Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v${j}/MINIAOD \
			Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/${name}notFinishedLumis.json \
			Data.unitsPerJob=1 JobType.maxJobRuntimeMin=60
		fi
	done
done
