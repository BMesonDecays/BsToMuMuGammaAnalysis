#!/usr/bin/bash

tag='JpsiGEndMarch_resub'

for run in B C D F G H ; do
    for i in {0..7} ; do
        crab status -d 24${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} >> status.txt
    done
done
