#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3

import ROOT

# Enable multi-threading for speed (optional)
ROOT.EnableImplicitMT()

# 1. Directly target the files and the internal ntuple tree name
# RDataFrame bypasses the broken TDirectoryFile::Get wrapper
df1 = ROOT.RDataFrame("tOut", "24allBtoI_JpsiGMidApril.root")
df2 = ROOT.RDataFrame("tOut", "23BCD_JpsiGStartApril.root")


# 2. Extract the columns available in file2 (your clean target structure)
# This creates a vector list of strings for C++ to read directly
columns_to_keep = ROOT.std.vector('string')()
for col in df2.GetColumnNames():
    columns_to_keep.push_back(str(col))

# 3. Snapshot file1 directly, passing our column whitelist to filter it
# This drops all unwanted columns and writes a perfectly healthy ROOT file
df1.Snapshot("tOut", "file1_fixed.root", columns_to_keep)

print("Step 1/2: Created a perfectly clean, uncorrupted version of file1.")

# 4. Use a virtual TChain to cleanly append the fixed file1 and file2
chain = ROOT.TChain("tOut")
chain.Add("file1_fixed.root")
chain.Add("23BCD_JpsiGStartApril.root")

# 5. Write the final fully merged file to your disk
output_file = ROOT.TFile.Open("combined_output.root", "RECREATE")
final_ntuple = chain.CloneTree(-1, "fast")

output_file.Write()
output_file.Close()

print("Step 2/2: Successfully merged and saved the final output!")
