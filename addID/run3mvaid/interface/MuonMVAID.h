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

#include "addID/run3mvaid/interface/XGBooster.h"
#include "addID/run3mvaid/interface/CommonTools.h"

// 
// MiniRun3IdProducer is designed to add Run3 mva id to miniaod
//

using namespace std;
typedef reco::Candidate::LorentzVector LorentzVector;
typedef pair<const reco::MuonChamberMatch*, const reco::MuonSegmentMatch*> MatchPair;

///////////////////////////////////////////////////////////////////////////
///                             P L U G I N
///////////////////////////////////////////////////////////////////////////

class MuonMVAID {

public:

  explicit MuonMVAID(const edm::ParameterSet &iConfig);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  ~MuonMVAID() override {};

  vector<float> produce(const std::vector<reco::Muon>& muons);


private:
  // GenMatchInfo getGenMatchInfo( const pat::PackedCandidate& track1,
  // const pat::PackedCandidate& track2);
  void fillMatchInfo(pat::Muon& cand, const pat::Muon& muon);
  void fillSoftMva(pat::Muon& mu_cand);

  // ----------member data ---------------------------

  vector<string> features_;
  vector<string> xgboost_models_;
  vector<string> xgboost_variable_names_;
  vector<XGBooster> softMuonMva_;
};

#endif
