#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the tuple
tupleFile = r.TFile("./outputData/getDeltaR.root","READ")
ntuple = tupleFile.Get("tOut")
ntuple.Print()

cut = "triggerRes > 0.0"

r.gStyle.SetOptStat("emro")

# Create a histogram
branchName = "dR_photonFittedDimuon"
histo = r.TH1D("h"+branchName,branchName, 80,0.0,0.5)
ntuple.Project("h"+branchName,branchName)#,cut)
histo.SetDirectory(0)

# branchName = "dRMuon2"
# histo2 = r.TH1D("h"+branchName,branchName, 100,0.,10.0)
# ntuple.Project("h"+branchName, branchName)
# histo2.SetDirectory(0)

tupleFile.Close()

# # Histo options
# histo.SetTitle("0.8 < cos < 0.96"+";dimuon mass [GeV];Counts")
# #r.gStyle.SetTitleSize(0.1,"t")
# histo.SetStats(0)
# histo.GetXaxis().SetLabelSize(0.04)
# histo.GetXaxis().SetTitleSize(0.04)
# histo.GetXaxis().SetTitleOffset(1.0)
# histo.GetYaxis().SetLabelSize(0.04)
# histo.GetYaxis().SetTitleSize(0.04)
# histo.GetYaxis().SetTitleOffset(0.95)

# histo.Sumw2()
# # histo2.Sumw2()

# # Add h2 to h1
# histo.Add(histo2)

histo.SetFillColorAlpha(18, 0.4)
histo.SetAxisRange(0.0,120.0,"Y")


# Draw and save the histogram
canvas = r.TCanvas("c"+str(histo.GetTitle()))
#canvas.SetLogy(1)
# canvas.SetLeftMargin(0.08)
# canvas.SetRightMargin(0.02)
# canvas.SetTopMargin(0.07)
canvas.cd()

histo.SetBit(r.TH1.kNoTitle)
histo.Draw()

canvas.Update()

stats = histo.GetListOfFunctions().FindObject("stats")

stats.SetX1NDC(0.7)  # Left edge
stats.SetY1NDC(0.7)  # Bottom edge
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

canvas.Print("temp.pdf")
input('press enter to exit')