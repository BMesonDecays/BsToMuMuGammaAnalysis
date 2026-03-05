#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

fileName = "BsToJpsiGammaMC_JpsiMassConstrained_HLT"
tupleName = "tKalman"

# Get the tuple
tupleFile = r.TFile("./outputData/"+fileName+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Define the cuts
cutList = []

cutList.append(r.TCut("cosPointingAngleCut","cosPointingAngle > 0.9"))
cutList.append(r.TCut("muonsKalmanVxProbCut","muonsKalmanVxProb > 0.1"))
cutList.append(r.TCut("displXY_bestPVCut","displXY_bestPV > 0.1"))
cutList.append(r.TCut("BsXYlifetimeCut","BsXYlifetime < 9.0"))
#cutList.append(r.TCut("maxMuonsCompCut","maxMuonsComp < 0.4"))

# trigger cuts
cutList.append(r.TCut("trigger2Cut","hltPaths % 1.E3 >= 1.E2"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

# project the final mass histogram after cuts
histo = r.TH1D("hMMGMass","twoMuonsPhotonMass", 100,4.4,6.4)
ntuple.Project("hMMGMass","twoMuonsPhotonMass", str(totalCut))

# save the histogram and applied cuts
outputFile = r.TFile("cut2_Kalman_MCfinalMass_BsToJpsiGammaMC_JpsiMassConstrained.root","RECREATE")
totalCut.Write()
histo.Write()
outputFile.Close()
tupleFile.Close()
