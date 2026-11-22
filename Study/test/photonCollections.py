#!/usr/bin/env python3
"""Expected B_s -> mu mu gamma yields per photon collection from the photonCollections_cfg.py outputs.

    python3 photonCollections.py [--lumi 297261] [--br 5e-9] [--plot mass.png]

N_expected = sigma * filter efficiency * BR * L * N_step / N_processed (N_processed: the 'all events' bin).
  new MC (Run3Summer22 BsTo2MuG, gen cuts): sigma and filter efficiency of the dataset (GenRunInfoProduct and
      GenFilterInfo, stored in the v1 ntuple meta.json);
  legacy MC (zero bias, no gen cuts): pythia sigma 5.822e8 pb, filter efficiency 0.1298.
The default luminosity 297261 pb^-1 is the good-luminosity equivalent of 326 fb^-1 recorded (34.65 x 326 / 38),
the normalisation of the Combine study.
"""
import argparse
import os

import ROOT

ROOT.gROOT.SetBatch(True)
HERE = os.path.dirname(os.path.abspath(__file__))
SAMPLES = {   # file, sigma [pb], filter efficiency, collections
    'legacy': ('photonCollections_legacy.root', 5.822e8, 1.298e-1, ['photons', 'gedPhotons']),
    'new_aod': ('photonCollections_new_aod.root', 574561851.8057579, 0.0017011784271218377, ['photons', 'gedPhotons']),
    'new_miniaod': ('photonCollections_new_miniaod.root', 574561851.8057579, 0.0017011784271218377,
                    ['slimmedPhotons']),
}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--lumi', type=float, default=297261., help='pb^-1')
    ap.add_argument('--br', type=float, default=5e-9)
    ap.add_argument('--plot', default=os.path.join(HERE, 'photonCollections_mass.png'))
    ap.add_argument('--ptcut', type=float, default=4.0, help='legacy: also the yield with gen photon pt > ptcut')
    args = ap.parse_args()

    rows, hists = [], []
    for sample, (fname, xsec, eff, colls) in SAMPLES.items():
        path = os.path.join(HERE, fname)
        if not os.path.exists(path):
            print('missing %s' % path)
            continue
        f = ROOT.TFile.Open(path)
        for c in colls:
            d = 'pho_' + c
            cf = f.Get(d + '/cutflow')
            n = cf.GetBinContent(1)
            scale = xsec * eff * args.br * args.lumi / n
            steps = [cf.GetBinContent(i) for i in range(1, cf.GetNbinsX() + 1)]
            labels = [cf.GetXaxis().GetBinLabel(i) for i in range(1, cf.GetNbinsX() + 1)]
            gm = f.Get(d + '/genPhoPtMatched')
            above = gm.Integral(gm.FindBin(args.ptcut + 1e-6), gm.GetNbinsX() + 1)
            rows.append((sample, c, n, scale, steps, labels, above))
            h = f.Get(d + '/mass').Clone('%s_%s' % (sample, c))
            h.SetDirectory(0)
            h.Scale(scale)
            hists.append(h)

    print('expected events at L = %.0f pb^-1, BR = %g (raw MC counts in brackets)\n' % (args.lumi, args.br))
    labels = rows[0][5]
    head = '%-12s %-15s' % ('sample', 'photons') + ''.join('%22s' % l for l in labels)
    print(head)
    for sample, c, n, scale, steps, _, _ in rows:
        print('%-12s %-15s' % (sample, c) + ''.join('%14.1f (%5d)' % (s * scale, s) for s in steps))
    print('\nefficiency (photon matched / trigger + muons) and the matched events with gen photon pt > %g GeV:'
          % args.ptcut)
    for sample, c, n, scale, steps, _, above in rows:
        print('  %-12s %-15s photon eff %.3f   matched with gen pt > %g: %.1f (%d of %d)'
              % (sample, c, steps[4] / steps[3] if steps[3] else 0, args.ptcut, above * scale, above, steps[4]))

    c = ROOT.TCanvas('c', '', 800, 600)
    colors = [ROOT.kBlack, ROOT.kRed + 1, ROOT.kBlue + 1, ROOT.kGreen + 2, ROOT.kMagenta + 1]
    leg = ROOT.TLegend(0.55, 0.65, 0.88, 0.88)
    hmax = max(h.GetMaximum() for h in hists)
    for i, h in enumerate(hists):
        h.SetLineColor(colors[i % len(colors)])
        h.SetLineWidth(2)
        h.SetStats(0)
        h.SetMaximum(1.2 * hmax)
        h.SetTitle('expected events, %.0f fb^{-1}' % (args.lumi / 1000))
        h.Draw('hist' if i == 0 else 'hist same')
        leg.AddEntry(h, '%s (%.0f)' % (h.GetName(), h.Integral()), 'l')
    leg.Draw()
    c.SaveAs(args.plot)


if __name__ == '__main__':
    main()
