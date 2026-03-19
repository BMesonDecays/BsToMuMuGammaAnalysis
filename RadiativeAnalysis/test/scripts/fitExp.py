#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

histname = "h100BsXYlifetime_twoMuons_cut2"
xmin = 0.5
xmax = 100.0
##########################################
histfilename = "hBsXYlifetime_twoMuons.root"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get(histname)
histo.SetDirectory(0)
histfile.Close()

# Fitting function
expression = "[1]*TMath::Exp(-x/[0]) / [0] + [3]*TMath::Exp(-x/[2]) / [2]"
fitFunc = r.TF1("fitFunc",expression,xmin,xmax,4)
fitFunc.SetParameters(1.3,800000.,30.6019,25000.)
#fitFunc.FixParameter(2,)
#fitFunc.FixParameter(3,4912.7)

results = histo.Fit(fitFunc,"ERSLB")

#'''
funcFile = r.TFile.Open(histname+"_fitFunc.root","RECREATE")
fitFunc.Write()
funcFile.Close()

with open(histname+'_fitResults.txt','a') as of:
    print(results, file=of)
#'''
    
canvas = r.TCanvas("canvas")
canvas.cd()
canvas.SetLogy(True)

#histo.SetAxisRange(4.6,6.2)
#histo.SetAxisRange(3.5, 6., "X")
#histo.SetAxisRange(1500, 3.e3, "Y")
#histo.SetTitle("Lifetime of B^{#pm};t;Counts")
#histo.SetStats(0)
histo.Draw("h")
fitFunc.Draw("same")


canvas.Print(histname+"_Fit.pdf")
input('press enter to exit')