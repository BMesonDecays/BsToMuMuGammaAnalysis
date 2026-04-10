#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

etaMin = 1.65
etaMax = 2.5
xMin = 50.0
xMax = 90.0

tupleFileName = "genRecoPhotonEnergyCaloEta.root"
tupleName = "tBsPhotons"
outputName = "EES"

###########################
tupleFile = r.TFile(tupleFileName,"READ")
ntuple = tupleFile.Get(tupleName)

cutString = "TMath::Abs(eta) > "+str(etaMin)
cutString += " && TMath::Abs(eta) < "+str(etaMax)
#cutString += " && recoEnergy - genEnergy > -3.0"
#cutString += " && recoEnergy - genEnergy < 4.0"
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
expression = "[0] + [1]*x "#+ [2]*x*x"
fitFunc = r.TF1("fitFunc",expression,xMin,xMax,2)
fitFunc.SetParameter(0,0.0)
#fitFunc.FixParameter(1,1.0)

results = graph.Fit(fitFunc,"SEMRN")
covM = results.GetCovarianceMatrix()
#covM.Print()

funcFile = r.TFile.Open(tupleName+outputName+"_"+str(int(xMin))+"_"+str(int(xMax))+"_fitFunc.root","RECREATE")
fitFunc.Write()
funcFile.Close()

with open(tupleName+outputName+"_"+str(int(xMin))+"_"+str(int(xMax))+'_fitResults.txt','a') as of:
    print(xMin,xMax,results,results.GetCovarianceMatrix()(0,1), file=of)

# Drawing
canvas = r.TCanvas("canvas")
canvas.cd()

graph.SetMarkerSize(0.2)
graph.SetMarkerStyle(20)
graph.GetXaxis().SetRangeUser(xMin-10.0,xMax+10.0)
graph.GetYaxis().SetRangeUser(xMin-10.0,xMax+10.0)
#graph.GetXaxis().SetRangeUser(10.0,30.0)
#graph.GetYaxis().SetRangeUser(5.0,40.0)
graph.Draw("APX")

r.gStyle.SetLineWidth(1)
r.gStyle.SetLineStyle(7)

fitFunc.SetLineWidth(1)
fitFunc.SetLineStyle(1)
fitFunc.Draw("same")

# line y=x
start = xMin-10.
end = xMax+10.
line = r.TLine(start,start,end,end)
line.SetLineColor(4)
line.Draw("same")


canvas.Print(tupleName+outputName+"_"+str(int(xMin))+"_"+str(int(xMax))+"_Fit.pdf")
input('press enter to exit')

'''
# save just graph pdf
c2 = r.TCanvas("c2")
c2.cd()
graph.GetXaxis().SetRangeUser(0,60)
graph.GetYaxis().SetRangeUser(0,60)
graph.Draw("APX")
line.DrawLine(0.0,0.0,60.,60.)
c2.Print(tupleName+outputName+".pdf")
input('press enter to exit')
'''
