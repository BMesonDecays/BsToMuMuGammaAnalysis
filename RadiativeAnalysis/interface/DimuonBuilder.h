#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_DimuonBuilder_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_DimuonBuilder_h

// All opposite-sign pairs of selected muons (not sharing segments), with a Kalman vertex fit and
// the dimuon observables. Replaces the three copies of the dimuon block of the legacy builders.

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include <vector>

namespace bmmg {

  std::vector<DimuonCandidate> buildDimuons(const std::vector<SelectedMuon>& muons, const reco::BeamSpot& beamSpot);

  int dimuonResonance(double mass);

}  // namespace bmmg

#endif
