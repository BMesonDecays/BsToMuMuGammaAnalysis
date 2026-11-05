#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_CandidateObservables_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_CandidateObservables_h

// Pure functions computing candidate observables (no event or ntuple access).

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TLorentzVector.h"

#include <vector>

namespace bmmg {

  // ---- primary vertices ----
  // good PV: valid, not fake, ndof > 4, |z| < 24 cm, rho < 2 cm
  bool isGoodPrimaryVertex(const reco::Vertex& pv);
  // index of the PV closest to the line through sv along p (-1 if there is no PV)
  int lowestDCAVertex(const std::vector<reco::Vertex>& pvs, const GlobalPoint& sv, const GlobalVector& p);

  // ---- flight distance and proper decay length ----
  // ct3D = m (d . p) / |p|^2 and ct2D = m (d_T . p_T) / p_T^2 with d = SV - PV (signed projections);
  // errors from the SV and PV position covariances projected on p (momentum errors neglected).
  // l3d/lxy and their errors from VertexDistance3D/XY; cosAlpha = cos of the angle between d and p.
  FlightInfo flightInfo(const reco::Vertex& pv, const GlobalPoint& sv, const GlobalError& svErr,
                        const GlobalVector& p, double massHypothesis);
  // same, with the ct errors propagated from the full covariance of the fitted state
  // (x, y, z, px, py, pz: SV position, momentum and their correlations) plus the PV covariance
  FlightInfo flightInfo(const reco::Vertex& pv, const GlobalPoint& sv, const GlobalVector& p,
                        const AlgebraicSymMatrix66& svMomentumCov, double massHypothesis);

  // ---- angles ----
  // cos of the angle of the daughter in the parent rest frame w.r.t. the parent direction
  float helicityAngle(const TLorentzVector& daughter, const TLorentzVector& parent);
  // angle between the plane (p1, p2) and the plane (p3, p4)
  float coplanarityAngle(const TLorentzVector& p1, const TLorentzVector& p2,
                         const TLorentzVector& p3, const TLorentzVector& p4);
  // cos of the angle of the positive lepton in the dilepton rest frame w.r.t. the dilepton direction
  float leptonAngle(const TLorentzVector& lPlus, const TLorentzVector& lMinus);

  // ---- isolation and compatibility ----
  // pt(B) / (pt(B) + sum pt of the charged PV tracks with pt > ptMin within dR < drMax of the B,
  // excluding the two muon tracks)
  float candidateIsolation(const reco::Candidate::LorentzVector& candP4, const reco::Muon& mu1, const reco::Muon& mu2,
                           const reco::Vertex& pv, double drMax = 0.7, double ptMin = 0.9);
  // highest vertex probability of the two muon tracks with one other PV track (pt > ptMin)
  float maxTrackCompatibility(const reco::TransientTrack& mu1, const reco::TransientTrack& mu2, const reco::Vertex& pv,
                              const TransientTrackBuilder& ttBuilder, double ptMin = 0.9);

  // transverse distance of closest approach of two tracks (-1 if not computable)
  float trackDCA(const reco::TransientTrack& t1, const reco::TransientTrack& t2);

  TLorentzVector toTLV(const reco::Candidate::LorentzVector& p4);

}  // namespace bmmg

#endif
