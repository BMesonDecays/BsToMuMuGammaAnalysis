
import argparse
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

import numpy as np, pandas as pd, matplotlib.pyplot as plt
import ROOT
from bmmg_ana import config, io

def fourvec(pt, eta, phi, m):
    """Return a ROOT.TLorentzVector from pt, eta, phi, m."""
    v = ROOT.TLorentzVector()
    v.SetPtEtaPhiM(pt, eta, phi, m)
    return v

def main():
    ROOT.gROOT.SetBatch(True)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('config')
    args = ap.parse_args()
    cfg = config.load_config(args.config)

    out = cfg['output']
    os.makedirs(out, exist_ok=True)

    columns = ['genMatched',
            'gen_pho1_pt', 'gen_pho1_eta', 'gen_pho1_phi'
            'pho_pt', 'pho_eta', 'pho_phi', 'pho_energy', 'pho_energyErr',
            'pho_isEB', 'pho_isEE',
            'pho_hasConversionTracks', 'pho_hasClusterHits',
            'pho_scEnergy', 'pho_scRawEnergy',
            'pho_tightId']
    
    sig = io.load(cfg['parquet'], cfg['samples']['signal']['datasets'], cfg['channel'],
                None, query=cfg['samples']['signal']['query'])
    
    for col in sig.columns:
        print(col, sig[col].dtype)

    tightId_energy_profile = ROOT.TProfile('tightId_energy_profile', 'Photon tight ID profile; E [GeV]; <tight ID>', 10, 0, 50, -0.5, 1.5)
    tightId_pt_profile = ROOT.TProfile('tightId_pt_profile', 'Photon tight ID profile; p_{T} [GeV]; <tight ID>', 10, 0, 30, -0.5, 1.5)

    h_pt = ROOT.TH1F('h_pt', 'Photon p_{T}; p_{T} [GeV]; candidates', 100, 0, 30)
    h_eta = ROOT.TH1F('h_eta', 'Photon #eta; #eta; candidates', 100, -3.0, 3.0)
    h_energy = ROOT.TH1F('h_energy', 'Photon energy; E [GeV]; candidates', 100, 0, 50)

    h_pt_reco = ROOT.TH1F('h_pt_reco', 'Photon p_{T} (reco); p_{T} [GeV]; candidates', 100, 0, 30)
    h_eta_reco = ROOT.TH1F('h_eta_reco', 'Photon #eta (reco); #eta; candidates', 100, -3.0, 3.0)
    h_energy_reco = ROOT.TH1F('h_energy_reco', 'Photon energy (reco); E [GeV]; candidates', 100, 0, 50)

    h_energy_res_EE = ROOT.TH1F('h_energy_res_EE', 'Photon energy resolution (EE); (E_{reco}-E_{gen})/E_{gen}; candidates', 100, -0.5, 0.5)
    h_energy_res_EB = ROOT.TH1F('h_energy_res_EB', 'Photon energy resolution (EB); (E_{reco}-E_{gen})/E_{gen}; candidates', 100, -0.5, 0.5)
    energy_profile_EE = ROOT.TProfile('energy_profile_EE', 'Photon energy resolution profile (EE); E_{gen} [GeV]; <(E_{reco}-E_{gen})/E_{gen}>', 10, 0, 50, -0.5, 1.0)
    energy_profile_EB = ROOT.TProfile('energy_profile_EB', 'Photon energy resolution profile (EB); E_{gen} [GeV]; <(E_{reco}-E_{gen})/E_{gen}>', 10, 12, 50, -0.5, 0.5)
    eta_profile = ROOT.TProfile('eta_profile', 'Photon energy resolution profile; #eta; <(E_{reco}-E_{gen})/E_{gen}>', 10, -2.5, 2.5, -0.5, 1.0)


    for index, evt in sig.iterrows():


        gen_pho = fourvec(evt['gen_pho1_pt'], evt['gen_pho1_eta'], evt['gen_pho1_phi'], 0)
        pho_energy_diff = evt['pho_energy'] - gen_pho.E()
        pho_energy_res = pho_energy_diff / gen_pho.E()

        tightId_energy_profile.Fill(gen_pho.E(), evt['pho_tightId'])
        tightId_pt_profile.Fill(evt['gen_pho1_pt'], evt['pho_tightId'])

        h_pt.Fill(evt['gen_pho1_pt'])
        h_eta.Fill(evt['gen_pho1_eta'])
        h_energy.Fill(gen_pho.E())
        h_pt_reco.Fill(evt['pho_pt'])
        h_eta_reco.Fill(evt['pho_eta'])
        h_energy_reco.Fill(evt['pho_energy'])

        eta_profile.Fill(evt['gen_pho1_eta'], pho_energy_res)

        if evt['pho_isEB']:
            h_energy_res_EB.Fill(pho_energy_res)
            energy_profile_EB.Fill(gen_pho.E(), pho_energy_res)
        elif evt['pho_isEE']:
            h_energy_res_EE.Fill(pho_energy_res)
            energy_profile_EE.Fill(gen_pho.E(), pho_energy_res)


    # Draw histograms
    c = ROOT.TCanvas()
    # Draw tight ID histograms
    tightId_energy_profile.Draw()
    c.SaveAs(os.path.join(out, 'tightId_energy_profile.png'))
    c.Clear()

    tightId_pt_profile.Draw()
    c.SaveAs(os.path.join(out, 'tightId_pt_profile.png'))
    c.Clear()

    # gen kinematics
    h_pt.Draw()
    c.SaveAs(os.path.join(out, 'photon_pt.png'))
    c.Clear()

    h_eta.Draw()
    c.SaveAs(os.path.join(out, 'photon_eta.png'))
    c.Clear()

    h_energy.Draw()
    c.SaveAs(os.path.join(out, 'photon_energy.png'))
    c.Clear()

    # reco kinematics
    h_pt_reco.Draw()
    c.SaveAs(os.path.join(out, 'photon_pt_reco.png'))
    c.Clear()

    h_eta_reco.Draw()
    c.SaveAs(os.path.join(out, 'photon_eta_reco.png'))
    c.Clear()

    h_energy_reco.Draw()
    c.SaveAs(os.path.join(out, 'photon_energy_reco.png'))
    c.Clear()

    # energy resolution histograms
    h_energy_res_EB.Draw()
    c.SaveAs(os.path.join(out, 'photon_energy_resolution_EB.png'))
    c.Clear()
    
    h_energy_res_EE.Draw()
    c.SaveAs(os.path.join(out, 'photon_energy_resolution_EE.png'))
    c.Clear()

    # energy resolution profiles
    energy_profile_EB.Draw()
    c.SaveAs(os.path.join(out, 'energy_resolution_profile_EB.png'))
    c.Clear()

    energy_profile_EE.Draw()
    c.SaveAs(os.path.join(out, 'energy_resolution_profile_EE.png'))
    c.Clear()

    eta_profile.Draw()
    c.SaveAs(os.path.join(out, 'eta_resolution_profile.png'))
    c.Clear()



if __name__ == '__main__':
    main()