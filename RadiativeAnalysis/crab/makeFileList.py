#!/usr/bin/env python3
"""Write a list of LFNs (for crabConfig.py, BMMG_FILELIST) from a glob pattern of files on /eos/cms.

    python3 makeFileList.py '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_*/**/*.root' BsToMuMuGammaFileList.txt
"""
import glob
import sys

if len(sys.argv) != 3:
    sys.exit(__doc__)
pattern, output = sys.argv[1], sys.argv[2]
files = sorted(glob.glob(pattern, recursive=True))
with open(output, 'w') as out:
    for f in files:
        out.write(f.replace('/eos/cms', '', 1) + '\n')
print('%s: %d files' % (output, len(files)))
