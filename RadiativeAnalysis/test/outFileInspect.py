#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

analysisTag = "JpsiGStartApril2_MC"
tupleName = "tOut"

# Get the tuple
tupleFile = r.TFile(analysisTag+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Define the cuts
cutList = []
cutList.append(r.TCut("lXY_fittedDimuon_bSpot_sigCut","lXY_fittedDimuon_bSpot_sig > 5.0"))
cutList.append(r.TCut("cosAngleBsBSpot2DCut","cosAngleBsBSpot2D > 0.997"))
cutList.append(r.TCut("cosAnDimuonBSpot2DCut","cosAnDimuonBSpot2D > 0.98"))
cutList.append(r.TCut("maxMuonsVertexCompCut","maxMuonsVertexComp < 0.1"))


# trigger cuts
cutList.append(r.TCut("triggerCut","triggerRes > 0.0"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

# draw a plot
canvas = r.TCanvas("can")
canvas.cd()
ntuple.Draw("cosAngleBsBSpot2D",str(totalCut))
canvas.Print("temp.pdf")
input("Press enter to exit")