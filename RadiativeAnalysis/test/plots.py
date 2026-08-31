#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get histogram(s)
histoFile = r.TFile("recoSpectra.root","READ")
histo = histoFile.Get("hRecoPhotonPt")
# histo2 = histoFile.Get("hXxxx")

histo.SetDirectory(0)
# histo2.SetDirectory(0)
histoFile.Close()

# Add histograms
# histo.Sumw2()
# histo2.Sumw2()

# histo.Add(histo2)

# Histogram options
histo.Rebin(20)
histo.SetAxisRange(0.,30.)
r.gStyle.SetOptStat("emruo")
r.gStyle.SetStatFontSize(0.04)
histo.UseCurrentStyle()
histo.SetFillColorAlpha(18, 0.4)

# Drawing
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
stats.SetY1NDC(0.68)  # Bottom edge
stats.SetX2NDC(0.9)  # Right edge
stats.SetY2NDC(0.9)  # Top edge

canvas.Modified()
canvas.Update()

canvas.Print("hTemp.pdf")
input('press enter to exit')