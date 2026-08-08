#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# histname = "hcandBsMass"
histname = "hfittedDimuonMass"
xmin = 2.95
xmax = 3.24
##########################################
histfilename = "JpsiGApril_23_24_July_BsMasscut0.root"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# histo.Rebin(4)

# Fitting function
# expression = "0.5*[0]*(exp((-(x-[1])**2)/(2*[2]**2))/([2]*TMath::Sqrt(2*TMath::Pi())) + exp((-(x-[1])**2)/(2*[3]**2))/([3]*TMath::Sqrt(2*TMath::Pi())))"
#expression = "[0]*(exp((-(x-[1])**2)/(2*[2]**2))/([2]*TMath::Sqrt(2*TMath::Pi())) )"
# expression = "[0]*([4]*exp((-(x-[1])**2)/(2*[2]**2))/([2]*TMath::Sqrt(2*TMath::Pi())) + (1-[4])*exp((-(x-[1])**2)/(2*[3]**2))/([3]*TMath::Sqrt(2*TMath::Pi())))"
expression = "[0]*exp((-(x-[1])**2)/(2*[2]**2)) + [3]*exp(-[4]*x)"
# expression = "[0]*exp((-(x-[1])**2)/(2*[2]**2)) + [3]+[4]*x"


fitFunc = r.TF1("fitFunc",expression,xmin,xmax,5)
fitFunc.SetParameters(60.e3,3.097,0.02,2.e6,2.0)

results = histo.Fit(fitFunc,"ERSLB")

# funcFile = r.TFile.Open(histname+"July_MC_fitFuncExt.root","RECREATE")
#fitFunc.Write()
#funcFile.Close()

with open(histname+histfilename[:-5]+'_fitResults.txt','a') as of:
    print(results, file=of)

    
canvas = r.TCanvas("canvas")
canvas.cd()
# canvas.SetLogy(True)

# histo.SetAxisRange(4.5,6.4)
#histo.SetAxisRange(3.5, 6., "X")
#histo.SetAxisRange(1500, 3.e3, "Y")
#histo.SetTitle("Lifetime of B^{#pm};t;Counts")
histo.SetStats(0)
histo.SetFillColorAlpha(18, 0.4)

histo.Draw("h")
fitFunc.Draw("same")


canvas.Print("temp_Fit.pdf")
input('press enter to exit')