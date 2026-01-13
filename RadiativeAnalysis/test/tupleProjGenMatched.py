#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np


# Prepare a directory for the output histograms
dirPath = 'tupleProjections/recoGenMatched_wFP'

# Get the tuple
tupleFile = r.TFile("./outputData/BsToJpsiGammaGenMatched_FlPath.root","READ")
ntuple = tupleFile.Get("tMuMuGamma")

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the binnings
binInfo = {
    branchNames[0] : (100,2.,5.),
    branchNames[1] : (100,0.,1.),
    branchNames[2] : (100,-3.,3.),
    branchNames[3] : (100,0.,0.5),
    branchNames[4] : (100,0.,0.05),
    branchNames[5] : (100,0.,10.),
    branchNames[6] : (100,0.,5.)
}

# Create and fill the histograms
histoList = []
for bname in branchNames:
    binning = binInfo[bname]
    histo = r.TH1D("h"+bname,bname, binning[0],binning[1],binning[2])
    ntuple.Project("h"+bname,bname)
    histo.SetDirectory(0)
    histoList.append(histo)

tupleFile.Close()

# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    canvas.cd()
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")

