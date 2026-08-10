#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

analysisTag = "JpsiGMidApril_MC_July"
tupleName = "tOut"

# Prepare a directory for the output histograms
logBool = 0
dirPath = 'tupleProjections/'+analysisTag+'/cut0hlt_againBin'
if logBool  :
    dirPath += 'log'
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile("./outputData/JpsiGMidApril_MC.root","READ")
ntuple = tupleFile.Get(tupleName)

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []

# cutList.append(r.TCut("muonIdCut","muon1Id > 0.7 && muon2Id > 0.7"))
# cutList.append(r.TCut("fittedDimuonVertexProbCut","fittedDimuonVertexProb > 0.2"))
# cutList.append(r.TCut("fittedDimuonMassCut","TMath::Abs(fittedDimuonMass - 3.097) < 0.06"))
# cutList.append(r.TCut("lXY_fittedDimuon_bSpot_sigCut","lXY_fittedDimuon_bSpot_sig > 5.0"))
# cutList.append(r.TCut("dR_photonFittedDimuonCut","dR_photonFittedDimuon < 0.5 && dR_photonFittedDimuon > 0.05"))

# cutList.append(r.TCut("cosAnDimuonBSpot2DCut","cosAnDimuonBSpot2D > 0.99"))
# cutList.append(r.TCut("cosAngleBsBSpot2DCut","cosAngleBsBSpot2D > 0.9999"))
# cutList.append(r.TCut("cosAngleBsPV3DCut","cosAngleBsPV3D > 0.9999"))
# cutList.append(r.TCut("maxMuonsVertexCompCut","maxMuonsVertexComp < 0.2"))

'''
cutList.append(r.TCut("lXY_fittedDimuon_bSpotCut","lXY_fittedDimuon_bSpot > 0.5"))
cutList.append(r.TCut("tightMuonCut","tight1 == 1.0 && tight2 == 1.0"))
'''
# trigger cuts
cutList.append(r.TCut("triggerAnyCut","triggerRes > 0.0"))

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)

# Define the binnings
binInfo = {
    branchNames[0] : (100,0.0,1.0),
    branchNames[1] : (500,2.9,3.3),
    branchNames[2] : (100,0.,1.0),
    branchNames[3] : (100,0.,3.),
    branchNames[4] : (100,0.,150.),
    branchNames[5] : (80,0.,0.5),
    branchNames[6] : (100,-3.0,3.0),
    branchNames[7] : (100,0.,140.),
    branchNames[8] : (100,0.9,1.01),
    branchNames[9] : (1000,3.5,7.5),
    branchNames[10] : (100,3.5,7.5),
    branchNames[11] : (80,0.999,1.0),    #Bs to beam spot
    branchNames[12] : (100,0.9998,1.0),
    branchNames[13] : (100,0.,5.),
    branchNames[14] : (100,0.,5.),
    branchNames[15] : (100,0.9999,1.0),      #Bs to PV
    branchNames[16] : (100,0.9997,1.0),
    branchNames[17] : (100,0.,15.),
    branchNames[18] : (100,0.,15.),
    branchNames[19] : (100,0.,1.),
    branchNames[20] : (100,0.,1.),
    branchNames[21] : (2,0.,1.+1.E-8),
    branchNames[22] : (2,0.,1.+1.E-8),
    branchNames[23] : (100,-0.6,0.6),
    branchNames[24] : (100,0.9,1.),
    branchNames[25] : (100,0.97,1.),   #dimuon
    branchNames[26] : (100,0.0,80.0),
    branchNames[27] : (100,0.0,50.0),
    branchNames[28] : (100,-3.,3.),
    # triggerRes
    branchNames[30] : (2,0.,1.+1.E-8),
    branchNames[31] : (2,0.,1.+1.E-8)
}

# For "Mod" histos, removes entries without modScale
totalCutMod = totalCut + r.TCut("modScaleCut","modScale > 0.0")

# Create and fill the histograms
r.gStyle.SetOptStat("emr")
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

# tupleFile.Close()


# save selected histograms
outFile = r.TFile(analysisTag+"cut0HltJpsi.root",'RECREATE')
histoList[1].Write()
totalCut.Write()
outFile.Close()
tupleFile.Close()


# Draw histograms and save the images
for histo in histoList:
    canvas = r.TCanvas("c"+str(histo.GetTitle()))
    canvas.SetLogy(logBool)
    canvas.cd()
    histo.SetStats(0)
    histo.SetLabelSize(0.03)
    histo.Draw()
    
    '''
    canvas.Update()
    stats = histo.GetListOfFunctions().FindObject("stats")
    stats.SetX1NDC(0.7)  # Left edge
    stats.SetY1NDC(0.8)  # Bottom edge
    stats.SetX2NDC(0.9)  # Right edge
    stats.SetY2NDC(0.9)  # Top edge
    canvas.Modified()
    canvas.Update()
    '''
    
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")

