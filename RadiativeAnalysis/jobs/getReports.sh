#!/usr/bin/bash

tag='JpsiGEndMarch'

for run in E I ; do
    for i in {0..7} ; do
        crab report --dir 24${run}_${tag}_BsToJpsiGamma/crab_${tag}vol1_dset${i} --outputdir reports/${tag}vol1/24${run}_${i}
        crab report --dir 24${run}_${tag}_BsToJpsiGamma/crab_${tag}vol2_dset${i} --outputdir reports/${tag}vol2/24${run}_${i}
    done
done