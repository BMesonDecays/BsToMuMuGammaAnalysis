#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

# option = "8hImpreb5"
histname = "hcandBsMass"
xmin = 4.8
xmax = 6.1
##########################################
histfilename = "./looserPointingAng/JpsiGMidApril_MC_looserPointingAngcut4_reb8.root"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Fitting function
expression = "0.5*[0]*(exp((-(x-[1])**2)/(2*[2]**2))/([2]*TMath::Sqrt(2*TMath::Pi())) + exp((-(x-[1])**2)/(2*[3]**2))/([3]*TMath::Sqrt(2*TMath::Pi())))"
#expression = "[0]*(exp((-(x-[1])**2)/(2*[2]**2))/([2]*TMath::Sqrt(2*TMath::Pi())) )"
fitFunc = r.TF1("fitFunc",expression,xmin,xmax,4)
fitFunc.SetParameters(15.,5.4,0.1,0.1)

results = histo.Fit(fitFunc,"ERSLB")

funcFile = r.TFile.Open(histname+"_MC_fitFunc.root","RECREATE")
fitFunc.Write()
funcFile.Close()

with open(histname+'_MC_fitResults.txt','a') as of:
    print(results, file=of)

    
canvas = r.TCanvas("canvas")
canvas.cd()
#canvas.SetLogy(True)

histo.SetAxisRange(4.5,6.5)
#histo.SetAxisRange(3.5, 6., "X")
#histo.SetAxisRange(1500, 3.e3, "Y")
#histo.SetTitle("Lifetime of B^{#pm};t;Counts")
histo.SetStats(0)
histo.SetFillColorAlpha(18, 0.4)

histo.Draw("h")
fitFunc.Draw("same")


canvas.Print("temp_Fit.pdf")
input('press enter to exit')