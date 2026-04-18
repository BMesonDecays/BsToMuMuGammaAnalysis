#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

etaMin = 1.65
etaMax = 2.5

tupleFileName = "genRecoPhotonEt.root"
tupleName = "tBsPhotons"
outputName = "EES"

###########################
tupleFile = r.TFile(tupleFileName,"READ")
ntuple = tupleFile.Get(tupleName)

cutString = ""
#cutString = "TMath::Abs(eta) > "+str(etaMin)
#cutString += " && TMath::Abs(eta) < "+str(etaMax)
#cutString += " && recoEnergy - genEnergy > -3.0"
#cutString += " && recoEnergy - genEnergy < 4.0"
print(cutString)

newNtuple = ntuple.CopyTree(cutString)
nPoints = newNtuple.GetEntries()

prof = r.TProfile("prof",outputName+" Photon energy;E_{gen}^{T};E_{reco}^{T}",20,0.0,50.0)


for i in range(nPoints):
    newNtuple.GetEntry(i)
    prof.Fill(newNtuple.genEt,newNtuple.recoEt)#,1/newNtuple.recoEnergyError)


# Drawing
canvas = r.TCanvas("canvas")
canvas.cd()

r.gStyle.SetErrorX(0)
#r.gStyle.SetMarkerSize(0.2)
#r.gStyle.SetMarkerStyle(20)
#r.gStyle.SetLineWidth(1)

prof.SetStats(0)
prof.Draw("||")

# line y=x
start = 0.
end = 50.
line = r.TLine(start,start,end,end)
line.SetLineColor(4)
line.Draw("same")


canvas.Print(tupleName+outputName+"Profile.pdf")
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
