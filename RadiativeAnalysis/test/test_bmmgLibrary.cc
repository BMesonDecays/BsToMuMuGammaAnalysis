// Unit tests of the bmmg library (pure functions and BranchSet). Run with: scram b runtests
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BranchSet.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ConversionScaleMap.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DimuonBuilder.h"

#include "TMath.h"
#include "TTree.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>

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

TEST_CASE("PV track sums and muon association", "[observables]") {
  auto track = [](double px, double py, double pz, int q) {
    return reco::Track(1., 5., reco::Track::Point(0, 0, 0), reco::Track::Vector(px, py, pz), q,
                       reco::Track::CovarianceMatrix());
  };
  const reco::Track mu1 = track(3, 0, 1, 1), mu2 = track(0, 4, 1, -1), kaon = track(-1, -1, 0.5, 1);
  const std::vector<bmmg::PVTrack> pvTracks{
      {mu1, 0}, {track(1, 0, 0, 1), 0}, {track(-2, 0, 0, -1), 0}, {kaon, 0},   // PV 0: mu1, two tracks, kaon
      {mu2, 1}, {track(0, 5, 0, 1), 1}};                                      // PV 1: mu2, one track
  const bmmg::PVTrackSums s0 = bmmg::pvTrackSums(pvTracks, 0, mu1, mu2, {&kaon});
  CHECK(s0.mu1InPV == 1);
  CHECK(s0.mu2InPV == 0);
  CHECK(s0.nTracks == 2);                    // muon and kaon excluded
  CHECK(s0.sumPt == Approx(3.));
  CHECK(s0.vecSumPt == Approx(1.));
  const bmmg::PVTrackSums s0k = bmmg::pvTrackSums(pvTracks, 0, mu1, mu2);
  CHECK(s0k.nTracks == 3);
  CHECK(s0k.vecSumPt == Approx(std::hypot(-2., -1.)));
  const bmmg::PVTrackSums s1 = bmmg::pvTrackSums(pvTracks, 1, mu1, mu2);
  CHECK((s1.mu1InPV == 0 && s1.mu2InPV == 1 && s1.nTracks == 1));
  CHECK(s1.sumPt == Approx(5.));
  const bmmg::PVTrackSums none = bmmg::pvTrackSums(pvTracks, 7, mu1, mu2);
  CHECK((none.nTracks == 0 && none.sumPt == 0.f && none.mu1InPV == 0));
}

TEST_CASE("daughter isolation without the B products", "[observables]") {
  auto track = [](double pt, double eta, double phi, int q) {
    return reco::Track(1., 5., reco::Track::Point(0, 0, 0),
                       reco::Track::Vector(pt * std::cos(phi), pt * std::sin(phi), pt * std::sinh(eta)), q,
                       reco::Track::CovarianceMatrix());
  };
  // daughter direction (eta, phi) = (0, 0)
  const reco::Track otherMuon = track(4., 0.1, 0.1, -1);     // in the cone, a B product
  const std::vector<bmmg::PVTrack> pvTracks{
      {otherMuon, 0},
      {track(1.0, 0.2, 0.0, 1), 0},    // counted
      {track(0.4, 0.1, 0.0, 1), 0},    // below minChargedPt
      {track(2.0, 0.5, 0.0, 1), 0},    // outside the cone
      {track(3.0, 0.1, 0.0, 1), 1}};   // other PV
  const std::vector<bmmg::NeutralParticle> neutrals{
      {2.0f, 0.05f, 0.0f, 10},         // footprint of a candidate photon (vetoed)
      {1.5f, 0.0f, 0.2f, 11},          // counted
      {0.7f, 0.005f, 0.0f, 12},        // inside the inner veto cone (0.01) only
      {5.0f, 1.0f, 0.0f, 13}};         // outside the cone
  const bmmg::IsoSums withVeto = bmmg::daughterIsolation(0., 0., pvTracks, 0, {&otherMuon}, neutrals, {10}, 0.01);
  CHECK(withVeto.charged == Approx(1.0));
  CHECK(withVeto.neutral == Approx(1.5));
  const bmmg::IsoSums noVeto = bmmg::daughterIsolation(0., 0., pvTracks, 0, {}, neutrals, {}, 0.);
  CHECK(noVeto.charged == Approx(5.0));
  CHECK(noVeto.neutral == Approx(2.0 + 1.5 + 0.7));
  const bmmg::IsoSums pv1 = bmmg::daughterIsolation(0., 0., pvTracks, 1, {}, {}, {}, 0.);
  CHECK(pv1.charged == Approx(3.0));
  CHECK(pv1.neutral == 0.f);
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

namespace {
  std::string writeFile(const std::string& name, const std::string& text) {
    std::ofstream(name) << text;
    return name;
  }
}  // namespace

TEST_CASE("converted-photon energy-scale map", "[convscale]") {
  // pt_est edges [1, 2, 5] x |eta| edges [0, 1.5, 2.5]; scale[pt][eta]
  const std::string vars =
      R"J([{"name": "pt", "expr": "pt_est", "edges": [1, 2, 5]}, {"name": "abseta", "expr": "abs(conv_eta)", "edges": [0, 1.5, 2.5]}])J";
  const std::string path = writeFile("test_conv_scale_map.json",
                                     R"J({"variables": )J" + vars + R"J(, "scale": [[0.99, 0.95], [0.98, 0.90]]})J");
  const bmmg::ConversionScaleMap map(path);
  CHECK(map.scaleAt(1.5, 0.3) == Approx(0.99));
  CHECK(map.scaleAt(3., -2.0) == Approx(0.90));   // |eta|
  CHECK(map.scaleAt(0.2, 0.3) == Approx(0.99));   // clamped below and above
  CHECK(map.scaleAt(50., 3.0) == Approx(0.90));
  // reconstructed pt 1.97 in the endcap: s(1.97) = 0.95, true pt ~ 1.97 / 0.95 = 2.07 -> the 2-5 GeV bin
  CHECK(map.scale(1.97, 2.0) == Approx(0.90));
  CHECK(map.scale(1.85, 0.5) == Approx(0.99));   // 1.85 / 0.99 = 1.87 stays in the first bin

  // variables in the other order: the scale array is [eta][pt]
  const std::string swapped = writeFile(
      "test_conv_scale_map_swapped.json",
      R"J({"variables": [{"name": "abseta", "expr": "abs(conv_eta)", "edges": [0, 1.5, 2.5]}, {"name": "pt", "expr": "pt_est", "edges": [1, 2, 5]}], "scale": [[0.99, 0.98], [0.95, 0.90]]})J");
  CHECK(bmmg::ConversionScaleMap(swapped).scaleAt(3., 2.0) == Approx(0.90));
  CHECK(bmmg::ConversionScaleMap(swapped).scaleAt(1.5, 2.0) == Approx(0.95));

  // a map in other variables (e.g. the MC truth map in gen pt) or an implausible scale is refused
  const std::string wrong = writeFile(
      "test_conv_scale_map_wrong.json",
      R"J({"variables": [{"name": "pt", "expr": "gen_pho1_pt", "edges": [1, 2]}, {"name": "abseta", "expr": "abs(conv_eta)", "edges": [0, 2.5]}], "scale": [[0.99]]})J");
  CHECK_THROWS(bmmg::ConversionScaleMap(wrong));
  const std::string bad =
      writeFile("test_conv_scale_map_bad.json", R"J({"variables": )J" + vars + R"J(, "scale": [[0.99, 0.95], [0.98, 0.0]]})J");
  CHECK_THROWS(bmmg::ConversionScaleMap(bad));
  CHECK_THROWS(bmmg::ConversionScaleMap("does_not_exist.json"));
  for (const auto& f : {path, swapped, wrong, bad}) std::remove(f.c_str());
}
