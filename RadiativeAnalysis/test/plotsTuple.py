#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("JpsiGStartApril_MC.root","READ")
ntuple = tupleFile.Get("tOut")
ntuple.Print()

# Create a histogram
branchName = "dPhi"
histo = r.TH1D("h"+branchName, "cos "+branchName, 100,0.9,1.)
ntuple.Project("h"+branchName, "TMath::Cos(dPhi)")
histo.SetDirectory(0)

tupleFile.Close()

# Histo options
#histo.SetTitle("Dimuon mass difference after taking momenta from refitted muon tracks;#DeltaE;Counts")
#histo.SetStats(0)
#histo.GetXaxis().SetLabelSize(0.02)

# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
#canvas.SetLogy(1)
canvas.cd()
histo.Draw()

'''
l = r.TLatex()
l.SetTextFont(42)
l.SetTextSize(0.04)
l.DrawLatex(0.5,200.,"lXY_muonsKalman_bSpot_significance > 3.0")
'''

canvas.Print("h"+branchName+"Temp"+".pdf")
input('press enter to exit')