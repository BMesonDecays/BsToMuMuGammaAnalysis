#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import numpy as np

# Get the tuple
tupleFile = r.TFile("./outputData/tBsToJpsiGamma_RecoOnly_allMC.root","READ")
ntuple = tupleFile.Get("tMuMuGamma")

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []
cutList.append(r.TCut("M_dimuonCut","TMath::Abs(M_dimuon - 3.0969) < 0.06"))    #Jpsi mass constraint
#cutList.append(r.TCut("commonMuonVrtxProbCut","ProbOfCommonMuonVertex > 0.1"))
cutList.append(r.TCut("deltaR_dimuon_photonCut","deltaR_dimuon_photon < 0.4"))  #based on GenMatched tuple
cutList.append(r.TCut("minPCA_distanceCut","minPCA_distance < 0.008"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

with open('tupleProjections/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)

# Define the binnings
binInfo = {
    branchNames[0] : (100,2.,5.),
    branchNames[1] : (100,0.,1.),
    branchNames[2] : (100,-3.,3.),
    branchNames[3] : (100,0.,0.5),
    branchNames[4] : (100,0.,0.05),
    branchNames[5] : (100,0.,10.)
}

# Create and fill the histograms
histoList = []
for bname in branchNames:
    binning = binInfo[bname]
    histo = r.TH1D("h"+bname,bname, binning[0],binning[1],binning[2])
    ntuple.Project("h"+bname,bname, str(totalCut))
    histo.SetDirectory(0)
    histoList.append(histo)

tupleFile.Close()

# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    canvas.cd()
    histo.Draw()
    canvas.Print("tupleProjections/"+str(histo.GetName())+".pdf")

