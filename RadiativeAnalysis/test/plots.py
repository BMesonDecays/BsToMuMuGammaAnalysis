#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3

import sys
import math
from ROOT import *


print ("Hello ROOT")
fileName = "histos.root"

print ('Read data from: ', fileName)
gROOT.Reset()
f = TFile(fileName);
f.ls();

c1 = TCanvas('cHisto','cHisto',1000,600)
histo = gROOT.FindObject('hPhotondR')
histo.SetTitle("all reco vs one gen photon;dR ;")
histo.Draw()
c1.Print("hPhotondR.pdf")
input('press enter to exit')