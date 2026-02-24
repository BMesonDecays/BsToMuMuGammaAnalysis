#!/usr/bin/bash

tag='JpsiMassConstrained1_resubmit3'
prevTag='JpsiMassConstrained1_resubmit2'
run='24D'


for i in {0..7} ; do
	N=`ls -1 ./reports/${prevTag}/${run}${i}/ | wc -l`
	if [ $N == 7 ] ; then
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=50 \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}${i}/notFinishedLumis.json \
		Data.unitsPerJob=1 JobType.maxJobRuntimeMin=240
	fi
done

