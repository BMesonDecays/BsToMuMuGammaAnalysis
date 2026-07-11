#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

lumiRatio = 1.07 * (5/8) /10

# Get the histogram
histfilename = "./looserPointingAng/JpsiGStartMidApril_23_24_looserPointingAngcut4_reb8.root"
histname = "hcandBsMass"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Get the function
funcFileName = "hcandBsMass_MC"+"_fitFunc.root"
funcName = "fitFunc"
funcFile = r.TFile.Open(funcFileName,"READ")
func = funcFile.Get(funcName)

# Set the function parameters
func.SetName("func")
A = func.GetParameter(0)
A *= lumiRatio
func.SetParameter(0,A)
func.SetLineWidth(1)
r.gROOT.GetListOfFunctions().Add(func)

# funcD = func.Clone("funcD")
# r.gROOT.GetListOfFunctions().Add(funcD)
# funcD.SetParameter(0,9.55)
# funcD.SetParameter(1,5.3519)

funcFile.Close()

# funcSum = r.TF1("funcSum","funcS + funcD",4.8,6.1)

#histo.SetAxisRange(0.,180.,"Y")
#histo.SetAxisRange(3.8,7.,"X")
histo.SetAxisRange(4.6,7.0,"X")
histo.SetFillColorAlpha(18, 0.4)

# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw("histo")
#func.Draw("same")
canvas.Print("signalTemp.pdf")
input('press enter to exit')
