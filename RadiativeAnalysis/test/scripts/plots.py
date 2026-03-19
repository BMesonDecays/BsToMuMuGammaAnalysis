#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3

import sys
import math
from ROOT import *


print ("Hello ROOT")
fileName = "outputData/hMMGgen_bestPV_dist.root"

print ('Read data from: ', fileName)
gROOT.Reset()
f = TFile(fileName);
f.ls();

histoname = "hTriggered"

c1 = TCanvas('cHisto','cHisto',1000,600)
histo = gROOT.FindObject(histoname)
#histo.SetAxisRange(0.,0.03,"X")
#histo.SetTitle("ScaledRecoVsGenPhotonEnergy (PV[0])")
histo.Draw()
c1.Print("images/"+histoname+".pdf")
input('press enter to exit')
