#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys

mean = 5.43914
sigma = 0.150

histfilename = "JpsiGMidApril_MC_Julycut10_bins.root"
histfile = r.TFile.Open(histfilename,"READ")
histo = histfile.Get("hcandBsMass")
histo.SetDirectory(0)
histfile.Close()

nOfEntries = histo.Integral(histo.FindBin(mean-sigma), histo.FindBin(mean+sigma))

with open('numberOfEntries.txt','a') as of:
    print(histfilename, file=of)
    print(f"Mean: {mean} \t Sigma: {sigma}",file=of)
    print(f"Number of entries within the range [mean-sigma; mean+sigma]: {nOfEntries} \n",file=of)