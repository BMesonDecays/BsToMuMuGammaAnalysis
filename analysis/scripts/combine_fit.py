#!/usr/bin/env python3
"""Mass fits, Combine workspaces and datacards, and the expected (blind) significance per BDT cut.

    cd <combine CMSSW area>/src && cmsenv          # Combine v11.1.0 (config: combine.cmssw)
    python3 scripts/combine_fit.py config/combine_mmg.yaml [--cuts 0.99,0.995] [--no-combine]

Input: <output>/mmg_mass_hists.root and yields.csv of scripts/combine_hists.py. Per BDT cut:
  - signal model: double-sided Crystal Ball (RooCrystalBall) fitted to the weighted signal MC histogram (the
    shape changes with the cut, so it is refitted at every cut); its parameters are then fixed. Rate S = the
    signal MC yield at the Run 3 luminosity in the fit range;
  - background model: exponential fitted to the unscaled data sidebands (binned Poisson likelihood in the
    sideband ranges only); yield in the fit range = sideband count / sideband fraction of the exponential,
    times the luminosity factor. Slope and yield float in Combine (flatParam): the background is constrained
    only by the (Asimov) data;
  - workspace <output>/combine/ws/ws_<cut>.root (mass, sig, bkg, bkg_norm, data_obs) and datacard
    <output>/combine/cards/card_<cut>.txt. data_obs is the background-only expectation, a placeholder:
    the data are never used, Combine generates Asimov data from the model (-t -1);
  - combine -M Significance -t -1 --expectSignal 1   (expected significance for B = the assumed BR)
    combine -M AsymptoticLimits --run blind           (expected upper limit on r = BR / assumed BR)
Output: <output>/combine/results.csv, fits/*.png, significance_vs_cut.png, logs/.
"""
import argparse
import csv
import math
import os
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from bmmg_ana import config  # noqa: E402

HIST_FILE = 'mmg_mass_hists.root'


def cut_name(cut):
    return 'cut_' + ('%g' % cut).replace('.', 'p')


def read_yields(path):
    with open(path) as f:
        return {float(r['cut']): r for r in csv.DictReader(f)}


class Fitter:
    def __init__(self, cfg):
        import ROOT
        ROOT.gROOT.SetBatch(True)
        ROOT.RooMsgService.instance().setGlobalKillBelow(ROOT.RooFit.WARNING)
        self.R = ROOT
        self.cfg = cfg
        cc = cfg['combine']
        self.lo, self.hi = cc['mass_fit']['range']
        self.nbins = int(round((self.hi - self.lo) / cc['mass_fit']['bin_width']))
        self.factor = cc['data_factor']
        self.sidebands = [(max(a, self.lo), min(b, self.hi)) for a, b in cfg['mass']['sidebands']
                          if b > self.lo and a < self.hi]
        self.window = tuple(cfg['mass']['signal_window'])

    def mass(self):
        R = self.R
        m = R.RooRealVar('mass', 'm(#mu#mu#gamma) [GeV]', self.lo, self.hi)
        m.setBins(self.nbins)
        for i, (a, b) in enumerate(self.sidebands):
            m.setRange('sb%d' % i, a, b)
        m.setRange('window', *self.window)
        m.setRange('full', self.lo, self.hi)
        return m

    def fit_signal(self, hist, m):
        R = self.R
        dh = R.RooDataHist('sig_mc', 'signal MC', R.RooArgList(m), hist)
        p = dict(mean=R.RooRealVar('sig_mean', 'mean', 5.367, 5.2, 5.5),
                 sigma=R.RooRealVar('sig_sigma', 'sigma', 0.1, 0.02, 0.4),
                 aL=R.RooRealVar('sig_alphaL', 'alphaL', 1.5, 0.1, 10.), nL=R.RooRealVar('sig_nL', 'nL', 3., 0.5, 100.),
                 aR=R.RooRealVar('sig_alphaR', 'alphaR', 1.5, 0.1, 10.), nR=R.RooRealVar('sig_nR', 'nR', 5., 0.5, 100.))
        pdf = R.RooCrystalBall('sig', 'signal', m, p['mean'], p['sigma'], p['aL'], p['nL'], p['aR'], p['nR'])
        res = pdf.fitTo(dh, R.RooFit.SumW2Error(True), R.RooFit.Save(True), R.RooFit.PrintLevel(-1),
                        R.RooFit.Strategy(2))
        for v in p.values():
            v.setConstant(True)
        return pdf, dh, p, res

    def fit_background(self, hist, m):
        R = self.R
        dh = R.RooDataHist('sb_raw', 'data sidebands', R.RooArgList(m), hist)
        slope = R.RooRealVar('bkg_slope', 'slope', -0.5, -20., 5.)
        pdf = R.RooExponential('bkg', 'background', m, slope)
        ranges = ','.join('sb%d' % i for i in range(len(self.sidebands)))
        res = pdf.fitTo(dh, R.RooFit.Range(ranges), R.RooFit.Save(True), R.RooFit.PrintLevel(-1),
                        R.RooFit.Strategy(2))
        n_sb = sum(hist.GetBinContent(i) for i in range(1, hist.GetNbinsX() + 1)
                   if any(a <= hist.GetBinCenter(i) < b for a, b in self.sidebands))
        nset = R.RooArgSet(m)
        full = pdf.createIntegral(nset, R.RooFit.NormSet(nset), R.RooFit.Range('full')).getVal()
        frac_sb = sum(pdf.createIntegral(nset, R.RooFit.NormSet(nset), R.RooFit.Range('sb%d' % i)).getVal()
                      for i in range(len(self.sidebands))) / full
        frac_win = pdf.createIntegral(nset, R.RooFit.NormSet(nset), R.RooFit.Range('window')).getVal() / full
        n_full = n_sb / frac_sb
        # deviance over the sideband bins (goodness of fit)
        dev, nb = 0., 0
        for i in range(1, hist.GetNbinsX() + 1):
            x = hist.GetBinCenter(i)
            if not any(a <= x < b for a, b in self.sidebands):
                continue
            m.setVal(x)
            mu = n_full * pdf.getVal(nset) * hist.GetBinWidth(i)
            n = hist.GetBinContent(i)
            dev += 2 * (mu - n + (n * math.log(n / mu) if n > 0 else 0.))
            nb += 1
        return pdf, dh, slope, res, dict(n_sb=n_sb, n_full=n_full, frac_win=frac_win, deviance=dev, ndf=nb - 2)

    def plot(self, m, data, pdf, path, title, blind=False, norm=None):
        R = self.R
        c = R.TCanvas('c', '', 800, 700)
        top = R.TPad('top', '', 0, 0.3, 1, 1)
        bot = R.TPad('bot', '', 0, 0, 1, 0.3)
        top.SetBottomMargin(0.02)
        bot.SetTopMargin(0.02)
        bot.SetBottomMargin(0.3)
        top.Draw()
        bot.Draw()
        top.cd()
        fr = m.frame(R.RooFit.Title(title))
        if blind:
            ranges = ','.join('sb%d' % i for i in range(len(self.sidebands)))
            data.plotOn(fr, R.RooFit.CutRange(ranges))
            pdf.plotOn(fr, R.RooFit.Range('full'), R.RooFit.NormRange(ranges), R.RooFit.LineColor(R.kBlue))
        else:
            data.plotOn(fr, R.RooFit.DataError(R.RooAbsData.SumW2))
            pdf.plotOn(fr, R.RooFit.LineColor(R.kRed))
        fr.Draw()
        bot.cd()
        pulls = fr.pullHist()
        fp = m.frame(R.RooFit.Title(''))
        fp.addPlotable(pulls, 'P')
        fp.SetMinimum(-4)
        fp.SetMaximum(4)
        fp.GetYaxis().SetTitle('pull')
        fp.GetYaxis().SetTitleSize(0.1)
        fp.GetXaxis().SetTitleSize(0.1)
        fp.GetXaxis().SetLabelSize(0.08)
        fp.GetYaxis().SetLabelSize(0.08)
        fp.Draw()
        c.SaveAs(path)
        c.Close()

    def plot_background(self, m, hist, pdf, n_full, path, title):
        """Sideband data (only the sideband bins: the blind window is not drawn) and the fitted exponential,
        normalised to the fitted yield over the full range, with the pulls of the sideband bins."""
        R = self.R
        c = R.TCanvas('c', '', 800, 700)
        top = R.TPad('top', '', 0, 0.3, 1, 1)
        bot = R.TPad('bot', '', 0, 0, 1, 0.3)
        top.SetBottomMargin(0.02)
        bot.SetTopMargin(0.02)
        bot.SetBottomMargin(0.3)
        top.Draw()
        bot.Draw()
        nset = R.RooArgSet(m)
        g, gp = R.TGraphAsymmErrors(), R.TGraphErrors()
        ymax = 0.
        for i in range(1, hist.GetNbinsX() + 1):
            x = hist.GetBinCenter(i)
            if not any(a <= x < b for a, b in self.sidebands):
                continue
            n = hist.GetBinContent(i)
            lo = n - 0.5 * R.Math.chisquared_quantile_c(0.84135, 2 * n) if n > 0 else 0.
            hi = 0.5 * R.Math.chisquared_quantile_c(0.15865, 2 * (n + 1)) - n
            k = g.GetN()
            g.SetPoint(k, x, n)
            g.SetPointError(k, 0., 0., lo, hi)
            m.setVal(x)
            mu = n_full * pdf.getVal(nset) * hist.GetBinWidth(i)
            gp.SetPoint(gp.GetN(), x, (n - mu) / math.sqrt(mu) if mu > 0 else 0.)
            ymax = max(ymax, n + hi)
        top.cd()
        fr = m.frame(R.RooFit.Title(title))
        pdf.plotOn(fr, R.RooFit.Normalization(n_full, R.RooAbsReal.NumEvent), R.RooFit.Range('full'),
                   R.RooFit.NormRange('full'), R.RooFit.LineColor(R.kBlue))
        fr.SetMaximum(1.15 * ymax)
        fr.SetMinimum(0.)
        fr.GetYaxis().SetTitle('candidates / %.3f GeV' % hist.GetBinWidth(1))
        fr.Draw()
        box = R.TBox(self.cfg['mass']['blind'][0], 0., self.cfg['mass']['blind'][1], 1.15 * ymax)
        box.SetFillColorAlpha(R.kGray, 0.35)
        box.Draw()
        g.SetMarkerStyle(20)
        g.Draw('P same')
        bot.cd()
        fp = m.frame(R.RooFit.Title(''))
        fp.SetMinimum(-4)
        fp.SetMaximum(4)
        fp.GetYaxis().SetTitle('pull')
        fp.GetYaxis().SetTitleSize(0.1)
        fp.GetXaxis().SetTitleSize(0.1)
        fp.GetXaxis().SetLabelSize(0.08)
        fp.GetYaxis().SetLabelSize(0.08)
        fp.Draw()
        gp.SetMarkerStyle(20)
        gp.Draw('P same')
        c.SaveAs(path)
        c.Close()

    def workspace(self, path, m, sig, bkg, b_full):
        R = self.R
        w = R.RooWorkspace('w', 'w')
        imp = getattr(w, 'import')
        imp(sig)
        imp(bkg, R.RooFit.RecycleConflictNodes())
        norm = R.RooRealVar('bkg_norm', 'background yield', b_full, 0., 10. * b_full + 50.)
        imp(norm)
        # placeholder data_obs: the background-only expectation (Combine generates Asimov data with -t -1)
        asimov = bkg.generateBinned(R.RooArgSet(m), b_full, R.RooFit.ExpectedData(True))
        asimov.SetName('data_obs')
        imp(asimov)
        w.writeToFile(path)

    def asimov_z_known_bkg(self, m, sig, bkg, s, b):
        """Binned Asimov Z with a known background (no floating parameters): an upper bound of the Combine Z."""
        R = self.R
        nset = R.RooArgSet(m)
        z2 = 0.
        width = (self.hi - self.lo) / self.nbins
        for i in range(self.nbins):
            a = self.lo + i * width
            m.setRange('bin', a, a + width)
            si = s * sig.createIntegral(nset, R.RooFit.NormSet(nset), R.RooFit.Range('bin')).getVal()
            bi = b * bkg.createIntegral(nset, R.RooFit.NormSet(nset), R.RooFit.Range('bin')).getVal()
            if bi > 0 and si > 0:
                z2 += 2 * ((si + bi) * math.log1p(si / bi) - si)
        return math.sqrt(z2)


DATACARD = """# B_s -> mu mu gamma (mmg), BDT score > {cut}: Run 3 projection ({lumi:.0f} pb^-1 recorded), blind
# signal: double-sided Crystal Ball from the signal MC (fixed); background: exponential, slope and yield float
imax 1
jmax 1
kmax *
---------------
shapes sig      mmg {ws} w:sig
shapes bkg      mmg {ws} w:bkg
shapes data_obs mmg {ws} w:data_obs
---------------
bin         mmg
observation -1
---------------
bin         mmg     mmg
process     sig     bkg
process     0       1
rate        {s:.6g}  1
---------------
bkg_slope flatParam
bkg_norm  flatParam
"""


def run(cmd, log, cwd):
    with open(log, 'w') as f:
        f.write('$ ' + ' '.join(cmd) + '\n')
        f.flush()
        p = subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, cwd=cwd)
    if p.returncode != 0:
        raise RuntimeError('%s failed (exit %d), see %s' % (cmd[0], p.returncode, log))


def read_limit_tree(path):
    import ROOT
    f = ROOT.TFile.Open(path)
    t = f.Get('limit')
    out = [(t.quantileExpected, t.limit) for _ in t]
    f.Close()
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    ap.add_argument('--cuts', default='', help='only these cuts (comma-separated values of bdt_cuts)')
    ap.add_argument('--no-combine', action='store_true', help='only the fits, workspaces and datacards')
    args = ap.parse_args()
    cfg = config.load_combine_config(args.config)
    cc = cfg['combine']
    out = cfg['output']
    if not args.no_combine and not shutil.which('combine'):
        sys.exit('combine not found: run cmsenv in the Combine CMSSW area (%s)' % cc['combine']['cmssw'])
    work = os.path.join(out, 'combine')
    for d in ('ws', 'cards', 'fits', 'logs', 'results'):
        os.makedirs(os.path.join(work, d), exist_ok=True)
    yields = read_yields(os.path.join(out, 'yields.csv'))
    cuts = [float(c) for c in args.cuts.split(',')] if args.cuts else [float(c) for c in cc['bdt_cuts']]

    fitter = Fitter(cfg)
    R = fitter.R
    hf = R.TFile.Open(os.path.join(out, HIST_FILE))
    rows = []
    for cut in cuts:
        name = cut_name(cut)
        y = yields.get(cut)
        if y is None:
            print('%s: not in yields.csv (run combine_hists.py with this cut)' % name)
            continue
        if y['fit'] != 'True':
            print('%s: %s raw sideband candidates (< min_sideband): skipped' % (name, y['n_sb_raw']))
            rows.append({'cut': cut, 'status': 'too few sideband candidates', 'n_sb_raw': y['n_sb_raw']})
            continue
        m = fitter.mass()
        hsig = hf.Get(name + '/signal')
        hsb = hf.Get(name + '/data_sb_raw')
        sig, sigdh, sp, sres = fitter.fit_signal(hsig, m)
        bkg, bkgdh, slope, bres, binfo = fitter.fit_background(hsb, m)
        s_full = hsig.Integral()
        b_full = binfo['n_full'] * fitter.factor
        nset = R.RooArgSet(m)
        s_win = s_full * sig.createIntegral(nset, R.RooFit.NormSet(nset), R.RooFit.Range('window')).getVal()
        b_win = b_full * binfo['frac_win']
        fitter.plot(m, sigdh, sig, os.path.join(work, 'fits', 'sig_%s.png' % name), 'signal MC, BDT > %g' % cut)
        fitter.plot_background(m, hsb, bkg, binfo['n_full'], os.path.join(work, 'fits', 'bkg_%s.png' % name),
                               'data sidebands (2022, unscaled), BDT > %g' % cut)
        ws = os.path.join(work, 'ws', 'ws_%s.root' % name)
        fitter.workspace(ws, m, sig, bkg, b_full)
        card = os.path.join(work, 'cards', 'card_%s.txt' % name)
        with open(card, 'w') as f:
            f.write(DATACARD.format(cut=cut, lumi=cc['lumi']['target_recorded_pb'], ws=os.path.relpath(ws, os.path.dirname(card)),
                                    s=s_full))
        z_known = fitter.asimov_z_known_bkg(m, sig, bkg, s_full, b_full)
        row = {'cut': cut, 'status': 'ok', 'S': s_full, 'S_window': s_win, 'B': b_full, 'B_window': b_win,
               'n_sb_raw': int(round(binfo['n_sb'])), 'sig_sigma': sp['sigma'].getVal(), 'sig_mean': sp['mean'].getVal(),
               'sig_fit_status': sres.status(), 'bkg_slope': slope.getVal(), 'bkg_slope_err': slope.getError(),
               'bkg_fit_status': bres.status(), 'bkg_deviance': binfo['deviance'], 'bkg_ndf': binfo['ndf'],
               'Z_known_bkg': z_known}
        print('%-11s S = %7.2f (window %6.2f)  B = %9.1f (window %8.1f)  sb raw %5d  sigma %.4f  slope %.2f +- %.2f'
              '  dev/ndf %.1f/%d  Z(known B) %.2f'
              % (name, s_full, s_win, b_full, b_win, row['n_sb_raw'], row['sig_sigma'], row['bkg_slope'],
                 row['bkg_slope_err'], binfo['deviance'], binfo['ndf'], z_known))
        if not args.no_combine:
            res_dir = os.path.join(work, 'results')
            card_root = os.path.join(res_dir, 'card_%s.root' % name)
            run(['text2workspace.py', os.path.abspath(card), '-o', card_root],
                os.path.join(work, 'logs', 't2w_%s.log' % name), cwd=os.path.dirname(os.path.abspath(card)))
            run(['combine', '-M', 'Significance', card_root, '-t', '-1', '--expectSignal', '1', '-n', '.' + name],
                os.path.join(work, 'logs', 'significance_%s.log' % name), cwd=res_dir)
            z = read_limit_tree(os.path.join(res_dir, 'higgsCombine.%s.Significance.mH120.root' % name))
            row['Z_combine'] = z[0][1] if z else float('nan')
            if cc['combine'].get('limits', True):
                run(['combine', '-M', 'AsymptoticLimits', card_root, '--run', 'blind', '--rMax', '500', '-n', '.' + name],
                    os.path.join(work, 'logs', 'limits_%s.log' % name), cwd=res_dir)
                for q, v in read_limit_tree(os.path.join(res_dir, 'higgsCombine.%s.AsymptoticLimits.mH120.root' % name)):
                    key = {0.025: 'r_m2s', 0.16: 'r_m1s', 0.5: 'r_exp', 0.84: 'r_p1s', 0.975: 'r_p2s'}.get(round(q, 3))
                    if key:
                        row[key] = v
            print('            Combine: expected significance %.3f' % row['Z_combine']
                  + ('   expected limit r < %.3f (+1s %.3f, -1s %.3f)' % (row['r_exp'], row['r_p1s'], row['r_m1s'])
                     if 'r_exp' in row else ''))
        rows.append(row)
    hf.Close()

    keys = list(dict.fromkeys(k for r in rows for k in r))
    with open(os.path.join(work, 'results.csv'), 'w', newline='') as f:
        wr = csv.DictWriter(f, fieldnames=keys)
        wr.writeheader()
        wr.writerows(rows)
    ok = [r for r in rows if r.get('status') == 'ok' and 'Z_combine' in r]
    if ok:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(7, 4.5))
        x = range(len(ok))
        ax.plot(x, [r['Z_combine'] for r in ok], 'o-', label='Combine (background slope and yield float)')
        ax.plot(x, [r['Z_known_bkg'] for r in ok], 's--', mfc='none', label='Asimov, known background')
        ax.set_xticks(list(x))
        ax.set_xticklabels(['%g' % r['cut'] for r in ok], rotation=45)
        ax.set_xlabel('BDT score cut')
        ax.set_ylabel('expected significance')
        ax.set_title('B$_s\\to\\mu\\mu\\gamma$, Run 3 (%.0f fb$^{-1}$), $\\mathcal{B}$ = %.0e'
                     % (cc['lumi']['target_recorded_pb'] / 1000., cfg['samples']['signal'].get('br_scale', 1.)), fontsize=10)
        ax.grid(alpha=0.3)
        ax.legend(fontsize=9)
        fig.savefig(os.path.join(work, 'significance_vs_cut.png'), bbox_inches='tight')
    print('\nresults in %s' % os.path.join(work, 'results.csv'))


if __name__ == '__main__':
    main()
