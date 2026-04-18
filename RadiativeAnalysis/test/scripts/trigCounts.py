#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np
import pandas as pd

filename = "./outputData/23BCD_JpsiGStartApril_cut2"
tag = "dataCut9"

# Get the tuple
tupleFile = r.TFile(filename+".root","READ")
ntupleOrig = tupleFile.Get("tOut")

# Define the cuts
cutList = []

cutList.append(r.TCut("muonIdCut","muon1Id > 0.7 && muon2Id > 0.7"))
cutList.append(r.TCut("fittedDimuonVertexProbCut","fittedDimuonVertexProb > 0.2"))
cutList.append(r.TCut("maxMuonsVertexCompCut","maxMuonsVertexComp < 0.1"))
cutList.append(r.TCut("fittedDimuonMassCut","TMath::Abs(fittedDimuonMass - 3.097) < 0.06"))
cutList.append(r.TCut("lXY_fittedDimuon_bSpot_sigCut","lXY_fittedDimuon_bSpot_sig > 5.0"))
cutList.append(r.TCut("cosAngleBsBSpot2DCut","cosAngleBsBSpot2D > 0.99998"))
cutList.append(r.TCut("cosAnDimuonBSpot2DCut","cosAnDimuonBSpot2D > 0.98"))


totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

# Cut ntuple
ntuple = ntupleOrig.CopyTree(str(totalCut))

# Consider HLT paths
triggerRes = [(int)(getattr(event, "triggerRes")) for event in ntuple]
triggerRes = ["{:08d}".format(x) for x in triggerRes]

values = [list(val) for val in triggerRes]

pathNames = ['DoubleMu2_Jpsi_LowPt','DoubleMu4_3_Jpsi',\
    'DoubleMu4_3_LowMass','DoubleMu4_JpsiTrkTrk_Displaced','DoubleMu4_Jpsi_Displaced',\
    'DoubleMu4_Jpsi_NoVertexing','DoubleMu4_LowMass_Displaced','DoubleMu4_MuMuTrk_Displaced']

df = pd.DataFrame(values,columns=pathNames)
for col in pathNames:
    df[col] = df[col].astype(int)

with open(tag+"HLT.txt",'w') as f:
    print("Cuts:\n"+totalCut.GetTitle()+"\n", file=f)
    print(df.sum(), file=f)
    print("\nTotal entries: "+str(len(df.index))+"\n \nCorrelations:\n", file=f)
    with pd.option_context('display.max_rows', None, 
                       'display.max_columns', None, 
                       'display.width', 2000, 
                       'display.precision', 2):
        print(df.corr().round(2), file=f)