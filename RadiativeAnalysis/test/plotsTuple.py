#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("McSampleDetails_PhotonMatched.root","READ")
ntuple = tupleFile.Get("tOut")
ntuple.Print()

# Create a histogram
branchName = "dRPhoton"
histo = r.TH1D("h"+branchName,branchName, 100,0.,0.1)
ntuple.Project("h"+branchName, branchName)
histo.SetDirectory(0)

tupleFile.Close()

# Histo options
histo.SetTitle("#DeltaR between the generated and matched photons;#DeltaR;Counts")
histo.SetStats(0)
#histo.GetXaxis().SetLabelSize(0.03)
histo.GetXaxis().SetTitleSize(0.05)
histo.GetXaxis().SetTitleOffset(0.8)

histo.SetFillColorAlpha(18, 0.4)

# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
#canvas.SetLogy(1)
canvas.cd()
histo.Draw("hist")

'''
l = r.TLatex()
l.SetTextFont(42)
l.SetTextSize(0.04)
l.DrawLatex(0.5,200.,"lXY_muonsKalman_bSpot_significance > 3.0")
'''

canvas.Print("h"+branchName+"Temp"+".pdf")
input('press enter to exit')