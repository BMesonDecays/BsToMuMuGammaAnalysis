#!/usr/bin/env python3
# Like plotAllBranches.py, but plots only the branches listed in `branches` below, in that order.
#   python3 plotSelectedBranches.py output.pdf file1.root [file2.root ...]
# All histograms are filled in a single (multi-threaded) RDataFrame event loop.
import sys
import ctypes

if len(sys.argv) < 3:
    sys.exit(f"Usage: python3 {sys.argv[0]} output.pdf file1.root [file2.root ...]")

import ROOT
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(111111)   # statsbox: name, entries, mean, std dev, underflow, overflow
ROOT.EnableImplicitMT(8)          # must be called before the RDataFrame is created; uses all available cores

out = sys.argv[1]
if not out.endswith(".pdf"):
    sys.exit(f"Error: the first argument must be the output .pdf file, got {out}")

chain = ROOT.TChain("tree")
for f in sys.argv[2:]:
    chain.Add(f)

#######################
# selection applied to all histograms; it is also written on every page of the pdf
selection = "bsFitSuccessful && resonanceFlag==1 && bsLxySig>5 && dimuonVtxProb>0.1 && bsVtxProb>0.1 && photonMVA>0.6"
#######################

# branches that hold only generator-level information; skipped for data
mcOnlyBranches = {"dimuonGenPhotonMass", "genBMass", "genDimuonMass", "genMumugMass"}

# branches to plot, in this order: name -> binning (nbins, xmin, xmax) [GeV, cm], or None for an automatic range
branches = {
    "mu1Pt":                  (100, 0, 100),
    "mu2Pt":                  (100, 0, 100),
    "maxMuonsVertexComp":     (100, 0, 1),
    "DiMuon_KalmanVtxProb":   (100, 0, 1),
    "dimuonMass":             (100, 2.0, 3.5),
    "dimuonFitMass":          (100, 2.0, 3.5),
    "dimuonVtxProb":          (100, 0, 1),

    "photonPt":               (100, 0, 100),
    "photonEta":              (100, -2.5, 2.5),
    "photonDeltaRs":          (100, 0, 0.5),
    "photonMVA":              (100, -1, 1),
    "photonDimuonMass":       (100, 4.4, 6.1),
    "dimuonGenPhotonMass":    (500, 4.8, 5.6),
    "photonDimuonLxy":        (100, 0, 1.0),
    "photonDimuonLxySig":     (100, 0, 100),
    "photonDimuonCosAlpha":   (100, 0.9, 1),
    "photonDimuonCosAlpha2D": (100, 0.9, 1),

    "bsVtxProb":              (100, 0, 1),
    "bsMass":                 (100, 4.4, 6.1),
    "bsLxy":                  (100, 0, 1.0),
    "bsLxySig":               (100, 0, 100),
    "bsPt":                   (100, 0, 60),

    "genBMass":               (100, 5.365, 5.368),
    "genDimuonMass":          (500, 2.8, 3.2),
    "genMumugMass":           (500, 5.25, 5.4),
}

# ---- book everything on one RDataFrame; nothing runs until the first result is requested ----
df = ROOT.RDataFrame(chain)
columns = set(str(c) for c in df.GetColumnNames())

# Reproduce the TTree::Draw behaviour for the mixed scalar / vector branches:
# the per-candidate branches (photon*, bs*, PV*) are vectors with one element per photon candidate, the others are
# per-event scalars. selMask_ = 1 for each candidate passing the selection (scalars in it are broadcast to all candidates).
df = df.Define("selMask_", f"ROOT::RVecI(bsMass.size(), 1) && ({selection})")

# MC vs data: genBMass keeps its default (-9999) unless the gen decay chain was found, which never happens in data
nGen = df.Filter("genBMass > -999").Count() if "genBMass" in columns else None
nAll = df.Count()
nSelected = df.Filter("Any(selMask_)").Count()

hists = {}
for name, rng in branches.items():
    if name not in columns:
        print(f"Warning: branch {name} not found in the tree, skipped")
        continue
    # skip the default values (-999 / -9999) of unfilled entries, so they do not stretch the axis
    if "vector" in df.GetColumnType(name).lower() or "rvec" in df.GetColumnType(name).lower():
        values = f"{name}[selMask_ && {name} > -999]"                         # passing candidates
    else:
        values = f"ROOT::RVecD({name} > -999 ? Sum(selMask_) : 0, {name})"   # once per passing candidate, like TTree::Draw
    col = f"plot_{name}"
    dfh = df.Define(col, values)
    hists[name] = dfh.Histo1D(ROOT.RDF.TH1DModel(f"h_{name}", name, *rng), col) if rng else dfh.Histo1D(col)

# ---- the single event loop runs here ----
isMC = nGen is not None and nGen.GetValue() > 0
print(f"Input recognized as {'MC (simulation)' if isMC else 'data'}")
print(f"{nSelected.GetValue()} of {nAll.GetValue()} entries pass the selection")

canvas = ROOT.TCanvas("c", "c", 800, 600)   # created first: the text width below is measured on it

# selection label: plain text (TText, not TLatex): "_" in branch names would otherwise become subscripts.
# TText does not wrap lines -> split the text between words so that each line fits within maxWidth (NDC)
labelX, labelY, labelSize, lineSpacing, maxWidth = 0.01, 0.015, 0.03, 0.035, 0.98
def textWidthNDC(text):
    t = ROOT.TText(0, 0, text)
    t.SetNDC()
    t.SetTextSize(labelSize)
    w, h = ctypes.c_uint(0), ctypes.c_uint(0)
    t.GetTextExtent(w, h, text)
    return w.value / canvas.GetWw()
lines = []
for word in f"Selection: {selection}".split(" "):
    if lines and textWidthNDC(lines[-1] + " " + word) <= maxWidth:
        lines[-1] += " " + word
    else:
        lines.append(word)
selectionLabels = []   # first line on top, the last one at labelY
for i, line in enumerate(lines):
    label = ROOT.TText(labelX, labelY + (len(lines) - 1 - i) * lineSpacing, line)
    label.SetNDC()
    label.SetTextSize(labelSize)
    label.SetTextColor(ROOT.kGray + 2)
    selectionLabels.append(label)
# make room for the extra lines below the x axis title
canvas.SetBottomMargin(canvas.GetBottomMargin() + (len(lines) - 1) * lineSpacing)


canvas.Print(out + "[")
for name, hptr in hists.items():
    if not isMC and name in mcOnlyBranches:
        continue
    h = hptr.GetValue()
    if h.GetEntries() == 0:
        print(f"Warning: no entries for {name} after the selection, skipped")
        continue
    h.SetTitle(name)
    h.Draw("hist")
    for label in selectionLabels:
        label.Draw()
    # TPad::Print searches the whole option string for format keywords: "tex", "svg" or "Standalone" in a page
    # title switch the output format and crash ROOT (e.g. maxMuonsVertexComp) -> break them up (case-sensitive match)
    pageTitle = name
    for kw in ("tex", "svg", "Standalone"):
        pageTitle = pageTitle.replace(kw, kw[:-1] + kw[-1].upper())
    canvas.Print(out, f"Title:{pageTitle}")
canvas.Print(out + "]")
print(f"Written {out}")


# Another option: without RDataFrame
'''
# speed-up: evaluate the selection once over the whole chain and keep only the entries with at least one passing
# candidate; each Draw below then reads only those entries (the selection is still applied there per candidate)
chain.Draw(">>selectedEntries", selection, "entrylist")
selectedEntries = ROOT.gDirectory.Get("selectedEntries")
chain.SetEntryList(selectedEntries)
print(f"{selectedEntries.GetN()} of {chain.GetEntries()} entries pass the selection")
'''