#!/usr/bin/env python3
"""Validation of the mu mu gamma kinematic fit (MMGVertexFit).

Usage:
    python3 validateMMGFit.py --ntuple ntuple.root [--example histos.root] [--outdir plots]
                              [--prefix mmg_] [--min-vtx-prob 1e-3] [--matched]

The ntuple can be the new one (tree bmmg/Events, candidates with prefix mmg_/mmcg_/mmgg_/mmkk_) or the
legacy one (tree BMMGTree, one candidate per event); the format is detected automatically.
From the ntuple, for fitted candidates:
  - B mass: pre-fit 4-vector and fitted, residual to the gen B mass (mean, RMS, core Gaussian width)
  - ct3D: residual to the gen ct3D, ct3D error, pull
The selection can be restricted to gen-matched candidates (--matched).
From the MMGVertexFitExample histogram file: mean, RMS and core Gaussian width of every 1D histogram
(the pulls should have core width ~1).
Plots are written as PNG to --outdir.
"""
import argparse
import os

import numpy as np
import uproot
import ROOT

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(1110)
ROOT.gStyle.SetOptFit(111)


def core_gauss(h, nsigma=1.5, iterations=3):
    """Iterative Gaussian fit to the core of a histogram; returns (mean, sigma, sigma_err) or None."""
    if h.GetEntries() < 20:
        return None
    mean, rms = h.GetMean(), h.GetRMS()
    f = None
    for _ in range(iterations):
        f = ROOT.TF1("core_%s" % h.GetName(), "gaus", mean - nsigma * rms, mean + nsigma * rms)
        h.Fit(f, "QR0")
        mean, rms = f.GetParameter(1), abs(f.GetParameter(2))
    h.Fit(f, "QR")  # keep the final fit attached for drawing
    return mean, rms, f.GetParError(2)


def hist(name, title, values, nbins, lo, hi):
    h = ROOT.TH1D(name, title, nbins, lo, hi)
    h.SetDirectory(0)
    for v in values:
        h.Fill(v)
    return h


def save(h, outdir, name):
    c = ROOT.TCanvas("c_" + name, "", 700, 500)
    h.Draw()
    c.SaveAs(os.path.join(outdir, name + ".png"))


def summarize(label, values, h=None):
    values = np.asarray(values)
    line = "%-28s N=%6d  mean=%+9.5f  RMS=%9.5f" % (label, len(values), values.mean(), values.std())
    if h is not None:
        g = core_gauss(h)
        if g:
            line += "  core: mean=%+9.5f sigma=%8.5f +- %.5f" % g
    print(line)


def ntuple_validation(path, outdir, matched, prefix="mmg_", minVtxProb=-1.):
    f = uproot.open(path)
    if "bmmg" in f:   # new ntuple: vectors of candidates
        import awkward as ak
        t = f["bmmg/Events"]
        a = t.arrays([prefix + n for n in ["mass", "prefitMass", "fitOK", "vtxProb", "ct3D", "ct3DErr"]] +
                     ([prefix + "genMatched"] if prefix + "genMatched" in t.keys() else []) + ["gen_B_mass", "gen_B_ct3D"])
        sel = (a[prefix + "fitOK"] == 1) & (a[prefix + "vtxProb"] > minVtxProb)
        if matched: sel = sel & (a[prefix + "genMatched"] == 1)
        gm = ak.broadcast_arrays(a.gen_B_mass, a[prefix + "mass"])[0]
        gct = ak.broadcast_arrays(a.gen_B_ct3D, a[prefix + "mass"])[0]
        flat = lambda x: ak.to_numpy(ak.flatten(x[sel])).astype(float)
        fit_mass, pre_mass, gen_mass = flat(a[prefix + "mass"]), flat(a[prefix + "prefitMass"]), flat(gm)
        ct, cterr, gen_ct = flat(a[prefix + "ct3D"]), flat(a[prefix + "ct3DErr"]), flat(gct)
        n = t.num_entries
        print("\n=== ntuple %s (new format, %s): %d events, %d fitted candidates%s%s"
              % (path, prefix, n, len(fit_mass), " (gen-matched)" if matched else "",
                 ", vtxProb > %g" % minVtxProb if minVtxProb > 0 else ""))
        ok = np.ones(len(fit_mass), dtype=bool)
    else:              # legacy ntuple
        t = f["BMMGTree"]
        n = t.num_entries
        if "nEvents" in t.keys():  # the old tree has one extra endJob row (plan P2.17)
            n -= 1
        arr = lambda b: t[b].array(library="np", entry_stop=n).astype(float)
        fit_mass = arr("VertexfitBsMass_mmrecog")
        pre_mass = arr("FourvectorBsMass_mmrecog")
        gen_mass = arr("GenB_mass")
        ct, cterr, gen_ct = arr("BsCt3D_mmrecog"), arr("VertexfitBsCtErr3D_mmrecog"), arr("GenBct3D")
        ok = (fit_mass > 0) & (gen_mass > 0)
        if matched:
            ok &= (arr("Topology_isMatched") > 0)
        print("\n=== ntuple %s (legacy format): %d events, %d with a fitted mu mu gamma candidate%s"
              % (path, n, ok.sum(), " (gen-matched topology)" if matched else ""))

    for name, m in [("pre-fit mass - gen", pre_mass), ("fitted mass - gen", fit_mass)]:
        r = (m - gen_mass)[ok]
        h = hist(name.replace(" ", "_").replace("-", "m"), "%s;m - m_{gen} [GeV];candidates" % name, r, 120, -0.6, 0.6)
        summarize(name, r, h)
        save(h, outdir, h.GetName())
    h = hist("fitted_mass", "fitted B mass;m [GeV];candidates", fit_mass[ok], 100, 4.5, 6.5)
    save(h, outdir, "fitted_mass")

    ok = ok & (cterr > 0)
    r = (ct - gen_ct)[ok]
    h = hist("ct3D_residual", "ct3D - gen;#Delta ct [cm];candidates", r, 100, -0.05, 0.05)
    summarize("ct3D - gen [cm]", r, h)
    save(h, outdir, "ct3D_residual")
    summarize("ct3D error [cm]", cterr[ok])
    p = r / cterr[ok]
    h = hist("ct3D_pull", "ct3D pull;(ct - ct_{gen})/#sigma;candidates", p, 100, -10, 10)
    summarize("ct3D pull", p, h)
    save(h, outdir, "ct3D_pull")


def example_validation(path, outdir):
    f = ROOT.TFile.Open(path)
    print("\n=== MMGVertexFitExample histograms %s" % path)
    for key in f.GetListOfKeys():
        h = key.ReadObj()
        if h.InheritsFrom("TH2"):
            continue
        h.SetDirectory(0)
        g = core_gauss(h) if ("Pull" in h.GetName() or "Residual" in h.GetName() or "Mass" in h.GetName()) else None
        line = "%-22s N=%7d  mean=%+10.5f  RMS=%9.5f" % (h.GetName(), h.GetEntries(), h.GetMean(), h.GetRMS())
        if g:
            line += "  core: mean=%+9.5f sigma=%8.5f +- %.5f" % g
        print(line)
        save(h, outdir, "example_" + h.GetName())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ntuple")
    ap.add_argument("--example")
    ap.add_argument("--outdir", default="validation_plots")
    ap.add_argument("--matched", action="store_true", help="only gen-matched candidates (new: <prefix>genMatched, legacy: Topology_isMatched)")
    ap.add_argument("--prefix", default="mmg_", help="candidate prefix in the new ntuple (mmg_, mmcg_, mmgg_, mmkk_)")
    ap.add_argument("--min-vtx-prob", type=float, default=-1., help="cut on the fitted vertex probability (new ntuple)")
    args = ap.parse_args()
    os.makedirs(args.outdir, exist_ok=True)
    if args.ntuple:
        ntuple_validation(args.ntuple, args.outdir, args.matched, args.prefix, args.min_vtx_prob)
    if args.example:
        example_validation(args.example, args.outdir)


if __name__ == "__main__":
    main()
