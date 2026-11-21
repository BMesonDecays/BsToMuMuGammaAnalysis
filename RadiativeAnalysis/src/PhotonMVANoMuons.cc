#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/PhotonMVANoMuons.h"

#include "CommonTools/MVAUtils/interface/GBRForestTools.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace bmmg {

  PhotonMVANoMuons::PhotonMVANoMuons(const Config& config)
      : eb_(load(config.weightsEB)), ee_(load(config.weightsEE)), cutEB_(config.cutEB), cutEE_(config.cutEE) {}

  PhotonMVANoMuons::Category PhotonMVANoMuons::load(const edm::FileInPath& file) {
    // names as in RecoEgamma/PhotonIdentification/data/PhotonMVAEstimatorRun3VariablesWinter22V1.txt
    static const std::map<std::string, Variable> known{
        {"pho_SCRawE", kSCRawE},
        {"pho_R9Full5x5", kR9Full5x5},
        {"pho_SigmaIEtaIEtaFull5x5", kSigmaIEtaIEtaFull5x5},
        {"pho_SCEtaWidth", kSCEtaWidth},
        {"pho_SCPhiWidth", kSCPhiWidth},
        {"pho_SigmaIEtaIPhiFull5x5", kSigmaIEtaIPhiFull5x5},
        {"pho_S4", kS4},
        {"pho_ConeHoverE", kConeHoverE},
        {"pho_EcalPFClusterIso", kEcalPFClusterIso},
        {"pho_HcalPFClusterIso", kHcalPFClusterIso},
        {"pho_trkSumPtHollowConeDR03", kTrkSumPtHollowConeDR03},
        {"pho_trkSumPtSolidConeDR04", kTrkSumPtSolidConeDR04},
        {"pho_PFChIso", kPFChIso},
        {"pho_PFChWorstIso", kPFChWorstIso},
        {"pho_SCEta", kSCEta},
        {"evt_rho", kRho},
        {"pho_ESEffSigmaRR", kESEffSigmaRR},
        {"pho_PreShEbySCRawE", kPreShEbySCRawE}};
    Category c;
    std::vector<std::string> names;
    c.forest = createGBRForest(file, names);
    if (!c.forest || names.empty())
      throw cms::Exception("PhotonMVANoMuons") << "cannot read the GBRForest and its variable names from "
                                               << file.fullPath();
    for (const auto& n : names) {
      const auto it = known.find(n);
      if (it == known.end())
        throw cms::Exception("PhotonMVANoMuons") << "unknown variable " << n << " in " << file.fullPath();
      c.variables.push_back(it->second);
    }
    return c;
  }

  bool PhotonMVANoMuons::isEB(const reco::Photon& photon) { return std::abs(photon.superCluster()->eta()) < 1.479; }

  float PhotonMVANoMuons::value(Variable v, const reco::Photon& pho, double rho, const MuonTerms& removed) {
    const reco::SuperClusterRef& sc = pho.superCluster();
    const reco::Photon::ShowerShape& ss = pho.full5x5_showerShapeVariables();
    auto minus = [](double x, double y) { return static_cast<float>(std::max(0., x - y)); };
    switch (v) {
      case kSCRawE: return sc->rawEnergy();
      case kR9Full5x5: return pho.full5x5_r9();
      case kSigmaIEtaIEtaFull5x5: return ss.sigmaIetaIeta;
      case kSCEtaWidth: return sc->etaWidth();
      case kSCPhiWidth: return sc->phiWidth();
      case kSigmaIEtaIPhiFull5x5: return ss.sigmaIetaIphi;
      case kS4: return ss.e2x2 / ss.e5x5;   // as the official expression (no protection)
      case kConeHoverE: return minus(pho.hcalOverEcal(), removed.hcalOverEcal);
      case kEcalPFClusterIso: return pho.ecalPFClusterIso();
      case kHcalPFClusterIso: return minus(pho.hcalPFClusterIso(), removed.hcalPFClusterIso);
      case kTrkSumPtHollowConeDR03: return minus(pho.trkSumPtHollowConeDR03(), removed.trkHollowConeDR03);
      case kTrkSumPtSolidConeDR04: return minus(pho.trkSumPtSolidConeDR04(), removed.trkSolidConeDR04);
      case kPFChIso: return pho.chargedHadronIso();
      case kPFChWorstIso: return pho.chargedHadronWorstVtxIso();
      case kSCEta: return sc->eta();
      case kRho: return rho;
      case kESEffSigmaRR: return ss.effSigmaRR;
      case kPreShEbySCRawE: return sc->preshowerEnergy() / sc->rawEnergy();
    }
    return 0.f;
  }

  PhotonMVANoMuons::MuonTerms PhotonMVANoMuons::muonTerms(const reco::Photon& pho,
                                                          const std::vector<const reco::Muon*>& muons,
                                                          const reco::BeamSpot::Point& beamSpot) {
    MuonTerms t;
    const reco::SuperClusterRef& sc = pho.superCluster();
    const double phoVz = pho.vertex().z();
    for (const reco::Muon* mu : muons) {
      if (!mu) continue;
      // track isolations (PhotonTkIsolation, isolationCalculator_cfi cones A = 0.4 and B = 0.3)
      const reco::TrackRef trk = mu->innerTrack();
      if (trk.isNonnull() && trk.isAvailable() && std::abs(trk->vz() - phoVz) < 0.2 &&
          std::abs(trk->dxy(beamSpot)) < 0.1 && std::abs(trk->eta() - pho.eta()) >= 0.015) {
        const double dr2 = reco::deltaR2(*trk, pho);
        if (dr2 < 0.4 * 0.4) t.trkSolidConeDR04 += trk->pt();
        if (dr2 >= 0.04 * 0.04 && dr2 < 0.3 * 0.3) t.trkHollowConeDR03 += trk->pt();
      }
      // HCAL deposit of the muon at its HCAL impact point
      if (mu->isEnergyValid()) {
        const reco::MuonEnergy& e = mu->calEnergy();
        if (e.had > 0 && e.hcal_position.rho() > 0) {
          const double dr2 = reco::deltaR2(e.hcal_position.eta(), e.hcal_position.phi(), sc->eta(), sc->phi());
          if (dr2 < 0.3 * 0.3) t.hcalPFClusterIso += e.had / std::cosh(e.hcal_position.eta());
          if (dr2 < 0.15 * 0.15 && sc->energy() > 0) t.hcalOverEcal += e.had / sc->energy();
        }
      }
    }
    return t;
  }

  float PhotonMVANoMuons::mva(const reco::Photon& photon, double rho, const MuonTerms& removed) const {
    const Category& c = isEB(photon) ? eb_ : ee_;
    std::vector<float> x;
    x.reserve(c.variables.size());
    for (Variable v : c.variables) x.push_back(value(v, photon, rho, removed));
    return std::tanh(c.forest->GetResponse(x.data()));
  }

  bool PhotonMVANoMuons::passWP90(const reco::Photon& photon, float mva) const {
    return mva > (isEB(photon) ? cutEB_ : cutEE_);
  }

  void PhotonMVANoMuons::fillCandidates(std::vector<BCandidate>& candidates, const std::vector<SelectedMuon>& muons,
                                        const std::vector<SelectedPhoton>& photons, double rho,
                                        const reco::BeamSpot& beamSpot) const {
    for (BCandidate& c : candidates) {
      const std::vector<const reco::Muon*> mus{muons.at(c.mu1).muon, muons.at(c.mu2).muon};
      c.legPhotonMvaNoMu.clear();
      c.legPhotonTightIdNoMu.clear();
      for (int leg : c.legs) {
        const reco::Photon& pho = *photons.at(leg).photon;
        const float v = mva(pho, rho, muonTerms(pho, mus, beamSpot.position()));
        c.legPhotonMvaNoMu.push_back(v);
        c.legPhotonTightIdNoMu.push_back(passWP90(pho, v) ? 1 : 0);
      }
    }
  }

}  // namespace bmmg
