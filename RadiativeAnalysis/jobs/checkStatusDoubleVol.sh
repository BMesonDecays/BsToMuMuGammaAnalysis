#!/usr/bin/bash

tag='JpsiGMidApril_resub'

for run in E I ; do
	for i in {0..7} ; do
        crab status -d 24${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i}_vol1 >> statusD.txt
        crab status -d 24${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i}_vol2 >> statusD.txt
	done
    echo -e "\n\n" >> statusD.txt
done
