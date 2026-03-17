#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys


# Get the histogram
histfilename = "hBsXYlifetime_twoMuons.root"
histname = "h100BsXYlifetime_twoMuons_cut2"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Get the function
funcFileName = histname+"_fitFunc.root"
funcName = "fitFunc"
funcFile = r.TFile.Open(funcFileName,"READ")
func = funcFile.Get(funcName)

# Set the function parameters
#func.SetParameter(0,0.125)

histo.SetAxisRange(0.,8.E3,"Y")
# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw()
func.Draw("same")
canvas.Print(histname+"_lin3.pdf")
input('press enter to exit')

funcFile.Close()
