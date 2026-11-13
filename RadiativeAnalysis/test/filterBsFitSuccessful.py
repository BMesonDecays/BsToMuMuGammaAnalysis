#!/usr/bin/env python3
# Skim RadiativeAnalysis ntuples to candidates with a successful Bs vertex fit.
#   - entries (dimuons) without any successful Bs fit are dropped
#   - in the kept entries, every per-photon vector branch keeps only the elements with bsFitSuccessful == true
#     (all vector branches of "tree" are indexed per photon candidate, like bsFitSuccessful)
#   - scalar branches are copied unchanged; the "summary" tree is copied as is
#
#   python3 filterBsFitSuccessful.py output.root input1.root [input2.root ...]
import sys
import ROOT
ROOT.gROOT.SetBatch(True)

if len(sys.argv) < 3:
    sys.exit(f'usage: {sys.argv[0]} output.root input1.root [input2.root ...]')
outName, inNames = sys.argv[1], sys.argv[2:]
if outName in inNames:
    sys.exit('output file must differ from the input files')

chain = ROOT.TChain('tree')
for f in inNames:
    chain.Add(f)

vectorBranches = [b.GetName() for b in chain.GetListOfBranches()
                  if b.GetClassName().startswith('vector') and b.GetName() != 'bsFitSuccessful']

df = ROOT.RDataFrame(chain)
df = df.Define('fitMask_', 'ROOT::RVecB(bsFitSuccessful.begin(), bsFitSuccessful.end())') \
       .Filter('ROOT::VecOps::Any(fitMask_)', 'at least one successful Bs fit')
# convert back to std::vector, so the output branches keep the input types (RVec would otherwise be written)
def masked(b):
    return (f'using T = std::decay_t<decltype({b})>::value_type; '
            f'auto r = {b}[fitMask_]; return std::vector<T>(r.begin(), r.end());')
for b in vectorBranches:
    df = df.Redefine(b, masked(b))
df = df.Redefine('bsFitSuccessful', masked('bsFitSuccessful'))

columns = [b.GetName() for b in chain.GetListOfBranches()]   # original branch order, without fitMask_
nOut = df.Count()                       # booked before the Snapshot, filled in the same event loop
df.Snapshot('tree', outName, columns)

# copy the generator summary tree (one entry per input file)
summary = ROOT.TChain('summary')
for f in inNames:
    summary.Add(f)
if summary.GetEntries() > 0:
    out = ROOT.TFile.Open(outName, 'UPDATE')
    summary.CloneTree(-1, 'fast').Write()
    out.Close()

print(f'{outName}: kept {nOut.GetValue()} / {chain.GetEntries()} entries')
