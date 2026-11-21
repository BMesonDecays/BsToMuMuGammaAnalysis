#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonMVANoMuons_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonMVANoMuons_h

// Photon MVA ID (Winter22 V1, the one of pho_mva / pho_tightId) recomputed with the contributions of given muons
// removed from its isolation inputs. In B -> mu mu gamma the signal muons are mostly within dR < 0.4 of the photon and
// enter the track and HCAL isolations of the MVA; with them the wp90 efficiency of the signal photons is ~0.6
// (report.md, 2026-11-21).
//
// Removed per muon:
//   trkSumPtSolidConeDR04, trkSumPtHollowConeDR03: inner-track pt, with the PhotonTkIsolation selection
//     (dR < 0.4 / 0.04 <= dR < 0.3, |deta| >= 0.015, |vz - z(photon vertex)| < 0.2 cm, |dxy(beam spot)| < 0.1 cm).
//     Exact: the same tracks are summed by the official isolation.
//   hcalPFClusterIso (EE MVA only): calEnergy().had * sin(theta) if the muon HCAL impact point is within dR < 0.3 of
//     the supercluster; hcalOverEcal: calEnergy().had / E_SC if within dR < 0.15. Approximate (PF clusters /
//     rechits vs the muon's crossed-tower energy).
// Residuals are clipped at 0. Without muons the value equals the official one.

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "CondFormats/GBRForest/interface/GBRForest.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"

#include <memory>
#include <string>
#include <vector>

namespace bmmg {

  // contributions of muons to the isolation inputs of the photon MVA
  struct PhotonMuonTerms {
    float trkSolidConeDR04 = 0.f, trkHollowConeDR03 = 0.f, hcalPFClusterIso = 0.f, hcalOverEcal = 0.f;
  };

  class PhotonMVANoMuons {
  public:
    struct Config {
      edm::FileInPath weightsEB, weightsEE;   // GBRForest .root files with the variable names
      double cutEB = 0.0439603, cutEE = -0.249526;   // wp90 of mvaPhoID-RunIIIWinter22-v1
    };

    using MuonTerms = PhotonMuonTerms;

    explicit PhotonMVANoMuons(const Config& config);

    static MuonTerms muonTerms(const reco::Photon& photon, const std::vector<const reco::Muon*>& muons,
                               const reco::BeamSpot::Point& beamSpot);
    // tanh of the GBRForest response, as the official PhotonMVAEstimator value
    float mva(const reco::Photon& photon, double rho, const MuonTerms& removed = MuonTerms()) const;
    bool passWP90(const reco::Photon& photon, float mva) const;

    // for every candidate and every photon leg: the MVA without the candidate's two muons
    // (BCandidate::legPhotonMvaNoMu / legPhotonTightIdNoMu, in the order of the legs)
    void fillCandidates(std::vector<BCandidate>& candidates, const std::vector<SelectedMuon>& muons,
                        const std::vector<SelectedPhoton>& photons, double rho, const reco::BeamSpot& beamSpot) const;

  private:
    enum Variable {
      kSCRawE, kR9Full5x5, kSigmaIEtaIEtaFull5x5, kSCEtaWidth, kSCPhiWidth, kSigmaIEtaIPhiFull5x5, kS4,
      kConeHoverE, kEcalPFClusterIso, kHcalPFClusterIso, kTrkSumPtHollowConeDR03, kTrkSumPtSolidConeDR04,
      kPFChIso, kPFChWorstIso, kSCEta, kRho, kESEffSigmaRR, kPreShEbySCRawE
    };
    struct Category {
      std::unique_ptr<const GBRForest> forest;
      std::vector<Variable> variables;
    };
    static Category load(const edm::FileInPath& file);
    static float value(Variable v, const reco::Photon& photon, double rho, const MuonTerms& removed);
    static bool isEB(const reco::Photon& photon);

    Category eb_, ee_;
    double cutEB_, cutEE_;
  };

}  // namespace bmmg

#endif
