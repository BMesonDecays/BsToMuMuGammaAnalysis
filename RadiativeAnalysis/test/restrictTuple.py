#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

tag = "JpsiGMidApril_MC"

# Get the tuple
tupleFile = r.TFile("./outputData/"+tag+".root","READ")
ntuple = tupleFile.Get("tOut")

# Save restricted tuple
outputFile = r.TFile(tag+"_EES.root","RECREATE")
newNtuple = ntuple.CopyTree("TMath::Abs(etaPhoton) > 1.65 && TMath::Abs(etaPhoton) < 2.5 && initPhotonEnergy < 90.0")
newNtuple.Write()
outputFile.Close()