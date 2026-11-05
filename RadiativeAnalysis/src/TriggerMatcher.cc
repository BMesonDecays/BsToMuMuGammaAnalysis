#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TriggerMatcher.h"

#include "DataFormats/HLTReco/interface/TriggerTypeDefs.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <iostream>

namespace bmmg {

  TriggerMatcher::TriggerMatcher(std::vector<std::string> paths, std::string process, double maxDeltaR)
      : paths_(std::move(paths)), process_(std::move(process)), maxDeltaR_(maxDeltaR) {}

  void TriggerMatcher::beginRun(const edm::Run& run, const edm::EventSetup& setup, bool verbose) {
    bool changed = true;
    initialized_ = hltConfig_.init(run, setup, process_, changed);
    if (!initialized_) {
      edm::LogError("TriggerMatcher") << "HLTConfigProvider::init failed for process " << process_;
      return;
    }
    if (changed && verbose) {
      for (const auto& path : hltConfig_.triggerNames())
        for (const auto& base : paths_) {
          if (path.find(base + "_v") != 0) continue;
          std::cout << "[TriggerMatcher] " << path << " terminal filters (their HLT muon objects are used):";
          for (const auto& f : terminalFilters(path)) std::cout << " " << f;
          std::cout << std::endl;
        }
    }
  }

  // terminal EDFilters of a path (walking backwards until the first producer after a filter)
  std::vector<std::string> TriggerMatcher::terminalFilters(const std::string& path) const {
    std::vector<std::string> filters;
    if (!initialized_) return filters;
    const std::vector<std::string>& modules = hltConfig_.moduleLabels(path);
    for (int i = static_cast<int>(modules.size()) - 1; i >= 0; --i) {
      const std::string& mod = modules[i];
      if (!mod.empty() && (mod[0] == '-' || mod[0] == '!')) continue;   // ignored / inverted result
      const std::string type = hltConfig_.moduleType(mod);
      if (type == "HLTBool" || type == "TriggerResultsFilter") continue;
      if (hltConfig_.moduleEDMType(mod) == "EDFilter")
        filters.push_back(mod);
      else if (!filters.empty())
        break;
    }
    return filters;
  }

  std::vector<std::string> TriggerMatcher::resolvePaths(const edm::TriggerResults& results,
                                                        const edm::TriggerNames& names, Result& r,
                                                        size_t nMuons) const {
    r.fired.assign(paths_.size(), -1);
    r.muonMatched.assign(paths_.size(), std::vector<int>(nMuons, 0));
    std::vector<std::string> resolved(paths_.size());
    for (size_t ip = 0; ip < paths_.size(); ++ip)
      for (unsigned int it = 0; it < names.size(); ++it)
        if (names.triggerName(it).find(paths_[ip] + "_v") == 0) {   // prefix match on "<base>_v"
          resolved[ip] = names.triggerName(it);
          r.fired[ip] = results.accept(it) ? 1 : 0;
          break;
        }
    return resolved;
  }

  TriggerMatcher::Result TriggerMatcher::evaluate(const edm::TriggerResults& results, const edm::TriggerNames& names,
                                                  const std::vector<pat::TriggerObjectStandAlone>& objects,
                                                  const std::vector<const reco::Muon*>& muons) const {
    Result r;
    const std::vector<std::string> resolved = resolvePaths(results, names, r, muons.size());
    // unpacked copies of the HLT muon objects
    std::vector<pat::TriggerObjectStandAlone> muonObjects;
    for (const auto& obj : objects) {
      if (!obj.hasTriggerObjectType(trigger::TriggerMuon)) continue;
      muonObjects.push_back(obj);
      muonObjects.back().unpackPathNames(names);
    }
    for (size_t ip = 0; ip < paths_.size(); ++ip) {
      if (resolved[ip].empty() || r.fired[ip] != 1) continue;
      for (size_t im = 0; im < muons.size(); ++im)
        for (const auto& obj : muonObjects)
          if (obj.hasPathName(resolved[ip], true, true) &&
              reco::deltaR(muons[im]->eta(), muons[im]->phi(), obj.eta(), obj.phi()) < maxDeltaR_) {
            r.muonMatched[ip][im] = 1;
            break;
          }
    }
    return r;
  }

  TriggerMatcher::Result TriggerMatcher::evaluate(const edm::TriggerResults& results, const edm::TriggerNames& names,
                                                  const trigger::TriggerEvent* summary,
                                                  const std::vector<const reco::Muon*>& muons) const {
    Result r;
    const std::vector<std::string> resolvedPaths = resolvePaths(results, names, r, muons.size());

    for (size_t ip = 0; ip < paths_.size(); ++ip) {
      const std::string& resolved = resolvedPaths[ip];
      if (resolved.empty() || r.fired[ip] != 1 || !summary) continue;

      // muon objects of each terminal filter (legs)
      std::vector<std::vector<const trigger::TriggerObject*>> legs;
      for (const auto& filter : terminalFilters(resolved)) {
        const size_t idx = summary->filterIndex(edm::InputTag(filter, "", process_));
        if (idx >= summary->sizeFilters()) continue;
        const trigger::Keys& keys = summary->filterKeys(idx);
        const trigger::Vids& ids = summary->filterIds(idx);
        std::vector<const trigger::TriggerObject*> leg;
        for (size_t k = 0; k < keys.size(); ++k)
          if (ids[k] == trigger::TriggerMuon) leg.push_back(&summary->getObjects()[keys[k]]);
        if (!leg.empty()) legs.push_back(std::move(leg));
      }
      if (legs.empty()) continue;

      for (size_t im = 0; im < muons.size(); ++im) {
        bool all = true;
        for (const auto& leg : legs) {
          bool found = false;
          for (const auto* obj : leg)
            if (reco::deltaR(muons[im]->eta(), muons[im]->phi(), obj->eta(), obj->phi()) < maxDeltaR_) {
              found = true;
              break;
            }
          if (!found) {
            all = false;
            break;
          }
        }
        r.muonMatched[ip][im] = all ? 1 : 0;
      }
    }
    return r;
  }

}  // namespace bmmg
