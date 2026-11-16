#!/bin/bash
# Merge the 2024 data CRAB outputs into one file per era.
# For each era it takes all ParkingDoubleMuonLowMass0-7 datasets (both Run2024I-PromptReco-v1 and -v2 for era I)
# and all timestamp directories (original task + recovery tasks), then checks that the merged
# 'tree' has as many entries as all the inputs together.
#
#
# With --filter, the inputs are instead merged and skimmed in one pass by test/filterBsFitSuccessful.py
# (only candidates with a successful Bs fit kept), giving RadiativeAnalysis_Run2024<era>_bsFit.root.
# The skim is written to a local temporary directory, copied to EOS and its 'tree' entries are checked.
#
#   cd $CMSSW_BASE/src/BsToMuMuGammaAnalysis/RadiativeAnalysis/crab_data; cmsenv
#   ./merge_DATA_2024.sh B C D        # merge the given eras
#   nohup ./merge_DATA_2024.sh B C D E F G H > merge.log 2>&1 &
#   nohup ./merge_DATA_2024.sh --filter B C D E F G H > merge_filter.log 2>&1 &
set -e

inDir=/eos/user/p/psajdak/crab_out
outDir=/eos/user/p/psajdak/crab_out/merged
outURL=root://eosuser.cern.ch/${outDir}   # write through xrootd: more robust than FUSE for large files
# closing a large file on EOS takes long (the server checksums the whole file); the default 60 s
# stream timeout made the close of the 18 GB era E file fail with "Socket timeout"
export XRD_STREAMTIMEOUT=1800
export XRD_REQUESTTIMEOUT=3600

filter=0
if [ "$1" == "--filter" ]; then filter=1; shift; fi
filterScript=$(dirname $(readlink -f $0))/../test/filterBsFitSuccessful.py
nThreads=8                                   # threads for the filter (lxplus nodes have 16 cores)
tmpDir=/tmp/${USER}/merge_DATA_2024          # local disk for the filter output before copying it to EOS

[ $# -gt 0 ] || { echo "usage: $0 [--filter] ERA [ERA ...]   (e.g. B C D)"; exit 1; }
which hadd > /dev/null || { echo "hadd not found: run cmsenv first"; exit 1; }
mkdir -p ${outDir} ${tmpDir}

for era in "$@"; do
    inputs=${outDir}/RadiativeAnalysis_Run2024${era}_inputs.txt
    if [ ${filter} == 1 ]; then out=RadiativeAnalysis_Run2024${era}_bsFit.root; else out=RadiativeAnalysis_Run2024${era}.root; fi
    echo "=== Run2024${era} ($(date)) ==="
    for d in ${inDir}/ParkingDoubleMuonLowMass*/RadiativeAnalysis_Run2024${era}-*/*/; do
        echo "  $(find $d -name '*.root' | wc -l) files in ${d#${inDir}/}"
    done
    find ${inDir}/ParkingDoubleMuonLowMass*/RadiativeAnalysis_Run2024${era}-* -name '*.root' | sort -V > ${inputs}
    echo "  $(wc -l < ${inputs}) input files -> ${outDir}/${out}"

    if [ ${filter} == 1 ]; then
        log=${outDir}/${out%.root}.log
        python3 ${filterScript} -j ${nThreads} ${tmpDir}/${out} $(cat ${inputs}) > ${log} 2>&1 \
            || { echo "  filter FAILED, see ${log}"; exit 1; }
        nKept=$(sed -n 's/.*: kept \([0-9]*\) \/ \([0-9]*\) entries/\1/p' ${log})
        [ -n "${nKept}" ] || { echo "  no 'kept' line in ${log}"; exit 1; }
        echo "  $(grep ': kept' ${log} | sed 's/.*: //')"
        xrdcp -f ${tmpDir}/${out} ${outURL}/${out}
        python3 - ${outURL}/${out} ${nKept} <<'EOF'
import sys, ROOT
f = ROOT.TFile.Open(sys.argv[1])
nOut, nKept = f.Get('tree').GetEntries(), int(sys.argv[2])
print(f'  entries on EOS: {nOut}, expected {nKept} -> {"OK" if nOut == nKept else "MISMATCH"}')
sys.exit(0 if nOut == nKept else 1)
EOF
        rm -f ${tmpDir}/${out}
        continue
    fi

    # -fk: keep the input compression, so baskets are copied without recompression (fast)
    # -n 100: at most 100 input files opened at once
    hadd -fk -n 100 ${outURL}/${out} $(cat ${outDir}/${out%.root}_inputs.txt) > ${outDir}/${out%.root}_hadd.log 2>&1 \
        || { echo "  hadd FAILED, see ${outDir}/${out%.root}_hadd.log"; exit 1; }
    # hadd returns 0 even when closing the output fails, so check its log for ROOT errors as well
    if grep -q "Error in <" ${outDir}/${out%.root}_hadd.log; then
        echo "  hadd reported errors:"; grep "Error in <" ${outDir}/${out%.root}_hadd.log | sed 's/^/    /'
        exit 1
    fi

    python3 - ${outDir}/${out%.root}_inputs.txt ${outURL}/${out} <<'EOF'
import sys, ROOT
inputs = [l.strip() for l in open(sys.argv[1]) if l.strip()]
chain = ROOT.TChain('tree')
for f in inputs:
    chain.Add(f)
nIn = chain.GetEntries()
f = ROOT.TFile.Open(sys.argv[2])
nOut = f.Get('tree').GetEntries()
print(f'  entries: inputs {nIn}, merged {nOut} -> {"OK" if nIn == nOut else "MISMATCH"}')
sys.exit(0 if nIn == nOut else 1)
EOF
done
echo "=== done ($(date)) ==="
