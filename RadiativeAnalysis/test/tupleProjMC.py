#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

datasetName = "JpsiMassG_BsToJpsiGamma_MC_hlt"
tupleName = "tOut"

# Prepare a directory for the output histograms
dirPath = 'tupleProjections/'+datasetName+'/cut4_log_hlt'
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile("./outputData/"+datasetName+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []

cutList.append(r.TCut("muonsKalmanVxProbCut","muonsKalmanVxProb > 0.1"))
cutList.append(r.TCut("lXY_twoMuons_PV_signCut","lXY_twoMuons_PV_sign > 3.0"))
cutList.append(r.TCut("dR_photon_twoMuonsCut","dR_photon_twoMuons > 0.05 && dR_photon_twoMuons < 0.4"))
cutList.append(r.TCut("twoMuonsPhotonMassCut","TMath::Abs(twoMuonsPhotonMass - 5.367) < 1.5"))
cutList.append(r.TCut("cosPointingAngle_twoMuonsCut","cosPointingAngle_twoMuons > 0.95"))
cutList.append(r.TCut("lXY_twoMuons_bSpotCut","lXY_twoMuons_bSpot > 0.1"))
cutList.append(r.TCut("maxMuonsCompCut","maxMuonsComp < 0.1"))


# trigger cuts
cutList.append(r.TCut("trigger2Cut","hltRes % 1.E3 >= 1.E2"))


totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)

# Define the binnings
binInfo = {
    branchNames[0] : (100,2.8,3.4),
    branchNames[1] : (100,0.,1.),
    branchNames[2] : (100,0.,1.),
    branchNames[3] : (100,0.,1.),
    branchNames[4] : (100,0.,2.),
    branchNames[5] : (100,0.,2.),
    branchNames[6] : (100,0.,0.6),
    branchNames[7] : (100,0.,0.6),
    branchNames[8] : (80,4.4,6.4),
    branchNames[9] : (100,4.4,6.4),
    branchNames[10] : (100,0.,2.),
    branchNames[11] : (100,0.,2.),
    branchNames[12] : (100,0.,300.),
    branchNames[13] : (100,0.,300.),
    branchNames[14] : (100,0.,10.),
    branchNames[15] : (100,0.,10.),
    branchNames[16] : (100,-1.,1.),
    branchNames[17] : (100,-1.,1.),
    branchNames[18] : (100,0.,100.),
    branchNames[19] : (100,0.,100.)
}

# Create and fill the histograms
histoList = []
for bname in branchNames[:-1]:
    binning = binInfo[bname]
    histo = r.TH1D("h"+bname,bname, binning[0],binning[1],binning[2])
    #histo.SetAxisRange(7.e2,5.e4,"Y")
    ntuple.Project("h"+bname,bname, str(totalCut))
    histo.SetDirectory(0)
    histoList.append(histo)

tupleFile.Close()

'''
# save selected histograms
outFile = r.TFile("hMassAfterComp.root",'UPDATE')
histoList[8].Write("MChTwoMuonsPhotonMassCut4_hlt2")
totalCut.Write()
outFile.Close()
'''

# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    canvas.SetLogy(1)
    canvas.cd()
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")

