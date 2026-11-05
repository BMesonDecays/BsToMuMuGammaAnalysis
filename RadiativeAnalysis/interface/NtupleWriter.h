#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_NtupleWriter_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_NtupleWriter_h

// The ntuple of the new RadiativeAnalysis module: tree "Events" (one entry per event, collections
// stored as vectors with a common prefix) and tree "RunInfo" (one entry per job).
// Each block is booked and filled next to each other in NtupleWriter.cc; NTUPLE.md documents the branches.

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BranchSet.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/Candidates.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GenChain.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TriggerMatcher.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

#include <memory>
#include <string>
#include <vector>

class TFileService;

namespace bmmg {

  class NtupleWriter {
  public:
    NtupleWriter(TFileService& fs, const std::vector<std::string>& triggerPaths, bool isMC);

    void beginEvent(const edm::EventID& id, unsigned int lumi);
    void fillPileup(const std::vector<PileupSummaryInfo>& pu);
    void fillVertices(const reco::BeamSpot& bs, const std::vector<reco::Vertex>& pvs);
    void fillTrigger(const TriggerMatcher::Result& trigger);
    // gen may be nullptr (data); gen matches of the objects are filled from it
    void fillMuons(const std::vector<SelectedMuon>& muons, const reco::BeamSpot& bs,
                   const TriggerMatcher::Result& trigger, const GenDecayChain* gen);
    void fillPhotons(const std::vector<SelectedPhoton>& photons, EcalClusterLazyTools& lazyTools,
                     const MMGVertexFit& fitter, const GenDecayChain* gen);
    void fillDimuons(const std::vector<DimuonCandidate>& dimuons);
    void fillMuMuGamma(const std::vector<MMGCandidate>& candidates, const std::vector<reco::Vertex>& pvs);
    void fillGen(const GenDecayChain& gen);
    void fill();

    struct RunInfo {
      double xsec = -1., xsecErr = -1., filterEff = -1., filterEffErr = -1., lumiEquivalent = -1., lumiEquivalentErr = -1.;
      unsigned long long nEvents = 0;
    };
    void fillRunInfo(const RunInfo& info);

  private:
    void book();

    TTree* events_;
    TTree* runInfo_;
    BranchSet b_;
    BranchSet r_;
    std::vector<std::string> triggerPaths_;
    bool isMC_;
    // per event: gen match result of the selected muons / photons (used for the candidate truth flag)
    std::vector<int> muonFromB_, photonFromB_;
  };

}  // namespace bmmg

#endif
