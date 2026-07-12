#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

datasetName = "23_24_JpsiGApril_cut2"
tupleName = "tOut"

# Get the tuple
tupleFile = r.TFile("./outputData/"+datasetName+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Get the list of branch entries
branchName = "cosAnDimuonBSpot2D"
entriesVals = [getattr(event, branchName) for event in ntuple]

# Calculate the theta angles
thetaVals = np.arccos(entriesVals)
thetaVals = np.degrees(thetaVals)
nOfEntries = thetaVals.size

# Create and save a histogram
histo = r.TH1D("histo","Pointing angle values [deg]",1000,0.,180.)
histo.FillN(nOfEntries, thetaVals, np.ones(nOfEntries))

canvas = r.TCanvas("canvas")
canvas.cd()
canvas.SetLogy(1)
histo.Draw()
canvas.Print("temp.pdf")

input('press enter to exit')

