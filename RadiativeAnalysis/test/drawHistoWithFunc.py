#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys


# Get the histogram
histfilename = "cut3_Kalman_24BCD_finalMass_BsToJpsiGammaMC_JpsiMassConstrained.root"
histname = "hMMGMass"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Get the function
funcFileName = "hMMGMassnew_fitFunc.root"
funcName = "fitFunc"
funcFile = r.TFile.Open(funcFileName,"READ")
func = funcFile.Get(funcName)

# Set the function parameters
func.SetParameter(0,0.125)

# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw()
func.Draw("same")
canvas.Print("cut3KalmanSignal.pdf")

funcFile.Close()
