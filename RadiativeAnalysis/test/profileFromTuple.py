#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

tupleFileName = "./outputData/23_24_JpsiGApril_cut2.root"
tupleName = "tOut"
# outputName = "EES"

###########################
tupleFile = r.TFile(tupleFileName,"READ")
ntuple = tupleFile.Get(tupleName)

nPoints = ntuple.GetEntries()

prof = r.TProfile("prof","profile",10,-1.0,1.0)


for i in range(nPoints):
    ntuple.GetEntry(i)
    prof.Fill(ntuple.cosAngleBsBSpot2D,ntuple.lXY_fittedDimuon_bSpot)


# Drawing
canvas = r.TCanvas("canvas")
canvas.cd()

r.gStyle.SetErrorX(0)
#r.gStyle.SetMarkerSize(0.2)
#r.gStyle.SetMarkerStyle(20)
#r.gStyle.SetLineWidth(1)

prof.SetStats(0)
prof.Draw("||")


canvas.Print("tempProfile.pdf")
input('press enter to exit')

'''
# save just graph pdf
c2 = r.TCanvas("c2")
c2.cd()
graph.GetXaxis().SetRangeUser(0,60)
graph.GetYaxis().SetRangeUser(0,60)
graph.Draw("APX")
line.DrawLine(0.0,0.0,60.,60.)
c2.Print(tupleName+outputName+".pdf")
input('press enter to exit')
'''
