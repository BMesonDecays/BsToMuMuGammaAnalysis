#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3
import ROOT as r
import sys
import os
import numpy as np

# Get the tuples
tupleFile1 = r.TFile("temp.root","READ")
ntuple1 = tupleFile1.Get('tOut')

tupleFile2 = r.TFile("./outputData/23BCD_JpsiGStartApril.root","READ")
ntuple2 = tupleFile2.Get('tOut')

# # Make a list with branch names
# branchNames = []
# for branch in ntuple1.GetListOfBranches():
#     branchNames.append(str(branch.GetFullName()))

# for bName in branchNames[-6:]:
#     ntuple1.SetBranchStatus(bName,0)

# tempFile = r.TFile.Open("temp.root",'RECREATE')
# cleaned_ntuple1 = ntuple1.CloneTree(-1, "fast")
# cleaned_ntuple1.Write()
# tempFile.Close()


# 3. Create a TList and add both ntuples to it
list_of_ntuples = r.TList()
list_of_ntuples.Add(ntuple1)

# 4. Create the output file
output_file = r.TFile.Open("combined_output.root", "RECREATE")

# 5. Merge the ntuples together into the new file
# Merge() returns a new TTree/TNtuple pointer or an integer status depending on ROOT version
combined_ntuple = ntuple2.Merge(list_of_ntuples)

# 6. Write and close everything cleanly
output_file.Write()
output_file.Close()

tupleFile1.Close()
tupleFile2.Close()

print("Ntuples successfully merged and saved!")

