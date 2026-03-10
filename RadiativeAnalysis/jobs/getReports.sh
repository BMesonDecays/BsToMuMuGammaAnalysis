#!/usr/bin/bash

tag='JpsiMassG_imp_resub'

run='24G'

for i in {0..7} ; do
    crab report --dir ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} --outputdir reports/${tag}/${run}_${i}
done
