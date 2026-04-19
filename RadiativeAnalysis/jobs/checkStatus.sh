#!/usr/bin/bash

tag='JpsiGMidApril_resub2'

for run in B C D F G H ; do
	for i in {0..7} ; do
        crab status -d 24${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} >> status.txt
	done
    echo -e "\n\n" >> status.txt
done
