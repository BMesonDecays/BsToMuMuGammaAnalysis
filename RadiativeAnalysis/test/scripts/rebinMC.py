#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

cutTitle = "8hImp"
histname = "hcandBsMass"
rebinN = 5

# Open files and get histograms
f1 = r.TFile.Open("JpsiGMidApril_MC"+cutTitle+".root")

h1 = f1.Get(histname)

h1.SetFillColorAlpha(18, 0.4)

# Rebin the histogram
h1 = h1.Rebin(rebinN)

# Draw and keep the canvas alive
c = r.TCanvas()
h1.Draw("hist")

c.Print("JpsiGMidApril_MC_"+cutTitle+"reb"+str(rebinN)+".pdf")
input('press enter to exit')

outFile = r.TFile("JpsiGMidApril_MC_"+cutTitle+"reb"+str(rebinN)+".root",'RECREATE')
h1.Write()
outFile.Close()