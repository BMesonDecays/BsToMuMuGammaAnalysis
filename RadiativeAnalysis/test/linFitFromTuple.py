#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

etaMin = 0.0
etaMax = 1.479
xMin = 40.0
xMax = 120.0

tupleFileName = "genRecoPhotonEnergyCaloEta.root"
tupleName = "tBsPhotons"
outputName = "EB40_"

###########################
tupleFile = r.TFile(tupleFileName,"READ")
ntuple = tupleFile.Get(tupleName)

cutString = "TMath::Abs(eta) > "+str(etaMin)
cutString += " && TMath::Abs(eta) < "+str(etaMax)
print(cutString)

newNtuple = ntuple.CopyTree(cutString)
nPoints = newNtuple.GetEntries()
graph = r.TGraphErrors(nPoints)
graph.SetTitle(outputName+" Photon energy;E_{gen};E_{reco}")

for i in range(nPoints):
    newNtuple.GetEntry(i)
    graph.SetPoint(i, newNtuple.genEnergy, newNtuple.recoEnergy)
    graph.SetPointError(i,0.0,newNtuple.recoEnergyError)
# graph ready
# now fitting

# Fitting function
expression = "[0] + [1]*x"
fitFunc = r.TF1("fitFunc",expression,xMin,xMax,2)
fitFunc.SetParameters(0.,1.)

results = graph.Fit(fitFunc,"SEMRN")
covM = results.GetCovarianceMatrix()
#covM.Print()

funcFile = r.TFile.Open(tupleName+outputName+str(int(xMax))+"_fitFunc.root","RECREATE")
fitFunc.Write()
funcFile.Close()

with open(tupleName+outputName+str(int(xMax))+'_fitResults.txt','a') as of:
    print(xMin,xMax,results,results.GetCovarianceMatrix()(0,1), file=of)

# Drawing
canvas = r.TCanvas("canvas")
canvas.cd()

graph.SetMarkerSize(0.2)
graph.SetMarkerStyle(20)
graph.GetXaxis().SetRangeUser(xMin,xMax)
graph.GetYaxis().SetRangeUser(xMin,xMax+10.0)
graph.Draw("APX")

r.gStyle.SetLineWidth(1)
r.gStyle.SetLineStyle(7)

fitFunc.SetLineWidth(1)
fitFunc.SetLineStyle(1)
fitFunc.Draw("same")

# line y=x
start = xMin
end = xMax
line = r.TLine(start,start,end,end)
line.SetLineColor(4)
line.Draw("same")


canvas.Print(tupleName+outputName+str(int(xMax))+"_Fit.pdf")
input('press enter to exit')

# save just graph pdf
c2 = r.TCanvas("c2")
c2.cd()
graph.GetXaxis().SetRangeUser(0,120)
graph.GetYaxis().SetRangeUser(0,140)
graph.Draw("AP")
line.DrawLine(0.0,0.0,120.,120.)
c2.Print(tupleName+outputName+".pdf")
input('press enter to exit')

