#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("./outputData/23_24_JpsiGApril_cut2_mass.root","READ")
ntuple = tupleFile.Get("tOut")
ntuple.Print()

cut = "cosAnDimuonBSpot2D < 0.96"

# Create a histogram
branchName = "fittedDimuonMass"
histo = r.TH1D("h"+branchName,branchName, 100,2.9,3.3)
ntuple.Project("h"+branchName, branchName,"cosAnDimuonBSpot2D > 0.8 && "+cut)
histo.SetDirectory(0)

# branchName = "dRMuon2"
# histo2 = r.TH1D("h"+branchName,branchName, 100,0.,10.0)
# ntuple.Project("h"+branchName, branchName)
# histo2.SetDirectory(0)

tupleFile.Close()

# Histo options
histo.SetTitle("0.8 < cos < 0.96"+";dimuon mass [GeV];Counts")
#r.gStyle.SetTitleSize(0.1,"t")
histo.SetStats(0)
histo.GetXaxis().SetLabelSize(0.04)
histo.GetXaxis().SetTitleSize(0.04)
histo.GetXaxis().SetTitleOffset(1.0)
histo.GetYaxis().SetLabelSize(0.04)
histo.GetYaxis().SetTitleSize(0.04)
histo.GetYaxis().SetTitleOffset(0.95)

# histo.Sumw2()
# # histo2.Sumw2()

# # Add h2 to h1
# histo.Add(histo2)

histo.SetFillColorAlpha(18, 0.4)

# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
#canvas.SetLogy(1)
canvas.SetLeftMargin(0.08)
canvas.SetRightMargin(0.02)
canvas.SetTopMargin(0.07)
canvas.cd()
histo.Draw("hist")

'''
l = r.TLatex()
l.SetTextFont(42)
l.SetTextSize(0.04)
l.DrawLatex(0.5,200.,"lXY_muonsKalman_bSpot_significance > 3.0")
'''

canvas.Print("hCut3"+branchName+"Cos_"+cut[-2:]+".pdf")
input('press enter to exit')