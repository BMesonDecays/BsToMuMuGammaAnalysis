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
outputFile = r.TFile(tag+"_AllReco.root","RECREATE")
newNtuple = ntuple.CopyTree("GenDecay == 1.0 && dRPhoton < 100.0 && twoReMaMuonsDR < 100.0")
newNtuple.Write()
outputFile.Close()
tupleFile.Close()