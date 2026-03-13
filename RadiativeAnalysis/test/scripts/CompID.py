#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r

# Get the tuple
tupleFile = r.TFile("../outputData/BsToJpsiGamma_CompCheckJpsiMassG.root","READ")
ntuple = tupleFile.Get("tCompID")

# Get value counts
counts = {}
for entry in ntuple:
    val = entry.ID
    counts[val] = counts.get(val, 0) + 1

# Display results
for value, count in sorted(counts.items()):
    print(f"Value: {value}, Count: {count}")
