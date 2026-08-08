#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

#cutTitle = "cut8h"
#histname = "hcandBsMass"
# rebinN = 10

# Open files and get histograms
f1 = r.TFile.Open("JpsiGApril_23_24_July_BsMasscut0.root")
#f2 = r.TFile.Open("JpsiGMidApril_24"+cutTitle+".root")

h1 = f1.Get("hmuon1Id")
h2 = f1.Get("hmuon2Id")

# Ensure errors are handled properly
h1.Sumw2()
h2.Sumw2()

# Add h2 to h1
h1.Add(h2)
r.gStyle.SetOptStat("emr")
r.gStyle.SetStatFontSize(0.04)
h1.UseCurrentStyle()
h1.SetFillColorAlpha(18, 0.4)

# Rebin the histogram
# h1 = h1.Rebin(rebinN)

# Draw and keep the canvas alive
c = r.TCanvas()
# h1.SetStats(0)
h1.SetTitle("Muon Id score")
h1.Draw("hist")
c.Update()

stats = h1.GetListOfFunctions().FindObject("stats")

stats.SetX1NDC(0.15)  # Left edge
stats.SetY1NDC(0.72)  # Bottom edge
stats.SetX2NDC(0.45)  # Right edge
stats.SetY2NDC(0.85)  # Top edge

c.Modified()
c.Update()


c.Print("muonIdsDATACut0BsMass.pdf")
input('press enter to exit')


# outFile = r.TFile("JpsiGStartMidApril_23_24_looserPointingAngcut4_"+"reb"+str(rebinN)+".root",'RECREATE')
# h1.Write()
# outFile.Close()