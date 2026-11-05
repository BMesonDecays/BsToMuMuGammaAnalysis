#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_CandidateBuilders_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_CandidateBuilders_h

// B candidates built from the dimuons of the event and other legs:
//   mu mu gamma        (ECAL photon)              buildMuMuGamma
//   mu mu gamma(conv)  (photon conversion e+ e-)   buildMuMuConvGamma
//   mu mu gamma gamma  (e.g. J/psi eta)            buildMuMuGammaGamma
//   mu mu K K          (Bs -> J/psi phi, normalisation)  buildMuMuKK
// For every combination inside the pre-fit mass window: the PV with the lowest DCA to the line
// through the dimuon vertex along the pre-fit momentum is chosen, the candidate is fitted with
// MMGVertexFit (vertex fit + pointing constraint to that PV) and the observables are computed.
// Candidates whose fit fails are kept (BCandidate::fit empty).

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include <vector>

namespace bmmg {

  struct CandidateConfig {
    double minPrefitMass = 4.0;      // window on the pre-fit B mass
    double maxPrefitMass = 7.0;
    double massHypothesis = pdg::bsMass;   // used for ct
    bool computeTrackCompatibility = true;
  };

  struct DiphotonConfig {
    double minMass = 0.05;           // window on the diphoton mass (covers pi0, eta, eta')
    double maxMass = 1.1;
  };

  struct KaonConfig {
    double minPt = 0.7;
    double maxAbsEta = 2.5;
    double maxDeltaR = 1.5;          // to the dimuon direction
    double maxDz = 1.0;              // |z(track at closest approach) - z(dimuon vertex)| [cm]
    double minKKMass = 0.99;         // phi window
    double maxKKMass = 1.05;
    bool jpsiOnly = true;            // only J/psi dimuons (normalisation channel)
    double minBMass = 4.9;           // pre-fit B mass window (used by the caller as CandidateConfig window)
    double maxBMass = 5.9;
  };

  std::vector<BCandidate> buildMuMuGamma(const std::vector<SelectedMuon>& muons,
                                         const std::vector<DimuonCandidate>& dimuons,
                                         const std::vector<SelectedPhoton>& photons,
                                         const std::vector<reco::Vertex>& pvs,
                                         const EcalClusterLazyTools& lazyTools,
                                         const TransientTrackBuilder& ttBuilder,
                                         const MMGVertexFit& fitter,
                                         const CandidateConfig& config);

  std::vector<BCandidate> buildMuMuConvGamma(const std::vector<SelectedMuon>& muons,
                                             const std::vector<DimuonCandidate>& dimuons,
                                             const std::vector<SelectedConversion>& conversions,
                                             const std::vector<reco::Vertex>& pvs,
                                             const TransientTrackBuilder& ttBuilder,
                                             const CandidateConfig& config);

  std::vector<BCandidate> buildMuMuGammaGamma(const std::vector<SelectedMuon>& muons,
                                              const std::vector<DimuonCandidate>& dimuons,
                                              const std::vector<SelectedPhoton>& photons,
                                              const std::vector<reco::Vertex>& pvs,
                                              const EcalClusterLazyTools& lazyTools,
                                              const TransientTrackBuilder& ttBuilder,
                                              const MMGVertexFit& fitter,
                                              const CandidateConfig& config,
                                              const DiphotonConfig& diphoton);

  // kaon tracks: "tracks" are the preselected tracks (selectKaonTracks); only the tracks used in a
  // candidate are kept in "usedTracks", and the candidate legs index usedTracks
  std::vector<BCandidate> buildMuMuKK(const std::vector<SelectedMuon>& muons,
                                      const std::vector<DimuonCandidate>& dimuons,
                                      const std::vector<SelectedTrack>& tracks,
                                      const std::vector<reco::Vertex>& pvs,
                                      const TransientTrackBuilder& ttBuilder,
                                      const CandidateConfig& config,
                                      const KaonConfig& kaons,
                                      std::vector<SelectedTrack>& usedTracks);

}  // namespace bmmg

#endif
