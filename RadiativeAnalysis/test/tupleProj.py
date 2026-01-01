#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import numpy as np

# Get the tuple
tupleFile = r.TFile("./outputData/tBsToJpsiGamma_RecoOnly_allMC.root","READ")
ntuple = tupleFile.Get("tMuMuGamma")

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []
cutList.append(r.TCut("M_dimuonCut","M_dimuon > 0.00"))
cutList.append(r.TCut("commonMuonVrtxProbCut","ProbOfCommonMuonVertex > 0.1"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

# Define the binnings
binInfo = {
    branchNames[0] : (100,2.,10.),
    branchNames[1] : (100,0.,1.),
    branchNames[2] : (100,-3.,3.),
    branchNames[3] : (100,0.,0.5),
    branchNames[4] : (100,0.,0.5),
    branchNames[5] : (100,0.,10.)
}

# Create and fill the histograms
histoList = []
for bname in branchNames:
    binning = binInfo[bname]
    histo = r.TH1D("h"+bname,bname, binning[0],binning[1],binning[2])
    ntuple.Project("h"+bname,bname, str(totalCut))
    histo.SetDirectory(0)
    histoList.append(histo)

tupleFile.Close()

# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    canvas.cd()
    histo.Draw()
    canvas.Print("tupleProjections/"+str(histo.GetName())+".pdf")




'''
# Draw histograms and save resulting canvases
for histogram in histoList:
    print(str(histogram.GetName()), str(histogram.GetTitle()))
    ntuple.Project(str(histogram.GetName()),str(histogram.GetTitle()),str(totalCut))
    canvas = r.TCanvas("c"+str(histogram.GetTitle()))
    canvas.cd()
    histogram.Draw()
    canvas.Print("tupleProjections/"+histogram.GetName()+".pdf")
'''




'''
dR_cut =            r.TCut("dR_min < 0.2")
deltaMJ_psi_cut =   r.TCut("TMath::Abs(deltaMJ_psi) < 0.02")
deltaM_Bpm_cut =    r.TCut("TMath::Abs(deltaM_Bpm) < 0.03")
cuts = r.TCut(dR_cut + deltaMJ_psi_cut + deltaM_Bpm_cut)
cuts.Print()
'''

'''
histo = r.TH1D("histo","temp",80,0.0,8.e-12)
#tLifetime.Project("histo","deltaMJ_psi")
tLifetime.Project("histo","properTime/(3*TMath::Power(10.,10))",str(cuts))

# File for fitting
outfile = r.TFile("hBpmLT.root","RECREATE")
histo.Write()
dR_cut.Write()
deltaMJ_psi_cut.Write()
deltaM_Bpm_cut.Write()


canvas = r.TCanvas("canvas")
canvas.cd()
canvas.SetLogy(1)
histo.Draw()

canvas.Print("temp.pdf")
input("press enter to exit")

'''

'''
tupleFile = r.TFile("./outputData/tBsToJpsiGamma_RecoOnly_allMC.root","READ")
ntuple = tupleFile.Get("tMuMuGamma")


histo = r.TH1D("histo","histo",100,0.,10.)
ntuple.Project("histo","M_dimuon")

histo.SetDirectory(0)
tupleFile.Close()

canvas = r.TCanvas("canvas")
canvas.cd()
histo.Draw()
canvas.Print("temp.pdf")
input("press enter to exit")
'''

'''
# Create histograms
histoList = []
for bName in branchNames:
    histo = r.TH1D()
    histo.SetNameTitle("h"+bName,bName)
    histoList.append(histo)


# Set histogram bins
histoList[0].SetBins(100,2.,10.)
histoList[1].SetBins(100,0.,1.)
histoList[2].SetBins(100,-3.,3.)
histoList[3].SetBins(100,0.,0.5)
histoList[4].SetBins(100,0.,0.5)
histoList[5].SetBins(100,0.,10.)

'''