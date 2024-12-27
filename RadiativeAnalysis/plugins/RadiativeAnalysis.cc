// -*- C++ -*-
// Package:    BsToMuMuGammaAnalysis/RadiativeAnalysis
// Class:      RadiativeAnalysis
// Original Author:  Alibordi Muhammad
//         Created:  Fri, 12 Jul 2024 10:27:36 GMT
//============================================================
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeAnalysis.h"


#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include <iostream>
#include <TMath.h>
#include <regex>
#include <algorithm>
#include <typeinfo>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "RecoVertex/AdaptiveVertexFit/interface/AdaptiveVertexFitter.h"
#include "DataFormats/PatCandidates/interface/IsolatedTrack.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/Registry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Common/interface/Association.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Framework/interface/TriggerNamesService.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/ParticleMass.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include <RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h>
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/Candidate/interface/CompositeCandidateFwd.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonTrackLinks.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/CaloMuon.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "DataFormats/Candidate/interface/CandMatchMap.h"
#include "DataFormats/Candidate/interface/OverlapChecker.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/Measurement1D.h"
#include "DataFormats/L1Trigger/interface/L1MuonParticle.h"
#include "DataFormats/L1Trigger/interface/L1MuonParticleFwd.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingVertexContainer.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"
#include "DataFormats/TrackReco/interface/DeDxHit.h"
#include "RecoTracker/DeDx/interface/DeDxTools.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripMatchedRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/ProjectedSiStripRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "RecoVertex/PrimaryVertexProducer/interface/PrimaryVertexProducerAlgorithm.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/MuonSeed/interface/L2MuonTrajectorySeed.h"
#include "DataFormats/MuonSeed/interface/L2MuonTrajectorySeedCollection.h"
#include "DataFormats/MuonSeed/interface/L3MuonTrajectorySeed.h"
#include "DataFormats/MuonSeed/interface/L3MuonTrajectorySeedCollection.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "MuonAnalysis/MuonAssociators/interface/L1MuonMatcherAlgo.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "DataFormats/PatCandidates/interface/GenericParticle.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/BTauReco/interface/JetTag.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/JetCorrFactors.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/BTauReco/interface/SecondaryVertexTagInfo.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "DataFormats/PatCandidates/interface/IsolatedTrack.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
using namespace reco;
using namespace edm;
using namespace std;
using namespace pat;


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
	genParticlesLabel                 = iConfig.getParameter<InputTag>("genParticlesLabel");
        genParticlesTok                   = consumes<edm::View<reco::GenParticle>>(genParticlesLabel);
        MuonTag                           = iConfig.getParameter<edm::InputTag>("MuonTag");
        MuonTagTok                        = consumes<edm::View<pat::Muon>>(MuonTag);
	JetTag                            = iConfig.getParameter<edm::InputTag>("JetTag");
        JetTagTok                         = consumes<edm::View<pat::Jet>>(JetTag);
	PhotonTag                         = iConfig.getParameter<edm::InputTag>("PhotonTag");
        PhotonTagTok                      = consumes<edm::View<reco::Photon>>(PhotonTag);
	OOTPhotonTag                      = iConfig.getParameter<edm::InputTag>("OOTPhotonTag");
        OOTPhotonTagTok                   = consumes<edm::View<pat::Photon>>(OOTPhotonTag);
	ElectronTag                       = iConfig.getParameter<edm::InputTag>("ElectronTag");
        ElectronTagTok                    = consumes<edm::View<pat::Electron>>(ElectronTag);
	/*SuperClusterTag                   = iConfig.getParameter<edm::InputTag>("SuperClusterTag");
	SuperClusterTagTok                = consumes<edm::View<reco::SuperCluster>>(SuperClusterTag);
	OOTSuperClusterTag                = iConfig.getParameter<edm::InputTag>("OOTSuperClusterTag");
        OOTSuperClusterTagTok             = consumes<edm::View<reco::SuperCluster>>(OOTSuperClusterTag);
	*/
	PUInfo                            = iConfig.getParameter<InputTag>("PUInfo");
        PUInfoTok                         = consumes<edm::View<PileupSummaryInfo>>(PUInfo);
        vertexBeamSpot                    = iConfig.getParameter<edm::InputTag>("vertexBeamSpot");
        vertexBeamSpotTok                 = consumes<reco::BeamSpot>(vertexBeamSpot);
	primaryvertex                     = iConfig.getParameter<edm::InputTag>("primaryvertex");
        primaryvertexTok                  = consumes<edm::View<reco::Vertex>>(primaryvertex);
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
	trackTagTok                       = consumes<edm::View<reco::Track>>(trackTag);
	if(isMINIAOD_){
		IsoTrackTag                       = iConfig.getParameter<edm::InputTag>("IsoTrackTag");
		IsoTrackTagTok                    = consumes<edm::View<pat::IsolatedTrack>>(IsoTrackTag);
	}
        trackBuilderTok                   = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));


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


	int    VtxIndex              = -99;
	//====================================================Beam Spot
        double BSx         = -9999999.;
	double BSy         = -9999999.;
	double BSz         = -9999999.;
	double BSdx        = -9999999.;
	double BSdy        = -9999999.;
	double BSdz        = -9999999.;
	double BSdxdz      = -9999999.;
	double BSdydz      = -9999999.;
	double BSsigmaZ    = -9999999.;
	double BSdsigmaZ   = -9999999.;
	//========================================================PV
	double PVx         = -9999999.;
	double PVy         = -9999999.;
	double PVz         = -9999999.;
	double PVerrx      = -9999999.;
	double PVerry      = -9999999.;
	double PVerrz      = -9999999.;
	
	excludedPhotons.clear();	
	
	
	
	
	edm::Handle<reco::BeamSpot> vertexBeamSpot ;
	iEvent.getByToken(vertexBeamSpotTok,vertexBeamSpot);
	BSx = vertexBeamSpot->x0();
	BSy = vertexBeamSpot->y0();
	BSz = vertexBeamSpot->z0();
	BSdx = vertexBeamSpot->x0Error();
	BSdy = vertexBeamSpot->y0Error();
	BSdz = vertexBeamSpot->z0Error();
	BSdxdz = vertexBeamSpot->dxdz();
	BSdydz = vertexBeamSpot->dydz();
	BSsigmaZ = vertexBeamSpot->sigmaZ();
	BSdsigmaZ = vertexBeamSpot->sigmaZ0Error();

	
	edm::Handle<edm::View<reco::Vertex>> recVtxs;
	iEvent.getByToken(primaryvertexTok, recVtxs);
	if(recVtxs->empty())return;
	const reco::Vertex &recVtxs_track = recVtxs->front();
	bmmgRootTree_->NTracksPVbeforecut_ = recVtxs_track.nTracks();
        bmmgRootTree_->NVerticesbeforecut_ = recVtxs->size();	
	NSelectedVertices = 0; 
	/*for (const auto& vertex : *recVtxs) {
		for (auto trackRef = vertex.tracks_begin(); trackRef != vertex.tracks_end(); ++trackRef) {
			std::cout << "Referenced track key: " << trackRef->key() << "\n";
			std::cout << "Referenced track weight: " << vertex.trackWeight(*trackRef) << "\n";
		}
	}*/
	
	for (size_t iVtx = 0; iVtx < recVtxs->size(); ++iVtx) {
		VtxIndex = iVtx;
		const reco::Vertex& vtx = (*recVtxs)[iVtx];
		int iteratorCov =0;
		for(int i = 0; i < 3 ; ++i){
			for(int j =0; j< 3; ++j){
				bmmgRootTree_->PVcovariance_[iteratorCov++] = vtx.covariance(i, j);
			}
		}

	        bmmgRootTree_->PVndof_ = vtx.ndof();
	        bmmgRootTree_->PVrho_ = vtx.position().Rho();
	        if(!vtx.isValid())continue;
	        if(vtx.isFake())continue;
	        if(vtx.ndof() < 4)continue;
	        if(fabs(vtx.z()) >= 24.0)continue;
	        if(vtx.position().Rho() >= 2)continue;	
	 	NSelectedVertices++;
   

			 /*for(reco::Vertex::trackRef_iterator trackRef = vtx.tracks_begin(); trackRef !=vtx.tracks_end(); ++trackRef){
				 const reco::TrackBaseRef& vtx_trackRef = *trackRef;
				 if (vtx_trackRef.isNonnull() && vtx_trackRef.isAvailable()) {
					 const reco::Track& VtxTrack = *vtx_trackRef.castTo<reco::TrackRef>();
					 PtSumVertex += std::abs(VtxTrack.pt());
				 }
				 else {
					 std::cout << "Invalid track reference : "<<"\n";
				 }
			 }
			 if (PtSumVertex >  MinPtVertex) {
				 VtxIndex = iVtx;
				 MinPtVertex = PtSumVertex;
				 std::cout<<" min/max pt sum vertex : "<< MinPtVertex <<"\t"<< " & veretx index : " << VtxIndex<< "\n";
			 }*/
		 
	}
	bmmgRootTree_->NVerticesaftercut_ = NSelectedVertices;
	
	const Vertex &RecVtx = (*recVtxs)[VtxIndex];
	if(VtxIndex !=-99) {
		bmmgRootTree_->isPV_ = 1;
		PVx = RecVtx.x();
		PVy= RecVtx.y();
		PVz= RecVtx.z();
		PVerrx=RecVtx.xError();
		PVerry=RecVtx.yError();
		PVerrz=RecVtx.zError();
	}
	else { 
		bmmgRootTree_->isBS_ = 1;
		PVx=BSx;
		PVy=BSy;
		PVz=BSz;
		PVerrx=BSdx;
		PVerry=BSdy;
		PVerrz=BSdz;
	}
	bmmgRootTree_->getVtx(BSx,BSy,BSz,PVx,PVy,PVz,PVerrx,PVerry,PVerrz);
	bmmgRootTree_->BSdx_       = BSdx;
        bmmgRootTree_->BSdy_       = BSdy;
        bmmgRootTree_->BSdz_       = BSdz;
        bmmgRootTree_->BSsigmaZ_   = BSsigmaZ;
        bmmgRootTree_->BSdsigmaZ_  = BSdsigmaZ;
        bmmgRootTree_->BSdxdz_     = BSdxdz;
        bmmgRootTree_->BSdydz_     = BSdydz;
	
	
	int nBs=0;
	edm::Handle<edm::View<reco::GenParticle> > genParticles;
	if(isMCstudy_)
              {
                     iEvent.getByToken(genParticlesTok, genParticles);
                     //std::cout<<"genparticles:   "<<genParticles->size()<<"\n";
		     for( size_t i = 0; i < genParticles->size(); ++ i ) {
		     const GenParticle & genBsCand = (*genParticles)[ i ];
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



	//edm::ESHandle<TransientTrackBuilder> theB;
	//iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theB);
	const auto& trackBuilder = iSetup.getData(trackBuilderTok);
        
	//edm::Handle<View<pat::PackedCandidate>> pfCands;
	//iEvent.getByToken(pfCandTagTok, pfCands);
	
	edm::Handle<edm::View<reco::Photon>> photon;
        iEvent.getByToken(PhotonTagTok, photon);
        bmmgRootTree_->photonMultiplicity_ = photon->size();
	for(size_t iPhoton =0 ; iPhoton < photon->size() ; ++iPhoton){
		
		//const pat::Photon& ipatPhoton = (*photon)[iPhoton];
		const auto ipatPhoton = (*photon)[iPhoton];
		if (excludedPhotons.find(iPhoton) != excludedPhotons.end()) continue;
		bool photonExcluded = false;

		for(size_t jPhoton = iPhoton+1; jPhoton < photon->size() ; ++jPhoton){
      
			//const pat::Photon& jpatPhoton = (*photon)[jPhoton];
			const auto jpatPhoton = (*photon)[jPhoton];
			const reco::Photon::ShowerShape& jShowerShape = jpatPhoton.full5x5_showerShapeVariables();

			pat::CompositeCandidate DiGammaCandidate;
			DiGammaCandidate.addDaughter(ipatPhoton);
			DiGammaCandidate.addDaughter(jpatPhoton);
			AddFourMomenta addP4;
			addP4.set(DiGammaCandidate);


			if ( abs(DiGammaCandidate.mass() - nominalPiZeroMass ) <  PionZeroMassWindowNoFit_ ){

				excludedPhotons.insert(iPhoton);
				excludedPhotons.insert(jPhoton);
				photonExcluded = true;
				bmmgRootTree_->PiZeroM_alone_        = DiGammaCandidate.mass();
				bmmgRootTree_->PiZeroEta_alone_      = DiGammaCandidate.eta();
				bmmgRootTree_->PiZeroPhi_alone_      = DiGammaCandidate.phi();
				bmmgRootTree_->PiZeroPt_alone_       = DiGammaCandidate.pt();
			}
			else if (abs(DiGammaCandidate.mass() - nominalEtaMesonMass ) < EtaMesonMassWindowNoFit_){

				excludedPhotons.insert(iPhoton);
				excludedPhotons.insert(jPhoton);
				photonExcluded = true;
				bmmgRootTree_->EtaMesonM_alone_   = DiGammaCandidate.mass();
				bmmgRootTree_->EtaMesonEta_alone_ = DiGammaCandidate.eta();
				bmmgRootTree_->EtaMesonPhi_alone_ = DiGammaCandidate.phi();
				bmmgRootTree_->EtaMesonPt_alone_  = DiGammaCandidate.pt();
			}
			else if (abs(DiGammaCandidate.mass() - nominalEtaPrimeMass ) < EtaPrimeMassWindowNoFit_){

                                excludedPhotons.insert(iPhoton);
                                excludedPhotons.insert(jPhoton);
                                photonExcluded = true;
                                bmmgRootTree_->EtaPrimeM_alone_   = DiGammaCandidate.mass();
                                bmmgRootTree_->EtaPrimeEta_alone_ = DiGammaCandidate.eta();
                                bmmgRootTree_->EtaPrimePhi_alone_ = DiGammaCandidate.phi();
                                bmmgRootTree_->EtaPrimePt_alone_  = DiGammaCandidate.pt();
			}
			if (photonExcluded) break; // Exit inner loop if current photon is excluded
 		}
		        if (photonExcluded) continue;






/*
			//BsToPhi(KK)Gamma - need corresponding MC sample,run on data if the MC is not available
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

			//BsToPhi(mumu)Gamma 
			//BsToJpsiEta

	                const reco::Photon::ShowerShape& iShowerShape = ipatPhoton.full5x5_showerShapeVariables();
		        bmmgRootTree_->photonSSSigmaiEtaiEta_ = iShowerShape.sigmaIetaIeta;
			bmmgRootTree_->photonSSSigmaiEtaiPhi_ = iShowerShape.sigmaIetaIphi;
			std::cout<< " some outputs : pt : "<< ipatPhoton.pt() << "\n sshape : "<< iShowerShape.e1x5 << "\t enenrgy supercluster : "<< ipatPhoton.superCluster()->energy()<< "\n";
			bmmgRootTree_->photonSSSigmaiPhiiPhi_ = iShowerShape.sigmaIphiIphi;
			bmmgRootTree_->photonSSSigmaEtaEta_   = iShowerShape.sigmaEtaEta;
			bmmgRootTree_->photonSSe1x5_ = iShowerShape.e1x5;
			bmmgRootTree_->photonSSe2x5_ = iShowerShape.e2x5;
			bmmgRootTree_->photonSSe3x3_ = iShowerShape.e3x3;
			bmmgRootTree_->photonSSe5x5_ = iShowerShape.e5x5;
			bmmgRootTree_->photonSShcalDepth1OverEcal_   = iShowerShape.hcalDepth1OverEcal;
			bmmgRootTree_->photonSShcalDepth2OverEcal_   = iShowerShape.hcalDepth2OverEcal;
			bmmgRootTree_->photonSShcalDepth1OverEcalBc_ = iShowerShape.hcalDepth1OverEcalBc;
			bmmgRootTree_->photonSShcalDepth2OverEcalBc_ = iShowerShape.hcalDepth2OverEcalBc;
			std::fill(std::begin(bmmgRootTree_->photonSShcalOverEcal_), std::end(bmmgRootTree_->photonSShcalOverEcal_), 0.f);
			std::fill(std::begin(bmmgRootTree_->photonSShcalOverEcalBc_), std::end(bmmgRootTree_->photonSShcalOverEcalBc_), 0.f);
			for (size_t k = 0; k < iShowerShape.hcalOverEcal.size(); ++k)   {bmmgRootTree_->photonSShcalOverEcal_[k]   = iShowerShape.hcalOverEcal[k];}
			for (size_t k = 0; k < iShowerShape.hcalOverEcalBc.size(); ++k) {bmmgRootTree_->photonSShcalOverEcalBc_[k] = iShowerShape.hcalOverEcalBc[k];}
			bmmgRootTree_->photonSSmaxEnergyXtal_  = iShowerShape.maxEnergyXtal;
			bmmgRootTree_->photonSSeffSigmaRR_     = iShowerShape.effSigmaRR;
			std::cout<< " shower shapes : "<< iShowerShape.sigmaIphiIphi << "\t : " << iShowerShape.sigmaEtaEta<< "\t";
			bmmgRootTree_->photonSCEnergy_         = ipatPhoton.superCluster()->energy();
			bmmgRootTree_->photonSCRawEnergy_      = ipatPhoton.superCluster()->rawEnergy();
			bmmgRootTree_->photonSCPreShowerEP1_   = ipatPhoton.superCluster()->preshowerEnergyPlane1();
			bmmgRootTree_->photonSCPreShowerEP1_   = ipatPhoton.superCluster()->preshowerEnergyPlane2();
			bmmgRootTree_->photonSCEta_            = ipatPhoton.superCluster()->eta();
			bmmgRootTree_->photonSCPhi_            = ipatPhoton.superCluster()->phi();
			bmmgRootTree_->photonSCEtaWidth_       = ipatPhoton.superCluster()->etaWidth();
			bmmgRootTree_->photonSCPhiWidth_       = ipatPhoton.superCluster()->phiWidth();
			bmmgRootTree_->photonSCBrem_           = ipatPhoton.superCluster()->phiWidth()/ipatPhoton.superCluster()->etaWidth();
			bmmgRootTree_->photonSCR9_             = ipatPhoton.r9();
			bmmgRootTree_->photonSCHadTowOverEm_   = ipatPhoton.hadTowOverEm();

			bmmgRootTree_->photonPt_        = ipatPhoton.pt();
			bmmgRootTree_->photonEta_       = ipatPhoton.eta();
			bmmgRootTree_->photonPhi_       = ipatPhoton.phi();
			bmmgRootTree_->photonEnergy_    = ipatPhoton.energy();
			bmmgRootTree_->photonET_        = ipatPhoton.et();
			/*
			bmmgRootTree_->photonTrkIso_    = ipatPhoton.trackIso();
			bmmgRootTree_->photonEcalIso_   = ipatPhoton.ecalIso();
			bmmgRootTree_->photonHcalIso_   = ipatPhoton.hcalIso();
			bmmgRootTree_->photonCaloIso_   = ipatPhoton.caloIso();
			*/
					
			photoncounter_++;
		                       
	}//photon loop ends 






	




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


  if(verbose_ == true){
    cout<<"NewEvent-------"<<endl;
  }
  for( size_t i = 0; i < genParticles->size(); ++ i ) {
    const GenParticle & genBsCand = (*genParticles)[ i ];
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
	cout << "JpsiPhotoIdx.size() + JpsiMuIdx.size() = " <<  JpsiPhotoIdx.size() + JpsiMuIdx.size() << " = " << numJpsiDaus <<"\n";
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
		   cout << "JpsiGenLxy_ : "   << bmmgRootTree_->JpsiGenLxy_ << " = " << GenDisplacementFromBeamspot.perp() << endl;
		   cout << "JpsiGenLxy/Pt:  " << bmmgRootTree_->JpsiGenLxyOverPt_ << endl;
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
