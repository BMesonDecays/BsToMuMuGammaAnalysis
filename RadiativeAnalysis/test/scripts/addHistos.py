#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

#cutTitle = "cut8h"
#histname = "hcandBsMass"
# rebinN = 10

# Open files and get histograms
f1 = r.TFile.Open("muonIds.root")
#f2 = r.TFile.Open("JpsiGMidApril_24"+cutTitle+".root")

h1 = f1.Get("hmuon1Id")
h2 = f1.Get("hmuon2Id")

# Ensure errors are handled properly
h1.Sumw2()
h2.Sumw2()

# Add h2 to h1
h1.Add(h2)
h1.SetFillColorAlpha(18, 0.4)

# Rebin the histogram
# h1 = h1.Rebin(rebinN)

# Draw and keep the canvas alive
c = r.TCanvas()
h1.SetStats(0)
h1.SetTitle("Muon Id score")
h1.Draw("hist")

c.Print("muonIdsMCCut0Hlt.pdf")
input('press enter to exit')


# outFile = r.TFile("JpsiGStartMidApril_23_24_looserPointingAngcut4_"+"reb"+str(rebinN)+".root",'RECREATE')
# h1.Write()
# outFile.Close()