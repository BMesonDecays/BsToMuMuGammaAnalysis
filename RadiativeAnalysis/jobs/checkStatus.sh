#!/usr/bin/bash

tag='JpsiMassG_imp_resub1'

run='24G'

for i in {0..7} ; do
    crab status -d ${run}_${tag}_BsToJpsiGamma/crab_${tag}_dset${i} >> status.txt
done