#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

analysisTag = "JpsiGEndMarch_MC"
tupleName = "tOut"

# Prepare a directory for the output histograms
dirPath = 'tupleProjections/'+analysisTag+'/cut_6binSave'
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile("./outputData/"+analysisTag+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []

cutList.append(r.TCut("muonIdCut","muon1Id > 0.7 && muon2Id > 0.7"))
cutList.append(r.TCut("fittedDimuonVertexProbCut","fittedDimuonVertexProb > 0.2"))
cutList.append(r.TCut("maxMuonsVertexCompCut","maxMuonsVertexComp < 0.1"))

#cutList.append(r.TCut("dR_photonFittedDimuonCut","dR_photonFittedDimuon < 0.4 && dR_photonFittedDimuon > 0.14"))
cutList.append(r.TCut("fittedDimuonMassCut","TMath::Abs(fittedDimuonMass - 3.097) < 0.06"))
cutList.append(r.TCut("lXY_fittedDimuon_bSpot_sigCut","lXY_fittedDimuon_bSpot_sig > 5.0"))
#cutList.append(r.TCut("lXY_fittedDimuon_bSpotCut","lXY_fittedDimuon_bSpot > 0.5"))
cutList.append(r.TCut("cosAngleBsBSpot2DCut","cosAngleBsBSpot2D > 0.99"))

# trigger cuts
cutList.append(r.TCut("trigger2Cut","triggerRes % 1.E3 >= 1.E2"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)

# Define the binnings
binInfo = {
    branchNames[0] : (100,0.0,1.0),
    branchNames[1] : (100,2.8,3.4),
    branchNames[2] : (100,0.,1.),
    branchNames[3] : (100,0.,5.),
    branchNames[4] : (100,0.,150.),
    branchNames[5] : (100,0.,0.6),
    branchNames[6] : (100,-3.0,3.0),
    branchNames[7] : (100,0.,200.),
    branchNames[8] : (100,0.9,1.01),
    branchNames[9] : (80,4.5,6.5),
    branchNames[10] : (80,4.5,6.5),
    branchNames[11] : (100,0.98,1.0),
    branchNames[12] : (100,0.95,1.0),
    branchNames[13] : (100,0.,10.),
    branchNames[14] : (100,0.,10.),
    branchNames[15] : (100,0.9,1.0),
    branchNames[16] : (100,0.9,1.0),
    branchNames[17] : (100,0.,50.),
    branchNames[18] : (100,0.,50.),
    branchNames[19] : (100,0.,1.),
    branchNames[20] : (100,0.,1.),
    branchNames[21] : (100,0.,1.),
    branchNames[22] : (100,0.,1.)
}

# For "Mod" histos, removes entries without modScale
totalCutMod = totalCut + r.TCut("modScaleCut","modScale > 0.0")

# Create and fill the histograms
histoList = []
for bname in branchNames[:-1]:
    binning = binInfo[bname]
    histo = r.TH1D("h"+bname,bname, binning[0],binning[1],binning[2])
    if ("Mod" in bname):
        ntuple.Project("h"+bname,bname, str(totalCutMod))
    else:
        ntuple.Project("h"+bname,bname, str(totalCut))
    histo.SetDirectory(0)
    histoList.append(histo)

tupleFile.Close()

# save selected histograms
outFile = r.TFile("hMCcandBsMassCut6.root",'UPDATE')
histoList[10].Write()
totalCut.Write()
outFile.Close()
'''
'''


# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    #canvas.SetLogy(1)
    canvas.cd()
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")

