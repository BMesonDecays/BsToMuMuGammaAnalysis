#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

datasetName = "24BCD_BsToJpsiGamma_JpsiMassConstrained"
tupleName = "tJpsi"

# Get the tuple
tupleFile = r.TFile("./outputData/"+datasetName+".root","READ")
ntuple = tupleFile.Get(tupleName)

# Define the cuts
cutList = []

cutList.append(r.TCut("cosPointingAngleCut","cosPointingAngle > 0.9"))
cutList.append(r.TCut("fittedJpsiVxProbCut","fittedJpsiVxProb > 0.1"))
cutList.append(r.TCut("muonsKalmanVxProbCut","muonsKalmanVxProb > 0.1"))
cutList.append(r.TCut("lXY_fittedJpsi_PVCut","lXY_fittedJpsi_PV > 0.1"))
cutList.append(r.TCut("BsXYlifetimeCut","BsXYlifetime < 9.0"))
cutList.append(r.TCut("maxMuonsCompCut","maxMuonsComp < 0.4"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

# project the final mass histogram after cuts
histo = r.TH1D("hJpsiGMass","fittedJpsiPhotonMass", 100,4.4,6.4)
ntuple.Project("hJpsiGMass","fittedJpsiPhotonMass", str(totalCut))

# save the histogram and applied cuts
outputFile = r.TFile("cut3_24BCD_finalMass_BsToJpsiGammaMC_JpsiMassConstrained.root","RECREATE")
totalCut.Write()
histo.Write()
outputFile.Close()
tupleFile.Close()
