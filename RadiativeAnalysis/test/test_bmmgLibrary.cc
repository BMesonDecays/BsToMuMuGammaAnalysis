// Unit tests of the bmmg library (pure functions and BranchSet). Run with: scram b runtests
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BranchSet.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DimuonBuilder.h"

#include "TMath.h"
#include "TTree.h"

#include <cmath>

namespace {
  reco::Vertex makeVertex(double x, double y, double z, double err = 1e-4) {
    reco::Vertex::Error e;
    e(0, 0) = e(1, 1) = e(2, 2) = err * err;
    return reco::Vertex(reco::Vertex::Point(x, y, z), e, 1., 10., 10);
  }
}  // namespace

TEST_CASE("flight information and ct", "[observables]") {
  const reco::Vertex pv = makeVertex(0, 0, 0);
  const GlobalPoint sv(0., 0.3, 0.4);   // |d| = 0.5
  const GlobalError svErr(1e-6, 0., 1e-6, 0., 0., 1e-6);

  SECTION("momentum along the flight direction") {
    const GlobalVector p(0., 6., 8.);   // |p| = 10, parallel to d
    const bmmg::FlightInfo f = bmmg::flightInfo(pv, sv, svErr, p, 5.);
    CHECK(f.l3d == Approx(0.5));
    CHECK(f.lxy == Approx(0.3));
    CHECK(f.cosAlpha3D == Approx(1.));
    CHECK(f.cosAlpha2D == Approx(1.));
    CHECK(f.ct3D == Approx(5. * 0.5 / 10.));       // m L / p
    CHECK(f.ct2D == Approx(5. * 0.3 / 6.));        // m Lxy / pT
    CHECK(f.ct3DErr > 0.);
  }
  SECTION("momentum opposite to the flight direction gives negative ct") {
    const bmmg::FlightInfo f = bmmg::flightInfo(pv, sv, svErr, GlobalVector(0., -6., -8.), 5.);
    CHECK(f.ct3D == Approx(-0.25));
    CHECK(f.cosAlpha3D == Approx(-1.));
  }
  SECTION("full-covariance version: same central values, error >= position-only error") {
    const GlobalVector p(0., 6., 8.);
    AlgebraicSymMatrix66 cov;
    for (int i = 0; i < 3; ++i) cov(i, i) = 1e-6;
    for (int i = 3; i < 6; ++i) cov(i, i) = 0.01;
    const bmmg::FlightInfo a = bmmg::flightInfo(pv, sv, svErr, p, 5.);
    const bmmg::FlightInfo b = bmmg::flightInfo(pv, sv, p, cov, 5.);
    CHECK(b.ct3D == Approx(a.ct3D));
    CHECK(b.ct3DErr >= a.ct3DErr * 0.999);
  }
}

TEST_CASE("primary vertex closest to the B line", "[observables]") {
  const std::vector<reco::Vertex> pvs{makeVertex(0, 0, 5), makeVertex(0, 0, 0.1), makeVertex(0, 0, -3)};
  // SV at (0, 0.1, 1.1) with momentum along (0, 1, 10): the line passes through (0, 0, 0.1)
  CHECK(bmmg::lowestDCAVertex(pvs, GlobalPoint(0., 0.1, 1.1), GlobalVector(0., 1., 10.)) == 1);
  CHECK(bmmg::lowestDCAVertex({}, GlobalPoint(0., 0., 0.), GlobalVector(0., 0., 1.)) == -1);
}

TEST_CASE("angles", "[observables]") {
  // daughter emitted along the parent direction in the parent rest frame
  TLorentzVector parent, daughter;
  daughter.SetXYZM(0., 0., 1., 0.);
  parent.SetXYZM(0., 0., 0., 5.);
  const TVector3 boost(0., 0., 0.5);
  daughter.Boost(boost);
  parent.Boost(boost);
  CHECK(bmmg::helicityAngle(daughter, parent) == Approx(1.f));

  TLorentzVector a(1, 0, 0, 1), b(0, 1, 0, 1), c(0, 0, 1, 1), d(1, 0, 0, 1);
  CHECK(bmmg::coplanarityAngle(a, b, c, d) == Approx(TMath::Pi() / 2));   // xy plane vs xz plane

  TLorentzVector lp, lm;
  lp.SetXYZM(0., 0., 2., 0.105);
  lm.SetXYZM(0., 0., -2., 0.105);   // back to back at rest: direction undefined
  CHECK(bmmg::leptonAngle(lp, lm) == Approx(-2.f));
}

TEST_CASE("dimuon resonance flags", "[dimuon]") {
  CHECK(bmmg::dimuonResonance(3.097) == bmmg::kJpsi);
  CHECK(bmmg::dimuonResonance(3.686) == bmmg::kPsi2S);
  CHECK(bmmg::dimuonResonance(1.019) == bmmg::kPhi);
  CHECK(bmmg::dimuonResonance(4.5) == bmmg::kNonResonant);
}

TEST_CASE("BranchSet", "[ntuple]") {
  TTree tree("t", "t");
  tree.SetDirectory(nullptr);
  bmmg::BranchSet b(&tree);
  b.bookScalar<int>("n", -1);
  b.bookVector<float>("x");
  CHECK_THROWS(b.bookVector<float>("x"));      // booked twice
  CHECK_THROWS(b.vec<float>("y"));             // not booked
  CHECK_THROWS(b.scalar<float>("n"));          // booked with another type

  b.scalar<int>("n") = 2;
  b.vec<float>("x") = {1.f, 2.f};
  tree.Fill();
  b.reset();
  CHECK(b.scalar<int>("n") == -1);
  CHECK(b.vec<float>("x").empty());
  tree.Fill();

  CHECK(tree.GetEntries() == 2);
  CHECK(tree.Draw("x", "", "goff") == 2);      // two values, both in the first entry
}
