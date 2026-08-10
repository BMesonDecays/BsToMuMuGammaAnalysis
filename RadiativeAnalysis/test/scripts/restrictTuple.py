#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

tag = "outputData/McSampleDetails"

# Get the tuple
tupleFile = r.TFile(tag+".root","READ")
ntuple = tupleFile.Get("tOut")

# Save restricted tuple
outputFile = r.TFile(tag+"_SuccMatched.root","UPDATE")
newNtuple = ntuple.CopyTree("dRMuon1 < 0.01 && dRMuon2 < 0.01 && twoReMaMuonsDR > 0.0 && dRPhoton < 0.03")
newNtuple.Write("bothMuonsAndPhoton")
outputFile.Close()
tupleFile.Close()