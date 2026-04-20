#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

analysisTag = "JpsiGMidApril_24"
tupleName = "tOut"
tag = "24DataCut4"

# Get the tuple
tupleFile = r.TFile("./outputData/24allBtoI_JpsiGMidApril_cut1.root","READ")
#tupleFile = r.TFile("./outputData/"+analysisTag+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Define the cuts
cutList = []

cutList.append(r.TCut("muonIdCut","muon1Id > 0.7 && muon2Id > 0.7"))
cutList.append(r.TCut("fittedDimuonVertexProbCut","fittedDimuonVertexProb > 0.1"))
cutList.append(r.TCut("maxMuonsVertexCompCut","maxMuonsVertexComp < 0.1"))
cutList.append(r.TCut("fittedDimuonMassCut","TMath::Abs(fittedDimuonMass - 3.097) < 0.06"))
cutList.append(r.TCut("lXY_fittedDimuon_bSpot_sigCut","lXY_fittedDimuon_bSpot_sig > 5.0"))

cutList.append(r.TCut("cosAnDimuonBSpot2DCut","cosAnDimuonBSpot2D > 0.985"))
cutList.append(r.TCut("cosAngleBsBSpot2DCut","cosAngleBsBSpot2D > 0.99998"))

# trigger cuts
#cutList.append(r.TCut("triggerAnyCut","triggerRes > 0.0"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

# draw a plot
ntuple.Draw("cosAngleBsBSpot2D:cosAnDimuonBSpot2D",str(totalCut))
graph = r.gPad.GetPrimitive("Graph")
graph.SetTitle(tag+";cosAnDimuonBSpot2D;cosAngleBsBSpot2D")
graph.SetMarkerSize(0.2)
graph.SetMarkerStyle(20)
graph.GetXaxis().SetLimits(0.984,1.0002)
graph.GetYaxis().SetRangeUser(0.99994,1.000001)

canvas = r.TCanvas("can","",700,600)
canvas.SetLeftMargin(0.18)
canvas.SetRightMargin(0.02)
canvas.cd()
graph.Draw("APX")


canvas.Print(tag+"PAngle.pdf")
input("Press enter to exit")