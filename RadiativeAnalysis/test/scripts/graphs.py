#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3

import sys
import math
from ROOT import *


print ("Hello ROOT")
fileName = "outputData/hgLargestPtNTr.root"

print ('Read data from: ', fileName)
gROOT.Reset()
f = TFile(fileName);
f.ls();

graphname = "gNTrLargestBest"

#'''
c1 = TCanvas('cHisto','cHisto',1000,600)
c1.SetLogx()
c1.SetLogy()


# graph
graph = gROOT.FindObject(graphname)
graph.GetXaxis().SetTitle("Largest nTracks among reco PVs")
graph.GetYaxis().SetTitle("nTracks of the best (chosen) PV")

graph.SetMarkerSize(0.2)
graph.SetMarkerStyle(20)
graph.Draw("A*")

# line y=x
start = 6.
end = 130.
line = TLine(start,start,end,end)
line.SetLineColor(2)
line.Draw()



c1.Print("images/"+graphname+"G.pdf")
input('press enter to exit')
#'''