#!/usr/bin/bash

tag='JpsiMassG_resub'
prevTag='JpsiMassG'

run='24I'

vol=1
for i in {0..7} ; do
	N=`ls -1 ./reports/${prevTag}/${run}_vol${vol}_${i}/ | wc -l`
	if [ $N == 7 ] ; then
		crab submit --config='crabConfigData.py' General.requestName=${tag}_vol${vol}_dset${i} JobType.priority=50 \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v${vol}/MINIAOD \
		Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}_vol${vol}_${i}/notFinishedLumis.json \
		Data.unitsPerJob=5 JobType.maxJobRuntimeMin=180
	fi
done

vol=2
for i in {0..7} ; do
	N=`ls -1 ./reports/${prevTag}/${run}_vol${vol}_${i}/ | wc -l`
	if [ $N == 7 ] ; then
		crab submit --config='crabConfigData.py' General.requestName=${tag}_vol${vol}_dset${i} JobType.priority=50 \
		General.workArea=${run}_${tag}_BsToJpsiGamma Data.outputDatasetTag=${run}_${tag}_BsToJpsiGamma \
		Data.inputDataset=/ParkingDoubleMuonLowMass${i}/Run20${run}-PromptReco-v${vol}/MINIAOD \
		Data.lumiMask=/eos/user/p/psajdak/CMSSW_14_1_1/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/jobs/reports/${prevTag}/${run}_vol${vol}_${i}/notFinishedLumis.json \
		Data.unitsPerJob=5 JobType.maxJobRuntimeMin=180
	fi
done
