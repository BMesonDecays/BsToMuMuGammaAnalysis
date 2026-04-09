#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys


# Get the histogram
histfilename = "300Cut6hcandBsMass"
histname = "hcandBsMass"
histfile = r.TFile.Open(histfilename+'.root',"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Get the function
funcFileName = "JpsiGEndMarchMCmassFits/hMCcandBsMassCut6"+"_fitFunc.root"
funcName = "fitFunc"
funcFile = r.TFile.Open(funcFileName,"READ")
func = funcFile.Get(funcName)

# Set the function parameters
func.SetParameter(0,2.264)
func.SetLineWidth(1)

#histo.SetAxisRange(0.,180.,"Y")
#histo.SetAxisRange(3.8,7.,"X")
histo.SetAxisRange(4.8,6.2,"X")

# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw("histo")
func.Draw("same")
canvas.Print("signal"+histfilename+".pdf")
input('press enter to exit')

funcFile.Close()
