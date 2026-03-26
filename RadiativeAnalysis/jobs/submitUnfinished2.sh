#!/usr/bin/bash

tag='JpsiGEndMarch_resub'
prevTag='JpsiGEndMarch'

for run in E I ; do
	for i in {0..7} ; do
		N=`ls -1 ./reports/${prevTag}vol1/24${run}_${i}/ | wc -l`
		if [ $N == 7 ] ; then
			crab submit --config='crabConfigData.py' General.requestName=${tag}vol1_dset${i} JobType.priority=50 \
			General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
			Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v1/MINIAOD \
			Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}vol1/24${run}_${i}/notFinishedLumis.json \
			Data.unitsPerJob=50 JobType.maxJobRuntimeMin=480
		fi
	done
	for i in {0..7} ; do
		N=`ls -1 ./reports/${prevTag}vol2/24${run}_${i}/ | wc -l`
		if [ $N == 7 ] ; then
			crab submit --config='crabConfigData.py' General.requestName=${tag}vol2_dset${i} JobType.priority=50 \
			General.workArea=24${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=24${run}_${tag}_BsToJpsiGamma \
			Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run2024${run}-PromptReco-v2/MINIAOD \
			Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}vol2/24${run}_${i}/notFinishedLumis.json \
			Data.unitsPerJob=50 JobType.maxJobRuntimeMin=480
		fi
	done
done
