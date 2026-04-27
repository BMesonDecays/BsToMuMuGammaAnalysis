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
func.SetName("funcS")
func.SetParameter(0,2.33)
func.SetLineWidth(1)
r.gROOT.GetListOfFunctions().Add(func)

funcD = func.Clone("funcD")
r.gROOT.GetListOfFunctions().Add(funcD)
funcD.SetParameter(0,9.55)
#funcD.SetParameter(1,5.3519)


funcSum = r.TF1("funcSum","funcS + funcD",4.8,6.1)
funcSum.SetParameter(4,9.55)
funcSum.SetParameter(5,5.280)
funcSum.SetParameter(6,funcD.GetParameter(2))
funcSum.SetParameter(7,funcD.GetParameter(3))
funcSum.SetParameter(1,5.367)

funcSum.Print()
#r.gROOT.GetListOfFunctions().Print()

params = funcSum.GetParameters() # Returns a C-style array
param_list = [params[i] for i in range(funcSum.GetNpar())]
print(param_list)




#histo.SetAxisRange(0.,180.,"Y")
#histo.SetAxisRange(3.8,7.,"X")
#histo.SetAxisRange(4.5,6.5,"X")

# Draw and save
canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw("histo")
funcSum.Draw("same")
canvas.Print("signalTemp.pdf")
input('press enter to exit')


funcFile.Close()
