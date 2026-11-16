#!/usr/bin/env python3
#   python3 plotAllBranches.py output.pdf file1.root [file2.root ...]
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
selection = "bsFitSuccessful && bsLxySig>5"      # && bsLxy>0.5"


# fixed binning (nbins, xmin, xmax) for the main branches [GeV, cm]
# all others get an automatic range
ranges = {
    # masses
    "bsMass":                 (100, 4.4, 6.1),
    "photonDimuonMass":       (100, 4.4, 6.1),
    "dimuonGenPhotonMass":    (500, 4.8, 5.6),
    "dimuonMass":             (100, 2.0, 3.5),
    "dimuonFitMass":          (100, 2.0, 3.5),
    # kinematics
    "mu1Pt":                  (100, 0, 100),
    "mu2Pt":                  (100, 0, 100),
    "dimuonPt":               (100, 0, 100),
    "bsPt":                   (100, 0, 60),
    "photonPt":               (100, 0, 100),
    "photonEnergy":           (100, 0, 200),
    "photonEta":              (100, -2.5, 2.5),
    "photonPhi":              (64, -3.2, 3.2),
    "photonDeltaRs":          (100, 0, 0.5),
    "photonMVA":              (100, -1, 1),
    # vertex probabilities
    "bsVtxProb":              (100, 0, 1),
    "dimuonVtxProb":          (100, 0, 1),
    "DiMuon_KalmanVtxProb":   (100, 0, 1),
    "maxMuonsVertexComp":     (100, 0, 1),
    # displacement and pointing
    "bsLxy":                  (100, 0, 1.0),
    "photonDimuonLxy":        (100, 0, 1.0),
    "bsLxySig":               (100, 0, 100),
    "photonDimuonLxySig":     (100, 0, 100),
    "bsCt":                   (100, 0, 0.2),
    "photonDimuonCosAlpha":   (100, 0.9, 1),
    "photonDimuonCosAlpha2D": (100, 0.9, 1),
    # gen-matching
    "mu1GenDeltaR":           (100,0,0.01),
    "mu2GenDeltaR":           (100,0,0.01),
    "photonGenDeltaR":        (100, 0, 0.1),
    # generator decay chain (MC only)
    "genIsHardProcess":       (2, -0.5, 1.5),
    "genNFSRPhotons":         (10, -0.5, 9.5),
    "genBPt":                 (100, 0, 60),
    "genBEta":                (60, -3, 3),
    "genBPhi":                (64, -3.2, 3.2),
    "genBMass":               (100, 5.365, 5.368),
    "genMuPlusPt":            (100, 0, 100),
    "genMuPlusEta":           (60, -3, 3),
    "genMuPlusPhi":           (64, -3.2, 3.2),
    "genMuMinusPt":           (100, 0, 100),
    "genMuMinusEta":          (60, -3, 3),
    "genMuMinusPhi":          (64, -3.2, 3.2),
    "genPhotonPt":            (100, 0, 100),
    "genPhotonEta":           (60, -3, 3),
    "genPhotonPhi":           (64, -3.2, 3.2),
    "genDimuonMass":          (500, 2.8, 3.2),
    "genMumugMass":           (500, 5.25, 5.4),
    # "genBProdVtxX":           (100, -0.5, 0.5),
    # "genBProdVtxY":           (100, -0.5, 0.5),
    # "genBProdVtxZ":           (100, -20, 20),
    # "genBDecayVtxX":          (100, -1, 1),
    # "genBDecayVtxY":          (100, -1, 1),
    # "genBDecayVtxZ":          (100, -20, 20),
    # "genLxy":                 (100, 0, 1.0),
    # "genCt":                  (100, 0, 0.2),
    # # event level
    # "nTracks":                (100, 0, 3000),
    # "nPhotons":               (20, 0, 20),
    # "nMuons":                 (10, 0, 10),
    # "PUTrueinteraction":      (80, 0, 80),
    # other
    "PVindex":                  (20,0,20),
}

# plain text (TText, not TLatex): "_" in branch names would otherwise become subscripts
selectionLabel = ROOT.TText(0.01, 0.015, f"Selection: {selection}")
selectionLabel.SetNDC()
selectionLabel.SetTextSize(0.03)
selectionLabel.SetTextColor(ROOT.kGray + 2)

canvas = ROOT.TCanvas("c", "c", 800, 600)
canvas.Print(out + "[")
for br in chain.GetListOfBranches():
    name = br.GetName()
    binning = "({},{},{})".format(*ranges[name]) if name in ranges else ""
    # skip the default values (-999 / -9999) of unfilled entries, so they do not stretch the axis
    # (added here only, so it does not appear in the selection label)
    sel = f"({selection}) && {name} > -999"
    n = chain.Draw(f"{name}>>h_{name}{binning}", sel, "goff")   # vectors are flattened automatically
    h = ROOT.gDirectory.Get(f"h_{name}")
    if n <= 0 or not h:
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
