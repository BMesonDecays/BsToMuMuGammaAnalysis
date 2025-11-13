import glob



with open('fileList.txt', 'w') as outFile:
    for f in glob.glob('/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV_09_07_2025/*/*/*/*.root'):
        outFile.write(f.replace('/eos/cms', '') + '\n')