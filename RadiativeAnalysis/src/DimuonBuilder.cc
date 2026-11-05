#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DimuonBuilder.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"

#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "TMath.h"

#include <cmath>

namespace bmmg {

  int dimuonResonance(double m) {
    if (m > window::jpsiLow && m < window::jpsiHigh) return kJpsi;
    if (m > window::psi2SLow && m < window::psi2SHigh) return kPsi2S;
    if (m > window::phiLow && m < window::phiHigh) return kPhi;
    return kNonResonant;
  }

  std::vector<DimuonCandidate> buildDimuons(const std::vector<SelectedMuon>& muons, const reco::BeamSpot& bs) {
    std::vector<DimuonCandidate> dimuons;
    KalmanVertexFitter fitter(true);
    for (size_t i = 0; i < muons.size(); ++i) {
      for (size_t j = i + 1; j < muons.size(); ++j) {
        const reco::Muon& m1 = *muons[i].muon;
        const reco::Muon& m2 = *muons[j].muon;
        if (m1.charge() * m2.charge() >= 0) continue;
        if (muon::overlap(m1, m2, 1, 1, true)) continue;   // share muon segments

        DimuonCandidate d;
        const bool firstPositive = m1.charge() > 0;
        d.mu1 = firstPositive ? i : j;   // mu1: positive muon
        d.mu2 = firstPositive ? j : i;
        const reco::Muon& muPlus = *muons[d.mu1].muon;
        const reco::Muon& muMinus = *muons[d.mu2].muon;

        d.p4 = muPlus.p4() + muMinus.p4();
        d.deltaR = reco::deltaR(muPlus, muMinus);
        d.isCowboy = (reco::deltaPhi(muPlus.phi(), muMinus.phi()) > 0) ? 1 : 0;
        d.resonance = dimuonResonance(d.p4.mass());
        d.dca = trackDCA(muons[d.mu1].track, muons[d.mu2].track);

        const TransientVertex v = fitter.vertex(std::vector<reco::TransientTrack>{muons[d.mu1].track, muons[d.mu2].track});
        if (v.isValid()) {
          d.vtxValid = true;
          d.vtxChi2 = v.totalChiSquared();
          d.vtxNdof = v.degreesOfFreedom();
          d.vtxProb = TMath::Prob(d.vtxChi2, static_cast<int>(std::lround(d.vtxNdof)));
          d.vtx = v.position();
          d.vtxErr = v.positionError();

          // transverse displacement from the beam spot at the z of the vertex (beam slope included)
          const double bx = bs.x(d.vtx.z()), by = bs.y(d.vtx.z());
          const GlobalVector disp(d.vtx.x() - bx, d.vtx.y() - by, 0.);
          d.lxyBS = disp.perp();
          if (d.lxyBS > 0) {
            const GlobalError err = d.vtxErr + GlobalError(bs.rotatedCovariance3D());
            d.lxyBSErr = std::sqrt(err.rerr(GlobalPoint(disp.x(), disp.y(), 0.)));
            d.cosAlphaBS = (disp.x() * d.p4.px() + disp.y() * d.p4.py()) / (d.lxyBS * d.p4.pt());
          }
        }
        dimuons.push_back(d);
      }
    }
    return dimuons;
  }

}  // namespace bmmg
