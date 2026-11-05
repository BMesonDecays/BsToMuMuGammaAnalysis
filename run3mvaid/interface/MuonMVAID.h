#ifndef MuonMVAID_h
#define MuonMVAID_h

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include <TLorentzVector.h>
#include <TVector.h>
#include <TMatrix.h>
#include <algorithm>

#include "XGBooster.h"
#include "CommonTools.h"

// 
// MiniRun3IdProducer is designed to add Run3 mva id to miniaod
//



class MuonMVAID {

public:

  MuonMVAID(const edm::ParameterSet &iConfig);
  ~MuonMVAID() = default;

  // Run-3 soft-muon MVA score (userFloat "run3muonmva") for every muon, in input order
  std::vector<float> produce(const std::vector<reco::Muon>& muons);


private:
  // GenMatchInfo getGenMatchInfo( const pat::PackedCandidate& track1,
  // const pat::PackedCandidate& track2);
  void fillMatchInfo(pat::Muon& cand, const pat::Muon& muon);
  void fillSoftMva(pat::Muon& mu_cand);

  // ----------member data ---------------------------

  std::vector<std::string> features_;
  std::vector<std::string> xgboost_models_;
  std::vector<std::string> xgboost_variable_names_;
  std::vector<XGBooster> softMuonMva_;
};

#endif
