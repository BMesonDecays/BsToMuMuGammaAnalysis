#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_Candidates_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_Candidates_h

// Plain data structures passed between the components of the new analyzer.
// Indices: mu*/photon/dimuon fields index the vectors of selected objects / dimuons of the
// event (the same order as in the ntuple), "index" fields index the input collections.

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/GlobalError.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MMGVertexFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Constants.h"

#include <optional>

namespace bmmg {

  struct SelectedMuon {
    int index = -1;                     // index in the muon collection
    const reco::Muon* muon = nullptr;
    float mva = -99.f;                  // Run-3 soft-muon MVA
    reco::TransientTrack track;         // inner track
    std::optional<RefCountedKinematicParticle> kinParticle;   // for the kinematic fit
  };

  struct SelectedPhoton {
    int index = -1;                     // index in the photon collection
    const reco::Photon* photon = nullptr;
    float mva = -99.f;                  // photon MVA (ValueMap), -99 if not available
    int tightId = -1;                   // cut-based tight ID (ValueMap), -1 if not available
  };

  struct DimuonCandidate {
    int mu1 = -1, mu2 = -1;             // selected muons; mu1 has positive charge
    reco::Candidate::LorentzVector p4;
    bool vtxValid = false;
    float vtxProb = -1.f, vtxChi2 = -1.f, vtxNdof = -1.f;
    GlobalPoint vtx;
    GlobalError vtxErr;
    float lxyBS = -1.f, lxyBSErr = -1.f, cosAlphaBS = -2.f;   // transverse w.r.t. the beam spot
    float dca = -1.f;                   // transverse distance of closest approach of the two tracks
    float deltaR = -1.f;
    int isCowboy = -1;
    int resonance = kNonResonant;       // DimuonResonance
  };

  // flight distance, pointing and proper decay length of a candidate w.r.t. a primary vertex
  struct FlightInfo {
    float l3d = -1.f, l3dErr = -1.f, lxy = -1.f, lxyErr = -1.f;
    float cosAlpha3D = -2.f, cosAlpha2D = -2.f;
    float ct3D = -999.f, ct3DErr = -1.f, ct2D = -999.f, ct2DErr = -1.f;
  };

  struct MMGCandidate {
    int dimuon = -1, mu1 = -1, mu2 = -1, photon = -1;
    int pv = -1;                        // index in the primary vertex collection (lowest DCA)
    reco::Candidate::LorentzVector prefitP4;
    std::optional<MMGVertexFit::Result> fit;   // empty if the fit failed
    FlightInfo flight;                  // from the fitted vertex and momentum
    float massErr = -1.f;
    float helicity = -2.f, coplanarity = -1.f, cosThetaL = -2.f;
    float isolation = -1.f, maxTrackCompatibility = -1.f;
  };

}  // namespace bmmg

#endif
