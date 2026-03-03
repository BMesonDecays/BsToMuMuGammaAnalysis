#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

fileName = "BsToJpsiGammaMC_JpsiMassConstrained_HLT.root"
tupleName = "tJpsi"

# Prepare a directory for the output histograms
dirPath = 'tupleProjections/'+tupleName+'/MC_BsToJpsiGamma_JpsiMassConstrained'+'/cut2_log'
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

cutList.append(r.TCut("cosPointingAngleCut","cosPointingAngle > 0.9"))
cutList.append(r.TCut("fittedJpsiVxProbCut","fittedJpsiVxProb > 0.1"))
cutList.append(r.TCut("muonsKalmanVxProbCut","muonsKalmanVxProb > 0.1"))
cutList.append(r.TCut("lXY_fittedJpsi_PVCut","lXY_fittedJpsi_PV > 0.1"))
#cutList.append(r.TCut("maxMuonsCompCut","maxMuonsComp < 0.7"))
cutList.append(r.TCut("BsXYlifetimeCut","BsXYlifetime < 9.0"))
'''
cutList.append(r.TCut("M_dimuonCut","TMath::Abs(M_dimuon - 3.0969) < 0.04"))    #Jpsi mass constraint
#cutList.append(r.TCut("commonMuonVrtxProbCut","ProbOfCommonMuonVertex > 0.1"))
cutList.append(r.TCut("deltaR_dimuon_photonCut","deltaR_dimuon_photon < 0.4"))#&& deltaR_dimuon_photon > 0.1"))  #based on GenMatched tuple
cutList.append(r.TCut("flightLengthCut","minFlightPath > 0.15"))
cutList.append(r.TCut("maxProb_MuMuNotGammaCut","maxProb_MuMuNotGamma < 0.1"))
cutList.append(r.TCut("minPCA_distanceCut","minPCA_distance < 0.01"))
'''
# trigger cuts
cutList.append(r.TCut("trigger2Cut","hltPaths % 1.E3 >= 1.E2"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()


with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)


# Define the binnings
binInfo = {
    branchNames[1] : (100,2.8,3.4),
    branchNames[2] : (100,0.,1.),
    branchNames[3] : (100,0.,1.),
    branchNames[4] : (100,0.,0.5),
    branchNames[5] : (100,4.4,6.4),
    branchNames[6] : (100,0.,2.),
    branchNames[7] : (100,0.,250.),
    branchNames[8] : (100,0.,5.),
    branchNames[9] : (100,-1.,1.),
    branchNames[10] : (100,0.,20.)
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
    canvas.SetLogy(1)
    canvas.cd()
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")
