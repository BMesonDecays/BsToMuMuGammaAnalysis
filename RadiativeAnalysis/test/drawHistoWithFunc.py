#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys


# Get the histogram
histfilename = "hMassAfterCompR.root"
histname = "hTwoMuonsphotonMassCut4"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Get the function
funcFileName = "MChTwoMuonsPhotonMassCut4_hlt2"+"_fitFunc.root"
funcName = "fitFunc"
funcFile = r.TFile.Open(funcFileName,"READ")
func = funcFile.Get(funcName)

# Set the function parameters
func.SetParameter(0,2.07)
func.SetLineWidth(1)

histo.SetAxisRange(0.,180.,"Y")
histo.SetAxisRange(5.2,5.7,"X")
# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw()
func.Draw("same")
canvas.Print("cut4DataRSignal.pdf")
input('press enter to exit')

funcFile.Close()
