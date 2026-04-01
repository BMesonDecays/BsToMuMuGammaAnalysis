#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys


# Get the histogram
histfilename = "hCandBsMassCut6.root"
histname = "hcandBsModMass"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Get the function
funcFileName = "JpsiGEndMarchMCmassFits/hMCcandBsModMassCut6"+"_fitFunc.root"
funcName = "fitFunc"
funcFile = r.TFile.Open(funcFileName,"READ")
func = funcFile.Get(funcName)

# Set the function parameters
func.SetParameter(0,1.73)
func.SetLineWidth(1)

#histo.SetAxisRange(0.,180.,"Y")
#histo.SetAxisRange(3.8,7.,"X")
histo.SetAxisRange(4.8,6.2,"X")

# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw()
func.Draw("same")
canvas.Print("temp.pdf")
input('press enter to exit')

funcFile.Close()
