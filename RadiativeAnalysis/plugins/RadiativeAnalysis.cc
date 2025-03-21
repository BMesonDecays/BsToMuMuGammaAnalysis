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
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GlobalIncludes.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ReferenceModeratorVertex.h"



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
	//pfCandTag                         = iConfig.getParameter<edm::InputTag>("pfCandTag");
    //pfCandTagTok                      = consumes<edm::View<pat::PackedCandidate>>(pfCandTag);
	trackTag                          = iConfig.getParameter<edm::InputTag>("pfCandTag");
	trackTagTok                       = consumes<std::vector<reco::Track>>(trackTag);
	if(isMINIAOD_){
		IsoTrackTag                       = iConfig.getParameter<edm::InputTag>("IsoTrackTag");
		IsoTrackTagTok                    = consumes<edm::View<pat::IsolatedTrack>>(IsoTrackTag);
	}
	convertedPhotonsTag               = iConfig.getParameter<edm::InputTag>("convertedPhotons");
	convertedPhotonsTagTok            = consumes<std::vector<pat::CompositeCandidate>>(convertedPhotonsTag);
	trackBuilderTok                   = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));
	theBFieldTok                      = esConsumes<MagneticField, IdealMagneticFieldRecord>();

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
	BsLowerMassCutAfterFit_           = iConfig.getParameter<double>("BsLowerMassCutAfterFit");
	BsUpperMassCutAfterFit_           = iConfig.getParameter<double>("BsUpperMassCutAfterFit");
	PionZeroPDGMass_                  = iConfig.getParameter<double>("PionZeroPDGMass");
	BdPDGMass_                        = iConfig.getParameter<double>("BdPDGMass");
	BpPDGMass_                        = iConfig.getParameter<double>("BpPDGMass");
	BsPDGMass_                        = iConfig.getParameter<double>("BsPDGMass");
	PionZeroPDGMass_                  = iConfig.getParameter<double>("PionZeroPDGMass");
	EtaMesonPDGMass_                  = iConfig.getParameter<double>("EtaMesonPDGMass");
	EtaPrimePDGMass_                  = iConfig.getParameter<double>("EtaPrimePDGMass");
	outputFile_                       = iConfig.getUntrackedParameter<std::string>("outputFile");
	verbose_                          = iConfig.getParameter<bool>("verbose");
	TestVerbose_                      = iConfig.getParameter<bool>("TestVerbose");

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
}

void RadiativeAnalysis::endJob() {
  bmmgRootTree_->writeFile();
  delete bmmgRootTree_;
  cout << "Total number of Events          : " << event_counter_ << endl;
  cout << "Total number of Tagged muons    : " << muoncounter_   << endl;
  cout << "Total number of Tagged electrons: " << elecounter_    << endl;
  cout << "Total number of Tagged jets     : " << jetcounter_    << endl;
  cout << "Max amount of Tag muons         : " << tagmucounter_ <<  endl;
  cout << "Max number of photon            : " << photoncounter_ << endl;
}

// ------------ method called for each event  ------------
void RadiativeAnalysis::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

	event_counter_++;
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
	excludedPhotons.clear();	
	
	edm::Handle<reco::BeamSpot> vertexBeamSpot ;
	iEvent.getByToken(vertexBeamSpotTok,vertexBeamSpot);
	edm::Handle<std::vector<reco::Vertex>> recVtxs;
        iEvent.getByToken(primaryvertexTok, recVtxs);
	if (!vertexBeamSpot.isValid() || recVtxs->empty()) {return;}

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
       	const auto& theBField    = iSetup.getData(theBFieldTok);
        	
	int nBs=0;
	edm::Handle<edm::View<reco::GenParticle> > genParticles;
	if(isMCstudy_)
              {
                     iEvent.getByToken(genParticlesTok, genParticles);
                     //std::cout<<"genparticles:   "<<genParticles->size()<<"\n";
		     for( size_t i = 0; i < genParticles->size(); ++ i ) {
		     const reco::GenParticle & genBsCand = (*genParticles)[ i ];
		     if(abs(genBsCand.pdgId())/100==5){
			     //std::cout<< " B Cand PDGID : "<< genBsCand.pdgId()<< "\n";
			     //if(abs(genBsCand.pdgId()) == 531)nBs++;			     
		     }
		     }
		     
		     //std::cout<<" number of BsCandidates : "<<  nBs << "\n";
                     fillMCInfo(genParticles);
               }



	 edm::Handle<edm::TriggerResults> hltbits;
	 iEvent.getByToken(triggerbitsTok, hltbits);
	 std::vector<std::string> triggersOfInterest = {
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

        const  edm::TriggerNames & triggerNames_ = iEvent.triggerNames(*hltbits);
		if(isMINIAOD_){
		 edm::Handle<edm::View<pat::TriggerObjectStandAlone>> triggerObjects;
                 iEvent.getByToken(triggerobjTok, triggerObjects);
                 edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;
                 iEvent.getByToken(prescaleTok, triggerPrescales);

		if (!triggerPrescales.isValid()) {
                edm::LogError("TriggerPrescales") << "Trigger prescales handle is invalid!";
                return;
        }
		 pat::PackedTriggerPrescales nonConstTriggerPrescales = *triggerPrescales;
		 nonConstTriggerPrescales.setTriggerNames(triggerNames_);

	}
	
	int ntrigs = hltbits->size();
	for (int itrig = 0; itrig != ntrigs; ++itrig){
		TString trigName = triggerNames_.triggerName(itrig);
		std::string triggerNameStd =  trigName.Data();
		for (const std::string& baselineTrigName : triggersOfInterest) {
			if (triggerNameStd.find(baselineTrigName) != std::string::npos) {
				//if(hltbits->accept(itrig)==1)std::cout << "Matched Trigger: " << trigName << ",  decision : "<< hltbits->accept(itrig) << "\n";
			}
		}

		
if(triggerNameStd.find("HLT_DoubleMu4_Jpsi_Displaced")!=std::string::npos)      bmmgRootTree_->triggerbit_HLTDimuon4JpsiDisplaced_           = hltbits->accept(itrig);
if(triggerNameStd.find("HLT_DoubleMu4_Jpsi_NoVertexing")!=std::string::npos)    bmmgRootTree_->triggerbit_HLTDimuon4JpsiNoVertexing_         = hltbits->accept(itrig);
if(triggerNameStd.find("HLT_DoubleMu4_JpsiTrkTrk_Displaced")!=std::string::npos)bmmgRootTree_->triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_     = hltbits->accept(itrig);
if(triggerNameStd.find("HLT_DoubleMu4_3_LowMass")!=std::string::npos)           bmmgRootTree_->triggerbit_HLT_DoubleMu4_LowMass_             = hltbits->accept(itrig);
if(triggerNameStd.find("HLT_DoubleMu4_LowMass_Displaced")!=std::string::npos)   bmmgRootTree_->triggerbit_HLT_DoubleMu4_LowMass_Displaced_   = hltbits->accept(itrig);
if(triggerNameStd.find("HLT_DoubleMu4_3_Bs")!=std::string::npos)                bmmgRootTree_->triggerbit_HLT_DoubleMu4_3_Bs_                = hltbits->accept(itrig);
if(triggerNameStd.find("HLT_DoubleMu4_3_Photon4_BsToMMG")!=std::string::npos)   bmmgRootTree_->triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_   = hltbits->accept(itrig);
if(triggerNameStd.find("HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG")!=std::string::npos) bmmgRootTree_->triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_ = hltbits->accept(itrig);
                
	}


        
	    //edm::Handle<View<pat::PackedCandidate>> pfCands;
	    //iEvent.getByToken(pfCandTagTok, pfCands);
	    edm::Handle<std::vector<reco::Muon>> muons;
        iEvent.getByToken(MuonTagTok, muons);
		edm::Handle<std::vector<pat::CompositeCandidate>> convPhotons;
        iEvent.getByToken(convertedPhotonsTagTok, convPhotons);
		const pat::CompositeCandidateCollection * conversions = convPhotons.product();
		edm::Handle<std::vector<reco::Track>> tracks;
		iEvent.getByToken(trackTagTok, tracks);


		
		TrippleObjectVertex  tripvtxObservables;
		auto triDecayVar = tripvtxObservables.TrippleObjectVertexObservables(*muons, *conversions, bsandvtxVar, theBField, 
			nominalMuonMass, nominalElectronMass);
		/*TetraObjectVertex tetradcObservables;
		auto tetraDecayVar = tetradcObservables.TetraObjectVertexObservables(*muons, *conversions, theBField, nominalMuonMass, nominalElectronMass);
		std::cout<< " the mass of the tetraobject vertex : "<< tetraDecayVar.mass << "\n";
		*/
	    ReferenceModeratorVertex refmodvtxObservables;
		auto refmodvtxVar = refmodvtxObservables.ReferenceModeratorVertexObservables(*muons, *tracks, bsandvtxVar, theBField, 
			nominalMuonMass, nominalKaonMass);
		std::cout<<"the dimuon mass from bs to jpsiphi ======================== : "<< refmodvtxVar.dimuonMass<< "\n";
        
		
		bmmgRootTree_->DiMuonM_beffit_ = triDecayVar.dimuonMass;
		bmmgRootTree_->DiMuonEta_beffit_ = triDecayVar.dimuonEta;
		bmmgRootTree_->DiMuonPhi_beffit_ = triDecayVar.dimuonPhi;
		bmmgRootTree_->DiMuonPt_beffit_ = triDecayVar.dimuonPt;
		bmmgRootTree_->DiMuon_ResonanceType_ = triDecayVar.resonanceFlag;
		bmmgRootTree_->DiMuon_vtxProb_       = triDecayVar.dimuonvtxprob;
		bmmgRootTree_->DiMuon_CosineAlpha_   = triDecayVar.opening_angle;
		bmmgRootTree_->DiMuon_DCA_           = triDecayVar.mumudca;
		bmmgRootTree_->DiMuon_Chi2pv_KVFvtx_ = triDecayVar.dimuonchi2;
		//bmmgRootTree_->DiMuon_Mahalanobis_   = triDecayVar.mahalanobis;
		bmmgRootTree_->DiMuon_Lxy_           = triDecayVar.dimuonlxy;
		bmmgRootTree_->DiMuon_Lxyerr_        = triDecayVar.dimuonlxyerr;
		bmmgRootTree_->DiMuon_LxyOverPt_     = triDecayVar.dimuonlxyOverPt;
		bmmgRootTree_->mu1Pt_beffit_         = triDecayVar.mu1pt;
		bmmgRootTree_->mu1Pz_beffit_         = triDecayVar.mu1pz;
		bmmgRootTree_->mu1Eta_beffit_        = triDecayVar.mu1eta;
		bmmgRootTree_->mu1Phi_beffit_        = triDecayVar.mu1phi;
		bmmgRootTree_->mu2Phi_beffit_        = triDecayVar.mu2phi;
		bmmgRootTree_->mu2Pt_beffit_         = triDecayVar.mu2pt;
		bmmgRootTree_->mu2Pz_beffit_         = triDecayVar.mu2pz;
		bmmgRootTree_->mu2Eta_beffit_        = triDecayVar.mu2eta;
		bmmgRootTree_->MuonPairDR_           = triDecayVar.muonpairdr;
		bmmgRootTree_->Mu1TrkBSDxy_          = triDecayVar.mu1trkbsxy;
		bmmgRootTree_->Mu1TrkBSDz_           = triDecayVar.mu1trkbsz;
		bmmgRootTree_->Mu2TrkBSDxy_          = triDecayVar.mu2trkbsxy;
		bmmgRootTree_->Mu2TrkBSDz_           = triDecayVar.mu2trkbsz;
		bmmgRootTree_->Mu1PixelHits_         = triDecayVar.mu1pixelhits;
		bmmgRootTree_->Mu1TrackerHits_       = triDecayVar.mu1trackerhits;
		bmmgRootTree_->Mu1isGood_            = triDecayVar.mu1isgood;
		bmmgRootTree_->Mu1InnerTrkHighQuality_ = triDecayVar.mu1innertrkhq;
		bmmgRootTree_->Mu2PixelHits_         = triDecayVar.mu2pixelhits;
		bmmgRootTree_->Mu2TrackerHits_       = triDecayVar.mu2trackerhits;
		bmmgRootTree_->Mu2isGood_            = triDecayVar.mu2isgood;
		bmmgRootTree_->Mu2InnerTrkHighQuality_ = triDecayVar.mu2innertrkhq;
		bmmgRootTree_->DiMuon_mu1Cat_alone_  = triDecayVar.diMuon_mu1Cat;
		bmmgRootTree_->DiMuon_mu2Cat_alone_  = triDecayVar.diMuon_mu2Cat;
		bmmgRootTree_->DiMuon_mu1nPixHits_alone_ = triDecayVar.diMuon_mu1PixelHits;
		bmmgRootTree_->DiMuon_mu2nPixHits_alone_ = triDecayVar.diMuon_mu2PixelHits;
		bmmgRootTree_->BsM_beffit_          = triDecayVar.BsMass;
		bmmgRootTree_->BsEta_beffit_        = triDecayVar.BsEta;
		bmmgRootTree_->BsPhi_beffit_        = triDecayVar.BsPhi;
		bmmgRootTree_->BsPt_beffit_         = triDecayVar.BsPt;
		bmmgRootTree_->HadronMass_fromVertexFit_ = triDecayVar.fittedBmass;
		bmmgRootTree_->Bs_vtxProb_          = triDecayVar.BsVtxProb;
		bmmgRootTree_->BsCt3D_              = triDecayVar.BsCt3D;
		bmmgRootTree_->BsCt2D_    	        = triDecayVar.BsCt2D;
		bmmgRootTree_->BsCt2DBS_    	    = triDecayVar.BsCt2DBS;
		
		edm::Handle<std::vector<reco::Photon>> photon;
		iEvent.getByToken(PhotonTagTok, photon);
		bmmgRootTree_->photonMultiplicity_ = photon->size();
		RecoPhotons recoPhotonObserbles;
		std::vector<RecoPhotons::PhotonVariables> photonVar = recoPhotonObserbles.PhotonObservables(*photon);
		if (!photonVar.empty()) {
		for (size_t iPhoton = 0; iPhoton < photonVar.size(); ++iPhoton) {
			const auto& ipatPhoton = (*photon)[iPhoton];
			if (ipatPhoton.pt() > 50.0 || !ipatPhoton.isEB()) {
				excludedPhotons.insert(iPhoton);
				continue;
			}
		const RecoPhotons::PhotonVariables& leadingPhoton = photonVar[0]; //leading photon
		bmmgRootTree_->photonPt_ = leadingPhoton.pt;
		bmmgRootTree_->photonEta_ = leadingPhoton.eta;
		bmmgRootTree_->photonPhi_ = leadingPhoton.phi;
		bmmgRootTree_->photonEnergy_ = leadingPhoton.energy;
		bmmgRootTree_->photonET_ = leadingPhoton.et;
		bmmgRootTree_->photonSSSigmaiEtaiEta_ = leadingPhoton.sigmaIEtaIEta;
		bmmgRootTree_->photonSSSigmaiEtaiPhi_ = leadingPhoton.sigmaIEtaIPhi;
		bmmgRootTree_->photonSSSigmaiPhiiPhi_ = leadingPhoton.sigmaIPhiIPhi;
		bmmgRootTree_->photonSCEnergy_ = leadingPhoton.scEnergy;
		bmmgRootTree_->photonSCRawEnergy_ = leadingPhoton.scRawEnergy;
		bmmgRootTree_->photonSCR9_ = leadingPhoton.r9;
		bmmgRootTree_->photonSCHadTowOverEm_ = leadingPhoton.hadTowOverEm;
		bmmgRootTree_->photonSShcalDepth1OverEcal_ = leadingPhoton.hcalDepth1OverEcal;
		bmmgRootTree_->photonSShcalDepth2OverEcal_ = leadingPhoton.hcalDepth2OverEcal;
		bmmgRootTree_->photonSShcalDepth1OverEcalBc_ = leadingPhoton.hcalDepth1OverEcalBc;
		bmmgRootTree_->photonSShcalDepth2OverEcalBc_ = leadingPhoton.hcalDepth2OverEcalBc;
		std::fill(std::begin(bmmgRootTree_->photonSShcalOverEcal_), std::end(bmmgRootTree_->photonSShcalOverEcal_), 0.f);
		std::fill(std::begin(bmmgRootTree_->photonSShcalOverEcalBc_), std::end(bmmgRootTree_->photonSShcalOverEcalBc_), 0.f);
		for (size_t k = 0; k < leadingPhoton.hcalOverEcal.size(); ++k) {
			bmmgRootTree_->photonSShcalOverEcal_[k] = leadingPhoton.hcalOverEcal[k];
		}
		for (size_t k = 0; k < leadingPhoton.hcalOverEcalBc.size(); ++k) {
			bmmgRootTree_->photonSShcalOverEcalBc_[k] = leadingPhoton.hcalOverEcalBc[k];
		}
		bmmgRootTree_->photonSSmaxEnergyXtal_ = leadingPhoton.maxEnergyXtal;
		bmmgRootTree_->photonSSeffSigmaRR_ = leadingPhoton.effSigmaRR;
		bmmgRootTree_->photonSCEta_ = leadingPhoton.scEta;
		bmmgRootTree_->photonSCPhi_ = leadingPhoton.scPhi;
		bmmgRootTree_->photonSCEtaWidth_ = leadingPhoton.scEtaWidth;
		bmmgRootTree_->photonSCPhiWidth_ = leadingPhoton.scPhiWidth;
		bmmgRootTree_->photonSCBrem_ = leadingPhoton.scPhiWidth / leadingPhoton.scEtaWidth;
		}
	}





	/*pat::CompositeCandidate DiGammaCandidate;
			DiGammaCandidate.addDaughter(ipatPhoton);
			DiGammaCandidate.addDaughter(jpatPhoton);
			AddFourMomenta addP4;
			addP4.set(DiGammaCandidate);*/

	


			/*BsToPhi(KK)Gamma - need corresponding MC sample,run on data if the MC is not available
		        for (size_t k=0; k< pfCands->size(); ++k){
				const pat::PackedCandidate & track1 = (*pfCands)[k];
				if (track1.charge()<0)continue;
				if (track1.pt() < KaonTrackPtCut_) continue;
				if (track1.numberOfHits() < 5)continue;
				if(!track1.trackHighPurity()) continue;
				const reco::Track &  pseudotrkkp = (*pfCands)[k].pseudoTrack();
				if (pseudotrkkp.charge()<0) continue;
				TransientTrack KPTT = trackBuilder.build(&pseudotrkkp);
				TrajectoryStateClosestToPoint KPTS = KPTT.impactPointTSCP();
				if(!KPTS.isValid())continue;
				if (!track1.clone()->hasTrackDetails())continue;
				pat::PackedCandidate *trackkp = track1.clone();
				for (size_t l=k+1; l< pfCands->size(); ++l){
					const pat::PackedCandidate & track2 = (*pfCands)[l];
					if ( !track2.hasTrackDetails() )continue;
					if (track2.charge()>0) continue;
					if (track2.pt() < KaonTrackPtCut_) continue;
					if ( track2.numberOfHits()<5) continue;
					if(!track2.trackHighPurity()) continue;
					const reco::Track &  pseudotrkkm = (*pfCands)[l].pseudoTrack();
					if (pseudotrkkm.charge()>0) continue;
					TransientTrack KMTT = trackBuilder.build(&pseudotrkkm);
					TrajectoryStateClosestToPoint KMTS = KMTT.impactPointTSCP();
					if(!KMTS.isValid())continue;
					if (KPTS.isValid() && KMTS.isValid()) {
						ClosestApproachInRPhi cAppK;
						cAppK.calculate(KPTS.theState(), KMTS.theState());
						KKDCA = cAppK.distance();
					}
					if(KKDCA > 0.5)continue;
					if (!track2.clone()->hasTrackDetails())continue;
					pat::PackedCandidate *trackkm = track2.clone();
					pat::CompositeCandidate phiCand;
					trackkp->setMass(kaonmass);
                                        phiCand.addDaughter(*trackkp);
                                        trackkm->setMass(kaonmass);
                                        phiCand.addDaughter(*trackkm);
					AddFourMomenta p4phi;
					p4phi.set(phiCand);
					if (abs(phiCand.mass()- nominalPhiMass) > PhiMassWindowBeforeFit_) continue;
					

					pat::CompositeCandidate phigammaCand;
					trackkp->setMass(kaonmass);
					phigammaCand.addDaughter(*trackkp);
					trackkm->setMass(kaonmass);
					phigammaCand.addDaughter(*trackkm);
					phigammaCand.addDaughter(ipatPhoton);
					AddFourMomenta p4phigamma;
					p4phigamma.set(phigammaCand);
					if (phigammaCand.mass() < BsLowerMassCutBeforeFit_ || phigammaCand.mass() > BsUpperMassCutBeforeFit_) continue;

					std::cout<< " could we take out the value of the bs mass  : kind of phi gamma candidate : " << phigammaCand.mass()<< "\n";
					vector<TransientTrack> phi_transienttrk;
					phi_transienttrk.push_back(trackBuilder.build(&pseudotrkkp));//pseudotrkkm
					phi_transienttrk.push_back(trackBuilder.build(&pseudotrkkm));
					KalmanVertexFitter kvfphi;
					TransientVertex tvphi = kvfphi.vertex(phi_transienttrk);
					if (!tvphi.isValid()) continue;
					GlobalError gigi=tvphi.positionError();
					//This would be the bs vertex since there will be no transient tracks for photons But I am not sure if this is the correct way to do it 
					Vertex kalmanvertex_phi = tvphi;
					double vtxProb_Phi = TMath::Prob(kalmanvertex_phi.chi2(),(int)kalmanvertex_phi.ndof());
					if (vtxProb_Phi < 1e-4) continue;
					

					bmmgRootTree_->K1Pt_beffit_   = track1.pt();
					bmmgRootTree_->K1Pz_beffit_   = track1.pz();
					bmmgRootTree_->K1Eta_beffit_  = track1.eta();
					bmmgRootTree_->K1Phi_beffit_  = track1.phi();
					bmmgRootTree_->K2Pt_beffit_   = track2.pt();
					bmmgRootTree_->K2Pz_beffit_   = track2.pz();
					bmmgRootTree_->K2Eta_beffit_  = track2.eta();
					bmmgRootTree_->K2Phi_beffit_  = track2.phi();
					bmmgRootTree_->PhiM_beffit_   = phiCand.mass();
					bmmgRootTree_->PhiEta_beffit_ = phiCand.eta();
					bmmgRootTree_->PhiPhi_beffit_ = phiCand.phi();
					bmmgRootTree_->PhiPt_beffit_  = phiCand.pt();

					KinematicConstrainedFit Kfitter;
					bool fitSuccess = Kfitter.dobsphikkgFit(phi_transienttrk, nominalKaonMass, nominalKaonMass);
					std::cout<< " the fit success : "<< fitSuccess << "\n";
					if(fitSuccess != 1) continue;

					math::XYZVector      pperp(track1.px() + track2.px(), track1.py() + track2.py(), 0.);
					reco::Vertex::Point  vpoint = kalmanvertex_phi.position();
					double chi2_pv_kalmanvtx = (PVx - vpoint.x()) * (PVx - vpoint.x()) / (PVx * PVx) +
					(PVy - vpoint.y()) * (PVy - vpoint.y()) / (PVy * PVy) +
					(PVz - vpoint.z()) * (PVz - vpoint.z()) / (PVz * PVz);
					AlgebraicVector3 predefinedPV(PVx, PVy, PVz);
					AlgebraicVector3 recoVtx( vpoint.x(), vpoint.y(), vpoint.z());
					AlgebraicVector3 diff = predefinedPV - recoVtx;
					TVectorD diffVector(3);
					diffVector[0] = diff[0];
					diffVector[1] = diff[1];
					diffVector[2] = diff[2];
					TMatrixD covarianceMatrix(3, 3);
   					covarianceMatrix(0,0) = gigi.cxx();
					covarianceMatrix(0,1) = 0.0;
					covarianceMatrix(0,2) = 0.0;
					covarianceMatrix(1,0) = gigi.cyx();
					covarianceMatrix(1,1) = gigi.cyy();
					covarianceMatrix(1,2) = 0.0;
   					covarianceMatrix(2,0) = gigi.czx();
  					covarianceMatrix(2,1) = gigi.czy();
					covarianceMatrix(2,2) = gigi.czz();
					//TMatrixD diffMatrix(3, 1);  // Column vector
					//diffMatrix(0, 0) = diff[0];
					//diffMatrix(1, 0) = diff[1];
					//diffMatrix(2, 0) = diff[2];
					TMatrixD invCovarianceMatrix = covarianceMatrix.Invert();
					//TMatrixD result = diffMatrix.T() * invCovarianceMatrix * diffMatrix;
					double mahalanobisDistanceSquared = diffVector * (invCovarianceMatrix * diffVector);
					double mahalanobisDistance = std::sqrt(mahalanobisDistanceSquared);
					if (mahalanobisDistance > 5.0) continue;
					GlobalPoint secondaryVertex (vpoint.x(), vpoint.y(), vpoint.z());
					GlobalPoint displacementFromBeamspot( -1*((BSx -  secondaryVertex.x()) +
					  (secondaryVertex.z() - BSz) * BSdxdz),-1*((BSy - secondaryVertex.y())+  (secondaryVertex.z() - BSz) * BSdydz), 0);
					reco::Vertex::Point vperp(displacementFromBeamspot.x(),displacementFromBeamspot.y(),0.);
					double cosAlpha = vperp.Dot(pperp)/(vperp.R()*pperp.R());


					
					
					
					
					//RefCountedKinematicParticle bs = Kfitter.getParticle();
					//RefCountedKinematicVertex bVertex = Kfitter.getVertex();
					//AlgebraicVector7 b_par = bs->currentState().kinematicParameters().vector();
					//AlgebraicSymMatrix77 bs_er = bs->currentState().kinematicParametersError().matrix();
					//GlobalError vertexPositionError  = tvphi.positionError();
					//std::cout<< " the global position error : "<< vertexPositionError << "\n";
					bmmgRootTree_->BsPhiGammaM_beffit_   = phigammaCand.mass() ;
					bmmgRootTree_->BsPhiGammaEta_beffit_ = phigammaCand.eta();
					bmmgRootTree_->BsPhiGammaPhi_beffit_ = phigammaCand.phi();
					bmmgRootTree_->BsPhiGammaPt_beffit_  = phigammaCand.pt();
					bmmgRootTree_->BsPhiGamma_vtxProb_   = vtxProb_Phi;
					bmmgRootTree_->BsPhiGamma_CosineAlpha_          = cosAlpha;
					bmmgRootTree_->BsPhiGamma_KKDCA_                = KKDCA;
					bmmgRootTree_->BsPhiGamma_Mahalanobis_          = mahalanobisDistance;
					bmmgRootTree_->BsPhiGamma_Chi2pv_KVFvtx_        = chi2_pv_kalmanvtx;
					bmmgRootTree_->BsPhiGamma_Mahalanobis_          = mahalanobisDistance;


					RefCountedKinematicTree reftree = Kfitter.getTree();
					vector< RefCountedKinematicParticle > bs_children = reftree->finalStateParticles();
					AlgebraicVector7 bs_par1 = bs_children[0]->currentState().kinematicParameters().vector();
					AlgebraicVector7 bs_par2 = bs_children[1]->currentState().kinematicParameters().vector();
					double pt1 = sqrt(bs_par1[3]*bs_par1[3]+bs_par1[4]*bs_par1[4]);
					double pt2 = sqrt(bs_par2[3]*bs_par2[3]+bs_par2[4]*bs_par2[4]);
					std::cout<< " the pt1 and pt2 : "<< pt1 << "\t" << pt2 << "\n";
					bmmgRootTree_->K1Pt_beffit_   = pt1;
					bmmgRootTree_->K2Pt_beffit_   = pt2;
					TLorentzVector pK1;
					double en1 = sqrt(bs_par1[3]*bs_par1[3]+bs_par1[4]*bs_par1[4]+bs_par1[5]*bs_par1[5]+bs_par1[6]*bs_par1[6]);
					pK1.SetPxPyPzE(bs_par1[3],bs_par1[4],bs_par1[5],en1);
					TLorentzVector pK2;
					double en2 = sqrt(bs_par2[3]*bs_par2[3]+bs_par2[4]*bs_par2[4]+bs_par2[5]*bs_par2[5]+bs_par2[6]*bs_par2[6]);
					pK2.SetPxPyPzE(bs_par2[3],bs_par2[4],bs_par2[5],en2);
					TLorentzVector pPhi = pK1 + pK2;
					bmmgRootTree_->BsPhiGamma_PhiM_fit_   = pPhi.M();
					std::cout << " the phi mass : " << pPhi.M() << "\n";
			

					
					
					
					

				}
			}//End of the BsToPhi(KK)Gamma loop 

*/






	




bmmgRootTree_->fill();
}

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
