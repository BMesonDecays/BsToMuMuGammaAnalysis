#!/usr/bin/env python3
"""Consistency check of an ntuple of a given MC decay mode (gen truth and reconstructed candidates).

Usage:
    python3 checkDecayModes.py ntuple1.root [ntuple2.root ...]

For each ntuple it prints:
  - gen: decay-mode and B-species counts, gen B and dimuon mass, ct, number and origin of gen photons,
    m(gamma gamma) of the photons from the neutral meson (4-body modes);
  - objects: fraction of events with two reconstructed signal muons and with >= 1 / >= 2 signal photons;
  - for each candidate channel (mmg_, mmcg_, mmgg_, mmkk_): candidates per event, gen-matched fraction,
    fitted and pre-fit mass, residual to the gen B mass, fitted dimuon mass, m(X), ct residual and pull
    (gen-matched candidates with fitOK and vtxProb > 1e-3), and how often the highest-vtxProb candidate
    is the gen-matched one.
Widths are s68 = half of the 16-84% interval. Expected values per mode are in report.md (2026-11-09).
"""
import sys
from collections import Counter

import awkward as ak
import numpy as np
import uproot

PFX = ["mmg_", "mmcg_", "mmgg_", "mmkk_"]


def s68(x):
    x = np.asarray(x, dtype=float)
    x = x[np.isfinite(x)]
    if len(x) < 5:
        return "n=%d" % len(x)
    q16, q50, q84 = np.percentile(x, [16, 50, 84])
    return "med=%.4f s68=%.4f n=%d" % (q50, (q84 - q16) / 2, len(x))


def main(path):
    t = uproot.open(path)["bmmg/Events"]
    keys = set(t.keys())
    a = t.arrays([k for k in keys if k.startswith(("gen_", "mu_gen", "pho_gen", "evt_")) or
                  any(k.startswith(p) for p in PFX) or k in ("dimu_mass", "dimu_resonance", "dimu_mu1", "dimu_mu2")])
    n = len(a)
    print("=" * 100)
    print(path, "events:", n)
    # ---------------------------------------------------------------- gen
    print("gen_valid frac %.3f" % ak.mean(a.gen_valid))
    print("decayMode", dict(Counter(ak.to_list(a.gen_decayMode))))
    print("B pdgId", dict(Counter(ak.to_list(a.gen_B_pdgId))))
    print("isSignal %.3f is3Body %.3f is4Body %.3f hasJpsi %.3f" %
          (ak.mean(a.gen_isSignal), ak.mean(a.gen_is3Body), ak.mean(a.gen_is4Body), ak.mean(a.gen_hasJpsi)))
    v = a.gen_valid == 1
    print("gen B mass", s68(a.gen_B_mass[v]), "| gen dimuon mass", s68(a.gen_dimuonMass[v]))
    print("gen B ct3D [cm]", s68(a.gen_B_ct3D[v]), "mean %.4f" % ak.mean(a.gen_B_ct3D[v]))
    print("gen photons/event", dict(Counter(ak.to_list(ak.num(a.gen_pho_pt[v])))))
    print("gen photon origin", dict(Counter(ak.to_list(ak.flatten(a.gen_pho_origin[v])))),
          "mother", dict(Counter(ak.to_list(ak.flatten(a.gen_pho_motherPdgId[v]))).most_common(6)))
    # m(gamma gamma) of the photons from the neutral meson
    neu = a.gen_pho_origin == 3
    pt, eta, phi = a.gen_pho_pt[neu], a.gen_pho_eta[neu], a.gen_pho_phi[neu]
    two = ak.num(pt) >= 2
    if ak.sum(two):
        p1 = (pt[two][:, 0], eta[two][:, 0], phi[two][:, 0])
        p2 = (pt[two][:, 1], eta[two][:, 1], phi[two][:, 1])
        mgg = np.sqrt(2 * p1[0] * p2[0] * (np.cosh(p1[1] - p2[1]) - np.cos(p1[2] - p2[2])))
        print("gen m(gg) from neutral meson", s68(mgg))
    if "gen_diphotonMass" in a.fields:
        f4 = a.gen_is4Body == 1
        print("gen_diphotonMass (4-body)", s68(a.gen_diphotonMass[f4]), "| gen_fourBodyMass", s68(a.gen_fourBodyMass[f4]),
              "| neutral meson", dict(Counter(ak.to_list(a.gen_neutralMesonPdgId))), "| vector meson", dict(Counter(ak.to_list(a.gen_vectorMesonPdgId))))
    print("gen helicity (3-body)", s68(a.gen_helicity[a.gen_is3Body == 1]))
    # ---------------------------------------------------------------- objects
    nmu = ak.sum(a.mu_genFromSignalB, axis=1)
    npho = ak.sum(a.pho_genFromSignalB, axis=1)
    print("events with 2 signal muons reco %.3f | >=1 signal photon %.3f | >=2 signal photons %.3f" %
          (ak.mean(nmu >= 2), ak.mean(npho >= 1), ak.mean(npho >= 2)))
    print("dimu resonance flags", dict(Counter(ak.to_list(ak.flatten(a.dimu_resonance)))))
    # ---------------------------------------------------------------- candidates
    for p in PFX:
        if p + "mass" not in a.fields:
            print(p, "not present")
            continue
        ncand = ak.num(a[p + "mass"])
        m = a[p + "genMatched"] == 1
        ok = (a[p + "fitOK"] == 1)
        good = m & ok & (a[p + "vtxProb"] > 1e-3)
        print("-- %s cand/event %.2f, events with cand %.3f, with gen-matched %.3f, fitOK frac %.3f, matched&fitOK&prob>1e-3 frac of matched %.3f"
              % (p, ak.mean(ncand), ak.mean(ncand > 0), ak.mean(ak.any(m, axis=1)), ak.mean(ak.flatten(ok)) if ak.sum(ncand) else -1,
                 ak.sum(good) / max(1, ak.sum(m))))
        if not ak.sum(m):
            continue
        fl = lambda b, s=good: ak.to_numpy(ak.flatten(a[p + b][s]))
        gm = ak.to_numpy(ak.flatten(ak.broadcast_arrays(a.gen_B_mass, a[p + "mass"])[0][good]))
        gct = ak.to_numpy(ak.flatten(ak.broadcast_arrays(a.gen_B_ct3D, a[p + "mass"])[0][good]))
        print("   matched: prefit mass", s68(fl("prefitMass", m)), "| fitted mass", s68(fl("mass")))
        print("   fitted - gen B mass", s68(fl("mass") - gm), "| prefit - gen", s68(fl("prefitMass") - gm))
        if p + "fitDimuonMass" in a.fields:
          print("   fitDimuonMass", s68(fl("fitDimuonMass")), "| J/psi-corr mass", s68(fl("mass") - fl("fitDimuonMass") + 3.0969))
        if p + "xMass" in a.fields:
            print("   xMass (matched)", s68(fl("xMass", m)))
        ct, cte = fl("ct3D"), fl("ct3DErr")
        sel = cte > 0
        print("   ct3D - gen", s68((ct - gct)[sel]), "| pull", s68(((ct - gct) / cte)[sel]))
        # best (highest vtxProb) candidate is the matched one?
        has = ak.num(a[p + "vtxProb"]) > 0
        best = ak.argmax(a[p + "vtxProb"][has], axis=1, keepdims=True)
        print("   best-vtxProb candidate is gen-matched: %.3f (events with any matched: %.3f)"
              % (ak.mean(ak.flatten(a[p + "genMatched"][has][best])), ak.mean(ak.any(m[has], axis=1))))


if __name__ == "__main__":
    for f in sys.argv[1:]:
        main(f)
