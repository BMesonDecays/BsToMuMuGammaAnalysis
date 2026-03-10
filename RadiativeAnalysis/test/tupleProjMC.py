#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

fileName = "MC_PhotonDirMod.root"
tupleName = "tOut"

# Prepare a directory for the output histograms
dirPath = 'tupleProjections/'+tupleName+'/MC_PhotonDirMod_BsToJpsiGamma'+'/cut2'
if not os.access(dirPath[:-5], os.F_OK):    # '/cut?' has 5 chars
    os.mkdir(dirPath[:-5])
if os.access(dirPath, os.F_OK):
    os.rmdir(dirPath)
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile("./outputData/"+fileName,"READ")
ntuple = tupleFile.Get(tupleName)

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []

cutList.append(r.TCut("probKalmanCut","probKalman > 0.02"))
cutList.append(r.TCut("probMassConstr","probMassConstr > 0.02"))


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
    branchNames[3] : (100,0.,0.5),
    branchNames[4] : (100,0.,0.5),
    branchNames[5] : (100,4.,7.),
    branchNames[6] : (100,4.,7.)
}

# Create and fill the histograms
histoList = []
for bname in branchNames[1:]:
    binning = binInfo[bname]
    histo = r.TH1D("h"+bname,bname, binning[0],binning[1],binning[2])
    #histo.SetAxisRange(7.e2,5.e4,"Y")
    ntuple.Project("h"+bname,bname, str(totalCut))
    histo.SetDirectory(0)
    histoList.append(histo)

tupleFile.Close()

# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    #canvas.SetLogy(1)
    canvas.cd()
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")
