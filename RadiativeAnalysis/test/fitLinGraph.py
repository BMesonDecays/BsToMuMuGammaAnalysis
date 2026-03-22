#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

graphFileName = "genRecoPhotonEnergyErBsPhotons.root"

graphName = "gBsPhotons"
xmin = 0.0
xmax = 150.0
##########################################
# Get the graph
graphFile = r.TFile.Open(graphFileName,"READ")
graph = graphFile.Get(graphName)
graphFile.Close()

# Fitting function
expression = "[0] + [1]*x"
fitFunc = r.TF1("fitFunc",expression,xmin,xmax,2)
fitFunc.SetParameters(0.,1.)

results = graph.Fit(fitFunc,"SEMR")#+"EX0")
covM = results.GetCovarianceMatrix()
covM.Print()

funcFile = r.TFile.Open(graphName+"_fitFunc.root","RECREATE")
fitFunc.Write()
funcFile.Close()

with open(graphName+'_fitResults.txt','a') as of:
    print(results,results.GetCovarianceMatrix()(0,1), file=of)


canvas = r.TCanvas("canvas")
canvas.cd()

graph.SetMarkerSize(0.2)
graph.SetMarkerStyle(20)
graph.GetXaxis().SetRangeUser(0,150)
graph.GetYaxis().SetRangeUser(0,160)
graph.Draw("APX")

r.gStyle.SetLineWidth(1)
r.gStyle.SetLineStyle(10)

fitFunc.SetLineWidth(1)
fitFunc.SetLineStyle(3)
fitFunc.Draw("same")

# line y=x
start = 0.
end = 150.
line = r.TLine(start,start,end,end)
line.SetLineColor(4)
line.Draw("same")


canvas.Print(graphName+"_Fit.pdf")
input('press enter to exit')
