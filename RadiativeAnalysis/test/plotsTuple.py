#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("./outputData/BsToJpsiGammaMC_triggerBit.root","READ")
ntuple = tupleFile.Get("tMuMuGamma")
ntuple.Print()

# Create a histogram
branchName = "flightLength"
histo = r.TH1D("h"+branchName, branchName+" 2024BC", 100, 0.,0.5)
ntuple.Project("h"+branchName, branchName, "triggerBit == 1")
histo.SetDirectory(0)

tupleFile.Close()

# Histo options
histo.SetTitle("Flight length between best PV and fitted SV; Distance [cm]; Counts")
histo.SetStats(0)

# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
canvas.SetLogy(1)
canvas.cd()
histo.Draw()

l = r.TLatex()
l.SetTextFont(42)
l.SetTextSize(0.04)
l.DrawLatex(2.5,190.,"HLT_DoubleMu4_3_LowMass_v1")


canvas.Print("h"+branchName+"Temp"+".pdf")
input('press enter to exit')