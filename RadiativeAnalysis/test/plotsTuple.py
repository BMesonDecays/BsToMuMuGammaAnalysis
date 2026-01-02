#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("./outputData/tBsToJpsiGamma_RecoOnly_allMC.root","READ")
ntuple = tupleFile.Get("tMuMuGamma")
#ntuple.Print()

# Create a histogram
branchName = "minPCA_distance"
histo = r.TH1D("h"+branchName, branchName, 100, 0.,0.2)
ntuple.Project("h"+branchName, branchName)
histo.SetDirectory(0)

tupleFile.Close()

# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
canvas.cd()
histo.Draw()
canvas.Print("h"+branchName+"Reco"+".pdf")
input('press enter to exit')
