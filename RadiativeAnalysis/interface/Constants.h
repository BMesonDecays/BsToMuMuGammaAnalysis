#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_Constants_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_Constants_h

// Single source of physics constants for the new code (namespace bmmg).
// Masses in GeV, PDG 2024.
namespace bmmg {
  namespace pdg {
    constexpr double muonMass = 0.1056583755;
    constexpr double electronMass = 0.00051099895;
    constexpr double pionMass = 0.13957039;
    constexpr double kaonMass = 0.493677;
    constexpr double pi0Mass = 0.1349768;
    constexpr double etaMass = 0.547862;
    constexpr double etaPrimeMass = 0.95778;
    constexpr double phiMass = 1.019461;
    constexpr double jpsiMass = 3.0969;
    constexpr double psi2SMass = 3.686097;
    constexpr double bdMass = 5.27966;
    constexpr double bsMass = 5.36692;
  }  // namespace pdg

  // dimuon mass regions (flag stored per dimuon)
  enum DimuonResonance : int { kNonResonant = 0, kJpsi = 1, kPsi2S = 2, kPhi = 3 };
  namespace window {
    constexpr double jpsiLow = 2.9, jpsiHigh = 3.3;
    constexpr double psi2SLow = 3.55, psi2SHigh = 3.8;
    constexpr double phiLow = 0.97, phiHigh = 1.07;
  }  // namespace window
}  // namespace bmmg

#endif
