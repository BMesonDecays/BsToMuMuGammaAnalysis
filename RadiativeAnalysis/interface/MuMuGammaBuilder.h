#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_MuMuGammaBuilder_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_MuMuGammaBuilder_h

// B -> mu mu gamma candidates: every dimuon (with a valid vertex) combined with every selected
// photon whose pre-fit mu mu gamma mass is inside the window, fitted with MMGVertexFit with the
// pointing constraint to the lowest-DCA primary vertex. Candidates whose fit fails are kept
// (MMGCandidate::fit empty).

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include <vector>

namespace bmmg {

  struct MuMuGammaConfig {
    double minPrefitMass = 4.0;
    double maxPrefitMass = 7.0;
    double massHypothesis = pdg::bsMass;   // used for ct
    bool computeTrackCompatibility = true;
  };

  std::vector<MMGCandidate> buildMuMuGamma(const std::vector<SelectedMuon>& muons,
                                           const std::vector<DimuonCandidate>& dimuons,
                                           const std::vector<SelectedPhoton>& photons,
                                           const std::vector<reco::Vertex>& pvs,
                                           const EcalClusterLazyTools& lazyTools,
                                           const TransientTrackBuilder& ttBuilder,
                                           const MMGVertexFit& fitter,
                                           const MuMuGammaConfig& config);

}  // namespace bmmg

#endif
