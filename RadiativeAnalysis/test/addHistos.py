#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

cutTitle = "cut8h"
histname = "hcandBsMass"
rebinN = 10

# Open files and get histograms
f1 = r.TFile.Open("JpsiGStartApril_23"+cutTitle+".root")
f2 = r.TFile.Open("JpsiGMidApril_24"+cutTitle+".root")

h1 = f1.Get(histname)
h2 = f2.Get(histname)

# Ensure errors are handled properly
h1.Sumw2()
h2.Sumw2()

# Add h2 to h1
h1.Add(h2)
h1.SetFillColorAlpha(18, 0.4)

# Rebin the histogram
h1 = h1.Rebin(rebinN)

# Draw and keep the canvas alive
c = r.TCanvas()
h1.Draw("hist")

c.Print("JpsiGStartMidApril_23_24_"+cutTitle+"reb"+str(rebinN)+".pdf")
input('press enter to exit')

outFile = r.TFile("JpsiGStartMidApril_23_24_"+cutTitle+"reb"+str(rebinN)+".root",'RECREATE')
h1.Write()
outFile.Close()