#!/usr/bin/bash

tag='JpsiMassG_imp_resub1'
prevTag='JpsiMassG_imp_resub'

run='24G'

for i in {0..7} ; do
	N=`ls -1 ./reports/${prevTag}/${run}_${i}/ | wc -l`
	if [ $N == 7 ] ; then
		crab submit --config='crabConfigData.py' General.requestName=${tag}_dset${i} JobType.priority=50 \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v1/MINIAOD \
		Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}_${i}/notFinishedLumis.json \
		Data.unitsPerJob=1 JobType.maxJobRuntimeMin=150
	fi
done
