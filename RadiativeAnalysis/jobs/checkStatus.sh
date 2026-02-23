#!/usr/bin/bash

tag='JpsiMassConstrained1'
run='24C'

for i in {0..7} ; do
    crab status -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} >> status.txt
done

run='24D'

for i in {0..7} ; do
    crab status -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} >> status.txt
done