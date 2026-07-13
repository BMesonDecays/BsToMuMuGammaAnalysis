#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

analysisTag = "JpsiGApril_23_24_July"
tupleName = "tOut"

# Prepare a directory for the output histograms
logBool = 1
statBool = 1
dirPath = 'tupleProjections/'+analysisTag+'/cut6'
if logBool  :
    dirPath += 'log'
if statBool  :
    dirPath += 'stats'
os.mkdir(dirPath)

# Get the tuple
tupleFile = r.TFile("./outputData/23_24_JpsiGApril_cut5.root","READ")
ntuple = tupleFile.Get(tupleName)

# Make a list with branch names
branchNames = []
for branch in ntuple.GetListOfBranches():
    branchNames.append(str(branch.GetFullName()))

# Define the cuts
cutList = []

cutList.append(r.TCut("muonIdCut","muon1Id > 0.7 && muon2Id > 0.7"))
cutList.append(r.TCut("fittedDimuonVertexProbCut","fittedDimuonVertexProb > 0.1"))
cutList.append(r.TCut("fittedDimuonMassCut","TMath::Abs(fittedDimuonMass - 3.097) < 0.06"))
cutList.append(r.TCut("lXY_fittedDimuon_bSpot_sigCut","lXY_fittedDimuon_bSpot_sig > 5.0"))
cutList.append(r.TCut("dR_photonFittedDimuonCut","dR_photonFittedDimuon < 0.5 && dR_photonFittedDimuon > 0.05"))
cutList.append(r.TCut("cosAnDimuonBSpot2DCut","cosAnDimuonBSpot2D > 0.99"))
cutList.append(r.TCut("cosAngleBsBSpot2DCut","cosAngleBsBSpot2D > 0.9999"))


'''
cutList.append(r.TCut("maxMuonsVertexCompCut","maxMuonsVertexComp < 0.1"))

cutList.append(r.TCut("cosAngleBsPV3DCut","cosAngleBsPV3D > 0.9999"))
cutList.append(r.TCut("tightMuonCut","tight1 == 1.0 && tight2 == 1.0"))
cutList.append(r.TCut("lXY_fittedDimuon_bSpotCut","lXY_fittedDimuon_bSpot > 10.0"))
'''

totalCut = r.TCut()
for cut in cutList:
    totalCut += cut
totalCut.Print()

with open(dirPath+'/cuts.txt','w') as of:
    print(totalCut.GetTitle(), file=of)

# Define the binnings
binInfo = {
    branchNames[0] : (1000,3.5,7.5),         #Bs mass
    branchNames[1] : (100,3.5,7.5),        #Bs mod mass  
    branchNames[2] : (100,0.99,1.),
    branchNames[3] : (100,0.999,1.),
    branchNames[4] : (100,-1.,1.),
    branchNames[5] : (100,-1.,1.),
    branchNames[6] : (100,-1.,1.),
    branchNames[7] : (100,-1.,1.),
    branchNames[8] : (100,-0.6,0.6),
    branchNames[9] : (100,0.,0.8),
    branchNames[10] : (100,-3.,3.),
    branchNames[11] : (100,2.9,3.3),
    branchNames[12] : (100,0.,1.),
    branchNames[13] : (100,0.,150.),
    branchNames[14] : (100,0.,250.),
    branchNames[15] : (100,0.,250.),
    branchNames[16] : (100,0.,100.),
    branchNames[17] : (100,0.0,300.),
    branchNames[18] : (100,0.,180.0), 
    branchNames[19] : (100,0.,180.0), 
    branchNames[20] : (100,0.,1.),
    branchNames[21] : (100,0.,2.),
    branchNames[22] : (100,0.,1.),
    branchNames[23] : (100,0.,1.),
    branchNames[24] : (2,0.,1.+1.E-8),
    branchNames[25] : (2,0.,1.+1.E-8)
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
    histo.SetStats(statBool)
    histo.SetLabelSize(0.03)
    histo.Draw()
    canvas.Print(dirPath+'/'+str(histo.GetName())+".pdf")

'''
# save selected histograms
outFile = r.TFile(analysisTag+"cut4.root",'UPDATE')
histoList[9].Write("23")
#histoList[10].Write()
totalCut.Write()
outFile.Close()


# Copy the ntuple limited by the cuts
outputFile = r.TFile("./outputData/23_24_JpsiGApril_cut2_mass.root","RECREATE")
totalCut.Write()
newNtuple = ntuple.CopyTree(str(totalCut))
newNtuple.Write()
outputFile.Close()
'''