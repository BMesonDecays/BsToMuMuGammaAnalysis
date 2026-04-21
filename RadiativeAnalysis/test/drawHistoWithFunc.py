#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# Get the histogram
histfilename = "./mass23_24JpsiGStartMidApril/dataFiles/JpsiGStartMidApril_23_24_cut8hreb5.root"
histname = "hcandBsMass"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Get the function
funcFileName = "hcandBsMass8hImpreb5"+"_fitFunc.root"
funcName = "fitFunc"
funcFile = r.TFile.Open(funcFileName,"READ")
func = funcFile.Get(funcName)

# Set the function parameters
func.SetParameter(0,2.33)
func.SetLineWidth(1)

#histo.SetAxisRange(0.,180.,"Y")
#histo.SetAxisRange(3.8,7.,"X")
#histo.SetAxisRange(4.5,6.5,"X")

# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw("histo")
func.Draw("same")
canvas.Print("signalTemp.pdf")
input('press enter to exit')

funcFile.Close()
