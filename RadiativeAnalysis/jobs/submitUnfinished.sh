#!/usr/bin/bash

tag='JpsiMassG_resub'
prevTag='JpsiMassG'

run='24B'

for i in {0..7} ; do
	N=`ls -1 ./reports/${prevTag}/${run}${i}/ | wc -l`
	if [ $N == 7 ] ; then
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=50 \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}${i}/notFinishedLumis.json \
		Data.unitsPerJob=2 JobType.maxJobRuntimeMin=480
	fi
done

run='24C'

for i in {0..7} ; do
	N=`ls -1 ./reports/${prevTag}/${run}${i}/ | wc -l`
	if [ $N == 7 ] ; then
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=50 \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}${i}/notFinishedLumis.json \
		Data.unitsPerJob=2 JobType.maxJobRuntimeMin=480
	fi
done

run='24D'

for i in {0..7} ; do
	N=`ls -1 ./reports/${prevTag}/${run}${i}/ | wc -l`
	if [ $N == 7 ] ; then
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=50 \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}${i}/notFinishedLumis.json \
		Data.unitsPerJob=2 JobType.maxJobRuntimeMin=480
	fi
done