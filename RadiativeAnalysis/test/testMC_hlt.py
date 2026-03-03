#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

filePath = "./outputData/BsToJpsiGammaMC_JpsiMassConstrained_HLT.root"
tupleName = "tJpsi"
triggerNumber = 3


# Get the tuple
tupleFile = r.TFile(filePath,"READ")
ntuple = tupleFile.Get(tupleName)

trigger3Cut = r.TCut("trigger3Cut","hltPaths % 10000 >= 1000")
trigger7Cut = r.TCut("trigger7Cut","hltPaths % 1.E8 >= 1.E7")
trigger8Cut = r.TCut("trigger8Cut","hltPaths % 1.E9 >= 1.E8")

# Copy the ntuple limited by the cuts
outputFile = r.TFile("testHlt.root","RECREATE")
newNtuple = ntuple.CopyTree(str(trigger8Cut))
newNtuple.Write("new")
outputFile.Close()
tupleFile.Close()
