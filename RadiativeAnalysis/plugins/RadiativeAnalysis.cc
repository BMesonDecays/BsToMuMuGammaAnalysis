// -*- C++ -*-
// Package:    BsToMuMuGammaAnalysis/RadiativeAnalysis
// Class:      RadiativeAnalysis
// Original Author:  Alibordi Muhammad
//         Created:  Fri, 12 Jul 2024 10:27:36 GMT
//============================================================
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeAnalysis.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TrippleObjectVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TetraObjectVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RecoPhotons.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ParticleFlowCandidate.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/SCRecHitAccumulator.h"

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceModeratorVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DecayChainVariables.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GenChain.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonSelector.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/PhotonSelector.h"





RadiativeAnalysis::RadiativeAnalysis(const edm::ParameterSet& iConfig): 
	theConfig_(iConfig),
	nominalJpsiMass( 3.096916 ),
	nominalPsiMass( 3.096916 ),
	nominalPhiMass(1.019 ),
	nominalElectronMass(0.00051099893),
	nominalMuonMass(0.1056583),
	nominalPiZeroMass(0.1349768),
	nominalEtaMesonMass(0.547862),
	nominalEtaPrimeMass(0.957780),
	nominalKaonMass(0.493677)
{
	generatorInfoTok                   = consumes<GenRunInfoProduct, edm::InRun>(edm::InputTag("generator"));
	isMCstudy_                        = iConfig.getParameter<bool>("isMCstudy");
	isMINIAOD_                        = iConfig.getParameter<bool>("isMINIAOD");
	genParticlesLabel                 = iConfig.getParameter<edm::InputTag>("genParticlesLabel");
	genParticlesTok                   = consumes<edm::View<reco::GenParticle>>(genParticlesLabel);
	MuonTag                           = iConfig.getParameter<edm::InputTag>("MuonTag");
	MuonTagTok                        = consumes<std::vector<reco::Muon>>(MuonTag);
	JetTag                            = iConfig.getParameter<edm::InputTag>("JetTag");
	JetTagTok                         = consumes<edm::View<pat::Jet>>(JetTag);
	PhotonTag                         = iConfig.getParameter<edm::InputTag>("PhotonTag");
	PhotonTagTok                      = consumes<std::vector<reco::Photon>>(PhotonTag);
	PFCandTag                         = iConfig.getParameter<edm::InputTag>("PFCandTag");
	PFCandTagTok                      = consumes<std::vector<reco::PFCandidate>>(PFCandTag);
	//PFCandTagTok                      = consumes<edm::View
	OOTPhotonTag                      = iConfig.getParameter<edm::InputTag>("OOTPhotonTag");
	OOTPhotonTagTok                   = consumes<edm::View<pat::Photon>>(OOTPhotonTag);
	ElectronTag                       = iConfig.getParameter<edm::InputTag>("ElectronTag");
	ElectronTagTok                    = consumes<edm::View<pat::Electron>>(ElectronTag);
	/*SuperClusterTag                   = iConfig.getParameter<edm::InputTag>("SuperClusterTag");
	SuperClusterTagTok                = consumes<edm::View<reco::SuperCluster>>(SuperClusterTag);
	OOTSuperClusterTag                = iConfig.getParameter<edm::InputTag>("OOTSuperClusterTag");
    OOTSuperClusterTagTok             = consumes<edm::View<reco::SuperCluster>>(OOTSuperClusterTag);
	*/
	PUInfo                            = iConfig.getParameter<edm::InputTag>("PUInfo");
	PUInfoTok                         = consumes<edm::View<PileupSummaryInfo>>(PUInfo);
	vertexBeamSpot                    = iConfig.getParameter<edm::InputTag>("vertexBeamSpot");
	vertexBeamSpotTok                 = consumes<reco::BeamSpot>(vertexBeamSpot);
	primaryvertex                     = iConfig.getParameter<edm::InputTag>("primaryvertex");
	primaryvertexTok                  = consumes<std::vector<reco::Vertex>>(primaryvertex);
	triggerbits                       = iConfig.getParameter<edm::InputTag>("triggerbits");
	triggerbitsTok                    = consumes<edm::TriggerResults>(triggerbits);
	if(isMINIAOD_){
		prescale                          = iConfig.getParameter<edm::InputTag>("prescale");
		prescaleTok                       = consumes<pat::PackedTriggerPrescales>(prescale);
		triggerobj                        = iConfig.getParameter<edm::InputTag>("triggerobj");
		triggerobjTok                     = consumes<edm::View<pat::TriggerObjectStandAlone>>(triggerobj);
	}
	triggerSummaryTag              = iConfig.getParameter<edm::InputTag>("triggerSummary");
	triggerSummaryTok              = consumes<trigger::TriggerEvent>(triggerSummaryTag);
	pfSupcluster                   = iConfig.getParameter<edm::InputTag>("pfSupcluster");
	pfSupclusterTok                = consumes<std::vector<reco::SuperCluster>>(pfSupcluster);
	ecalrechitEB                   = iConfig.getParameter<edm::InputTag>("ecalrechitEB");
	ecalrechitEBTok                = consumes<EcalRecHitCollection>(ecalrechitEB);
	ecalrechitEE                   = iConfig.getParameter<edm::InputTag>("ecalrechitEE");
	ecalrechitEETok                = consumes<EcalRecHitCollection>(ecalrechitEE);
	ecalrechitES                   = iConfig.getParameter<edm::InputTag>("ecalrechitES");
	ecalrechitESTok                = consumes<EcalRecHitCollection>(ecalrechitES);
	valMapTag					   = iConfig.getParameter<edm::InputTag>("mvaValuesMap");
	valMapTok					   = consumes<edm::ValueMap<float>>(valMapTag);
	valMapPhoTightTag				= iConfig.getParameter<edm::InputTag>("phoTightIDMap");
	valMapPhoTightTok				= consumes<edm::ValueMap<bool>>(valMapPhoTightTag);

	//pfCandTag                         = iConfig.getParameter<edm::InputTag>("pfCandTag");
    //pfCandTagTok                      = consumes<edm::View<pat::PackedCandidate>>(pfCandTag);
	trackTag                          = iConfig.getParameter<edm::InputTag>("pfCandTag");
	trackTagTok                       = consumes<std::vector<reco::Track>>(trackTag);
	if(isMINIAOD_){
		IsoTrackTag                   = iConfig.getParameter<edm::InputTag>("IsoTrackTag");
		IsoTrackTagTok                = consumes<edm::View<pat::IsolatedTrack>>(IsoTrackTag);
	}
	convertedPhotonsTag               = iConfig.getParameter<edm::InputTag>("convertedPhotons");
	convertedPhotonsTagTok            = consumes<std::vector<pat::CompositeCandidate>>(convertedPhotonsTag);
	trackBuilderTok                   = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));
	theBFieldTok                      = esConsumes<MagneticField, IdealMagneticFieldRecord>();
	caloGeomTok                       = esConsumes<CaloGeometry, CaloGeometryRecord>();
	iSetupGetTok                      = make_unique<EcalClusterLazyToolsBase::ESGetTokens>(consumesCollector());
	


	StoreDeDxInfo_                    = iConfig.getParameter<bool>("StoreDeDxInfo");
	PionZeroMassWindowNoFit_          = iConfig.getParameter<double>("PionZeroMassWindowNoFit");
	JpsiMassWindowBeforeFit_          = iConfig.getParameter<double>("JpsiMassWindowBeforeFit");
	JpsiMassWindowAfterFit_           = iConfig.getParameter<double>("JpsiMassWindowAfterFit");
	JpsiPtCut_                        = iConfig.getParameter<double>("JpsiPtCut");
	KaonTrackPtCut_                   = iConfig.getParameter<double>("KaonTrackPtCut");//https://arxiv.org/pdf/1307.2782.pdf
	PsiMassWindowBeforeFit_           = iConfig.getParameter<double>("PsiMassWindowBeforeFit");
	PsiMassWindowAfterFit_            = iConfig.getParameter<double>("PsiMassWindowAfterFit");
	PhiMassWindowBeforeFit_           = iConfig.getParameter<double>("PhiMassWindowBeforeFit");
	PhiMassWindowAfterFit_            = iConfig.getParameter<double>("PhiMassWindowAfterFit");
	EtaMesonMassWindowNoFit_          = iConfig.getParameter<double>("EtaMesonMassWindowNoFit");
	EtaPrimeMassWindowNoFit_          = iConfig.getParameter<double>("EtaPrimeMassWindowNoFit");
	BsLowerMassCutBeforeFit_          = iConfig.getParameter<double>("BsLowerMassCutBeforeFit");
	BsUpperMassCutBeforeFit_          = iConfig.getParameter<double>("BsUpperMassCutBeforeFit");
	BdLowerMassCutBeforeFit_          = iConfig.getParameter<double>("BdLowerMassCutBeforeFit");
	BdUpperMassCutBeforeFit_          = iConfig.getParameter<double>("BdUpperMassCutBeforeFit");
	BsLowerMassCutAfterFit_           = iConfig.getParameter<double>("BsLowerMassCutAfterFit");
	BsUpperMassCutAfterFit_           = iConfig.getParameter<double>("BsUpperMassCutAfterFit");
	BdPDGMass_                        = iConfig.getParameter<double>("BdPDGMass");
	BpPDGMass_                        = iConfig.getParameter<double>("BpPDGMass");
	BsPDGMass_                        = iConfig.getParameter<double>("BsPDGMass");
	PionZeroPDGMass_                  = iConfig.getParameter<double>("PionZeroPDGMass");
	EtaMesonPDGMass_                  = iConfig.getParameter<double>("EtaMesonPDGMass");
	EtaPrimePDGMass_                  = iConfig.getParameter<double>("EtaPrimePDGMass");
	outputFile_                       = iConfig.getUntrackedParameter<std::string>("outputFile");
	verbose_                          = iConfig.getParameter<bool>("verbose");
	TestVerbose_                      = iConfig.getParameter<bool>("TestVerbose");
	hltProcess_                       = iConfig.getUntrackedParameter<std::string>("HLTprocess","HLT");
	hltConfigInitialized_             = false;

	muMVACut_                         = iConfig.getParameter<double>("muMVACut");
	photonMVACut_                     = iConfig.getParameter<double>("photonMVACut");

	event_counter_ = 0;
	elecounter_    = 0;
	muoncounter_   = 0;
	jetcounter_    = 0;
	tagmucounter_  = 0;
	photoncounter_ = 0;

edm::LogInfo("BsToMuMuGammaAnalysis/RadiativeAnalysis")<< "Initializing Bs to MuMu Gamma  analyser  - Output file: " << outputFile_ <<"\n";
}

RadiativeAnalysis::~RadiativeAnalysis() {}

void RadiativeAnalysis::beginJob() {
  bmmgRootTree_ = new RadiativeRootTree();
  bmmgRootTree_->createTree(outputFile_);

  muonMVAIDProducer_ = new MuonMVAID(theConfig_);
}

void RadiativeAnalysis::endJob() {
  bmmgRootTree_->writeFile();
  delete bmmgRootTree_;

  delete muonMVAIDProducer_;
  cout << "Total number of Events          : " << event_counter_ << endl;
  cout << "Total number of Tagged muons    : " << muoncounter_   << endl;
  cout << "Total number of Tagged electrons: " << elecounter_    << endl;
  cout << "Total number of Tagged jets     : " << jetcounter_    << endl;
  cout << "Max amount of Tag muons         : " << tagmucounter_ <<  endl;
  cout << "Max number of photon            : " << photoncounter_ << endl;
}
void RadiativeAnalysis::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{
    bool changed = true;
    hltConfigInitialized_ = hltConfig_.init(iRun, iSetup, hltProcess_, changed);
    if (!hltConfigInitialized_) {
        edm::LogError("RadiativeAnalysis") 
            << "HLTConfigProvider::init failed for process: " << hltProcess_;
        return;
    }
    if (changed){
		 edm::LogInfo("RadiativeAnalysis")
            << "HLT menu changed. Table: " << hltConfig_.tableName();

	}
       
}
//------------Do not know when to use this function------------------
void RadiativeAnalysis::endRun(const edm::Run& iRun,const edm::EventSetup& iSetup)
{

	std::cout << ">>> before getByLabel" << std::endl;

   edm::Handle<GenRunInfoProduct> genRunInfo;
   iRun.getByToken(generatorInfoTok, genRunInfo);

    std::cout << ">>> after getByLabel" << std::endl;

    if (!genRunInfo.isValid()) {
      edm::LogWarning("GenRunInfo") << "GenRunInfoProduct not found!";
      return;
    }

     double genXsec_   = genRunInfo->internalXSec().value();
     double filterEff_ = genRunInfo->filterEfficiency();

    // IMPORTANT: GenRunInfoProduct does NOT store event counts.
    // This must be provided from event loop or counters elsewhere.
    double nEvents_ = 0.0;  // placeholder; must be filled externally

    double lumiEquivalent_ = -1.0;

	// Equivalent luminosity
	if (genXsec_ > 0 && filterEff_ > 0 && nEvents_ > 0)
	lumiEquivalent_ = nEvents_ / (genXsec_ * filterEff_);
	else
	lumiEquivalent_ = -1.0;

	std::cout << "GenXsec: " << genXsec_
			<< " FilterEff: " << filterEff_
			<< " nEvents: " << nEvents_
			<< " LumiEquivalent: " << lumiEquivalent_
			<< std::endl;

}
//------------- get filters for a given HLT path (helper function) -------------
std::vector<std::string>RadiativeAnalysis::getFiltersForPath(const std::string& pathName) const
{
    std::vector<std::string> filters;
    if (!hltConfigInitialized_) return filters;
	const std::vector<std::string>& modules = hltConfig_.moduleLabels(pathName);

    // Walk backwards through modules; collect terminal EDFilter-type modules.
    // Stop as soon as we leave the filter region (hit a producer going backward).
    for (int iMod = static_cast<int>(modules.size()) - 1; iMod >= 0; --iMod) {
        const std::string& mod     = modules[iMod];
        const std::string  modType = hltConfig_.moduleType(mod);
        const std::string  edmType = hltConfig_.moduleEDMType(mod);

        // Skip pure bookkeeping modules that never write TriggerObjects
        if (modType == "HLTBool" || modType == "TriggerResultsFilter") continue;
        if (edmType == "EDFilter") {
            filters.push_back(mod);
        } else {
            // Once we walk back into producer territory, stop
            if (!filters.empty()) break;
        }
    }
    return filters;
}
// ------------ method called for each event  ------------
void RadiativeAnalysis::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

	event_counter_++;
	excludedPhotons.clear();
	int nBs=0;
	bmmgRootTree_->resetEntries();
	

	bmmgRootTree_->runNumber_   = iEvent.id().run();
	bmmgRootTree_->eventNumber_ = (unsigned int)iEvent.id().event();
	bmmgRootTree_->lumiSection_ = iEvent.luminosityBlock();
	
	if(isMCstudy_){
		edm:: Handle<edm::View<PileupSummaryInfo> > PUinfo;
		iEvent.getByToken( PUInfoTok, PUinfo);
		edm::View<PileupSummaryInfo>::const_iterator PVI;
		int numInteraction = 0;
		int numTrueInteraction =0;
		for(PVI = PUinfo->begin(); PVI != PUinfo->end(); ++PVI){
			if (PVI->getBunchCrossing()==0){
				numTrueInteraction += PVI->getTrueNumInteractions();
				numInteraction += PVI->getPU_NumInteractions();
				}
				}
				bmmgRootTree_->PUinteraction_ = numInteraction;
				bmmgRootTree_->PUTrueinteraction_ = numTrueInteraction;
			}	
	
	
	
	const auto& theBField             = iSetup.getData(theBFieldTok);
	const auto& caloGeom              = iSetup.getData(caloGeomTok);
	const auto& lazyTools             = EcalClusterLazyTools(iEvent, iSetupGetTok->get(iSetup), ecalrechitEBTok, ecalrechitEETok);
	const auto& trackBuilder          = iSetup.getData(trackBuilderTok);
	edm::Handle<reco::BeamSpot> vertexBeamSpot ;
	iEvent.getByToken(vertexBeamSpotTok,vertexBeamSpot);
	edm::Handle<std::vector<reco::Vertex>> recVtxs;
	iEvent.getByToken(primaryvertexTok, recVtxs);
	if (!vertexBeamSpot.isValid() || recVtxs->empty()) {return;}
	edm::Handle<std::vector<reco::Muon>> muons;
        iEvent.getByToken(MuonTagTok, muons);
	edm::Handle<std::vector<pat::CompositeCandidate>> convPhotons;
        iEvent.getByToken(convertedPhotonsTagTok, convPhotons);
	const pat::CompositeCandidateCollection * conversions = convPhotons.product();
	edm::Handle<std::vector<reco::Track>> tracks;
	iEvent.getByToken(trackTagTok, tracks);
	//particle flow supercluster ECAL - This only does exists in AOD ???
	edm::Handle<std::vector<reco::SuperCluster>> supercluster;
	iEvent.getByToken(pfSupclusterTok, supercluster);
	edm::Handle<EcalRecHitCollection> ecalRecHits;
	iEvent.getByToken(ecalrechitEBTok, ecalRecHits);
	//std::cout<< " super cluster multiplicity : "<< supercluster->size()<< "\n";
	edm::Handle<std::vector<reco::Photon>> photons;
    iEvent.getByToken(PhotonTagTok, photons);
	edm::Handle<std::vector<reco::PFCandidate>> pfCandidates;
	iEvent.getByToken(PFCandTagTok, pfCandidates);

	edm::Handle<edm::ValueMap<float>> mvaValuesMap;
	iEvent.getByToken(valMapTok, mvaValuesMap);

	edm::Handle<edm::ValueMap<bool>> phoTightIDMap;
	iEvent.getByToken(valMapPhoTightTok, phoTightIDMap);


	bmmgRootTree_->nMuons_ = muons->size();
	bmmgRootTree_->nPhotons_ = photons->size();
	bmmgRootTree_->nConversions_ = conversions->size();
	bmmgRootTree_->nTracks_ = tracks->size();
    

	BeamSpotAndVertex bsandvtxObservables;
	auto bsandvtxVar = bsandvtxObservables.BSAndVtxObservables(*vertexBeamSpot, *recVtxs);
	bmmgRootTree_->NTracksPVbeforecut_ = bsandvtxVar.pv_ntracks;
	bmmgRootTree_->NVerticesbeforecut_ = bsandvtxVar.pv_multiplicity;
	for (int i = 0; i < 9; ++i) {bmmgRootTree_->PVcovariance_[i] = bsandvtxVar.pv_covariance[i];}
	bmmgRootTree_->PVndof_ = bsandvtxVar.pv_ndof;
	bmmgRootTree_->PVrho_ = bsandvtxVar.pv_rho;
	bmmgRootTree_->NVerticesaftercut_ = bsandvtxVar.pv_cutmultiplicity;
	bmmgRootTree_->getVtx(bsandvtxVar.bs_x, bsandvtxVar.bs_y, bsandvtxVar.bs_z,
                      bsandvtxVar.pv_x, bsandvtxVar.pv_y, bsandvtxVar.pv_z,
                      bsandvtxVar.pv_errx, bsandvtxVar.pv_erry, bsandvtxVar.pv_errz);
	bmmgRootTree_->BSdx_       = bsandvtxVar.bs_dx;
	bmmgRootTree_->BSdy_       = bsandvtxVar.bs_dy;
	bmmgRootTree_->BSdz_       = bsandvtxVar.bs_dz;
	bmmgRootTree_->BSsigmaZ_   = bsandvtxVar.bs_sigmaZ;
	bmmgRootTree_->BSdsigmaZ_  = bsandvtxVar.bs_dsigmaZ;
	bmmgRootTree_->BSdxdz_     = bsandvtxVar.bs_dxdz;
	bmmgRootTree_->BSdydz_     = bsandvtxVar.bs_dydz;



	/*for (const auto& vertex : *recVtxs) {
		for (auto trackRef = vertex.tracks_begin(); trackRef != vertex.tracks_end(); ++trackRef) {
			std::cout << "Referenced track key: " << trackRef->key() << "\n";
			std::cout << "Referenced track weight: " << vertex.trackWeight(*trackRef) << "\n";
		}
	}*/


	    //edm::ESHandle<TransientTrackBuilder> theB;
        //iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theB);
        //const auto& trackBuilder = iSetup.getData(trackBuilderTok);
       	





     // =========================================================
    // TRIGGER BITS — path-level decisions (unchanged logic,
    // substring-collision bugs fixed with _v anchor)
    // =========================================================


    edm::Handle<edm::TriggerResults> hltbits;
    iEvent.getByToken(triggerbitsTok, hltbits);

    static const std::vector<std::string> triggersOfInterest = {
        "HLT_DoubleMu4_3_Bs",
        "HLT_DoubleMu4_3_Jpsi",
        "HLT_DoubleMu4_3_LowMass",
        "HLT_DoubleMu4_LowMass_Displaced",
        "HLT_DoubleMu4_3_Photon4_BsToMMG",
        "HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG",
        "HLT_DoubleMu4_JpsiTrkTrk_Displaced",
        "HLT_DoubleMu4_Jpsi_NoVertexing",
        "HLT_DoubleMu4_Jpsi_Displaced",
    };

    const edm::TriggerNames& triggerNames_ = iEvent.triggerNames(*hltbits);

    if (isMINIAOD_) {
        edm::Handle<edm::View<pat::TriggerObjectStandAlone>> triggerObjects;
        iEvent.getByToken(triggerobjTok, triggerObjects);
        edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;
        iEvent.getByToken(prescaleTok, triggerPrescales);
        if (!triggerPrescales.isValid()) {
            edm::LogError("TriggerPrescales") << "Trigger prescales handle invalid!";
            return;
        }
        pat::PackedTriggerPrescales nonConstTriggerPrescales = *triggerPrescales;
        nonConstTriggerPrescales.setTriggerNames(triggerNames_);
    }

    // --- path-level bit saving ---
    // Note: longer paths checked BEFORE shorter substrings to prevent
    // "HLT_DoubleMu4_3_LowMass" matching inside "..._Displaced" etc.
    const int ntrigs = hltbits->size();
    for (int itrig = 0; itrig != ntrigs; ++itrig) {
        const std::string trigName = triggerNames_.triggerName(itrig);
        // anchor to _v so "LowMass" cannot fire "LowMass_Displaced"
        if (trigName.find("HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_ = hltbits->accept(itrig);
        else if (trigName.find("HLT_DoubleMu4_3_Photon4_BsToMMG_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_           = hltbits->accept(itrig);
        else if (trigName.find("HLT_DoubleMu4_LowMass_Displaced_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLT_DoubleMu4_LowMass_Displaced_           = hltbits->accept(itrig);
        else if (trigName.find("HLT_DoubleMu4_3_LowMass_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLT_DoubleMu4_LowMass_                     = hltbits->accept(itrig);
        else if (trigName.find("HLT_DoubleMu4_3_Bs_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLT_DoubleMu4_3_Bs_                        = hltbits->accept(itrig);
        else if (trigName.find("HLT_DoubleMu4_JpsiTrkTrk_Displaced_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_             = hltbits->accept(itrig);
        else if (trigName.find("HLT_DoubleMu4_Jpsi_Displaced_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLTDimuon4JpsiDisplaced_                   = hltbits->accept(itrig);
        else if (trigName.find("HLT_DoubleMu4_Jpsi_NoVertexing_v") != std::string::npos)
            bmmgRootTree_->triggerbit_HLTDimuon4JpsiNoVertexing_                 = hltbits->accept(itrig);
       
    }



    // =========================================================
    // TRIGGER OBJECT MATCHING — AOD path, no hardcoded filters
    // =========================================================
    muTrigMatchMap_.clear();

    edm::Handle<trigger::TriggerEvent> trigSummary;
    iEvent.getByToken(triggerSummaryTok, trigSummary);

    if (!trigSummary.isValid()) {
        edm::LogWarning("RadiativeAnalysis") 
            << "TriggerEvent handle invalid — skipping object matching";
    } else if (hltConfigInitialized_) {

        const trigger::TriggerObjectCollection& allObjects = trigSummary->getObjects();

        // --- Step A: for each path of interest, resolve versioned name,
        //             extract filters intrinsically, collect trigger objects ---
        // map: base path name -> [one TriggerObjectCollection per filter leg]
        std::map<std::string,
                 std::vector<trigger::TriggerObjectCollection>> pathFilterObjects;

        for (const std::string& basePath : triggersOfInterest) {

            // Resolve versioned path name from this event's menu
            std::string resolvedPath;
            for (int itrig = 0; itrig != ntrigs; ++itrig) {
                const std::string name = triggerNames_.triggerName(itrig);
                if (name.find(basePath + "_v") != std::string::npos) {
                    resolvedPath = name;
                    break;
                }
            }
            if (resolvedPath.empty()) continue; // not in this menu

            // Intrinsically get filter labels from HLTConfigProvider
            const std::vector<std::string> filterLabels =
                getFiltersForPath(resolvedPath);

            // Retrieve TriggerObjects for each filter leg
            std::vector<trigger::TriggerObjectCollection> filterObjsForPath;
            filterObjsForPath.reserve(filterLabels.size());

            for (const std::string& filterLabel : filterLabels) {
                trigger::TriggerObjectCollection filterObjs;
                const edm::InputTag filterTag(filterLabel, "", hltProcess_);
                const size_t filterIdx = trigSummary->filterIndex(filterTag);
                if (filterIdx < trigSummary->sizeFilters()) {
                    const trigger::Keys& keys = trigSummary->filterKeys(filterIdx);
                    filterObjs.reserve(keys.size());
                    for (size_t k = 0; k < keys.size(); ++k)
                        filterObjs.push_back(allObjects[keys[k]]);
                }
                filterObjsForPath.push_back(std::move(filterObjs));
            }
            pathFilterObjects[basePath] = std::move(filterObjsForPath);
        }

        // --- Step B: per-muon ΔR matching ---
        // A muon matches a path iff it ΔR-matches ≥1 object in EVERY
        // filter leg of that path (AND logic across legs)
        const size_t nMuons = muons->size();

        for (const std::string& basePath : triggersOfInterest) {
            std::vector<bool> muMatchVec(nMuons, false);

            auto it = pathFilterObjects.find(basePath);
            if (it != pathFilterObjects.end() && !it->second.empty()) {
                const auto& filterLegs = it->second;

                for (size_t iMu = 0; iMu < nMuons; ++iMu) {
                    const reco::Muon& mu = (*muons)[iMu];
                    bool matchedAllLegs  = true;

                    for (const auto& legObjs : filterLegs) {
                        bool matchedThisLeg = false;
                        for (const trigger::TriggerObject& tObj : legObjs) {
                            if (reco::deltaR(mu.eta(), mu.phi(),
                                             tObj.eta(), tObj.phi()) < 0.1) {
                                matchedThisLeg = true;
                                break;
                            }
                        }
                        if (!matchedThisLeg) { matchedAllLegs = false; break; }
                    }
                    muMatchVec[iMu] = matchedAllLegs;
                }
            }
            muTrigMatchMap_[basePath] = std::move(muMatchVec);
        }

        // --- Step C: write per-muon match booleans into ntuple ---
        // (clear vectors then fill per muon — matches your existing push_back style)
        bmmgRootTree_->trigMatch_DoubleMu4_3_Bs_->clear();
        bmmgRootTree_->trigMatch_DoubleMu4_3_LowMass_->clear();
        bmmgRootTree_->trigMatch_DoubleMu4_LowMass_Displaced_->clear();
        bmmgRootTree_->trigMatch_DoubleMu4_3_Photon4_BsToMMG_->clear();
        bmmgRootTree_->trigMatch_DoubleMu4_3_Displaced_Photon4_BsToMMG_->clear();
        bmmgRootTree_->trigMatch_DoubleMu4_JpsiTrkTrk_Displaced_->clear();
        bmmgRootTree_->trigMatch_DoubleMu4_Jpsi_NoVertexing_->clear();
        bmmgRootTree_->trigMatch_DoubleMu4_Jpsi_Displaced_->clear();

        auto fillMatch = [&](const std::string& path, std::vector<int>* branch) {
            auto it = muTrigMatchMap_.find(path);
            const size_t nMu = muons->size();
            for (size_t iMu = 0; iMu < nMu; ++iMu)
                branch->push_back(
                    (it != muTrigMatchMap_.end() && it->second[iMu]) ? 1 : 0);
        };

        fillMatch("HLT_DoubleMu4_3_Bs",
                   bmmgRootTree_->trigMatch_DoubleMu4_3_Bs_);
        fillMatch("HLT_DoubleMu4_3_LowMass",
                   bmmgRootTree_->trigMatch_DoubleMu4_3_LowMass_);
        fillMatch("HLT_DoubleMu4_LowMass_Displaced",
                   bmmgRootTree_->trigMatch_DoubleMu4_LowMass_Displaced_);
        fillMatch("HLT_DoubleMu4_3_Photon4_BsToMMG",
                   bmmgRootTree_->trigMatch_DoubleMu4_3_Photon4_BsToMMG_);
        fillMatch("HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG",
                   bmmgRootTree_->trigMatch_DoubleMu4_3_Displaced_Photon4_BsToMMG_);
        fillMatch("HLT_DoubleMu4_JpsiTrkTrk_Displaced",
                   bmmgRootTree_->trigMatch_DoubleMu4_JpsiTrkTrk_Displaced_);
        fillMatch("HLT_DoubleMu4_Jpsi_NoVertexing",
                   bmmgRootTree_->trigMatch_DoubleMu4_Jpsi_NoVertexing_);
        fillMatch("HLT_DoubleMu4_Jpsi_Displaced",
                   bmmgRootTree_->trigMatch_DoubleMu4_Jpsi_Displaced_);

    } // end trigSummary.isValid() && hltConfigInitialized_





        
	    //edm::Handle<View<pat::PackedCandidate>> pfCands;
	    //iEvent.getByToken(pfCandTagTok, pfCands);

		// muons mva id scores
		std::vector<float> muonMVAIDs = muonMVAIDProducer_->produce(*muons);
		// photon mva id scores
		std::vector<float> photonMVAIDs;
		photonMVAIDs.reserve(photons->size());
		if(mvaValuesMap.isValid()){
			for(size_t i = 0; i < photons->size(); ++i){
				const edm::Ptr<reco::Photon> phoPtr(photons, i);
				float phoMva = -99.f;
				if (mvaValuesMap->contains(phoPtr.id())) {
					phoMva = (*mvaValuesMap)[phoPtr];
				} else if (mvaValuesMap->idSize() == 1 && phoPtr.id() == edm::ProductID() && phoPtr.key() < mvaValuesMap->size()) {
					phoMva = mvaValuesMap->begin()[phoPtr.key()];
				} else {
					edm::LogWarning("RadiativeAnalysis")
						<< "Missing compatible photon MVA entry for photon index " << i
						<< " in run:lumi:event " << iEvent.id().run() << ":"
						<< iEvent.luminosityBlock() << ":" << iEvent.id().event();
				}
				photonMVAIDs.push_back(phoMva);
			}
		} else {
			edm::LogError("RadiativeAnalysis") << "Failed to retrieve MVA values map for photons.";
			photonMVAIDs.assign(photons->size(), -99.f);
		}
		


		// select 2 muons with the highest vertex probability and opposite charge
		vector<reco::Muon>* selectedMuons = new vector<reco::Muon>();
		MuonSelector muonSelector;
		std::pair<std::vector<reco::Muon>, std::vector<float>> selMuonsAndMVA = muonSelector.selectMuonPair(*muons, trackBuilder, *vertexBeamSpot, muonMVAIDs, muMVACut_);
		*selectedMuons = selMuonsAndMVA.first;
		std::vector<float> selectedMuonMVA = selMuonsAndMVA.second;
		
		bmmgRootTree_->mu1MVAScore_ = selectedMuonMVA.empty() ? -99.0 : selectedMuonMVA[0];
		bmmgRootTree_->mu2MVAScore_ = selectedMuonMVA.size() < 2 ? -99.0 : selectedMuonMVA[1];

		PhotonSelector photonSelector;
		vector<pat::CompositeCandidate>* selectedConvPhoton = new vector<pat::CompositeCandidate>();
		*selectedConvPhoton = photonSelector.selectConvertedPhoton(*convPhotons, *selectedMuons, trackBuilder);

		vector<reco::Photon>* selectedPhoton = new vector<reco::Photon>();
		std::pair<std::vector<reco::Photon>, std::vector<double>> selPhotonAndMVA = photonSelector.selectPhoton(*photons, *selectedMuons, trackBuilder, photonMVAIDs, photonMVACut_);
		*selectedPhoton = selPhotonAndMVA.first;
		std::vector<double> selectedPhotonMVA = selPhotonAndMVA.second;

		bmmgRootTree_->GammaMVAScore_mmrecog_ = selectedPhotonMVA.empty() ? -99.0 : selectedPhotonMVA[0];

		std::cout << "Selected Muons: " << selectedMuons->size() << ", Selected Photons: " << selectedPhoton->size() << std::endl;
	   
		DecayChainVariables decayVariables;
		if(selectedMuons->size()>=2 && (selectedConvPhoton->size() >=1 || selectedPhoton->size() >=1)){
			TrippleObjectVertex  tripvtxObservables;
			decayVariables = tripvtxObservables.TrippleObjectVertexObservables(
				*selectedMuons, 
				*selectedPhoton,
			    *recVtxs,	
				lazyTools, 
				*selectedConvPhoton, 
				bsandvtxVar, 
				theBField, 
				nominalMuonMass, 
				nominalElectronMass, 
				trackBuilder,
				bmmgRootTree_);
		        bmmgRootTree_->vertexTypeFlag_ = 1;

		}

		vector<pat::CompositeCandidate>* selectedConvPhotons = new vector<pat::CompositeCandidate>();
		*selectedConvPhotons = photonSelector.selectConvertedPhotons(*convPhotons, *selectedMuons, trackBuilder);

		vector<reco::Photon>* selectedPhotons = new vector<reco::Photon>();
		std::pair<std::vector<reco::Photon>, std::vector<double>> selPhotonsAndMVA = photonSelector.selectPhotons(*photons, *selectedMuons, trackBuilder, photonMVAIDs, photonMVACut_);
		*selectedPhotons = selPhotonsAndMVA.first;
		std::vector<double> selectedPhotonsMVA = selPhotonsAndMVA.second;
		bmmgRootTree_->Gamma1MVAScore_mmrecogg_ = selectedPhotonsMVA.size() < 2 ? -9999999 : selectedPhotonsMVA[0];
		bmmgRootTree_->Gamma2MVAScore_mmrecogg_ = selectedPhotonsMVA.size() < 2 ? -9999999 : selectedPhotonsMVA[1];

		if(selectedMuons->size()>=2 && (selectedConvPhotons->size() >=2 || selectedPhotons->size() >=2)){
			TetraObjectVertex tetradcObservables;
			decayVariables = tetradcObservables.TetraObjectVertexObservables(
				*selectedMuons, 
				*selectedPhotons,
			    *recVtxs,	
				lazyTools, 
				*selectedConvPhotons, 
				bsandvtxVar, 
				theBField, 
				nominalMuonMass, 
				nominalElectronMass, 
				trackBuilder,
				bmmgRootTree_);
		        bmmgRootTree_->vertexTypeFlag_ = 2;
		}

		// if(selectedMuons->size()==2 && tracks->size()>=2){
		// 	ReferenceModeratorVertex refmodvtxObservables;
		// 	decayVariables = refmodvtxObservables.ReferenceModeratorVertexObservables(*selectedMuons, *tracks, bsandvtxVar, theBField, 
		// 	nominalMuonMass, nominalKaonMass, bmmgRootTree_);
		// 	bmmgRootTree_->vertexTypeFlag_ = 3;
		// }

		
        
		
		bmmgRootTree_->DiMuonM_beffit_       = decayVariables.dimuonMass;
		bmmgRootTree_->DiMuonEta_beffit_     = decayVariables.dimuonEta;
		bmmgRootTree_->DiMuonPhi_beffit_     = decayVariables.dimuonPhi;
		bmmgRootTree_->DiMuonPt_beffit_      = decayVariables.dimuonPt;
		bmmgRootTree_->DiMuon_ResonanceType_ = decayVariables.resonanceFlag;
		bmmgRootTree_->DiMuon_vtxProb_       = decayVariables.dimuonvtxprob;
		bmmgRootTree_->DiMuon_CosineAlpha_   = decayVariables.opening_angle;
		bmmgRootTree_->DiMuon_DCA_           = decayVariables.mumudca;
		bmmgRootTree_->DiMuon_Chi2pv_KVFvtx_ = decayVariables.dimuonchi2;
		bmmgRootTree_->DiMuon_Mahalanobis_   = decayVariables.mahalanobis;
		bmmgRootTree_->DiMuon_Lxy_           = decayVariables.dimuonlxy;
		bmmgRootTree_->DiMuon_Lxyerr_        = decayVariables.dimuonlxyerr;
		bmmgRootTree_->DiMuon_LxyOverPt_     = decayVariables.dimuonlxyOverPt;
		bmmgRootTree_->mu1Pt_beffit_         = decayVariables.mu1pt;
		bmmgRootTree_->mu1Pz_beffit_         = decayVariables.mu1pz;
		bmmgRootTree_->mu1Eta_beffit_        = decayVariables.mu1eta;
		bmmgRootTree_->mu1Phi_beffit_        = decayVariables.mu1phi;
		bmmgRootTree_->mu1Energy_beffit_     = decayVariables.mu1energy;
		bmmgRootTree_->mu2Phi_beffit_        = decayVariables.mu2phi;
		bmmgRootTree_->mu2Pt_beffit_         = decayVariables.mu2pt;
		bmmgRootTree_->mu2Pz_beffit_         = decayVariables.mu2pz;
		bmmgRootTree_->mu2Eta_beffit_        = decayVariables.mu2eta;
		bmmgRootTree_->mu2Energy_beffit_     = decayVariables.mu2energy;
		bmmgRootTree_->MuonPairDR_           = decayVariables.muonpairdr;
		bmmgRootTree_->Mu1TrkBSDxy_          = decayVariables.mu1trkbsxy;
		bmmgRootTree_->Mu1TrkBSDz_           = decayVariables.mu1trkbsz;
		bmmgRootTree_->Mu2TrkBSDxy_          = decayVariables.mu2trkbsxy;
		bmmgRootTree_->Mu2TrkBSDz_           = decayVariables.mu2trkbsz;
		bmmgRootTree_->Mu1PixelHits_         = decayVariables.mu1pixelhits;
		bmmgRootTree_->Mu1TrackerHits_       = decayVariables.mu1trackerhits;
		bmmgRootTree_->Mu1isGood_            = decayVariables.mu1isgood;
		bmmgRootTree_->Mu1InnerTrkHighQuality_   = decayVariables.mu1innertrkhq;
		bmmgRootTree_->Mu2PixelHits_             = decayVariables.mu2pixelhits;
		bmmgRootTree_->Mu2TrackerHits_           = decayVariables.mu2trackerhits;
		bmmgRootTree_->Mu2isGood_                = decayVariables.mu2isgood;
		bmmgRootTree_->Mu2InnerTrkHighQuality_   = decayVariables.mu2innertrkhq;
		bmmgRootTree_->DiMuon_mu1Cat_alone_      = decayVariables.diMuon_mu1Cat;
		bmmgRootTree_->DiMuon_mu2Cat_alone_      = decayVariables.diMuon_mu2Cat;
		bmmgRootTree_->DiMuon_mu1nPixHits_alone_ = decayVariables.diMuon_mu1PixelHits;
		bmmgRootTree_->DiMuon_mu2nPixHits_alone_ = decayVariables.diMuon_mu2PixelHits;
		// mu mu conv gamma variables
		bmmgRootTree_->HadronMass_fromVertexFitConPhoton_  = decayVariables.fittedBmassConvertedPhoton;
		bmmgRootTree_->HadronMass_fromVertexFitRecoPhoton_ = decayVariables.fittedBmassRecoPhoton;
	
		  /*Task reminder here*/
		//1. for soft MVA we need L1 info in the samples , this will exploit composite candidate instead regional muon candidates + muons 
		//2. Ecal  Lazy tools covarinace matrix from photon/or ecal rechits for kinematic fit - this must go to photon class 
		//3. The mock decay vertex from Ecal RecHit time 
		//4. Helicity and angle between production plane and decay plane 
		//5. Photon BDT ID 


       



	   
	   SCRecHitAccumulator scraccumulator;
	   SCRecHitAccumulator::SCAndRecHitVariables screchitvars = scraccumulator.SCAndRecHitObservables(*supercluster,*ecalRecHits, bsandvtxVar,caloGeom);
	   bmmgRootTree_->PFECal_SC_Eta_                = screchitvars.sc_eta;
	   bmmgRootTree_->PFECal_SC_Phi_                = screchitvars.sc_phi;
	   bmmgRootTree_->PFECal_SC_EtaWidth_           = screchitvars.sc_eta_width;
	   bmmgRootTree_->PFECal_SC_PhiWidth_           = screchitvars.sc_phi_width;
	   bmmgRootTree_->PFECal_SC_ClusterEnergy_      = screchitvars.cluster_energy;
	   bmmgRootTree_->PFECAL_RecHit_RawId_          = screchitvars.rechit_rawid;
	   bmmgRootTree_->PFECAL_RecHit_Subdet_         = screchitvars.rechit_subdet;
	   bmmgRootTree_->PFECAL_RecHit_Energy_         = screchitvars.rechit_energy;
	   bmmgRootTree_->PFECAL_RecHit_Time_           = screchitvars.rechit_time;
	   bmmgRootTree_->PFECAL_RecHit_X_              = screchitvars.rechit_x;
	   bmmgRootTree_->PFECAL_RecHit_Y_              = screchitvars.rechit_y;
	   bmmgRootTree_->PFECAL_RecHit_Z_              = screchitvars.rechit_z;
	   bmmgRootTree_->PFECAL_RecHit_PVDistance_     = screchitvars.rechit_pv_distance;
	   bmmgRootTree_->PFECAL_RecHit_TOF_            = screchitvars.rechit_tof;
	   bmmgRootTree_->PFECAL_RecHit_CorrectedTime_  = screchitvars.rechit_corrected_time;
	   bmmgRootTree_->PFECAL_RecHit_IsWeird_        = screchitvars.rechit_is_weird;
	   bmmgRootTree_->PFECAL_RecHit_IsProblematic_  = screchitvars.rechit_is_problematic;
	   bmmgRootTree_->PFECAL_RecHit_PoorReco_       = screchitvars.rechit_poor_reco;
	   bmmgRootTree_->PFECAL_RecHit_EB_ieta_        = screchitvars.rechit_EB_ieta;
	   bmmgRootTree_->PFECAL_RecHit_EB_iphi_        = screchitvars.rechit_EB_iphi;	
	   bmmgRootTree_->PFECAL_RecHit_EE_ix_          = screchitvars.rechit_EE_ix;
	   bmmgRootTree_->PFECAL_RecHit_EE_iy_          = screchitvars.rechit_EE_iy;
	   bmmgRootTree_->PFECAL_RecHit_EE_zside_       = screchitvars.rechit_EE_zside;


	   muonleg1.SetPtEtaPhiE(decayVariables.mu1pt, decayVariables.mu1eta, decayVariables.mu1phi, decayVariables.mu1energy);
	   muonleg2.SetPtEtaPhiE(decayVariables.mu2pt, decayVariables.mu2eta, decayVariables.mu2phi, decayVariables.mu2energy);
	   mu1vec.SetPtEtaPhi(decayVariables.mu1pt, decayVariables.mu1eta, decayVariables.mu1phi);
	   mu2vec.SetPtEtaPhi(decayVariables.mu2pt, decayVariables.mu2eta, decayVariables.mu2phi);

	   


//================================================== Photon Observables =================================================//
// We could actually take converted and reco photon 


	   bmmgRootTree_->photonMultiplicity_ = photons->size();
	   RecoPhotons recoPhotonObserbles;
	   std::vector<RecoPhotons::PhotonVariables> photonVar = recoPhotonObserbles.PhotonObservables(*photons);
		if (!photonVar.empty()) {
		for (size_t iPhoton = 0; iPhoton < photonVar.size(); ++iPhoton) {
			//std::cout << "size of photon collection : " << photonVar.size() << "\n";
			const auto& ipatPhoton = (*photons)[iPhoton];
			if (ipatPhoton.pt() > 50.0 || !ipatPhoton.isEB()) {
				excludedPhotons.insert(iPhoton);
			}
		}

			// Store photon kinematics (up to 2 photons)
			for (size_t iPhoton = 0; iPhoton < photonVar.size(); ++iPhoton) {
				if (excludedPhotons.count(iPhoton)) continue;
				bmmgRootTree_->photonPt_[iPhoton] = photonVar[iPhoton].pt;
				bmmgRootTree_->photonEta_[iPhoton] = photonVar[iPhoton].eta;
				bmmgRootTree_->photonPhi_[iPhoton] = photonVar[iPhoton].phi;
				bmmgRootTree_->photonEnergy_[iPhoton] = photonVar[iPhoton].energy;
				bmmgRootTree_->photonET_[iPhoton] = photonVar[iPhoton].et;
				bmmgRootTree_->photonSSSigmaiEtaiEta_[iPhoton] = photonVar[iPhoton].sigmaIEtaIEta;
				bmmgRootTree_->photonSSSigmaiEtaiPhi_[iPhoton] = photonVar[iPhoton].sigmaIEtaIPhi;
				bmmgRootTree_->photonSSSigmaiPhiiPhi_[iPhoton] = photonVar[iPhoton].sigmaIPhiIPhi;
				bmmgRootTree_->photonSCEnergy_[iPhoton] = photonVar[iPhoton].scEnergy;
				bmmgRootTree_->photonSCRawEnergy_[iPhoton] = photonVar[iPhoton].scRawEnergy;
				bmmgRootTree_->photonSCR9_[iPhoton] = photonVar[iPhoton].r9;
				bmmgRootTree_->photonSCHadTowOverEm_[iPhoton] = photonVar[iPhoton].hadTowOverEm;
				bmmgRootTree_->photonSShcalDepth1OverEcal_[iPhoton] = photonVar[iPhoton].hcalDepth1OverEcal;
				bmmgRootTree_->photonSShcalDepth2OverEcal_[iPhoton] = photonVar[iPhoton].hcalDepth2OverEcal;
				bmmgRootTree_->photonSShcalDepth1OverEcalBc_[iPhoton] = photonVar[iPhoton].hcalDepth1OverEcalBc;
				bmmgRootTree_->photonSShcalDepth2OverEcalBc_[iPhoton] = photonVar[iPhoton].hcalDepth2OverEcalBc;
				/*std::fill(std::begin(bmmgRootTree_->photonSShcalOverEcal_), std::end(bmmgRootTree_->photonSShcalOverEcal_), 0.f);
				std::fill(std::begin(bmmgRootTree_->photonSShcalOverEcalBc_), std::end(bmmgRootTree_->photonSShcalOverEcalBc_), 0.f);
				for (size_t k = 0; k < photonVar[iPhoton].hcalOverEcal.size(); ++k) {
					bmmgRootTree_->photonSShcalOverEcal_[k][iPhoton] = photonVar[iPhoton].hcalOverEcal[k];
				}
				for (size_t k = 0; k < photonVar[iPhoton].hcalOverEcalBc.size(); ++k) {
					bmmgRootTree_->photonSShcalOverEcalBc_[k][iPhoton] = photonVar[iPhoton].hcalOverEcalBc[k];
				}*/
				for (size_t k = 0; k < photonVar[iPhoton].hcalOverEcal.size(); ++k) {
					bmmgRootTree_->photonSShcalOverEcal_[k][iPhoton] = static_cast<double>(photonVar[iPhoton].hcalOverEcal[k]);
				}
				for (size_t k = 0; k < photonVar[iPhoton].hcalOverEcalBc.size(); ++k) {
					bmmgRootTree_->photonSShcalOverEcalBc_[k][iPhoton] = static_cast<double>(photonVar[iPhoton].hcalOverEcalBc[k]);
				}
				bmmgRootTree_->photonSSmaxEnergyXtal_[iPhoton] = photonVar[iPhoton].maxEnergyXtal;
				bmmgRootTree_->photonSSeffSigmaRR_[iPhoton] = photonVar[iPhoton].effSigmaRR;
				bmmgRootTree_->photonSCEta_[iPhoton] = photonVar[iPhoton].scEta;
				bmmgRootTree_->photonSCPhi_[iPhoton] = photonVar[iPhoton].scPhi;
				bmmgRootTree_->photonSCEtaWidth_[iPhoton] = photonVar[iPhoton].scEtaWidth;
				bmmgRootTree_->photonSCPhiWidth_[iPhoton] = photonVar[iPhoton].scPhiWidth;
				bmmgRootTree_->photonSCBrem_[iPhoton] = photonVar[iPhoton].scPhiWidth / photonVar[iPhoton].scEtaWidth;
				photonleg1.SetPtEtaPhiE(photonVar[iPhoton].pt, photonVar[iPhoton].eta, photonVar[iPhoton].phi, photonVar[iPhoton].energy);
				TLorentzVector bslegTrippleObject = muonleg1 + muonleg2 + photonleg1;
				double M = bslegTrippleObject.M();
				bool inBs = (M >= BsLowerMassCutBeforeFit_ && M <= BsUpperMassCutBeforeFit_);
				bool inBd = (M >= BdLowerMassCutBeforeFit_ && M <= BdUpperMassCutBeforeFit_);
				if (!inBs && !inBd) continue;   // reject if in neither Bs nor Bd
				//mutually exclusive assignment
				if (inBs && !inBd) bmmgRootTree_->BmesonType_ = 1; // Bs only
				else if (inBd && !inBs) bmmgRootTree_->BmesonType_ = 0; // Bd only
				else bmmgRootTree_->BmesonType_ = -1; // ambiguous, ignore
				bmmgRootTree_->Bsmass_recommg_ = bslegTrippleObject.M();
				bmmgRootTree_->Bspt_recommg_ = bslegTrippleObject.Pt();
				bmmgRootTree_->Bseta_recommg_ = bslegTrippleObject.Eta();
				bmmgRootTree_->Bsphi_recommg_ = bslegTrippleObject.Phi();
				//std::cout<< " ----------------bs mass from photon l1 reco photon -------------------------------"<< bslegTrippleObject.M() << "\n";
				pgamma.SetPtEtaPhi(photonVar[iPhoton].pt, photonVar[iPhoton].eta, photonVar[iPhoton].phi);
				pbs.SetPtEtaPhi(bslegTrippleObject.Pt(), bslegTrippleObject.Eta(), bslegTrippleObject.Phi());
				float helicity = pgamma.Dot(pbs) / (pgamma.Mag() * pbs.Mag());
				TVector3 normal1 = mu1vec.Cross(mu2vec);
				TVector3 normal2 = pgamma.Cross(pbs);
				double cosPhi = normal1.Dot(normal2) / (normal1.Mag() * normal2.Mag());
				float coplanarity = TMath::ACos(TMath::Min(1.0, TMath::Max(-1.0, cosPhi)));
				bmmgRootTree_->Bshelicity_recommg_ = helicity;
				bmmgRootTree_->Bscoplanarity_recommg_ = coplanarity;
				bmmgRootTree_->isFourBody_ = 0;

				
			}//Photon loop for three body through photon kinematics are saved irrespective of their multiplicity 
			if (photonVar.size() - excludedPhotons.size() >= 2) {
				for (size_t iPhoton = 0; iPhoton < photonVar.size(); ++iPhoton) {
					if (excludedPhotons.count(iPhoton)) continue;
					for (size_t jPhoton = iPhoton + 1; jPhoton < photonVar.size(); ++jPhoton) {
						if (excludedPhotons.count(jPhoton)) continue;   
						photonleg1.SetPtEtaPhiE(photonVar[iPhoton].pt, photonVar[iPhoton].eta, photonVar[iPhoton].phi, photonVar[iPhoton].energy);
						photonleg2.SetPtEtaPhiE(photonVar[jPhoton].pt, photonVar[jPhoton].eta, photonVar[jPhoton].phi, photonVar[jPhoton].energy);
						TLorentzVector digamma = photonleg1 + photonleg2;
						bool isPi0 = std::abs(digamma.M() - PionZeroPDGMass_) < PionZeroMassWindowNoFit_;
						bool isEta    = std::abs(digamma.M() - EtaMesonPDGMass_) < EtaMesonMassWindowNoFit_ ;
						bool isEtaPrime = std::abs(digamma.M() - EtaPrimePDGMass_) < EtaPrimeMassWindowNoFit_ ;
						if (!(isPi0 || isEta || isEtaPrime)) continue;
						bmmgRootTree_->DiGammaM_alone_ = digamma.M();
						bmmgRootTree_->DiGammaEta_alone_ = digamma.Eta();
						bmmgRootTree_->DiGammaPhi_alone_ = digamma.Phi();
						bmmgRootTree_->DiGammaPt_alone_ = digamma.Pt();
						TLorentzVector bslegTetraObject = muonleg1 + muonleg2 + photonleg1 + photonleg2;
						double M = bslegTetraObject.M();
						bool inBs = (M >= BsLowerMassCutBeforeFit_ && M <= BsUpperMassCutBeforeFit_);
						bool inBd = (M >= BdLowerMassCutBeforeFit_ && M <= BdUpperMassCutBeforeFit_);
						if (!inBs && !inBd) continue;   // reject if in neither Bs nor Bd
						if (inBs && !inBd) bmmgRootTree_->BmesonType_ = 1; // Bs only
						else if (inBd && !inBs) bmmgRootTree_->BmesonType_ = 0; // Bd only
						else bmmgRootTree_->BmesonType_ = -1; // ambiguous, ignore
						pdigamma.SetPtEtaPhi(digamma.Pt(), digamma.Eta(), digamma.Phi());
						pbs.SetPtEtaPhi(bslegTetraObject.Pt(), bslegTetraObject.Eta(), bslegTetraObject.Phi());
						float helicity = pdigamma.Dot(pbs) / (pdigamma.Mag() * pbs.Mag());
						TVector3 normal1 = mu1vec.Cross(mu2vec);
                        TVector3 normal2 = pdigamma.Cross(pbs);
                        double cosPhi = normal1.Dot(normal2) / (normal1.Mag() * normal2.Mag());
						float coplanarity = TMath::ACos(TMath::Min(1.0, TMath::Max(-1.0, cosPhi)));
						bmmgRootTree_->isFourBody_ = 1;
						bmmgRootTree_->Bsmass_recommgg_ = bslegTetraObject.M();
						bmmgRootTree_->Bspt_recommgg_ = bslegTetraObject.Pt();
						bmmgRootTree_->Bseta_recommgg_ = bslegTetraObject.Eta();
						bmmgRootTree_->Bsphi_recommgg_ = bslegTetraObject.Phi();
						//std::cout<< " ----------------bs mass from photon l1 and l2 reco photon -------------------------------"<< bslegTetraObject.M() << "\n";
						bmmgRootTree_->Bshelicity_recommgg_ = helicity;
						bmmgRootTree_->Bscoplanarity_recommgg_ = coplanarity;

					}//second photon loop
				}//first photon loop
				
			}//size of the photon should at least 2 


	}//Photon empty 

	ParticleFlowCandidate  pfCandVariables;
	std::vector<ParticleFlowCandidate::PFCandidateVariables> pfCandVars = pfCandVariables.PFCandObservables(*pfCandidates);
	if (!pfCandVars.empty()) {
		for (size_t iPFCand = 0; iPFCand < pfCandVars.size(); ++iPFCand) {
			const auto& pfCand = (*pfCandidates)[iPFCand];
			if (pfCand.pt() < 0.5) continue; // Skip low momentum candidates
			
			if (pfCandVars.empty()) continue;

			if (pfCandVars[iPFCand].pt < 1.0) continue; // Skip low momentum candidates
			bmmgRootTree_->pfCandPt_[iPFCand] = pfCandVars[iPFCand].pt;
			bmmgRootTree_->pfCandEta_[iPFCand] = pfCandVars[iPFCand].eta;
			bmmgRootTree_->pfCandPhi_[iPFCand] = pfCandVars[iPFCand].phi;
			bmmgRootTree_->pfCandEnergy_[iPFCand] = pfCandVars[iPFCand].energy;
			//bmmgRootTree_->pfCandET_[iPFCand] = pfCandVars[iPFCand].et;
			bmmgRootTree_->pfCandMass_[iPFCand] = pfCandVars[iPFCand].mass;
			bmmgRootTree_->pfCandCharge_[iPFCand] = pfCandVars[iPFCand].charge;
			bmmgRootTree_->pfCandEcalEnergy_[iPFCand] = pfCandVars[iPFCand].ecalEnergy;
			bmmgRootTree_->pfCandRawEcalEnergy_[iPFCand] = pfCandVars[iPFCand].rawEcalEnergy;
			bmmgRootTree_->pfCandHcalEnergy_[iPFCand] = pfCandVars[iPFCand].hcalEnergy;
			bmmgRootTree_->pfCandRawHcalEnergy_[iPFCand] = pfCandVars[iPFCand].rawHcalEnergy;
			bmmgRootTree_->pfCandHoEnergy_[iPFCand] = pfCandVars[iPFCand].hoEnergy;
			bmmgRootTree_->pfCandRawHoEnergy_[iPFCand] = pfCandVars[iPFCand].rawHoEnergy;
			bmmgRootTree_->pfCandTime_[iPFCand] = pfCandVars[iPFCand].time;
			//bmmgRootTree_->pfCandTrkIso_[iPFCand] = pfCandVars[iPFCand].trkIso;
			//bmmgRootTree_->pfCandEcalIso_[iPFCand] = pfCandVars[iPFCand].ecalIso;
			//bmmgRootTree_->pfCandHcalIso_[iPFCand] = pfCandVars[iPFCand].hcalIso;
			//bmmgRootTree_->pfCandCaloIso_[iPFCand] = pfCandVars[iPFCand].caloIso;
			bmmgRootTree_->pfCandRefPhotonPt_[iPFCand] = pfCandVars[iPFCand].refphotonpt;
			bmmgRootTree_->pfCandRefPhotonEta_[iPFCand] = pfCandVars[iPFCand].refphotoneta;
			bmmgRootTree_->pfCandRefPhotonPhi_[iPFCand] = pfCandVars[iPFCand].refphotonphi;
			bmmgRootTree_->pfCandRefPhotonEnergy_[iPFCand] = pfCandVars[iPFCand].refphotonenergy;
			bmmgRootTree_->pfCandRefPhotonET_[iPFCand] = pfCandVars[iPFCand].refphotonet;
			bmmgRootTree_->pfCandRefPhotonSigmaIEtaIEta_[iPFCand] = pfCandVars[iPFCand].refphotonsigmaIEtaIEta;
			bmmgRootTree_->pfCandRefPhotonSigmaIEtaIPhi_[iPFCand] = pfCandVars[iPFCand].refphotonsigmaIEtaIPhi;
			bmmgRootTree_->pfCandRefPhotonSigmaIPhiIPhi_[iPFCand] = pfCandVars[iPFCand].refphotonsigmaIPhiIPhi;
			bmmgRootTree_->pfCandRefPhotonSigmaEtaEta_[iPFCand] = pfCandVars[iPFCand].refphotonsigmaEtaEta;
			bmmgRootTree_->pfCandRefPhotonE1x5_[iPFCand] = pfCandVars[iPFCand].refphotone1x5;
			bmmgRootTree_->pfCandRefPhotonE2x5_[iPFCand] = pfCandVars[iPFCand].refphotone2x5;
			bmmgRootTree_->pfCandRefPhotonE3x3_[iPFCand] = pfCandVars[iPFCand].refphotone3x3;
			bmmgRootTree_->pfCandRefPhotonE5x5_[iPFCand] = pfCandVars[iPFCand].refphotone5x5;
			bmmgRootTree_->pfCandRefPhotonHcalDepth1OverEcal_[iPFCand] = pfCandVars[iPFCand].refphotonhcalDepth1OverEcal;
			bmmgRootTree_->pfCandRefPhotonHcalDepth2OverEcal_[iPFCand] = pfCandVars[iPFCand].refphotonhcalDepth2OverEcal;
			bmmgRootTree_->pfCandRefPhotonHcalDepth1OverEcalBc_[iPFCand] = pfCandVars[iPFCand].refphotonhcalDepth1OverEcalBc;
			bmmgRootTree_->pfCandRefPhotonHcalDepth2OverEcalBc_[iPFCand] = pfCandVars[iPFCand].refphotonhcalDepth2OverEcalBc;
			bmmgRootTree_->pfCandRefPhotonScEnergy_[iPFCand] = pfCandVars[iPFCand].refphotonscEnergy;
			bmmgRootTree_->pfCandRefPhotonScRawEnergy_[iPFCand] = pfCandVars[iPFCand].refphotonscRawEnergy;
			bmmgRootTree_->pfCandRefPhotonScEta_[iPFCand] = pfCandVars[iPFCand].refphotonscEta;
			bmmgRootTree_->pfCandRefPhotonScPhi_[iPFCand] = pfCandVars[iPFCand].refphotonscPhi;
			bmmgRootTree_->pfCandRefPhotonScEtaWidth_[iPFCand] = pfCandVars[iPFCand].refphotonscEtaWidth;
			bmmgRootTree_->pfCandRefPhotonScPhiWidth_[iPFCand] = pfCandVars[iPFCand].refphotonscPhiWidth;
			bmmgRootTree_->pfCandRefPhotonScBrem_[iPFCand] = pfCandVars[iPFCand].refphotonscBrem;
			bmmgRootTree_->pfCandRefPhotonR9_[iPFCand] = pfCandVars[iPFCand].refphotonr9;
			bmmgRootTree_->pfCandRefPhotonHadTowOverEm_[iPFCand] = pfCandVars[iPFCand].refphotonhadTowOverEm;
			bmmgRootTree_->pfCandRefPhotonMaxEnergyXtal_[iPFCand] = pfCandVars[iPFCand].refphotonmaxEnergyXtal;
			bmmgRootTree_->pfCandRefPhotonEffSigmaRR_[iPFCand] = pfCandVars[iPFCand].refphotoneffSigmaRR;
			for (size_t k = 0; k < pfCandVars[iPFCand].refphotonhcalOverEcal.size(); ++k) {
				bmmgRootTree_->pfCandRefPhotonHcalOverEcal_[k][iPFCand] = static_cast<double>(pfCandVars[iPFCand].refphotonhcalOverEcal[k]);
			}
			for (size_t k = 0; k < pfCandVars[iPFCand].refphotonhcalOverEcalBc.size(); ++k) {
				bmmgRootTree_->pfCandRefPhotonHcalOverEcalBc_[k][iPFCand] = static_cast<double>(pfCandVars[iPFCand].refphotonhcalOverEcalBc[k]);
			}

		}// PFCandidate loop
	}// PFCandidate empty




	//====================================================================================================
	// Gen Particle information 
	//====================================================================================================
	edm::Handle<edm::View<reco::GenParticle> > genParticles;
	if(isMCstudy_)
              {
					 iEvent.getByToken(genParticlesTok, genParticles);
					 //std::cout<<"genparticles:   "<<genParticles->size()<<"\n";

					  std::vector<reco::GenParticle> genParticleVec;
					  for (const auto& gp : *genParticles) {
						genParticleVec.push_back(gp);
					}
					GenDecayChain gen(genParticleVec);
					const auto& G = gen.info();
					 if (!G.isValid) {
						 return;
					 }

					
					 bmmgRootTree_->GenMotherID_ = G.motherPdgId;
					 bmmgRootTree_->GenB_pt_      = G.Bp4.Pt();
					 bmmgRootTree_->GenB_eta_     = G.Bp4.Eta();
					 bmmgRootTree_->GenB_phi_     = G.Bp4.Phi();
					 bmmgRootTree_->GenB_mass_    = G.Bp4.M();
					 std::cout<<" Gen B Mass : "<< G.Bp4.M() << "Gen B Pt : "<< G.Bp4.Pt() << "\n";

					 bmmgRootTree_->GenBLxy_      = G.BLxy;
					 bmmgRootTree_->GenBct2D_       = G.Bct2D;
					 bmmgRootTree_->GenBct3D_       = G.Bct3D;

					 
					 if (G.hasMuonPlus) {
						 bmmgRootTree_->GenMuPlus_pt_  = G.muPlusP4.Pt();
						 bmmgRootTree_->GenMuPlus_eta_ = G.muPlusP4.Eta();
						 bmmgRootTree_->GenMuPlus_phi_ = G.muPlusP4.Phi();
						 bmmgRootTree_->GenMuPlus_E_   = G.muPlusP4.E();
					 }

					 if (G.hasMuonMinus) {
						 bmmgRootTree_->GenMuMinus_pt_  = G.muMinusP4.Pt();
						 bmmgRootTree_->GenMuMinus_eta_ = G.muMinusP4.Eta();
						 bmmgRootTree_->GenMuMinus_phi_ = G.muMinusP4.Phi();
						 bmmgRootTree_->GenMuMinus_E_   = G.muMinusP4.E();
					 }

					 
					 bmmgRootTree_->GenDimuon_mass_ = G.dimuonMass;
					 bmmgRootTree_->GenDimuon_pt_   = G.dimuonPt;

					 // Photon variables
					 bmmgRootTree_->GenNPhotons_ = G.nPhotons;
					 for (size_t i = 0; i < G.photons.size() && i < 4; i++) {
						 bmmgRootTree_->GenGamma_pt_[i]     = G.photons[i].p4.Pt();
						 bmmgRootTree_->GenGamma_eta_[i]    = G.photons[i].p4.Eta();
						 bmmgRootTree_->GenGamma_phi_[i]    = G.photons[i].p4.Phi();
						 bmmgRootTree_->GenGamma_E_[i]      = G.photons[i].p4.E();
						 bmmgRootTree_->GenGamma_origin_[i] = G.photons[i].origin;
						 bmmgRootTree_->GenGamma_motherId_[i] = G.photons[i].motherPdgId;
						 bmmgRootTree_->GenGamma_dRmu1_[i]  = G.photons[i].deltaR_mu1;
						 bmmgRootTree_->GenGamma_dRmu2_[i]  = G.photons[i].deltaR_mu2;
					 }
					 bmmgRootTree_->GenHelicity3_    = G.helicity3;
					 bmmgRootTree_->GenCoplanarity3_ = G.coplanarity3;
					 bmmgRootTree_->GenCosTheta_l_   = G.cosTheta_l;
					 bmmgRootTree_->GenTriBodyMass_  = G.triBodyMass;
					 bmmgRootTree_->GenHelicity4_    = G.helicity4;
					 bmmgRootTree_->GenCoplanarity4_ = G.coplanarity4;
					 bmmgRootTree_->GenFourBodyMass_ = G.fourBodyMass;
					 bmmgRootTree_->GenDiphoton_mass_ = G.diphotonMass;
					 bmmgRootTree_->GenDiphoton_pt_   = G.diphotonPt;
					 bmmgRootTree_->GenHasJpsi_      = G.hasJpsi;
					 bmmgRootTree_->GenJpsi_mass_    = G.hasJpsi ? G.jpsiP4.M() : -999;
					 bmmgRootTree_->GenJpsi_pt_      = G.hasJpsi ? G.jpsiP4.Pt() : -999;
					 bmmgRootTree_->GenHasNeutralMeson_ = G.hasNeutralMeson;
					 bmmgRootTree_->GenNeutralMesonId_  = G.neutralMesonId;
					 bmmgRootTree_->GenDecayMode_    = G.decayMode;
					 bmmgRootTree_->GenIs3Body_      = G.is3BodyDecay;
					 bmmgRootTree_->GenIs4Body_      = G.is4BodyDecay;
					 bmmgRootTree_->GenIsHardProcess_ = G.isHardProcess;
					 bmmgRootTree_->GenIsSignal_ = gen.isSignalLike();

					 if (selectedMuons->size() >= 2) {
        const reco::Muon& mu1 = (*selectedMuons)[0];
        const reco::Muon& mu2 = (*selectedMuons)[1];
        
        // Clean! No redundant parameters
        auto mu1Match = gen.matchMuonToGen(mu1);
        auto mu2Match = gen.matchMuonToGen(mu2);
        
        bmmgRootTree_->Mu1_isMatched_       = mu1Match.isMatched;
        bmmgRootTree_->Mu1_genPdgId_        = mu1Match.genPdgId;
        bmmgRootTree_->Mu1_genMotherPdgId_  = mu1Match.genMotherPdgId;
        bmmgRootTree_->Mu1_genGMotherPdgId_ = mu1Match.genGMotherPdgId;
        bmmgRootTree_->Mu1_genDeltaR_       = mu1Match.deltaR;
        bmmgRootTree_->Mu1_genDeltaPt_      = mu1Match.deltaPt;
        
        bmmgRootTree_->Mu2_isMatched_       = mu2Match.isMatched;
        bmmgRootTree_->Mu2_genPdgId_        = mu2Match.genPdgId;
        bmmgRootTree_->Mu2_genMotherPdgId_  = mu2Match.genMotherPdgId;
        bmmgRootTree_->Mu2_genGMotherPdgId_ = mu2Match.genGMotherPdgId;
        bmmgRootTree_->Mu2_genDeltaR_       = mu2Match.deltaR;
        bmmgRootTree_->Mu2_genDeltaPt_      = mu2Match.deltaPt;
        
        bmmgRootTree_->DiMuon_isMatched_ = mu1Match.isMatched && mu2Match.isMatched;
    }
		            


	for (size_t i = 0; i < selectedPhoton->size() && i < 4; ++i) {
        const reco::Photon& myphoton = (*selectedPhoton)[i];
        
        
        auto photonMatch = gen.matchPhotonToGen(myphoton);
        
        bmmgRootTree_->Photon_isMatched_[i]       = photonMatch.isMatched;
        bmmgRootTree_->Photon_genPdgId_[i]        = photonMatch.genPdgId;
        bmmgRootTree_->Photon_genMotherPdgId_[i]  = photonMatch.genMotherPdgId;
        bmmgRootTree_->Photon_genGMotherPdgId_[i] = photonMatch.genGMotherPdgId;
        bmmgRootTree_->Photon_genDeltaR_[i]       = photonMatch.deltaR;
		std::cout<<" Photon "<< i <<" deltaR : "<< photonMatch.deltaR << "\n";
        bmmgRootTree_->Photon_genDeltaPt_[i]      = photonMatch.deltaPt;
    }
    
    // Topology matching
    if (selectedMuons->size() >= 2 && photons->size() >= 1) {
        const reco::Muon& mu1 = (*selectedMuons)[0];
        const reco::Muon& mu2 = (*selectedMuons)[1];
        
        // Build photon pointer vector
        std::vector<const reco::Photon*> photonPtrs;
        for (size_t i = 0; i < photons->size() && i < 2; ++i) {
            photonPtrs.push_back(&((*photons)[i]));
        }
        
      
        auto topoMatch = gen.matchTopology(mu1, mu2, photonPtrs);
        
        bmmgRootTree_->Topology_isMatched_     = topoMatch.isMatched;
        bmmgRootTree_->Topology_mu1Matched_    = topoMatch.mu1Matched;
        bmmgRootTree_->Topology_mu2Matched_    = topoMatch.mu2Matched;
        bmmgRootTree_->Topology_photon1Matched_ = topoMatch.photon1Matched;
        bmmgRootTree_->Topology_photon2Matched_ = topoMatch.photon2Matched;
        bmmgRootTree_->Topology_decayModeMatched_ = topoMatch.decayModeMatched;
        
        bool isSignalMatched = topoMatch.isMatched && gen.isSignalLike();
        bmmgRootTree_->IsSignalMatched_ = isSignalMatched;
    }
    
    
    if (selectedMuons->size() >= 2) {
        const reco::Muon& mu1 = (*selectedMuons)[0];
        const reco::Muon& mu2 = (*selectedMuons)[1];
        
        // Muons from J/psi (443) from Bs (531)
        auto mu1JpsiMatch = gen.matchToGen(mu1, 13, 443, 531, 0.05, 0.5);
        auto mu2JpsiMatch = gen.matchToGen(mu2, 13, 443, 531, 0.05, 0.5);
        
        bmmgRootTree_->IsMuonsFromJpsi_ = mu1JpsiMatch.isMatched && mu2JpsiMatch.isMatched;
        
     
        auto mu1DirectMatch = gen.matchToGen(mu1, 13, 531, 0, 0.05, 0.5);
        auto mu2DirectMatch = gen.matchToGen(mu2, 13, 531, 0, 0.05, 0.5);
        
        bmmgRootTree_->IsMuonsDirectFromBs_ = mu1DirectMatch.isMatched && mu2DirectMatch.isMatched;
    }

		     fillMCInfo(genParticles); 
		     
               }

	bmmgRootTree_->fill();
}
//////////////////////////////////////////////////////////////
////////// Helper Functions///////////////////////////////////
//////////////////////////////////////////////////////////////
GlobalVector RadiativeAnalysis::flightDirection(const reco::Vertex &pv, reco::Vertex &sv){
  GlobalVector res(sv.position().X() - pv.position().X(),
                    sv.position().Y() - pv.position().Y(),
                    sv.position().Z() - pv.position().Z());
  return res;
}
void RadiativeAnalysis::fillMCInfo(edm::Handle<edm::View<reco::GenParticle>>& genParticles){
  int iNumberOfBdecays = 0;
  Int_t SVindex =0;
  std::set<int> listOfBmesonIds;
  listOfBmesonIds.insert(511 );   // Bd
  listOfBmesonIds.insert(521 );   // B+
  listOfBmesonIds.insert(10511 );    // B_0*0
  listOfBmesonIds.insert(10521 );    // B_0*+
  listOfBmesonIds.insert(513 );   // B*d
  listOfBmesonIds.insert(523 );   // B*d+
  listOfBmesonIds.insert(10513 );   // B1(L)0
  listOfBmesonIds.insert(10523 );   // B1(L)+
  listOfBmesonIds.insert(20513 );   // B1(H)0
  listOfBmesonIds.insert(20523 );   // B1(H)+
  listOfBmesonIds.insert(515 );    // B2*_0
  listOfBmesonIds.insert(525 );    // B2*_+
  listOfBmesonIds.insert(531 );   // Bs
  listOfBmesonIds.insert(10531 );    // B_s0*_0
  listOfBmesonIds.insert(533 );   // B*s
  listOfBmesonIds.insert(10533 );   // Bs1(L)0
  listOfBmesonIds.insert(20533 );   // Bs1(H)0
  listOfBmesonIds.insert(535 );    // Bs2*_0
  listOfBmesonIds.insert(541 );   // Bc+
  listOfBmesonIds.insert(10541 );   // B*c0+
  listOfBmesonIds.insert(543 );   // B*c+
  listOfBmesonIds.insert(10543 );   // Bc1(L)+
  listOfBmesonIds.insert(20543 );   // Bc1(H)+
  listOfBmesonIds.insert(545 );    // Bc2*_0
  listOfBmesonIds.insert(551 );   // etab(1S)
  listOfBmesonIds.insert(10551 );   // chib(1P)
  listOfBmesonIds.insert(100551 );   // etab(2S)
  listOfBmesonIds.insert(110551 );   // chib(2P)
  listOfBmesonIds.insert(200551 );   // etab(3S)
  listOfBmesonIds.insert(210551 );   // chib(3P)
  listOfBmesonIds.insert(553 );   // upsilon(1S)
  listOfBmesonIds.insert(10553 );   // hb(1P)
  listOfBmesonIds.insert(20553 );   // chib1(1P)
  listOfBmesonIds.insert(30553 );   // upsilon1(1D)
  listOfBmesonIds.insert(100553 );   // upsilon(2S)
  listOfBmesonIds.insert(110553 );   // hb(2P)
  listOfBmesonIds.insert(120553 );   // chib1(2P)
  listOfBmesonIds.insert(130553 );   // upsilon1(2D)
  listOfBmesonIds.insert(200553 );   // upsilon(3S)
  listOfBmesonIds.insert(210553 );   // hb(3P)
  listOfBmesonIds.insert(220553 );   // chib1(3P)
  listOfBmesonIds.insert(300553 );   // upsilon(4S)
  listOfBmesonIds.insert(9000553 );   // upsilon(10860)
  listOfBmesonIds.insert(9010553 );   // upsilon(11020)
  listOfBmesonIds.insert(555 );   // chib2(1P)
  listOfBmesonIds.insert(10555 );   // etab2(1D)
  listOfBmesonIds.insert(20555 );   // upsilon2(1D)
  listOfBmesonIds.insert(100555 );   // chib2(2P)
  listOfBmesonIds.insert(110555 );   // etab2(2D)
  listOfBmesonIds.insert(120555 );   // upsilon2(2D)
  listOfBmesonIds.insert(200555 );   // chib2(3P)
  listOfBmesonIds.insert(557 );   // upsilon3(1D)
  listOfBmesonIds.insert(100557 );   // upsilon3(2D)
  listOfBmesonIds.insert(5122 );   // lambda_b0
  listOfBmesonIds.insert(5112 );   // sigma_b-
  listOfBmesonIds.insert(5212 );   // sigma_b0
  listOfBmesonIds.insert(5222 );   // sigma_b+
  listOfBmesonIds.insert(5114 );   // sigma*_b-
  listOfBmesonIds.insert(5214 );   // sigma*_b0
  listOfBmesonIds.insert(5224 );   // sigma*_b+
  listOfBmesonIds.insert(5132 );   // Xi_b-
  listOfBmesonIds.insert(5232 );   // Xi_b0
  listOfBmesonIds.insert(5312 );   // Xi'_b-
  listOfBmesonIds.insert(5322 );   // Xi'_b0
  listOfBmesonIds.insert(5314 );   // Xi*_b-
  listOfBmesonIds.insert(5324 );   // Xi*_b0
  listOfBmesonIds.insert(5332 );   // Omega_b-
  listOfBmesonIds.insert(5334 );   // Omega*_b-
  listOfBmesonIds.insert(5142 );   // Xi_bc0
  listOfBmesonIds.insert(5242 );   // Xi_bc+
  listOfBmesonIds.insert(5412 );   // Xi'_bc0
  listOfBmesonIds.insert(5422 );   // Xi'_bc+
  listOfBmesonIds.insert(5414 );   // Xi*_bc0
  listOfBmesonIds.insert(5424 );   // Xi*_bc+
  listOfBmesonIds.insert(5342 );   // Omega_bc0
  listOfBmesonIds.insert(5432 );   // Omega'_bc0
  listOfBmesonIds.insert(5434 );   // Omega*_bc0
  listOfBmesonIds.insert(5442 );   // Omega_bcc+
  listOfBmesonIds.insert(5444 );   // Omega*_bcc+
  listOfBmesonIds.insert(5512 );   // Xi_bb-
  listOfBmesonIds.insert(5522 );   // Xi_bb0
  listOfBmesonIds.insert(5514 );   // Xi*_bb-
  listOfBmesonIds.insert(5524 );   // Xi*_bb0
  listOfBmesonIds.insert(5532 );   // Omega_bb-
  listOfBmesonIds.insert(5524 );   // Omega*_bb-
  listOfBmesonIds.insert(5542 );   // Omega_bbc0
  listOfBmesonIds.insert(5544 );   // Omega*_bbc0
  listOfBmesonIds.insert(554 );   // Omega_bbb-
  /*
  const Candidate * Jpsi = 0;
  const Candidate * Phi = 0;
  const Candidate * mup = 0;
  const Candidate * mum = 0;
  const Candidate * Kp = 0;
  const Candidate * Km = 0;
  const Candidate * Pi = 0;
  const Candidate * Gamma = 0; 
  const Candidate * Eta = 0; 
  const Candidate * EtaPrime = 0; 
*/


  for( size_t i = 0; i < genParticles->size(); ++ i ) {
    const reco::GenParticle & genBsCand = (*genParticles)[ i ];
    int MC_particleID=genBsCand.pdgId();
    int absMC_particleID = abs(MC_particleID);
	if(absMC_particleID ==443 ){
		unsigned int numJpsiDaus = genBsCand.numberOfDaughters();
		vector<unsigned int> JpsiMuIdx;
		vector<unsigned int> JpsiPhotoIdx;

		if (	numJpsiDaus > 1 && numJpsiDaus < 6){

 		 for(unsigned int a = 0; a < numJpsiDaus ; a++){
			if(abs(genBsCand.daughter(a)->pdgId() ) == 13){

				JpsiMuIdx.push_back(a);
			}
			if(abs(genBsCand.daughter(a)->pdgId()) == 22){
				JpsiPhotoIdx.push_back(a);

			}

 		 }	// ned of for loop
		} // end of if numJpsiDaus
	//cout << "JpsiPhotoIdx.size() + JpsiMuIdx.size() = " <<  JpsiPhotoIdx.size() + JpsiMuIdx.size() << " = " << numJpsiDaus <<"\n";
	if(JpsiMuIdx.size() == 2){
		  if( JpsiPhotoIdx.size() + JpsiMuIdx.size() == numJpsiDaus &&  genBsCand.daughter(JpsiMuIdx[0])->pdgId() == -genBsCand.daughter(JpsiMuIdx[1])->pdgId()  ) {

		   bmmgRootTree_->JpsiGenNumberOfCandidates_++;
	 	   bmmgRootTree_->JpsiGenPVx_ = genBsCand.vx();
	           bmmgRootTree_->JpsiGenPVy_ = genBsCand.vy();
		   bmmgRootTree_->JpsiGenPVz_ = genBsCand.vz();
		   bmmgRootTree_->JpsiGenLxyOld_= sqrt( pow( (bmmgRootTree_->BSx_ - genBsCand.vx()) , 2) + pow( (bmmgRootTree_->BSy_ - genBsCand.vy() ),2 )  );
                   GlobalPoint GenDisplacementFromBeamspot( -1*( (bmmgRootTree_->BSx_ -  genBsCand.vx() ) + ( genBsCand.vz() - bmmgRootTree_->BSz_) * bmmgRootTree_->BSdxdz_ ), -1*( (bmmgRootTree_->BSy_ - genBsCand.vy() )+  (genBsCand.vz() - bmmgRootTree_->BSz_) * bmmgRootTree_->BSdydz_), 0 );
                   bmmgRootTree_->JpsiGenLxy_ = GenDisplacementFromBeamspot.perp();
                   bmmgRootTree_->JpsiGenLxyOverPt_ = ( GenDisplacementFromBeamspot.x()*genBsCand.px()  +  GenDisplacementFromBeamspot.y()*genBsCand.py()  ) /( genBsCand.px() * genBsCand.px() + genBsCand.py() * genBsCand.py() );
	           bmmgRootTree_->JpsiGenPt_ = genBsCand.pt();
		   //cout << "JpsiGenLxy_ : "   << bmmgRootTree_->JpsiGenLxy_ << " = " << GenDisplacementFromBeamspot.perp() << endl;
		   //cout << "JpsiGenLxy/Pt:  " << bmmgRootTree_->JpsiGenLxyOverPt_ << endl;
		  }
	}
	}
  }
}
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void RadiativeAnalysis::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //edm::ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks", edm::InputTag("ctfWithMaterialTracks"));
  //descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(RadiativeAnalysis);
