// Plots the histograms written by MMGVertexFitExample.
// Residuals, pulls and mass distributions are fitted with a Gaussian in the core (mean +- 2 sigma),
// and the fit parameters are shown in the statistics box.
//
// usage: root -l -b -q 'Plot.C("histosMMGVertexFitExample.root", "plots")'

#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TLegend.h"
#include "TPaveStats.h"
#include "TObjArray.h"
#include "TString.h"

#include <iostream>
#include <vector>

namespace
{
    // Gaussian fit of the core of the distribution: start from mean and RMS,
    // then refit twice in mean +- nSigma * sigma of the previous fit
    TF1* fitCore(TH1* h, double nSigma = 2.)
    {
        if (h->GetEntries() < 20) return nullptr;
        double mean = h->GetMean(), sigma = h->GetRMS();
        TF1* f = new TF1(Form("gaus_%s", h->GetName()), "gaus", mean - nSigma * sigma, mean + nSigma * sigma);
        f->SetParameters(h->GetMaximum(), mean, sigma);
        for (int iter = 0; iter < 3; ++iter)
        {
            f->SetRange(mean - nSigma * sigma, mean + nSigma * sigma);
            if (h->Fit(f, "QR0") != 0) return nullptr;
            mean = f->GetParameter(1);
            sigma = std::abs(f->GetParameter(2));
        }
        f->SetParNames("Constant", "Mean", "Sigma");
        f->SetLineColor(kRed);
        f->SetLineWidth(2);
        return f;
    }

    void save(TCanvas* c, const TString& outDir, const TString& name)
    {
        c->SaveAs(outDir + "/" + name + ".png");
        c->SaveAs(outDir + "/" + name + ".pdf");
    }

    void plot1D(TFile* file, const TString& outDir, const char* name, bool fit, bool logy = false, const char* xTitle = nullptr)
    {
        TH1* h = dynamic_cast<TH1*>(file->Get(name));
        if (!h)
        {
            std::cout << "Plot.C: histogram " << name << " not found" << std::endl;
            return;
        }
        if (xTitle && TString(h->GetXaxis()->GetTitle()).IsNull()) h->GetXaxis()->SetTitle(xTitle);
        if (TString(h->GetYaxis()->GetTitle()).IsNull()) h->GetYaxis()->SetTitle("candidates");

        TCanvas c(Form("c_%s", name), name, 800, 600);
        c.SetLogy(logy);
        h->SetLineColor(kBlue + 1);
        h->SetLineWidth(2);
        h->SetMinimum(logy ? 0.5 : 0.);
        h->Draw("hist");

        if (fit)
        {
            // the fit stored with the histogram is drawn with it and shown in the stat box
            if (TF1* f = fitCore(h))
            {
                h->GetFunction(f->GetName())->ResetBit(TF1::kNotDraw);
                h->Draw("hist");
                h->GetFunction(f->GetName())->Draw("same");
            }
        }
        h->Draw("axis same");
        save(&c, outDir, name);
    }

    // B mass with and without the fit, normalised to unit area
    void plotMassComparison(TFile* file, const TString& outDir)
    {
        TH1* hFit = dynamic_cast<TH1*>(file->Get("hBsMass"));
        TH1* hNoFit = dynamic_cast<TH1*>(file->Get("hBsMassNoFit"));
        if (!hFit || !hNoFit) return;

        TCanvas c("c_BsMassComparison", "BsMassComparison", 800, 600);
        TH1* a = static_cast<TH1*>(hFit->Clone("hBsMassFitNorm"));
        TH1* b = static_cast<TH1*>(hNoFit->Clone("hBsMassNoFitNorm"));
        a->Scale(1. / a->Integral());
        b->Scale(1. / b->Integral());
        a->SetLineColor(kBlue + 1);
        b->SetLineColor(kGray + 2);
        a->SetLineWidth(2);
        b->SetLineWidth(2);
        a->SetStats(false);
        a->SetTitle("B_{s} mass");
        a->GetXaxis()->SetTitle("m_{#mu#mu#gamma} [GeV]");
        a->GetYaxis()->SetTitle("normalised");
        a->SetMaximum(1.15 * std::max(a->GetMaximum(), b->GetMaximum()));
        a->Draw("hist");
        b->Draw("hist same");

        TLegend leg(0.62, 0.75, 0.88, 0.88);
        leg.SetBorderSize(0);
        leg.AddEntry(a, Form("vertex fit (RMS %.0f MeV)", 1e3 * hFit->GetRMS()), "l");
        leg.AddEntry(b, Form("no fit (RMS %.0f MeV)", 1e3 * hNoFit->GetRMS()), "l");
        leg.Draw();
        save(&c, outDir, "BsMassComparison");
    }

    // energy pull versus generated energy, with the core mean and width in each energy bin
    void plotEnergyPullVsE(TFile* file, const TString& outDir)
    {
        TH2* h2 = dynamic_cast<TH2*>(file->Get("hPhotonEnergyPullVsE"));
        if (!h2) return;

        TCanvas c("c_PhotonEnergyPullVsE", "PhotonEnergyPullVsE", 800, 600);
        c.SetRightMargin(0.14);
        h2->SetStats(false);
        h2->Draw("colz");
        save(&c, outDir, "hPhotonEnergyPullVsE");

        // Gaussian fit in each E_gen bin with at least 30 entries
        TF1 g("gSlice", "gaus", -3., 3.);
        TObjArray slices;
        h2->FitSlicesY(&g, 0, -1, 30, "QNR", &slices);
        TH1* hMean = dynamic_cast<TH1*>(slices.At(1));
        TH1* hSigma = dynamic_cast<TH1*>(slices.At(2));
        if (!hMean || !hSigma) return;

        TCanvas cs("c_PhotonEnergyPullVsE_slices", "PhotonEnergyPullVsE slices", 800, 600);
        cs.SetGridy();
        hSigma->SetStats(false);
        hSigma->SetTitle("photon energy pull in bins of E_{gen}");
        hSigma->GetXaxis()->SetTitle(h2->GetXaxis()->GetTitle());
        hSigma->GetYaxis()->SetTitle("Gaussian fit of the pull");
        hSigma->SetMarkerStyle(20);
        hSigma->SetMarkerColor(kBlue + 1);
        hSigma->SetLineColor(kBlue + 1);
        hMean->SetMarkerStyle(24);
        hMean->SetMarkerColor(kRed + 1);
        hMean->SetLineColor(kRed + 1);
        hSigma->SetMinimum(-1.);
        hSigma->SetMaximum(3.);
        hSigma->Draw("e1");
        hMean->Draw("e1 same");

        TLegend leg(0.65, 0.78, 0.88, 0.88);
        leg.SetBorderSize(0);
        leg.AddEntry(hSigma, "#sigma", "lp");
        leg.AddEntry(hMean, "mean", "lp");
        leg.Draw();
        save(&cs, outDir, "hPhotonEnergyPullVsE_slices");
    }
}

void Plot(const char* fileName = "histosMMGVertexFitExample.root", const char* outDir = "plots")
{
    gROOT->SetBatch(true);
    gStyle->SetOptStat("emr");
    gStyle->SetOptFit(111);
    gStyle->SetStatW(0.22);
    gStyle->SetStatH(0.2);
    gStyle->SetPalette(kBird);

    // TFile::Open would first try xrootd for files on /eos and print an error before falling back
    TFile* file = new TFile(fileName, "READ");
    if (!file || file->IsZombie())
    {
        std::cout << "Plot.C: cannot open " << fileName << std::endl;
        return;
    }
    gSystem->mkdir(outDir, true);
    const TString dir(outDir);

    // B mass
    plot1D(file, dir, "hBsMass", true, false, "m_{#mu#mu#gamma} [GeV]");
    plot1D(file, dir, "hBsMassNoFit", true, false, "m_{#mu#mu#gamma} [GeV]");
    plotMassComparison(file, dir);
    plot1D(file, dir, "hBsMassResidual", true);
    plot1D(file, dir, "hBsMassPull", true);

    // decay vertex
    for (const char* coord : {"X", "Y", "Z"})
    {
        plot1D(file, dir, Form("hVtxResidual%s", coord), true);
        plot1D(file, dir, Form("hVtxPull%s", coord), true);
    }

    // fit quality
    plot1D(file, dir, "hChi2Ndf", false, true);
    plot1D(file, dir, "hVtxProb", false);

    // photon measurement
    plot1D(file, dir, "hClusterPullPhi", true);
    plot1D(file, dir, "hClusterPullTheta", true);
    plot1D(file, dir, "hPhotonEnergyPull", true);
    plotEnergyPullVsE(file, dir);

    file->Close();
}
