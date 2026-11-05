#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateClosestToPoint.h"
#include "TMath.h"

#include <cmath>
#include <limits>

namespace bmmg {

  bool isGoodPrimaryVertex(const reco::Vertex& pv) {
    return pv.isValid() && !pv.isFake() && pv.ndof() > 4 && std::abs(pv.z()) < 24. && pv.position().Rho() < 2.;
  }

  int lowestDCAVertex(const std::vector<reco::Vertex>& pvs, const GlobalPoint& sv, const GlobalVector& p) {
    const double p2 = p.mag2();
    if (p2 <= 0) return -1;
    int best = -1;
    double bestDist = std::numeric_limits<double>::max();
    for (size_t i = 0; i < pvs.size(); ++i) {
      const GlobalVector d(pvs[i].x() - sv.x(), pvs[i].y() - sv.y(), pvs[i].z() - sv.z());
      const GlobalVector perp = d - (d.dot(p) / p2) * p;   // PV distance to the line sv + s p
      if (perp.mag() < bestDist) {
        bestDist = perp.mag();
        best = static_cast<int>(i);
      }
    }
    return best;
  }

  FlightInfo flightInfo(const reco::Vertex& pv, const GlobalPoint& sv, const GlobalError& svErr,
                        const GlobalVector& p, double mass) {
    FlightInfo f;
    const GlobalPoint pvPos(pv.x(), pv.y(), pv.z());
    const GlobalError pvErr(pv.error());
    const VertexState svState(sv, svErr), pvState(pvPos, pvErr);

    const Measurement1D d3 = VertexDistance3D().distance(svState, pvState);
    const Measurement1D d2 = VertexDistanceXY().distance(svState, pvState);
    f.l3d = d3.value();
    f.l3dErr = d3.error();
    f.lxy = d2.value();
    f.lxyErr = d2.error();

    const GlobalVector d = sv - pvPos;
    const AlgebraicSymMatrix33 cov = svErr.matrix() + pvErr.matrix();
    if (p.mag() > 0 && d.mag() > 0) f.cosAlpha3D = d.dot(p) / (d.mag() * p.mag());
    if (p.perp() > 0 && d.perp() > 0)
      f.cosAlpha2D = (d.x() * p.x() + d.y() * p.y()) / (d.perp() * p.perp());

    const double p2 = p.mag2();
    if (p2 > 0) {
      f.ct3D = mass * d.dot(p) / p2;
      const AlgebraicVector3 pv3(p.x(), p.y(), p.z());
      f.ct3DErr = mass / p2 * std::sqrt(ROOT::Math::Similarity(pv3, cov));
    }
    const double pt2 = p.perp2();
    if (pt2 > 0) {
      f.ct2D = mass * (d.x() * p.x() + d.y() * p.y()) / pt2;
      const AlgebraicVector3 pt3(p.x(), p.y(), 0.);
      f.ct2DErr = mass / pt2 * std::sqrt(ROOT::Math::Similarity(pt3, cov));
    }
    return f;
  }

  FlightInfo flightInfo(const reco::Vertex& pv, const GlobalPoint& sv, const GlobalVector& p,
                        const AlgebraicSymMatrix66& cov, double mass) {
    const GlobalError svErr(cov.Sub<AlgebraicSymMatrix33>(0, 0));
    FlightInfo f = flightInfo(pv, sv, svErr, p, mass);   // distances, angles, central values

    // ct errors: Jacobian w.r.t. (SV x,y,z, px,py,pz) of the fitted state and w.r.t. the PV position
    const GlobalVector d(sv.x() - pv.x(), sv.y() - pv.y(), sv.z() - pv.z());
    const AlgebraicSymMatrix33 pvCov = GlobalError(pv.error()).matrix();
    const double p2 = p.mag2();
    if (p2 > 0) {
      const double dp = d.dot(p);
      AlgebraicVector6 j;   // d ct3D / d(x, y, z, px, py, pz)
      const double pc[3] = {p.x(), p.y(), p.z()}, dc[3] = {d.x(), d.y(), d.z()};
      for (int k = 0; k < 3; ++k) {
        j[k] = mass * pc[k] / p2;
        j[k + 3] = mass * (dc[k] / p2 - 2. * dp * pc[k] / (p2 * p2));
      }
      const AlgebraicVector3 jpv(-j[0], -j[1], -j[2]);
      f.ct3DErr = std::sqrt(ROOT::Math::Similarity(j, cov) + ROOT::Math::Similarity(jpv, pvCov));
    }
    const double pt2 = p.perp2();
    if (pt2 > 0) {
      const double dpt = d.x() * p.x() + d.y() * p.y();
      AlgebraicVector6 j;   // d ct2D / d(x, y, z, px, py, pz)
      const double pc[2] = {p.x(), p.y()}, dc[2] = {d.x(), d.y()};
      for (int k = 0; k < 2; ++k) {
        j[k] = mass * pc[k] / pt2;
        j[k + 3] = mass * (dc[k] / pt2 - 2. * dpt * pc[k] / (pt2 * pt2));
      }
      const AlgebraicVector3 jpv(-j[0], -j[1], 0.);
      f.ct2DErr = std::sqrt(ROOT::Math::Similarity(j, cov) + ROOT::Math::Similarity(jpv, pvCov));
    }
    return f;
  }

  float helicityAngle(const TLorentzVector& daughter, const TLorentzVector& parent) {
    TLorentzVector d = daughter;
    d.Boost(-parent.BoostVector());
    if (d.Vect().Mag() == 0 || parent.Vect().Mag() == 0) return -2.f;
    return d.Vect().Unit().Dot(parent.Vect().Unit());
  }

  float coplanarityAngle(const TLorentzVector& p1, const TLorentzVector& p2,
                         const TLorentzVector& p3, const TLorentzVector& p4) {
    const TVector3 n1 = p1.Vect().Cross(p2.Vect());
    const TVector3 n2 = p3.Vect().Cross(p4.Vect());
    if (n1.Mag() == 0 || n2.Mag() == 0) return -1.f;
    return std::acos(std::max(-1., std::min(1., n1.Dot(n2) / (n1.Mag() * n2.Mag()))));
  }

  float leptonAngle(const TLorentzVector& lPlus, const TLorentzVector& lMinus) {
    const TLorentzVector ll = lPlus + lMinus;
    TLorentzVector l = lPlus;
    l.Boost(-ll.BoostVector());
    if (l.Vect().Mag() == 0 || ll.Vect().Mag() == 0) return -2.f;
    return l.Vect().Unit().Dot(ll.Vect().Unit());
  }

  namespace {
    bool isExcluded(const reco::Track& trk, const std::vector<const reco::Track*>& exclude) {
      for (const reco::Track* e : exclude)
        if (reco::deltaR(*e, trk) < 0.01 && std::abs(e->pt() - trk.pt()) < 0.01 * trk.pt()) return true;
      return false;
    }
  }  // namespace

  float candidateIsolation(const reco::Candidate::LorentzVector& candP4, const reco::Muon& mu1, const reco::Muon& mu2,
                           const std::vector<PVTrack>& pvTracks, int pv, const std::vector<const reco::Track*>& exclude,
                           double drMax, double ptMin) {
    double sumPt = 0.;
    for (const PVTrack& t : pvTracks) {
      if (t.pv != pv) continue;
      const reco::Track& trk = t.track;
      if (trk.charge() == 0 || trk.pt() < ptMin) continue;
      if (reco::deltaR(mu1, trk) < 0.01 || reco::deltaR(mu2, trk) < 0.01) continue;   // muon tracks
      if (isExcluded(trk, exclude)) continue;
      if (reco::deltaR(candP4, trk) > drMax) continue;
      sumPt += trk.pt();
    }
    return candP4.pt() / (candP4.pt() + sumPt);
  }

  float maxTrackCompatibility(const reco::TransientTrack& mu1, const reco::TransientTrack& mu2,
                              const std::vector<PVTrack>& pvTracks, int pv, const TransientTrackBuilder& ttBuilder,
                              const std::vector<const reco::Track*>& exclude, double ptMin) {
    float maxProb = 0.f;
    KalmanVertexFitter fitter(true);
    for (const PVTrack& t : pvTracks) {
      if (t.pv != pv) continue;
      const reco::Track& trk = t.track;
      if (trk.charge() == 0 || trk.pt() < ptMin) continue;
      if (reco::deltaR(mu1.track(), trk) < 1e-4 || reco::deltaR(mu2.track(), trk) < 1e-4) continue;
      if (isExcluded(trk, exclude)) continue;
      const std::vector<reco::TransientTrack> tracks{mu1, mu2, ttBuilder.build(trk)};
      const TransientVertex v = fitter.vertex(tracks);
      if (!v.isValid()) continue;
      const float prob = TMath::Prob(v.totalChiSquared(), static_cast<int>(std::lround(v.degreesOfFreedom())));
      maxProb = std::max(maxProb, prob);
    }
    return maxProb;
  }

  float trackDCA(const reco::TransientTrack& t1, const reco::TransientTrack& t2) {
    const TrajectoryStateClosestToPoint s1 = t1.impactPointTSCP();
    const TrajectoryStateClosestToPoint s2 = t2.impactPointTSCP();
    if (!s1.isValid() || !s2.isValid()) return -1.f;
    ClosestApproachInRPhi ca;
    ca.calculate(s1.theState(), s2.theState());
    return ca.status() ? ca.distance() : -1.f;
  }

  TLorentzVector toTLV(const reco::Candidate::LorentzVector& p4) {
    return TLorentzVector(p4.px(), p4.py(), p4.pz(), p4.energy());
  }

}  // namespace bmmg
