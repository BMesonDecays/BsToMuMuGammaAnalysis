#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

tag = "McSampleDetails"

# Get the tuple
tupleFile = r.TFile(tag+".root","READ")
ntuple = tupleFile.Get("tOut")

# Save restricted tuple
outputFile = r.TFile(tag+"_MuonsMatchedDiff.root","RECREATE")
newNtuple = ntuple.CopyTree("dRMuon1 < 100.0 && dRMuon2 < 100.0 && twoReMaMuonsDR > 0.0")
newNtuple.Write()
outputFile.Close()
tupleFile.Close()