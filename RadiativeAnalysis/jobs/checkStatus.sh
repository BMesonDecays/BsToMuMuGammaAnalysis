#!/usr/bin/bash

tag='FL_dR_resub3'
run='24D'

for i in {0..7} ; do
    crab status -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} >> status.txt
done