#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

rebin = 4
magRatio = 1.16 * (250*rebin/1000) /10
xMin = 5.367 - 1.5
xMax = 5.367 + 1.5


# Get the histogram
histfilename = "JpsiGApril_23_24_Julycut10.root"
histname = "hcandBsMass"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

histo.Rebin(rebin)

# # Get the function
# funcFileName = "hcandBsMassJuly_MC_fitFuncExt.root"
# funcName = "fitFunc"
# funcFile = r.TFile.Open(funcFileName,"READ")
# func = funcFile.Get(funcName)

# # Set the function parameters
# func.SetName("func")
# A = func.GetParameter(0)
# A *= magRatio
# func.SetParameter(0,A)
# func.SetLineWidth(1)
# r.gROOT.GetListOfFunctions().Add(func)

# # funcD = func.Clone("funcD")
# # r.gROOT.GetListOfFunctions().Add(funcD)
# # funcD.SetParameter(0,9.55)
# # funcD.SetParameter(1,5.3519)

# funcFile.Close()

# funcSum = r.TF1("funcSum","funcS + funcD",4.8,6.1)

#histo.SetAxisRange(0.,180.,"Y")
histo.SetAxisRange(4.6,7.,"X")
#histo.SetAxisRange(xMin,xMax,"X")
histo.SetFillColorAlpha(18, 0.4)

# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw("histo")
# func.Draw("same")
canvas.Print("signalTemp.pdf")
input('press enter to exit')
