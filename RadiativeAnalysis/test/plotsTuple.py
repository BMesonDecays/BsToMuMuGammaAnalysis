#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("BsToJpsiGammaData.root","READ")
ntuple = tupleFile.Get("tMuMuGamma")
ntuple.Print()


# Create a histogram
branchName = "minFlightPath"
histo = r.TH1D("h"+branchName, branchName, 100, 0.,0.1)
ntuple.Project("h"+branchName, branchName, "TMath::Abs(M_dimuon - 3.0969) < 0.06")
histo.SetDirectory(0)

tupleFile.Close()

# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
canvas.cd()
histo.Draw()
canvas.Print("h"+branchName+"Temp"+".pdf")
input('press enter to exit')