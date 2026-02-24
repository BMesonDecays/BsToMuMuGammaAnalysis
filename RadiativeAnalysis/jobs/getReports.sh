#!/usr/bin/bash

tag='JpsiMassConstrained1_resubmit2'
run='24D'

for i in {0..7} ; do
    crab report --dir ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} --outputdir reports/${tag}/${run}${i}
done

