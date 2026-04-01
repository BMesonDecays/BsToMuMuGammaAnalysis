#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

datasetName = "24fullB_I_JpsiMassG_BsToJpsiGamma_cut1"
tupleName = "tOut"

# Get the tuple
tupleFile = r.TFile("./outputData/"+datasetName+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Define the cuts
cutList = []
cutList.append(r.TCut("muonsKalmanVxProbCut","muonsKalmanVxProb > 0.1"))
cutList.append(r.TCut("lXY_twoMuons_PV_signCut","lXY_twoMuons_PV_sign > 3.0"))
cutList.append(r.TCut("dR_photon_twoMuonsCut","dR_photon_twoMuons > 0.05 && dR_photon_twoMuons < 0.4"))
cutList.append(r.TCut("twoMuonsPhotonMassCut","TMath::Abs(twoMuonsPhotonMass - 5.367) < 1.5"))

cutList.append(r.TCut("lXY_twoMuons_bSpotCut","lXY_twoMuons_bSpot > 0.1"))
cutList.append(r.TCut("maxMuonsCompCut","maxMuonsComp < 0.1"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

# Make a new Ntuple with cuts applied
newNtuple = ntuple.CopyTree(str(totalCut))

# Get the list of branch entries
branchName = "cosPointingAngle_twoMuons"
entriesVals = [getattr(event, branchName) for event in newNtuple]

# Calculate the theta angles
thetaVals = np.arccos(entriesVals)
thetaVals = np.degrees(thetaVals)
nOfEntries = thetaVals.size

# Create and save a histogram
histo = r.TH1D("histo","Pointing angle values [deg]",1000,0.,180.)
histo.FillN(nOfEntries, thetaVals, np.ones(nOfEntries))

canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw()
canvas.Print("temp.pdf")

input('press enter to exit')

