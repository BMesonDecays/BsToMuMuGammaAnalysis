#!/usr/bin/bash

tag='JpsiMassG'

run='24I'

vol=1
for i in {0..7} ; do
    crab status -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_vol${vol}_dset${i} >> status.txt
done

vol=2
for i in {0..7} ; do
    crab status -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_vol${vol}_dset${i} >> status.txt
done