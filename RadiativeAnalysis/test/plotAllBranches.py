#!/usr/bin/env python3
import sys
import ROOT
ROOT.gROOT.SetBatch(True)

chain = ROOT.TChain("tree")
for f in sys.argv[1:]:
    chain.Add(f)
out = str(sys.argv[1])[:-5]+".pdf"

# fixed binning (nbins, xmin, xmax) for the main branches [GeV, cm]
# all others get an automatic range
ranges = {}
#     # masses
#     "bsMass":                 (60, 4.5, 6.0),
#     "photonDimuonMass":       (60, 4.5, 6.0),
#     "dimuonGenPhotonMass":    (60, 4.5, 6.0),
#     "dimuonMass":             (100, 0.0, 5.0),
#     "dimuonFitMass":          (100, 0.0, 5.0),
#     # kinematics
#     "mu1Pt":                  (50, 0, 30),
#     "mu2Pt":                  (50, 0, 30),
#     "dimuonPt":               (50, 0, 50),
#     "bsPt":                   (50, 0, 60),
#     "photonPt":               (50, 0, 25),
#     "photonEnergy":           (50, 0, 50),
#     "photonEta":              (50, -2.5, 2.5),
#     "photonPhi":              (64, -3.2, 3.2),
#     "photonDeltaRs":          (50, 0, 0.5),
#     "photonMVA":              (50, -1, 1),
#     # vertex probabilities
#     "bsVtxProb":              (50, 0, 1),
#     "dimuonVtxProb":          (50, 0, 1),
#     "DiMuon_KalmanVtxProb":   (50, 0, 1),
#     "maxMuonsVertexComp":     (50, 0, 1),
#     # displacement and pointing
#     "bsLxy":                  (50, 0, 1.0),
#     "photonDimuonLxy":        (50, 0, 1.0),
#     "bsLxySig":               (50, 0, 50),
#     "photonDimuonLxySig":     (50, 0, 50),
#     "bsCt":                   (50, 0, 0.2),
#     "photonDimuonCosAlpha":   (100, -1, 1),
#     "photonDimuonCosAlpha2D": (100, -1, 1),
#     # event level
#     "nTracks":                (50, 0, 3000),
#     "nPhotons":               (20, 0, 20),
#     "nMuons":                 (10, 0, 10),
#     "PUTrueinteraction":      (80, 0, 80),
# }


# selection applied to all histograms; it is also written on every page of the pdf
selection = "bsFitSuccessful"

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
