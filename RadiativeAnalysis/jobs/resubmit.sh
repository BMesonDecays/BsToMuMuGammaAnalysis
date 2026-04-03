#!/usr/bin/bash

tag='JpsiGStartApril'
run='23C'

for i in {0..7} ; do
	for j in {1..4} ; do
		crab resubmit --maxjobruntime=600 -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i}_vol${j}
	done
done
