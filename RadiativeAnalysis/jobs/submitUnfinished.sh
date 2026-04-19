#!/usr/bin/bash

tag='JpsiGMidApril_resub2'
prevTag='JpsiGMidApril_resub'

for run in B C D F G H ; do
	for i in {0..7} ; do
		name='reports/'24${run}'_'${prevTag}'_BsToJpsiGamma/crab_'${prevTag}'_dset'${i}'/'
		N=`ls -1 ./${name} | wc -l`
		if [ $N == 7 ] ; then
			crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=100 \
			General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
			Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v1/MINIAOD \
			Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/${name}notFinishedLumis.json \
			Data.unitsPerJob=10 JobType.maxJobRuntimeMin=480
		fi
	done
done
