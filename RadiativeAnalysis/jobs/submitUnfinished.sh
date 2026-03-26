#!/usr/bin/bash

tag='JpsiGEndMarch_resub'
prevTag='JpsiGEndMarch'

for run in B C D F G H ; do
	for i in {0..7} ; do
		N=`ls -1 ./reports/${prevTag}/24${run}_${i}/ | wc -l`
		if [ $N == 7 ] ; then
			crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=50 \
			General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
			Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v1/MINIAOD \
			Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/24${run}_${i}/notFinishedLumis.json \
			Data.unitsPerJob=50 JobType.maxJobRuntimeMin=480
		fi
	done
done
