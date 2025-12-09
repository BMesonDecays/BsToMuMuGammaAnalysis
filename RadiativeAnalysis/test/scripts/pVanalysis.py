#!/cvmfs/cms.cern.ch/el9_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/el9_amd64_gcc12/bin/python3
import ROOT as r
import sys
import numpy as np

tupleFile = r.TFile("tPVxyz.root","READ")
tPvxyz = tupleFile.Get("tPVxyz")
# crashes when one closes the file with TNtuple(D)

histoX = r.TH1D("histoX","dX",100,0.,15.)
histoY = r.TH1D("histoY","dY",100,0.,15.)
histoZ = r.TH1D("histoZ","dZ",100,0.,15.)

x=y=z = []
lastevNo = -1

for entry in tPvxyz:
    x.append(entry.x)
    y.append(entry.y)
    z.append(entry.z)
    evNo = entry.evNo
    if evNo!=lastevNo:
        lastevNo = evNo
        continue

    histoX.Fill(np.abs(x[-1]-x[-2]))
    histoY.Fill(np.abs(y[-1]-y[-2]))
    histoZ.Fill(np.abs(z[-1]-z[-2]))


# File
outfile = r.TFile("hPVxyz.root","RECREATE")
histoX.Write()
histoY.Write()
histoZ.Write()


canvas = r.TCanvas("canvas")
canvas.cd()
canvas.SetLogy(0)
histoX.Draw()

canvas.Print("temp.pdf")
input("press enter to exit")