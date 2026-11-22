// -*- C++ -*-
// Package:    BsToMuMuGammaAnalysis/RadiativeAnalysis
//
// Support for running the RadiativeAnalysis ntuplizer on legacy AOD samples without the endcap ECAL rechits
// (reducedEcalRecHitsEE), e.g. the private zero-bias, no-pileup B_s -> mu mu gamma MC of 2025-07-09
// (CMSSW_12_4_11_patch3). Used by test/runNtuplizer_legacy_cfg.py.
//
// EmptyEcalRecHitCollectionProducer: puts an empty EcalRecHitCollection into the event. The ntuplizer (and
// EcalClusterLazyTools) require an endcap rechit collection; with the endcap reco photons removed
// (photonMaxAbsSCEta = 1.479) no endcap rechit is ever looked up, so the empty collection only satisfies the
// interface. The converted photons do not use the ECAL and are unaffected in the endcaps.

#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include <memory>

class EmptyEcalRecHitCollectionProducer : public edm::global::EDProducer<> {
public:
  explicit EmptyEcalRecHitCollectionProducer(const edm::ParameterSet&) : token_(produces<EcalRecHitCollection>()) {}

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    descriptions.add("emptyEcalRecHitCollectionProducer", desc);
  }

  void produce(edm::StreamID, edm::Event& event, const edm::EventSetup&) const override {
    event.emplace(token_);
  }

private:
  const edm::EDPutTokenT<EcalRecHitCollection> token_;
};

DEFINE_FWK_MODULE(EmptyEcalRecHitCollectionProducer);
