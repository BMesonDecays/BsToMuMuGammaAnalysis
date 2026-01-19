#!/usr/bin/bash

tag='FL_dR_3perJob'
run='24C'

for i in {4,7} ; do
    crab report --dir ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} --outputdir reports/${tag}/${run}${i}
done