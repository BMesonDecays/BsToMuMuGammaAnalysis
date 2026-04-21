#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

analysisTag = "JpsiGMidApril_24"
tupleName = "tOut"

# Prepare a directory for the output histograms
logBool = 0
dirPath = 'tupleProjections/'+analysisTag+'/cut8_g'
if logBool  :
    dirPath += 'log'
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile("./outputData/24allBtoI_JpsiGMidApril_cut1.root","READ")
ntuple = tupleFile.Get(tupleName)

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []

cutList.append(r.TCut("muonIdCut","muon1Id > 0.7 && muon2Id > 0.7"))
cutList.append(r.TCut("fittedDimuonVertexProbCut","fittedDimuonVertexProb > 0.2"))
cutList.append(r.TCut("maxMuonsVertexCompCut","maxMuonsVertexComp < 0.1"))
cutList.append(r.TCut("fittedDimuonMassCut","TMath::Abs(fittedDimuonMass - 3.097) < 0.06"))
cutList.append(r.TCut("lXY_fittedDimuon_bSpot_sigCut","lXY_fittedDimuon_bSpot_sig > 5.0"))
cutList.append(r.TCut("cosAnDimuonBSpot2DCut","cosAnDimuonBSpot2D > 0.985"))
cutList.append(r.TCut("cosAngleBsBSpot2DCut","cosAngleBsBSpot2D > 0.99998"))
cutList.append(r.TCut("cosAngleBsPV3DCut","cosAngleBsPV3D > 0.9999"))
'''
cutList.append(r.TCut("tightMuonCut","tight1 == 1.0 && tight2 == 1.0"))
cutList.append(r.TCut("dR_photonFittedDimuonCut","dR_photonFittedDimuon < 0.4 && dR_photonFittedDimuon > 0.14"))
cutList.append(r.TCut("lXY_fittedDimuon_bSpotCut","lXY_fittedDimuon_bSpot > 0.5"))
'''
totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)

# Define the binnings
binInfo = {
    branchNames[0] : (100,0.0,1.0),
    branchNames[1] : (100,2.9,3.3),
    branchNames[2] : (100,0.,0.5),
    branchNames[3] : (100,0.,3.),
    branchNames[4] : (100,0.,150.),
    branchNames[5] : (100,0.,0.6),
    branchNames[6] : (100,-3.0,3.0),
    branchNames[7] : (100,0.,140.),
    branchNames[8] : (100,0.9,1.01),
    branchNames[9] : (100,3.5,7.5),
    branchNames[10] : (100,3.5,7.5),
    branchNames[11] : (100,0.99997,1.0),    #Bs to beam spot
    branchNames[12] : (100,0.99997,1.0),
    branchNames[13] : (100,0.,5.),
    branchNames[14] : (100,0.,5.),
    branchNames[15] : (100,0.9997,1.0),      #Bs to PV
    branchNames[16] : (100,0.9997,1.0),
    branchNames[17] : (100,0.,15.),
    branchNames[18] : (100,0.,15.),
    branchNames[19] : (100,0.,1.),
    branchNames[20] : (100,0.,1.),
    branchNames[21] : (2,0.,1.+1.E-8),
    branchNames[22] : (2,0.,1.+1.E-8),
    branchNames[23] : (100,-0.6,0.6),
    branchNames[24] : (100,0.9,1.),
    branchNames[25] : (100,0.984,1.),   #dimuon
    # triggerRes
    branchNames[27] : (100,-3.0,3.0),
    branchNames[28] : (100,0.0,80.0),
    branchNames[29] : (100,0.0,50.0),
    branchNames[30] : (100,0.0,50.0),
    branchNames[31] : (100,0.0,120.0),
    branchNames[32] : (100,0.0,120.0)
}

# For "Mod" histos, removes entries without modScale
totalCutMod = totalCut + r.TCut("modScaleCut","modScale > 0.0")

# Create and fill the histograms
histoList = []
for bname in branchNames:
    if bname == "triggerRes" : continue
    binning = binInfo[bname]
    histo = r.TH1D("h"+bname,bname, binning[0],binning[1],binning[2])
    histo.SetFillColorAlpha(18, 0.4)
    if ("Mod" in bname):
        ntuple.Project("h"+bname,bname, str(totalCutMod))
    else:
        ntuple.Project("h"+bname,bname, str(totalCut))
    histo.SetDirectory(0)
    histoList.append(histo)

tupleFile.Close()

# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    canvas.SetLogy(logBool)
    canvas.cd()
    histo.SetStats(0)
    histo.SetLabelSize(0.03)
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")



'''
# save selected histograms
outFile = r.TFile("hMass300Cut6tight.root",'RECREATE')
histoList[9].Write()
histoList[10].Write()
outFile.Close()


# Copy the ntuple limited by the cuts
outputFile = r.TFile("./outputData/23BCD_"+analysisTag+"_cut2.root","RECREATE")
totalCut.Write()
newNtuple = ntuple.CopyTree(str(totalCut))
newNtuple.Write()
outputFile.Close()

'''