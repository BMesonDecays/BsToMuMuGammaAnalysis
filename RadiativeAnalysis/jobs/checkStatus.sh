#!/usr/bin/bash

tag='JpsiGStartApril_resub'
run='23C'

for i in {0..7} ; do
    for j in {1..4} ; do
        crab status -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i}_vol${j} >> status.txt
    done
done

