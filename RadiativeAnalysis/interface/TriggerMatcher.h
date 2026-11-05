#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_TriggerMatcher_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_TriggerMatcher_h

// HLT path decisions and muon - trigger object matching (AOD: trigger::TriggerEvent).
// A muon matches a path if, for every terminal filter of the path that stores HLT muon objects,
// it is within maxDeltaR of one of them. Filters without muon objects (e/gamma legs) are skipped.

#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include <string>
#include <vector>

namespace bmmg {

  class TriggerMatcher {
  public:
    // paths: HLT path names without the version suffix
    TriggerMatcher(std::vector<std::string> paths, std::string process, double maxDeltaR);

    // to be called in beginRun; prints the filters used for the matching if verbose
    void beginRun(const edm::Run& run, const edm::EventSetup& setup, bool verbose);

    struct Result {
      std::vector<int> fired;                     // per path: 1 / 0, -1 if the path is not in the menu
      std::vector<std::vector<int>> muonMatched;  // [path][muon]: 1 if matched
    };
    Result evaluate(const edm::TriggerResults& results, const edm::TriggerNames& names,
                    const trigger::TriggerEvent* summary, const std::vector<const reco::Muon*>& muons) const;

    // MiniAOD: trigger objects (slimmedPatTrigger); a muon matches a path if it is within maxDeltaR of
    // an HLT muon object associated with the last filter of the path
    Result evaluate(const edm::TriggerResults& results, const edm::TriggerNames& names,
                    const std::vector<pat::TriggerObjectStandAlone>& objects,
                    const std::vector<const reco::Muon*>& muons) const;

    const std::vector<std::string>& paths() const { return paths_; }

  private:
    std::vector<std::string> terminalFilters(const std::string& path) const;
    // fills r.fired; returns the versioned path names in this menu ("" if absent)
    std::vector<std::string> resolvePaths(const edm::TriggerResults& results, const edm::TriggerNames& names,
                                          Result& r, size_t nMuons) const;

    std::vector<std::string> paths_;
    std::string process_;
    double maxDeltaR_;
    HLTConfigProvider hltConfig_;
    bool initialized_ = false;
  };

}  // namespace bmmg

#endif
