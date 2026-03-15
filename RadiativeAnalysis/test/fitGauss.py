#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

histname = "hdimuonModScaledPhotonMass"
xmin = 4.8
xmax = 6.0
##########################################
histfilename = "outFile.root"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Fitting function
expression = "0.5*[0]*(exp((-(x-[1])**2)/(2*[2]**2))/([2]*TMath::Sqrt(2*TMath::Pi())) + exp((-(x-[1])**2)/(2*[3]**2))/([3]*TMath::Sqrt(2*TMath::Pi())))"
fitFunc = r.TF1("fitFunc",expression,xmin,xmax,4)
fitFunc.SetParameters(15.,5.37,0.1,0.1)

results = histo.Fit(fitFunc,"ERSLB")

#'''
funcFile = r.TFile.Open(histname+"new"+"_fitFunc.root","RECREATE")
fitFunc.Write()
funcFile.Close()

with open(histname+"new"+'_fitResults.txt','a') as of:
    print(results, file=of)
#'''
    
canvas = r.TCanvas("canvas")
canvas.cd()
#canvas.SetLogy(True)

#histo.SetAxisRange(4.6,6.2)
#histo.SetAxisRange(3.5, 6., "X")
#histo.SetAxisRange(1500, 3.e3, "Y")
#histo.SetTitle("Lifetime of B^{#pm};t;Counts")
#histo.SetStats(0)
histo.Draw("h")
fitFunc.Draw("same")


canvas.Print(histname+"new"+"_Fit.pdf")
input('press enter to exit')