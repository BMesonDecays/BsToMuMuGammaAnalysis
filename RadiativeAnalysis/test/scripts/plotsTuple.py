#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("genSpectra.root","READ")
ntuple = tupleFile.Get("tOut")
ntuple.Print()

# cut = "TMath::Abs(cosAnDimuonBSpot2D) < 0.8 && TMath::Abs(candBsMass - 5.367) < 1.5"

# Create a histogram
branchName = "mu1Pt"
histo = r.TH1D("h"+branchName,branchName, 200,0.0,15.)
ntuple.Project("h"+branchName, branchName)
histo.SetDirectory(0)

branchName = "mu2Pt"
histo2 = r.TH1D("h"+branchName,branchName, 200,0.,15.0)
ntuple.Project("h"+branchName, branchName)
histo2.SetDirectory(0)

tupleFile.Close()

# Histo options
# histo.SetTitle("abs(cos) < 0.8")
# r.gStyle.SetTitleSize(0.1,"t")
# histo.SetStats(0)
# histo.GetXaxis().SetLabelSize(0.04)
# histo.GetXaxis().SetTitleSize(0.04)
# histo.GetXaxis().SetTitleOffset(1.0)
# histo.GetYaxis().SetLabelSize(0.04)
# histo.GetYaxis().SetTitleSize(0.04)
# histo.GetYaxis().SetTitleOffset(0.95)

histo.Sumw2()
histo2.Sumw2()

# Add h2 to h1
histo.Add(histo2)

r.gStyle.SetOptStat("emro")
r.gStyle.SetStatFontSize(0.04)
histo.UseCurrentStyle()
histo.SetFillColorAlpha(18, 0.4)

# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
#canvas.SetLogy(1)
# canvas.SetLeftMargin(0.08)
# canvas.SetRightMargin(0.02)
# canvas.SetTopMargin(0.07)
canvas.cd()
histo.Draw("hist")

canvas.Update()

stats = histo.GetListOfFunctions().FindObject("stats")

stats.SetX1NDC(0.65)  # Left edge
stats.SetY1NDC(0.65)  # Bottom edge
stats.SetX2NDC(0.9)  # Right edge
stats.SetY2NDC(0.9)  # Top edge

canvas.Modified()
canvas.Update()


'''
l = r.TLatex()
l.SetTextFont(42)
l.SetTextSize(0.04)
l.DrawLatex(0.5,200.,"lXY_muonsKalman_bSpot_significance > 3.0")
'''

canvas.Print("hTemp.pdf")
input('press enter to exit')