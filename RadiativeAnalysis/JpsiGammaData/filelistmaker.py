#!/cvmfs/cms.cern.ch/slc7_amd64_gcc12/cms/cmssw/CMSSW_14_0_2/external/slc7_amd64_gcc12/bin/python3

import glob
import os
import re

base = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing'

# List all directories starting with Private_
process_dirs = sorted([
    d for d in os.listdir(base)
    if d.startswith('Private_') and os.path.isdir(os.path.join(base, d))
])

def extract_process_name(dirname):
    """
    Convert:
        'Private_BsToPhiGamma_MCTunesRun3ECM13p6TeV'
    to:
        'BsToPhiGamma'
    """
    name = dirname[len("Private_"):]  # strip Private_
    name = name.split('_')[0]         # keep first block
    return name

for proc in process_dirs:
    proc_path = os.path.join(base, proc)

    # Extract clean physics process name
    process_name = extract_process_name(proc)

    # Output filename
    outname = f"{process_name}FileList.txt"

    with open(outname, 'w') as outFile:

        # recursively find all *.root files
        files = glob.glob(os.path.join(proc_path, "**", "*.root"), recursive=True)

        print(f"[INFO] Writing {outname}: {len(files)} files")

        for f in sorted(files):
            # convert full eos path to LFN
            lfn = f.replace('/eos/cms', '')
            outFile.write(lfn + '\n')

print("\nAll filelists written successfully!")
