// -*- C++ -*-
// Package:    BsToMuMuGammaAnalysis/RadiativeAnalysis
// Class:      RadiativeAnalysis
// Original Author:  Alibordi Muhammad
//         Created:  Fri, 12 Jul 2024 10:27:36 GMT
//============================================================
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeAnalysis.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DimuonVertexCompatibility.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GenChain.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuonSelector.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TripleObjectVertexFit.h"
#include <optional>





RadiativeAnalysis::RadiativeAnalysis(const edm::ParameterSet& iConfig): 
	theConfig_(iConfig)
{
	generatorInfoTok                  = consumes<GenRunInfoProduct, edm::InRun>(edm::InputTag("generator"));
	genFilterTok                      = consumes<GenFilterInfo, edm::InLumi>(edm::InputTag("genFilterEfficiencyProducer"));
	isMCstudy_                        = iConfig.getParameter<bool>("isMCstudy");
	isMINIAOD_                        = iConfig.getParameter<bool>("isMINIAOD");
	genParticlesLabel                 = iConfig.getParameter<edm::InputTag>("genParticlesLabel");
	genParticlesTok                   = consumes<std::vector<reco::GenParticle>>(genParticlesLabel);
	MuonTag                           = iConfig.getParameter<edm::InputTag>("MuonTag");
	MuonTagTok                        = consumes<edm::View<reco::Muon>>(MuonTag);
	PhotonTag                         = iConfig.getParameter<edm::InputTag>("PhotonTag");
	PhotonTagTok                      = consumes<edm::View<reco::Photon>>(PhotonTag);
	PUInfo                            = iConfig.getParameter<edm::InputTag>("PUInfo");
	PUInfoTok                         = consumes<edm::View<PileupSummaryInfo>>(PUInfo);
	vertexBeamSpot                    = iConfig.getParameter<edm::InputTag>("vertexBeamSpot");
	vertexBeamSpotTok                 = consumes<reco::BeamSpot>(vertexBeamSpot);
	primaryvertex                     = iConfig.getParameter<edm::InputTag>("primaryvertex");
	primaryvertexTok                  = consumes<std::vector<reco::Vertex>>(primaryvertex);
	triggerbits                       = iConfig.getParameter<edm::InputTag>("triggerbits");
	triggerbitsTok                    = consumes<edm::TriggerResults>(triggerbits);
	ecalrechitEB                   = iConfig.getParameter<edm::InputTag>("ecalrechitEB");
	ecalrechitEBTok                = consumes<EcalRecHitCollection>(ecalrechitEB);
	ecalrechitEE                   = iConfig.getParameter<edm::InputTag>("ecalrechitEE");
	ecalrechitEETok                = consumes<EcalRecHitCollection>(ecalrechitEE);
	valMapTag					   = iConfig.getParameter<edm::InputTag>("mvaValuesMap");
	valMapTok					   = consumes<edm::ValueMap<float>>(valMapTag);

	trackTag                          = iConfig.getParameter<edm::InputTag>("TrackTag");
	if(isMINIAOD_){
		packedTrackTagTok             = consumes<edm::View<pat::PackedCandidate>>(trackTag);
		lostTrackTag                  = iConfig.getParameter<edm::InputTag>("LostTrackTag");
		lostTrackTagTok               = consumes<edm::View<pat::PackedCandidate>>(lostTrackTag);
	}
	else{
		trackTagTok                   = consumes<std::vector<reco::Track>>(trackTag);
	}
	trackBuilderTok                   = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));
	theBFieldTok                      = esConsumes<MagneticField, IdealMagneticFieldRecord>();
	iSetupGetTok                     = make_unique<EcalClusterLazyToolsBase::ESGetTokens>(consumesCollector());
	


	outputFile_                       = iConfig.getUntrackedParameter<std::string>("outputFile");

	event_counter_ = 0;

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
    // nothing needed here for now
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

      const double runXsec = genRunInfo->internalXSec().value();
      // one sample = one cross section; only the last run's value is kept, so warn if runs disagree
      if (genXsec_ > 0 && std::abs(runXsec - genXsec_) > 1e-6 * genXsec_) {
          edm::LogWarning("GenRunInfo") << "Cross section changed between runs: " << genXsec_ << " -> " << runXsec
                                        << " (run " << iRun.run() << "); the last value is saved.";
      }
      genXsec_   = runXsec;
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

    // merge the counts of all luminosity blocks; the efficiency is computed in endJob
    totalGenFilter_.mergeProduct(*genFilter);
    hasGenFilter_ = true;
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
	const auto& trackBuilder          = iSetup.getData(trackBuilderTok);

	edm::Handle<reco::BeamSpot> vertexBeamSpot ;
		iEvent.getByToken(vertexBeamSpotTok,vertexBeamSpot);
	edm::Handle<std::vector<reco::Vertex>> recVtxs;
		iEvent.getByToken(primaryvertexTok, recVtxs);
	if (!vertexBeamSpot.isValid() || recVtxs->empty()) {return;}

	edm::Handle<edm::View<reco::Photon>> photons;
		iEvent.getByToken(PhotonTagTok, photons);
	// data only: skip events without any reconstructed photon (before the track collection and the fits)
	if (!isMCstudy_ && photons->empty()) {return;}

	// ECAL rechit access for the photon shower covariance; built only for events that are analysed
	const auto& lazyTools             = EcalClusterLazyTools(iEvent, iSetupGetTok->get(iSetup), ecalrechitEBTok, ecalrechitEETok);

	edm::Handle<edm::View<reco::Muon>> muons;
        iEvent.getByToken(MuonTagTok, muons);

	// all tracks of the event as transient tracks (used for nTracks and the dimuon vertex compatibility)
	// MINIAOD: the same tracks PATTrackAndVertexUnpacker would rebuild
	// An alternative is to run the mentioned process before this analyzer, that would produce the tracks as a separate collection
	std::vector<reco::TransientTrack> eventTrackTTs;
	if(isMINIAOD_){
		edm::Handle<edm::View<pat::PackedCandidate>> packedCands;
			iEvent.getByToken(packedTrackTagTok, packedCands);
		edm::Handle<edm::View<pat::PackedCandidate>> lostTracks;
			iEvent.getByToken(lostTrackTagTok, lostTracks);
		for (const auto& c : *packedCands)
			if (c.hasTrackDetails() && c.charge() != 0 && c.numberOfHits() > 0) eventTrackTTs.push_back(trackBuilder.build(c.pseudoTrack()));
		for (const auto& c : *lostTracks)
			if (c.hasTrackDetails()) eventTrackTTs.push_back(trackBuilder.build(c.pseudoTrack()));
	}
	else{
		edm::Handle<std::vector<reco::Track>> tracks;
			iEvent.getByToken(trackTagTok, tracks);
		eventTrackTTs = trackBuilder.build(tracks);
	}
	const size_t nTracks = eventTrackTTs.size();
	edm::Handle<edm::ValueMap<float>> mvaValuesMap;
		iEvent.getByToken(valMapTok, mvaValuesMap);


	//-------FILL THE TREE WITH THE GENERAL VARIABLES-------//
	outputTree_->nMuons_ = muons->size();
	outputTree_->nPhotons_ = photons->size();
	outputTree_->nTracks_ = nTracks;

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
	

	//-----GEN DECAY CHAIN (MC study only)-------//
	// with isMCstudy only the muons and photons matched to the gen decay chain are analyzed
	edm::Handle<std::vector<reco::GenParticle>> genParticles;
	std::optional<GenDecayChain> genChain;
	std::vector<reco::Photon> genMatchedPhotons;
	std::vector<float> genMatchedPhotonMVAIDs;		// parallel to genMatchedPhotons
	if(isMCstudy_){
		iEvent.getByToken(genParticlesTok, genParticles);
		genChain.emplace(*genParticles);
		if (!genChain->info().isValid) return;
		outputTree_->addGenInfo(genChain->info());
		for (size_t i = 0; i < photons->size(); ++i) {
			const auto& photon = photons->at(i);
			if (!genChain->matchPhotonToChain(photon).isMatched) continue;
			genMatchedPhotons.push_back(photon);
			genMatchedPhotonMVAIDs.push_back(photonMVAIDs.at(i));
		}
	}

	//-----START OF THE DECAY RECONSTRUCTION------//
	TripleObjectVertexFit tripleVertexFitter;

	MuonSelector muonSelector;
	// 3 vectors with the same number of elements
	// a given vector element index corresponds to one muon pair
	// (e.g. {firstMuonIndices[i], secondMuonIndices[i]}, where commonVtxProbs[i] is their common vertex probability)
	auto [firstMuonIndices, secondMuonIndices, commonVtxProbs] = muonSelector.selectMuonPairs( *muons, trackBuilder );

	// MC study: keep only the pair whose both muons are matched to the gen signal muons
	// (smallest sum of the two deltaRs if more than one pair is matched)
	size_t genMatchedPair = firstMuonIndices.size();	// no pair matched
	float genMatchedMu1DeltaR = -9999.;
	float genMatchedMu2DeltaR = -9999.;
	if (genChain) {
		float bestDeltaRSum = std::numeric_limits<float>::max();
		for (size_t iPair = 0; iPair < firstMuonIndices.size(); iPair++) {
			auto mu1Match = genChain->matchMuonToChain( muons->at(firstMuonIndices[iPair]) );
			auto mu2Match = genChain->matchMuonToChain( muons->at(secondMuonIndices[iPair]) );
			if (!mu1Match.isMatched || !mu2Match.isMatched) continue;
			if (mu1Match.deltaR + mu2Match.deltaR < bestDeltaRSum) {
				bestDeltaRSum = mu1Match.deltaR + mu2Match.deltaR;
				genMatchedPair = iPair;
				genMatchedMu1DeltaR = mu1Match.deltaR;
				genMatchedMu2DeltaR = mu2Match.deltaR;
			}
		}
	}

	// loop over muon pairs
	for (size_t iPair = 0; iPair < firstMuonIndices.size(); iPair++)
	{	
		// if MC study (i.e. genChain != 0), analyze only the best matched muon pair
		if (genChain && iPair != genMatchedPair) continue;

		size_t mu1Idx = firstMuonIndices[iPair];
		size_t mu2Idx = secondMuonIndices[iPair];
		const reco::Muon& mu1 = muons->at(mu1Idx);
		const reco::Muon& mu2 = muons->at(mu2Idx);
		// references, not copies, so that pat::Muon track accessors are kept
		std::pair<const reco::Muon&, const reco::Muon&> muonPair( mu1, mu2 );

		outputTree_->resetDecayEntries();

		// muon variables
		// soft/tight IDs w.r.t. the highest-sum-pT PV (index 0 in MINIAOD)
		outputTree_->addMuonPair(mu1, mu2, recVtxs->at(bsandvtxVar.VtxIndex));
		outputTree_->mu1MVAScore_ = muonMVAIDs.at(mu1Idx);
		outputTree_->mu2MVAScore_ = muonMVAIDs.at(mu2Idx);
		outputTree_->DiMuon_KalmanVtxProb_ = commonVtxProbs.at(iPair);
		// dimuon "isolation": best vertex probability of the two muons with any other track of the event
		outputTree_->maxMuonsVertexComp_ = DimuonVertexCompatibility::maxThirdTrackVtxProb(
			trackBuilder.build(mu1.track()), trackBuilder.build(mu2.track()), eventTrackTTs );
		if (genChain) {
			outputTree_->mu1GenDeltaR_ = genMatchedMu1DeltaR;
			outputTree_->mu2GenDeltaR_ = genMatchedMu2DeltaR;
		}

		// MMG vertex fit
		// The TripleObjectVertexFit::fitMmgVertices function takes the considered muon pair and all photons in the event
		// (only the gen-matched photons in the MC study)
		// A struct is returned with variables for this muon pair and all the photons
		TripleObjectVertexFit::MmgFitResults tripleVertexResults;
		if (genChain) {
			tripleVertexResults = tripleVertexFitter.fitMmgVertices( muonPair, genMatchedPhotons, genMatchedPhotonMVAIDs, *recVtxs, lazyTools, bsandvtxVar, theBField, trackBuilder, &*genChain );
		}
		else {
			tripleVertexResults = tripleVertexFitter.fitMmgVertices( muonPair, *photons, photonMVAIDs, *recVtxs, lazyTools, bsandvtxVar, theBField, trackBuilder );
		}
		outputTree_->addMmgFitResults(tripleVertexResults);

		// one tree entry for each muon pair
		outputTree_->fill();

	} // muon pairs (mu+,mu-)
} // analyze


////////////////////////////////////////////////////////////
////////////////End Job///////////////////////////////////
void RadiativeAnalysis::endJob() {
  
	cout << "Total number of Events          : " << event_counter_ << endl;


    int nEvents = event_counter_;

	// filter efficiency from the counts merged over all luminosity blocks
	if (hasGenFilter_) {
		filterEff_    = totalGenFilter_.filterEfficiency(-1);
		filterEffErr_ = totalGenFilter_.filterEfficiencyError(-1);
	}

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
