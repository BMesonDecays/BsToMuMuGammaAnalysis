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
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TripleObjectVertexFit.h"





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
	generatorInfoTok                  = consumes<GenRunInfoProduct, edm::InRun>(edm::InputTag("generator"));
	genFilterTok                      = consumes<GenFilterInfo, edm::InLumi>(edm::InputTag("genFilterEfficiencyProducer"));
	isMCstudy_                        = iConfig.getParameter<bool>("isMCstudy");
	isMINIAOD_                        = iConfig.getParameter<bool>("isMINIAOD");
	genParticlesLabel                 = iConfig.getParameter<edm::InputTag>("genParticlesLabel");
	genParticlesTok                   = consumes<edm::View<reco::GenParticle>>(genParticlesLabel);
	MuonTag                           = iConfig.getParameter<edm::InputTag>("MuonTag");
	MuonTagTok                        = consumes<edm::View<reco::Muon>>(MuonTag);
	JetTag                            = iConfig.getParameter<edm::InputTag>("JetTag");
	JetTagTok                         = consumes<edm::View<pat::Jet>>(JetTag);
	PhotonTag                         = iConfig.getParameter<edm::InputTag>("PhotonTag");
	PhotonTagTok                      = consumes<edm::View<reco::Photon>>(PhotonTag);
	PFCandTag                         = iConfig.getParameter<edm::InputTag>("PFCandTag");
	PFCandTagTok                      = consumes<edm::View<reco::Candidate>>(PFCandTag);
	OOTPhotonTag                      = iConfig.getParameter<edm::InputTag>("OOTPhotonTag");
	OOTPhotonTagTok                   = consumes<edm::View<pat::Photon>>(OOTPhotonTag);
	ElectronTag                       = iConfig.getParameter<edm::InputTag>("ElectronTag");
	ElectronTagTok                    = consumes<edm::View<pat::Electron>>(ElectronTag);
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
  outputTree_ = new OutputTree();
  outputTree_->createTree(outputFile_);

  muonMVAIDProducer_ = new MuonMVAID(theConfig_);
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
void RadiativeAnalysis::endRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{


   edm::Handle<GenRunInfoProduct> genRunInfo;
   iRun.getByToken(generatorInfoTok, genRunInfo);


    if (!genRunInfo.isValid()) {
      edm::LogWarning("GenRunInfo") << "GenRunInfoProduct not found!";
      return;
    }

      genXsec_   = genRunInfo->internalXSec().value();
	  genXsecErr_ = genRunInfo->internalXSec().error();


}
void RadiativeAnalysis::beginLuminosityBlock(const edm::LuminosityBlock& iLumi,const edm::EventSetup& iSetup)
{
    // nothing needed here for now
}
void RadiativeAnalysis::endLuminosityBlock(const edm::LuminosityBlock& iLumi, const edm::EventSetup& iSetup)
{
    edm::Handle<GenFilterInfo> genFilter;
    iLumi.getByToken(genFilterTok, genFilter);

    if (!genFilter.isValid()) {
        std::cout << "GenFilterInfo not found" << std::endl;
        return;
    }

    filterEff_ = genFilter->filterEfficiency(-1);
    filterEffErr_ = genFilter->filterEfficiencyError(-1);
}

void RadiativeAnalysis::processHLTBits(const edm::Event& iEvent)
{
	edm::Handle<edm::TriggerResults> hltbits;
	iEvent.getByToken(triggerbitsTok, hltbits);

	const edm::TriggerNames& triggerNames_ = iEvent.triggerNames(*hltbits);

	// --- path-level bit saving ---
	// Note: longer paths checked BEFORE shorter substrings to prevent
	// "HLT_DoubleMu4_3_LowMass" matching inside "..._Displaced" etc.
	const int ntrigs = hltbits->size();
	for (int itrig = 0; itrig != ntrigs; ++itrig) {
		const std::string trigName = triggerNames_.triggerName(itrig);
		// anchor to _v so "LowMass" cannot fire "LowMass_Displaced"
		if (trigName.find("HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_ = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_3_Photon4_BsToMMG_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_           = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_LowMass_Displaced_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_LowMass_Displaced_           = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_3_LowMass_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_3_LowMass_                   = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_3_Bs_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_3_Bs_                        = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_JpsiTrkTrk_Displaced_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_JpsiTrkTrk_Displaced_        = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_Jpsi_Displaced_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_Jpsi_Displaced_              = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_Jpsi_NoVertexing_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_Jpsi_NoVertexing_            = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu2_Jpsi_LowPt_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu2_Jpsi_LowPt_                   = hltbits->accept(itrig);
		else if (trigName.find("HLT_DoubleMu4_3_Jpsi_v") != std::string::npos)
			outputTree_->triggerbit_HLT_DoubleMu4_3_Jpsi_                      = hltbits->accept(itrig);
	}
}
    

// ------------ method called for each event  ------------
void RadiativeAnalysis::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

	event_counter_++;
	excludedPhotons.clear();
	int nBs=0;
	outputTree_->resetAllEntries();
	

	outputTree_->runNumber_   = iEvent.id().run();
	outputTree_->eventNumber_ = (unsigned int)iEvent.id().event();
	outputTree_->lumiSection_ = iEvent.luminosityBlock();
	
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
		outputTree_->PUinteraction_ = numInteraction;
		outputTree_->PUTrueinteraction_ = numTrueInteraction;
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

	edm::Handle<edm::View<reco::Muon>> muons;
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
	edm::Handle<edm::View<reco::Photon>> photons;
    	iEvent.getByToken(PhotonTagTok, photons);
	edm::Handle<edm::View<reco::Candidate>> pfCandidates;
		iEvent.getByToken(PFCandTagTok, pfCandidates);
	edm::Handle<edm::ValueMap<float>> mvaValuesMap;
		iEvent.getByToken(valMapTok, mvaValuesMap);
	edm::Handle<edm::ValueMap<bool>> phoTightIDMap;
		iEvent.getByToken(valMapPhoTightTok, phoTightIDMap);


	//-------FILL THE TREE WITH THE GENERAL VARIABLES-------//
	outputTree_->nMuons_ = muons->size();
	outputTree_->nPhotons_ = photons->size();
	outputTree_->nTracks_ = tracks->size();    

	BeamSpotAndVertex bsandvtxObservables(isMINIAOD_);
	auto bsandvtxVar = bsandvtxObservables.BSAndVtxObservables(*vertexBeamSpot, *recVtxs);
	outputTree_->addBSAndVtxObservables(bsandvtxVar);
	
	processHLTBits(iEvent);

	//--------PREPARE MVA ID SCORES----------------//
	// muons mva id scores
	// pass the View directly: copying pat::Muons into reco::Muon drops their embedded tracks (MINIAOD)
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
	} 
	else {
		edm::LogError("RadiativeAnalysis") << "Failed to retrieve MVA values map for photons.";
		photonMVAIDs.assign(photons->size(), -99.f);
	}
	

	//-----START OF THE DECAY RECONSTRUCTION------//
	TripleObjectVertexFit tripleVertexFitter;

	MuonSelector muonSelector;
	// 3 vectors with the same number of elements
	// a given vector element index corresponds to one muon pair
	// (e.g. {firstMuonIndices[i], secondMuonIndices[i]}, where commonVtxProbs[i] is their common vertex probability)
	auto [firstMuonIndices, secondMuonIndices, commonVtxProbs] = muonSelector.selectMuonPairs( *muons, trackBuilder );

	// loop over muon pairs
	for (size_t iPair = 0; iPair < firstMuonIndices.size(); iPair++)
	{
		size_t mu1Idx = firstMuonIndices[iPair];
		size_t mu2Idx = secondMuonIndices[iPair];
		const reco::Muon& mu1 = muons->at(mu1Idx);
		const reco::Muon& mu2 = muons->at(mu2Idx);
		// references, not copies, so that pat::Muon track accessors are kept
		std::pair<const reco::Muon&, const reco::Muon&> muonPair( mu1, mu2 );

		outputTree_->resetDecayEntries();

		// muon variables
		outputTree_->mu1MVAScore_ = muonMVAIDs.at(mu1Idx);
		outputTree_->mu2MVAScore_ = muonMVAIDs.at(mu2Idx);
		outputTree_->DiMuon_KalmanVtxProb_ = commonVtxProbs.at(iPair);

		// MMG vertex fit
		// The TripleObjectVertexFit::fitMmgVertices function takes the considered muon pair and all photons in the event
		// A struct is returned with variables for this muon pair and all the photons
		auto tripleVertexResults = tripleVertexFitter.fitMmgVertices( muonPair, *photons, *recVtxs, lazyTools, bsandvtxVar, theBField, trackBuilder );
		outputTree_->addMmgFitResults(tripleVertexResults);

		// one tree entry for each muon pair
		outputTree_->fill();
	} // muon pairs (mu+,mu-)
}

////////////////////////////////////////////////////////////
////////////////End Job///////////////////////////////////
void RadiativeAnalysis::endJob() {
  
	cout << "Total number of Events          : " << event_counter_ << endl;
	cout << "Total number of Tagged muons    : " << muoncounter_   << endl;
	cout << "Total number of Tagged electrons: " << elecounter_    << endl;
	cout << "Total number of Tagged jets     : " << jetcounter_    << endl;
	cout << "Max amount of Tag muons         : " << tagmucounter_ <<  endl;
	cout << "Max number of photon            : " << photoncounter_ << endl;


    int nEvents = event_counter_;

	if (genXsec_ > 0 && filterEff_ > 0 && nEvents > 0) {
		lumiEquivalent_ = nEvents / (genXsec_ * filterEff_);

		double relSigma = (genXsec_ > 0) ? (genXsecErr_ / genXsec_) : 0.0;
		double relEff   = (filterEff_ > 0) ? (filterEffErr_ / filterEff_) : 0.0;

		double relErr = std::sqrt(relSigma*relSigma + relEff*relEff);

		lumiEquivalentErr_ = lumiEquivalent_ * relErr;
	}
    

    outputTree_->GenXsec_        = genXsec_;
    outputTree_->GenXsecErr_     = genXsecErr_;
    outputTree_->FilterEff_      = filterEff_;
    outputTree_->FilterEffErr_   = filterEffErr_;
    outputTree_->nEvents_        = nEvents;
    outputTree_->LumiEquivalent_ = lumiEquivalent_;
    outputTree_->LumiEquivalentErr_ = lumiEquivalentErr_;

    std::cout<< " =========================Lumi Calculation============================\n";
	std::cout << "Total number of Events          : " << nEvents << "\n";
	std::cout << "FilterEff = " << filterEff_ << " +/- " << filterEffErr_ <<"\n";
	std::cout << "GenXsec = " << genXsec_ << " +/- " << genXsecErr_ <<"\n";
	std::cout << "LumiEquivalent = " << lumiEquivalent_ << " +/- " << lumiEquivalentErr_ <<"\n";

	outputTree_->fillSummary();

	outputTree_->writeFile();
	delete outputTree_;
	delete muonMVAIDProducer_;	      
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
