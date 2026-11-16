#!/usr/bin/env python3
# Like plotAllBranches.py, but plots only the branches listed in `branches` below, in that order.
#   python3 plotSelectedBranches.py output.pdf file1.root [file2.root ...]
import sys

if len(sys.argv) < 3:
    sys.exit(f"Usage: python3 {sys.argv[0]} output.pdf file1.root [file2.root ...]")

import ROOT
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(111111)   # statsbox: name, entries, mean, std dev, underflow, overflow

out = sys.argv[1]
if not out.endswith(".pdf"):
    sys.exit(f"Error: the first argument must be the output .pdf file, got {out}")

chain = ROOT.TChain("tree")
for f in sys.argv[2:]:
    chain.Add(f)

# selection applied to all histograms; it is also written on every page of the pdf
selection = "bsFitSuccessful && resonanceFlag==1 && bsLxySig>5 && dimuonVtxProb>0.1 && bsVtxProb>0.1"

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

# plain text (TText, not TLatex): "_" in branch names would otherwise become subscripts
selectionLabel = ROOT.TText(0.01, 0.015, f"Selection: {selection}")
selectionLabel.SetNDC()
selectionLabel.SetTextSize(0.03)
selectionLabel.SetTextColor(ROOT.kGray + 2)

canvas = ROOT.TCanvas("c", "c", 800, 600)
canvas.Print(out + "[")
for name, rng in branches.items():
    if not chain.GetBranch(name):
        print(f"Warning: branch {name} not found in the tree, skipped")
        continue
    binning = "({},{},{})".format(*rng) if rng else ""
    # skip the default values (-999 / -9999) of unfilled entries, so they do not stretch the axis
    # (added here only, so it does not appear in the selection label)
    sel = f"({selection}) && {name} > -999"
    n = chain.Draw(f"{name}>>h_{name}{binning}", sel, "goff")   # vectors are flattened automatically
    h = ROOT.gDirectory.Get(f"h_{name}")
    if n <= 0 or not h:
        print(f"Warning: no entries for {name} after the selection, skipped")
        continue
    h.SetTitle(name)
    h.Draw("hist")
    selectionLabel.Draw()
    # TPad::Print searches the whole option string for format keywords: "tex", "svg" or "Standalone" in a page
    # title switch the output format and crash ROOT (e.g. maxMuonsVertexComp) -> break them up (case-sensitive match)
    pageTitle = name
    for kw in ("tex", "svg", "Standalone"):
        pageTitle = pageTitle.replace(kw, kw[:-1] + kw[-1].upper())
    canvas.Print(out, f"Title:{pageTitle}")
canvas.Print(out + "]")
print(f"Written {out}")
