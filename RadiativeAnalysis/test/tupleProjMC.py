#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

datasetName = "MidMarchExtraJpsiG_MCfull"
tupleName = "tOut"

# Prepare a directory for the output histograms
dirPath = 'tupleProjections/'+datasetName+'/cut2'
if not os.access(dirPath[:-5], os.F_OK):    # '/cut?' has 5 chars
    os.mkdir(dirPath[:-5])
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile(datasetName+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []

cutList.append(r.TCut("lXY_muonsKalman_bSpot_significanceCut","lXY_muonsKalman_bSpot_significance > 3"))
cutList.append(r.TCut("dimuonMassCut","TMath::Abs(dimuonMass-3.097) < 0.1"))



# trigger cuts
#cutList.append(r.TCut("trigger2Cut","hltRes % 1.E3 >= 1.E2"))


totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)

# Define the binnings
binInfo = {
    branchNames[0] : (100,0.,1.),
    branchNames[1] : (100,2.6,3.6),
    branchNames[2] : (100,0.,2.5),
    branchNames[3] : (100,0.,300.),
    branchNames[4] : (100,-1.,1.),
    branchNames[5] : (100,0.,0.6),
    branchNames[6] : (80,5.,5.8),
    branchNames[7] : (80,5.,5.8),
    branchNames[8] : (80,5.,5.8),
    branchNames[9] : (80,5.,5.8),
    branchNames[10] : (80,5.,5.8),
    branchNames[11] : (80,5.,5.8)
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
outFile = r.TFile("outFile.root",'RECREATE')
histoList[7].Write()
histoList[11].Write()
outFile.Close()
'''

# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    #canvas.SetLogy(1)
    canvas.cd()
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")

