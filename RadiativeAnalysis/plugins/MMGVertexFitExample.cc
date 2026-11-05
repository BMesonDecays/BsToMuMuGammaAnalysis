#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"


#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MMGVertexFit.h"



#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"
#include "TString.h"

#include <sstream>
#include <iomanip> 
#include <utility>
#include <numeric>
#include <vector>
#include <limits>
#include <cmath>


using namespace std;


//object definition
class MMGVertexFitExample : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit MMGVertexFitExample(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~MMGVertexFitExample();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  bool isSameDecay(const std::vector<int>&, const std::vector<int>&);
private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  // muons and photons are read as views of the reco base classes, so the same code runs on
  // AOD (reco::Muon, reco::Photon) and MiniAOD (pat::Muon, pat::Photon)
  edm::EDGetTokenT < vector<reco::GenParticle> > theGenParticleToken;
  edm::EDGetTokenT < edm::View<reco::Muon> > theMuonToken;
  edm::EDGetTokenT < edm::View<reco::Photon> > thePhotonToken;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;
  edm::ESGetToken <TransientTrackBuilder, TransientTrackRecord> theTransientTrackBuilderToken;

  edm::EDGetTokenT<EcalRecHitCollection> ebRecHitsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> eeRecHitsToken_;

  edm::EDGetTokenT < reco::BeamSpot > theBeamSpotToken;
  edm::EDGetTokenT < vector<reco::Vertex> > theVertexToken;

  unique_ptr<EcalClusterLazyToolsBase::ESGetTokens> esGetTokens;

  // histograms

  TH1D* hBsMass;
  TH1D* hBsMassNoFit;

  // fit performance with respect to the generated Bs (index 0, 1, 2 = x, y, z)
  TH1D* hVtxResidual[3];
  TH1D* hVtxPull[3];
  TH1D* hBsMassResidual;
  TH1D* hBsMassPull;
  TH1D* hChi2Ndf;
  TH1D* hVtxProb;

  // photon measurement with respect to the generated photon
  TH1D* hClusterPullPhi;
  TH1D* hClusterPullTheta;
  TH1D* hPhotonEnergyPull;
  TH2D* hPhotonEnergyPullVsE;

  std::vector<TH1*> histograms;

  // scale factors for the photon measurement uncertainties in barrel and endcap, see MMGVertexFit
  MMGVertexFit::ErrorScale errorScaleEB;
  MMGVertexFit::ErrorScale errorScaleEE;
  // ECAL region of the matched photons: "EB", "EE" or "both"
  std::string photonRegion;

  int nConvPhotons = 0;
  std::vector<int> MuMuG = {22, 13, -13};
};


MMGVertexFitExample::MMGVertexFitExample(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0),
    errorScaleEB{conf.getUntrackedParameter<double>("clusterCovScalePhiEB", 1.),
                 conf.getUntrackedParameter<double>("clusterCovScaleThetaEB", 1.),
                 conf.getUntrackedParameter<double>("energyErrorScaleEB", 1.)},
    errorScaleEE{conf.getUntrackedParameter<double>("clusterCovScalePhiEE", 1.),
                 conf.getUntrackedParameter<double>("clusterCovScaleThetaEE", 1.),
                 conf.getUntrackedParameter<double>("energyErrorScaleEE", 1.)},
    photonRegion(conf.getUntrackedParameter<std::string>("photonRegion", "both"))
{
  if (photonRegion != "EB" && photonRegion != "EE" && photonRegion != "both")
    throw cms::Exception("Configuration") << "MMGVertexFitExample: photonRegion must be EB, EE or both, got " << photonRegion;
  cout <<" CTORXX" << endl;

  // collection names depend on the data format (AOD or MiniAOD), they are set in the config
  theGenParticleToken = consumes< vector<reco::GenParticle>  >(conf.getParameter<edm::InputTag>("genParticles"));
  theMuonToken = consumes< edm::View<reco::Muon>  >(conf.getParameter<edm::InputTag>("muons"));
  thePhotonToken = consumes< edm::View<reco::Photon>  >(conf.getParameter<edm::InputTag>("photons"));
  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();
  theTriggerResultsToken = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
  theTransientTrackBuilderToken = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));
  ebRecHitsToken_ = consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("ebRecHits"));
  eeRecHitsToken_ = consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("eeRecHits"));

  esGetTokens = make_unique<EcalClusterLazyToolsBase::ESGetTokens>(consumesCollector());

  theBeamSpotToken = consumes< reco::BeamSpot >(conf.getParameter<edm::InputTag>("beamSpot"));
  theVertexToken = consumes< vector<reco::Vertex>  >(conf.getParameter<edm::InputTag>("primaryVertices"));

}

MMGVertexFitExample::~MMGVertexFitExample()
{
  cout <<" DTOR" << endl;
}

bool MMGVertexFitExample::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}


void MMGVertexFitExample::beginJob()
{
  //create a histogram

  hBsMass = new TH1D("hBsMass", "hBsMass", 100, 5, 6);
  hBsMassNoFit = new TH1D("hBsMassNoFit", "hBsMassNoFit", 100, 5, 6);
  histograms = {hBsMass, hBsMassNoFit};

  const char* coord[3] = {"X", "Y", "Z"};
  for (int c = 0; c < 3; ++c) {
    hVtxResidual[c] = new TH1D(Form("hVtxResidual%s", coord[c]), Form("vertex %s_{fit} - %s_{gen};#Delta%s [cm];candidates", coord[c], coord[c], coord[c]), 100, -0.1, 0.1);
    hVtxPull[c] = new TH1D(Form("hVtxPull%s", coord[c]), Form("vertex %s pull;(%s_{fit} - %s_{gen})/#sigma;candidates", coord[c], coord[c], coord[c]), 100, -5, 5);
    histograms.insert(histograms.end(), {hVtxResidual[c], hVtxPull[c]});
  }
  hClusterPullPhi = new TH1D("hClusterPullPhi", "cluster position pull along e_{#phi};d #upoint e_{#phi}/#sigma;photons", 100, -5, 5);
  hClusterPullTheta = new TH1D("hClusterPullTheta", "cluster position pull along e_{#theta};d #upoint e_{#theta}/#sigma;photons", 100, -5, 5);
  histograms.insert(histograms.end(), {hClusterPullPhi, hClusterPullTheta});
  hBsMassResidual = new TH1D("hBsMassResidual", "B_{s} mass residual;m_{fit} - m_{gen} [GeV];candidates", 100, -0.5, 0.5);
  hBsMassPull = new TH1D("hBsMassPull", "B_{s} mass pull;(m_{fit} - m_{gen})/#sigma_{m};candidates", 100, -5, 5);
  hChi2Ndf = new TH1D("hChi2Ndf", "vertex #chi^{2}/ndf;#chi^{2}/ndf;candidates", 100, 0, 10);
  hVtxProb = new TH1D("hVtxProb", "vertex probability;P(#chi^{2}, ndf);candidates", 100, 0, 1);
  hPhotonEnergyPull = new TH1D("hPhotonEnergyPull", "photon energy pull;(E_{reco} - E_{gen})/#sigma_{E};photons", 100, -5, 5);
  hPhotonEnergyPullVsE = new TH2D("hPhotonEnergyPullVsE", "photon energy pull;E_{gen} [GeV];(E_{reco} - E_{gen})/#sigma_{E}", 50, 0, 100, 100, -5, 5);
  histograms.insert(histograms.end(), {hBsMassResidual, hBsMassPull, hChi2Ndf, hVtxProb, hPhotonEnergyPull, hPhotonEnergyPullVsE});

  cout << "HERE MMGVertexFitExample::beginJob()" << endl;
}

void MMGVertexFitExample::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data
  for (TH1* h : histograms) h->Write();

  myRootFile.Close();

  for (TH1* h : histograms) delete h;
  histograms.clear();


  cout << "HERE MMGVertexFitExample::endJob()" << endl;
}


void MMGVertexFitExample::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE MMGVertexFitExample::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const edm::View<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const edm::View<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  auto const& field = es.getData(m_fieldToken);

  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);

  const TransientTrackBuilder* TTBuilder = &es.getData(theTransientTrackBuilderToken);

  EcalClusterLazyTools lazyTools(ev, esGetTokens->get(es), ebRecHitsToken_, eeRecHitsToken_);


  vector<const reco::Candidate*> genMuons;
  vector<const reco::Muon*> recoMatchedMuons;
  vector<const reco::Candidate*> genMatchedMuons;

  vector<const reco::Candidate*> genPhotons;
  vector<const reco::Photon*> recoMatchedPhotons;
  vector<const reco::Candidate*> genMatchedPhotons;


  GlobalPoint genPV;
  GlobalPoint genSV;
  GlobalVector genBsMomentum;
  double genBsMass = 0.;

  for(const auto& genP : genPar)
  {
    if (abs(genP.pdgId()) == 531)
    {
      vector<int> daughters;
      for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
      {
        daughters.push_back(genP.daughter(i)->pdgId());
      }
      if(isSameDecay(daughters, MuMuG))
      {
        for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
        {
          if(abs(genP.daughter(i)->pdgId()) == 13) genMuons.push_back(genP.daughter(i));
          if(abs(genP.daughter(i)->pdgId()) == 22) genPhotons.push_back(genP.daughter(i));
        }
        genSV = GlobalPoint(genP.daughter(0)->vx(), genP.daughter(0)->vy(), genP.daughter(0)->vz());
        genBsMomentum = GlobalVector(genP.px(), genP.py(), genP.pz());
        genBsMass = genP.mass();

        // find the primary vertex: the production vertex of the first ancestor coming from a proton,
        // or of the oldest ancestor kept in the (pruned, in MiniAOD) gen record
        const reco::Candidate* ancestor = &genP;
        while(ancestor->numberOfMothers() > 0 && ancestor->mother()->pdgId() != 2212)
        {
          ancestor = ancestor->mother();
        }
        genPV = GlobalPoint(ancestor->vx(), ancestor->vy(), ancestor->vz());

      }
    }
  }

  const std::vector<reco::Vertex> genPrimaryVertices = {reco::Vertex(reco::Vertex::Point(genPV.x(), genPV.y(), genPV.z()), reco::Vertex::Error(), 0, 0, 0)};

  // reco muon matching
  for (const reco::Candidate* genMu : genMuons)
  {
    float minDR = 10;
    const reco::Muon* bestMatchedMuon;
    bool matched = false;
    for (const auto& recoMu : recoMuons)
    {
      float dR = reco::deltaR(recoMu, *genMu);
      if (dR < minDR)
      {
        minDR = dR;
        bestMatchedMuon = &recoMu;
        matched = true;
      }
    }
    // if (matched) hMuDeltaR->Fill(minDR);
    if (matched && minDR < 0.01)
    {
      recoMatchedMuons.push_back(bestMatchedMuon);
      genMatchedMuons.push_back(genMu);
      // hRecoVsGenMuPt->Fill(genMu->pt(), bestMatchedMuon->pt());
      // hMuPtError->Fill((bestMatchedMuon->pt() - genMu->pt())/genMu->pt());
    }
  }

  // reco photon matching
  for (const reco::Candidate* genPh : genPhotons)
  {
    float minDR = 10;
    const reco::Photon* bestMatchedPhoton;
    bool matched = false;
    for (const auto& recoPh : recoPhotons)
    {
      float dR = reco::deltaR(recoPh, *genPh);
      if (dR < minDR)
      {
        minDR = dR;
        bestMatchedPhoton = &recoPh;
        matched = true;
      }
    }
    // if (matched) hGammaDeltaR->Fill(minDR);
    if (!matched) continue;
    const bool inRegion = photonRegion == "both" || (photonRegion == "EB" && bestMatchedPhoton->isEB()) ||
                          (photonRegion == "EE" && bestMatchedPhoton->isEE());
    if (minDR < 0.03 && inRegion && MMGVertexFit::hasClusterHits(*bestMatchedPhoton))
    {
      recoMatchedPhotons.push_back(bestMatchedPhoton);
      genMatchedPhotons.push_back(genPh);
      // hRecoVsGenGammaPt->Fill(genPh->pt(), bestMatchedPhoton->pt());
      // hGammaPtError->Fill((bestMatchedPhoton->pt() - genPh->pt())/genPh->pt());
    }
  }

//////////////////////////////////////
/////////////////////////////////////
  MMGVertexFit mmgVertexFit(errorScaleEB, errorScaleEE);
  std::vector<MMGVertexFit::Result> fitResults = mmgVertexFit.fit(recoMatchedMuons, recoMatchedPhotons, lazyTools, genPrimaryVertices, TTBuilder);

  // the inputs are gen-matched, so every fitted candidate is compared with the generated Bs
  for (const auto& result : fitResults) {
    const KinematicState& state = result.particle->currentState();
    const double fitMass = state.mass();
    hBsMass->Fill(fitMass);

    // decay vertex
    const GlobalPoint fitSV = result.vertex->position();
    const GlobalError svError = result.vertex->error();
    const double residual[3] = {fitSV.x() - genSV.x(), fitSV.y() - genSV.y(), fitSV.z() - genSV.z()};
    const double variance[3] = {svError.cxx(), svError.cyy(), svError.czz()};
    for (int c = 0; c < 3; ++c) {
      hVtxResidual[c]->Fill(residual[c]);
      if (variance[c] > 0.) hVtxPull[c]->Fill(residual[c] / sqrt(variance[c]));
    }

    // mass, its variance is the (m, m) element of the (x, y, z, px, py, pz, m) covariance
    const double massVariance = state.kinematicParametersError().matrix()(6, 6);
    hBsMassResidual->Fill(fitMass - genBsMass);
    if (massVariance > 0.) hBsMassPull->Fill((fitMass - genBsMass) / sqrt(massVariance));

    // fit quality (vertex fit and pointing constraint)
    const double chi2 = result.vertex->chiSquared();
    const double ndf = result.vertex->degreesOfFreedom();
    if (ndf > 0.) {
      hChi2Ndf->Fill(chi2 / ndf);
      hVtxProb->Fill(TMath::Prob(chi2, ndf));
    }
  }

  // photon measurement pulls, for the photons used in the fit
  for (unsigned int iPh = 0; iPh < recoMatchedPhotons.size(); ++iPh) {
    const reco::Photon& recoPh = *recoMatchedPhotons[iPh];
    const reco::Candidate* genPh = genMatchedPhotons[iPh];

    // The fit measures the photon direction: only the displacement of the cluster perpendicular
    // to the photon line enters it, the error along the line (shower depth) drops out of the
    // projected covariance. The pull is therefore taken in the plane perpendicular to the
    // generated photon, along e_phi = (-sin phi, cos phi, 0) and
    // e_theta = (cos theta cos phi, cos theta sin phi, -sin theta), the same way in EB and EE.
    const double d[3] = {recoPh.caloPosition().x() - genPh->vx(),
                         recoPh.caloPosition().y() - genPh->vy(),
                         recoPh.caloPosition().z() - genPh->vz()};
    const double phi = genPh->phi(), theta = genPh->theta();
    const double ePhi[3] = {-sin(phi), cos(phi), 0.};
    const double eTheta[3] = {cos(theta) * cos(phi), cos(theta) * sin(phi), -sin(theta)};

    // same covariance as used in the fit
    const TMatrixD clusterCov = mmgVertexFit.clusterCovariance(recoPh, lazyTools);
    auto pull = [&](const double* e) {
      double residual = 0., variance = 0.;
      for (int a = 0; a < 3; ++a) {
        residual += d[a] * e[a];
        for (int b = 0; b < 3; ++b) variance += e[a] * clusterCov(a, b) * e[b];
      }
      return variance > 0. ? residual / sqrt(variance) : std::numeric_limits<double>::quiet_NaN();
    };
    const double pullPhi = pull(ePhi), pullTheta = pull(eTheta);
    if (std::isfinite(pullPhi)) hClusterPullPhi->Fill(pullPhi);
    if (std::isfinite(pullTheta)) hClusterPullTheta->Fill(pullTheta);

    // same energy and uncertainty as used in the fit
    const double energyError = mmgVertexFit.photonEnergyError(recoPh);
    if (energyError > 0.) {
      const double energyPull = (recoPh.energy() - genPh->energy()) / energyError;
      hPhotonEnergyPull->Fill(energyPull);
      hPhotonEnergyPullVsE->Fill(genPh->energy(), energyPull);
    }
  }

  if(recoMatchedMuons.size() == 2 && recoMatchedPhotons.size() == 1) {
    TLorentzVector mu1, mu2, pho;
    mu1.SetPxPyPzE(recoMatchedMuons[0]->px(), recoMatchedMuons[0]->py(), recoMatchedMuons[0]->pz(), recoMatchedMuons[0]->energy());
    mu2.SetPxPyPzE(recoMatchedMuons[1]->px(), recoMatchedMuons[1]->py(), recoMatchedMuons[1]->pz(), recoMatchedMuons[1]->energy());
    pho.SetPxPyPzE(recoMatchedPhotons[0]->px(), recoMatchedPhotons[0]->py(), recoMatchedPhotons[0]->pz(), recoMatchedPhotons[0]->energy());
    TLorentzVector total = mu1 + mu2 + pho;
    hBsMassNoFit->Fill(total.M());
  }
  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(MMGVertexFitExample);

