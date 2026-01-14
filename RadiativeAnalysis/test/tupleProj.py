#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np


datasetName = "2024BCprunedNew"

# Prepare a directory for the output histograms
dirPath = 'tupleProjections/'+datasetName+'/cut3'
if not os.access(dirPath[:-5], os.F_OK):    # '/cut?' has 5 chars
    os.mkdir(dirPath[:-5])
if os.access(dirPath, os.F_OK):
    os.rmdir(dirPath)
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile("./outputData/BsToJpsiGamma"+datasetName+".root","READ")
ntuple = tupleFile.Get("tMuMuGamma")

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []
cutList.append(r.TCut("M_dimuonCut","TMath::Abs(M_dimuon - 3.0969) < 0.15"))    #Jpsi mass constraint
cutList.append(r.TCut("commonMuonVrtxProbCut","ProbOfCommonMuonVertex > 0.1"))
cutList.append(r.TCut("deltaR_dimuon_photonCut","deltaR_dimuon_photon < 0.4 && deltaR_dimuon_photon > 0.1"))  #based on GenMatched tuple
cutList.append(r.TCut("minPCA_distanceCut","minPCA_distance < 0.007"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

'''
# Copy the ntuple limited by the cuts
outputFile = r.TFile("./outputData/BsToJpsiGamma"+datasetName+"prunedNew.root","RECREATE")
totalCut.Write()
newNtuple = ntuple.CopyTree(str(totalCut))
newNtuple.Write()
outputFile.Close()
tupleFile.Close()
'''


with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)


# Define the binnings
binInfo = {
    branchNames[0] : (100,2.,5.),
    branchNames[1] : (100,0.,1.),
    branchNames[2] : (100,-3.,3.),
    branchNames[3] : (100,0.,0.5),
    branchNames[4] : (100,0.,0.05),
    branchNames[5] : (100,3.,10.)
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
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")
