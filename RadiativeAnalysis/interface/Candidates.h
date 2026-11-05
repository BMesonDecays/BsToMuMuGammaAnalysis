#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_Candidates_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_Candidates_h

// Plain data structures passed between the components of the new analyzer.
// Indices: mu*/photon/dimuon fields index the vectors of selected objects / dimuons of the
// event (the same order as in the ntuple), "index" fields index the input collections.

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/GlobalError.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MMGVertexFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Constants.h"

#include <optional>
#include <vector>

namespace bmmg {

  struct SelectedMuon {
    int index = -1;                     // index in the muon collection
    const reco::Muon* muon = nullptr;
    float mva = -99.f;                  // Run-3 soft-muon MVA
    reco::TransientTrack track;         // inner track (kinParticle points to it: do not copy the muon and use the copy's particle)
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

  struct SelectedConversion {
    int index = -1;                     // index in the conversion collection
    const reco::Conversion* conversion = nullptr;
    reco::TransientTrack track1, track2; // positive, negative track
    std::optional<RefCountedKinematicParticle> photon;   // e+ e- fit with zero-mass constraint
    reco::Candidate::LorentzVector p4;  // from the fitted photon, else the pair momentum
  };

  struct SelectedTrack {
    int index = -1;                     // index in the track collection
    const reco::Track* track = nullptr;
    reco::TransientTrack ttrack;
  };

  // B candidate: dimuon + other legs (photon / conversion / two photons / two kaon tracks)
  struct BCandidate {
    int dimuon = -1, mu1 = -1, mu2 = -1;
    std::vector<int> legs;              // indices of the other legs in their collections
    int pv = -1;                        // index in the primary vertex collection (lowest DCA)
    reco::Candidate::LorentzVector prefitP4;   // B from the reconstructed momenta
    reco::Candidate::LorentzVector xP4;        // the non-dimuon system (gamma, gamma gamma, K K)
    std::optional<MMGVertexFit::Result> fit;   // empty if the fit failed; daughters: mu1, mu2, legs...
    FlightInfo flight;                  // from the fitted vertex and momentum
    float massErr = -1.f;
    float helicity = -2.f, coplanarity = -1.f, cosThetaL = -2.f;
    float isolation = -1.f, maxTrackCompatibility = -1.f;
  };
  using MMGCandidate = BCandidate;

}  // namespace bmmg

#endif
