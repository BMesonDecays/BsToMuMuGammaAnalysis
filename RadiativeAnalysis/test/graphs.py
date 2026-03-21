#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import sys
import math
from ROOT import *

fileName = "genRecoPhotonEnergyErBsPhotons.root"

print ('Read data from: ', fileName)
gROOT.Reset()
f = TFile(fileName);
f.ls();

graphname = "gBsPhotons"

#'''
c1 = TCanvas('cHisto','cHisto',800,600)
#c1.SetLogx()
#c1.SetLogy()


# graph
graph = gROOT.FindObject(graphname)
#graph.GetXaxis().SetTitle("Largest nTracks among reco PVs")
#graph.GetYaxis().SetTitle("nTracks of the best (chosen) PV")

print(graph.GetN())
graph.SetMarkerSize(0.2)
graph.SetMarkerStyle(20)
graph.GetXaxis().SetRangeUser(0,120)
graph.GetYaxis().SetRangeUser(0,150)
graph.Draw("A*")

#'''
# line y=x
start = 0.
end = 120.
line = TLine(start,start,end,end)
line.SetLineColor(2)
line.Draw()

#'''

c1.Print("temp.pdf")
input('press enter to exit')
#'''