#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3

import sys
import math
from ROOT import *


print ("Hello ROOT")
fileName = "JpsiGApril_23_24_Julycut8.root"

print ('Read data from: ', fileName)
gROOT.Reset()
f = TFile(fileName);
f.ls();

histoname = "hcandBsMass"

c1 = TCanvas('cHisto','cHisto',1000,600)
histo = gROOT.FindObject(histoname)
histo.SetAxisRange(4.5,7.0,"X")
#histo.SetTitle("ScaledRecoVsGenPhotonEnergy (PV[0])")
histo.Draw()
c1.Print("temp.pdf")
input('press enter to exit')
