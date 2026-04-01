#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

analysisTag = "JpsiGEndMarch_MC"
tupleName = "tOut"

# Get the tuple
tupleFile = r.TFile("./outputData/"+analysisTag+".root","READ")
ntuple = tupleFile.Get(tupleName)

triggerRes = [(int)(getattr(event, "triggerRes")) for event in ntuple]
triggerRes = ["{:08d}".format(x) for x in triggerRes]

values = [list(val) for val in triggerRes]

pathNames = ['HLT_DoubleMu2_Jpsi_LowPt_v5','HLT_DoubleMu4_3_Jpsi_v26',\
    'HLT_DoubleMu4_3_LowMass_v12','HLT_DoubleMu4_JpsiTrkTrk_Displaced_v18','HLT_DoubleMu4_Jpsi_Displaced_v18',\
    'HLT_DoubleMu4_Jpsi_NoVertexing_v18','HLT_DoubleMu4_LowMass_Displaced_v12','HLT_DoubleMu4_MuMuTrk_Displaced_v26']

df = pd.DataFrame(values,columns=pathNames)
for col in pathNames:
    df[col] = df[col].astype(int)

with open("new.txt",'w') as f:
    with pd.option_context('display.max_rows', None, 
                       'display.max_columns', None, 
                       'display.width', 2000, 
                       'display.precision', 2):
        print(df.corr(),file=f)
