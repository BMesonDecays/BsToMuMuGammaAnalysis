#!/usr/bin/bash

tag='JpsiMassG'

run='24B'

for i in {0..7} ; do
    crab kill -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i}
    crab report --dir ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} --outputdir reports/${tag}/${run}${i}
done
