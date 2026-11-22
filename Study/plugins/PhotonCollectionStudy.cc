// PhotonCollectionStudy: how many B_s -> mu mu gamma signal events survive with a given photon collection.
//
// Per event:
//   1. gen: a B_s (|pdgId| 531) whose daughters are mu+ mu- and at least one photon (the hardest photon is
//      taken if PHOTOS added more). Oscillated B_s: the decaying one is used (its daughters are mu mu gamma);
//   2. trigger: a path whose name starts with triggerPath (HLT_DoubleMu4_3_LowMass_v) accepted;
//   3. muons: each gen muon matched to a reco muon of the same charge, the closest one in dR < muonMaxDR
//      (two different reco muons);
//   4. photon: the gen photon matched to the closest reco photon in dR < photonMaxDR.
// Counts after every step go to the cutflow histogram; the mu mu gamma mass (plain sum of the reco 4-vectors,
// muon mass for the muons, no vertex or kinematic fit) is filled for the events passing all steps.
// The muon and photon collections are read as edm::View<reco::Muon> / edm::View<reco::Photon>, so the same
// module works on AOD (muons, photons, gedPhotons) and MiniAOD (slimmedMuons, slimmedPhotons).

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"

#include "TH1D.h"
#include "TH2D.h"

#include <string>
#include <vector>

namespace {
  constexpr double kMuonMass = 0.1056583755;

  enum Step { kAll, kGen, kTrigger, kMuons, kPhoton, kMassWindow, kNSteps };
  const char* kStepNames[kNSteps] = {"all events", "gen Bs->mumugamma", "trigger", "2 muons matched",
                                     "photon matched", "mass in [4.5, 6.5]"};

  // the decay products of a B_s -> mu mu gamma, or nullptrs
  struct GenDecay {
    const reco::GenParticle* bs = nullptr;
    const reco::Candidate* muPlus = nullptr;
    const reco::Candidate* muMinus = nullptr;
    const reco::Candidate* photon = nullptr;
  };

  GenDecay findDecay(const std::vector<reco::GenParticle>& gen) {
    for (const auto& p : gen) {
      if (std::abs(p.pdgId()) != 531)
        continue;
      GenDecay d;
      int nOther = 0;
      for (size_t i = 0; i < p.numberOfDaughters(); ++i) {
        const reco::Candidate* c = p.daughter(i);
        if (c->pdgId() == -13)
          d.muPlus = c;
        else if (c->pdgId() == 13)
          d.muMinus = c;
        else if (c->pdgId() == 22) {
          if (!d.photon || c->pt() > d.photon->pt())
            d.photon = c;
        } else
          ++nOther;
      }
      if (d.muPlus && d.muMinus && d.photon && nOther == 0) {
        d.bs = &p;
        return d;
      }
    }
    return GenDecay();
  }
}  // namespace

class PhotonCollectionStudy : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit PhotonCollectionStudy(const edm::ParameterSet&);
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  // index of the matched reco muon (closest in dR, same charge, not in 'used'), -1 if none
  int matchMuon(const edm::View<reco::Muon>& muons, const reco::Candidate& gen, int used) const;

  edm::EDGetTokenT<edm::TriggerResults> triggerToken_;
  edm::EDGetTokenT<std::vector<reco::GenParticle>> genToken_;
  edm::EDGetTokenT<edm::View<reco::Muon>> muonToken_;
  edm::EDGetTokenT<edm::View<reco::Photon>> photonToken_;
  std::string triggerPath_;
  double muonMaxDR_;
  double photonMaxDR_;

  TH1D* cutflow_;
  TH1D* mass_;
  TH1D* massWide_;
  TH1D* genPhoPt_;         // gen photon pt, events with trigger and both muons matched
  TH1D* genPhoPtMatched_;  // ... and the photon matched
  TH1D* genPhoEta_;
  TH1D* genPhoEtaMatched_;
  TH1D* recoPhoPt_;        // pt of the matched reco photon
  TH1D* phoDR_;            // dR of the closest reco photon to the gen photon
  TH2D* phoPtResponse_;    // reco/gen pt vs gen pt of the matched photon
  TH1D* nPhotons_;         // photons in the collection per event (all events)
  TH1D* allPhoPt_;         // pt of all photons in the collection
  bool printedPaths_ = false;
};

PhotonCollectionStudy::PhotonCollectionStudy(const edm::ParameterSet& cfg)
    : triggerToken_(consumes<edm::TriggerResults>(cfg.getParameter<edm::InputTag>("triggerResults"))),
      genToken_(consumes<std::vector<reco::GenParticle>>(cfg.getParameter<edm::InputTag>("genParticles"))),
      muonToken_(consumes<edm::View<reco::Muon>>(cfg.getParameter<edm::InputTag>("muons"))),
      photonToken_(consumes<edm::View<reco::Photon>>(cfg.getParameter<edm::InputTag>("photons"))),
      triggerPath_(cfg.getParameter<std::string>("triggerPath")),
      muonMaxDR_(cfg.getParameter<double>("muonMaxDR")),
      photonMaxDR_(cfg.getParameter<double>("photonMaxDR")) {
  usesResource("TFileService");
  edm::Service<TFileService> fs;
  cutflow_ = fs->make<TH1D>("cutflow", "events after each step", kNSteps, 0, kNSteps);
  for (int i = 0; i < kNSteps; ++i)
    cutflow_->GetXaxis()->SetBinLabel(i + 1, kStepNames[i]);
  mass_ = fs->make<TH1D>("mass", ";m(#mu#mu#gamma) [GeV];events", 80, 4.5, 6.5);
  massWide_ = fs->make<TH1D>("massWide", ";m(#mu#mu#gamma) [GeV];events", 120, 3.0, 9.0);
  genPhoPt_ = fs->make<TH1D>("genPhoPt", ";gen photon p_{T} [GeV];events", 60, 0, 30);
  genPhoPtMatched_ = fs->make<TH1D>("genPhoPtMatched", ";gen photon p_{T} [GeV];events", 60, 0, 30);
  genPhoEta_ = fs->make<TH1D>("genPhoEta", ";gen photon #eta;events", 60, -3, 3);
  genPhoEtaMatched_ = fs->make<TH1D>("genPhoEtaMatched", ";gen photon #eta;events", 60, -3, 3);
  recoPhoPt_ = fs->make<TH1D>("recoPhoPt", ";matched reco photon p_{T} [GeV];events", 60, 0, 30);
  phoDR_ = fs->make<TH1D>("phoDR", ";#DeltaR(gen, closest reco photon);events", 100, 0, 0.5);
  phoPtResponse_ = fs->make<TH2D>("phoPtResponse", ";gen photon p_{T} [GeV];reco / gen p_{T}", 30, 0, 30, 60, 0, 2);
  nPhotons_ = fs->make<TH1D>("nPhotons", ";photons per event;events", 30, 0, 30);
  allPhoPt_ = fs->make<TH1D>("allPhoPt", ";p_{T} of all photons [GeV];photons", 60, 0, 30);
}

int PhotonCollectionStudy::matchMuon(const edm::View<reco::Muon>& muons, const reco::Candidate& gen, int used) const {
  int best = -1;
  double bestDR = muonMaxDR_;
  for (size_t i = 0; i < muons.size(); ++i) {
    if (static_cast<int>(i) == used || muons[i].charge() != gen.charge())
      continue;
    const double dr = reco::deltaR(muons[i], gen);
    if (dr < bestDR) {
      bestDR = dr;
      best = i;
    }
  }
  return best;
}

void PhotonCollectionStudy::analyze(const edm::Event& event, const edm::EventSetup&) {
  cutflow_->Fill(kAll);
  const auto& photons = event.get(photonToken_);
  nPhotons_->Fill(photons.size());
  for (const auto& pho : photons)
    allPhoPt_->Fill(pho.pt());

  const GenDecay decay = findDecay(event.get(genToken_));
  if (!decay.bs)
    return;
  cutflow_->Fill(kGen);

  const auto& trig = event.get(triggerToken_);
  const edm::TriggerNames& names = event.triggerNames(trig);
  bool fired = false, found = false;
  for (size_t i = 0; i < names.size(); ++i) {
    if (names.triggerName(i).rfind(triggerPath_, 0) != 0)
      continue;
    found = true;
    if (!printedPaths_) {
      edm::LogPrint("PhotonCollectionStudy") << "trigger path: " << names.triggerName(i);
      printedPaths_ = true;
    }
    fired |= trig.accept(i);
  }
  if (!found && !printedPaths_) {
    edm::LogWarning("PhotonCollectionStudy") << "no trigger path starting with " << triggerPath_;
    printedPaths_ = true;
  }
  if (!fired)
    return;
  cutflow_->Fill(kTrigger);

  const auto& muons = event.get(muonToken_);
  const int iPlus = matchMuon(muons, *decay.muPlus, -1);
  const int iMinus = iPlus < 0 ? -1 : matchMuon(muons, *decay.muMinus, iPlus);
  if (iPlus < 0 || iMinus < 0)
    return;
  cutflow_->Fill(kMuons);

  const reco::Candidate& gpho = *decay.photon;
  genPhoPt_->Fill(gpho.pt());
  genPhoEta_->Fill(gpho.eta());
  int iPho = -1;
  double bestDR = 999;
  for (size_t i = 0; i < photons.size(); ++i) {
    const double dr = reco::deltaR(photons[i], gpho);
    if (dr < bestDR) {
      bestDR = dr;
      iPho = i;
    }
  }
  if (iPho >= 0)
    phoDR_->Fill(std::min(bestDR, 0.4999));
  if (iPho < 0 || bestDR > photonMaxDR_)
    return;
  cutflow_->Fill(kPhoton);
  const reco::Photon& pho = photons[iPho];
  genPhoPtMatched_->Fill(gpho.pt());
  genPhoEtaMatched_->Fill(gpho.eta());
  recoPhoPt_->Fill(pho.pt());
  phoPtResponse_->Fill(gpho.pt(), pho.pt() / gpho.pt());

  using PtEtaPhiM = math::PtEtaPhiMLorentzVector;
  const auto& m1 = muons[iPlus];
  const auto& m2 = muons[iMinus];
  const PtEtaPhiM p = PtEtaPhiM(m1.pt(), m1.eta(), m1.phi(), kMuonMass) +
                      PtEtaPhiM(m2.pt(), m2.eta(), m2.phi(), kMuonMass) +
                      PtEtaPhiM(pho.pt(), pho.eta(), pho.phi(), 0.);
  massWide_->Fill(p.M());
  mass_->Fill(p.M());
  if (p.M() >= 4.5 && p.M() < 6.5)
    cutflow_->Fill(kMassWindow);
}

DEFINE_FWK_MODULE(PhotonCollectionStudy);
