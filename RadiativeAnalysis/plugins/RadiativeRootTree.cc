#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"

RadiativeRootTree::RadiativeRootTree()
	{
		resetEntries();
		bmmgTree_ = 0;
		bmmgFile_ = 0;
	}

void RadiativeRootTree::createTree(const std::string filename)
{
  // open root file
  bmmgFile_ = new TFile (filename.c_str(), "RECREATE" );
  int bufsize = 256000;
  // create tree structure
  bmmgTree_ = new TTree("BMMGTree","BMMGTree",bufsize);
  bmmgTree_->Branch("runNumber",&runNumber_ ,"runNumber/I");
  bmmgTree_->Branch("eventNumber",&eventNumber_,"eventNumber/i");
  bmmgTree_->Branch("lumiSection",&lumiSection_,"lumiSection/I");
  bmmgTree_->Branch("PUinteraction",&PUinteraction_,"PUinteraction/I");
  bmmgTree_->Branch("PUTrueinteraction",&PUTrueinteraction_,"PUTrueinteraction/I");
  bmmgTree_->Branch("isPV",&isPV_,"isPV/I");
  bmmgTree_->Branch("NTracksPVbeforecut",&NTracksPVbeforecut_,"NTracksPVbeforecut/I");
  bmmgTree_->Branch("NVerticesbeforecut",&NVerticesbeforecut_,"NVerticesbeforecut/I");
  bmmgTree_->Branch("NVerticesaftercut",&NVerticesaftercut_,"NVerticesaftercut/I");
  bmmgTree_->Branch("PVx",&PVx_,"PVx/D");
  bmmgTree_->Branch("PVy",&PVy_,"PVy/D");
  bmmgTree_->Branch("PVz",&PVz_,"PVz/D");
  bmmgTree_->Branch("PVerrx",&PVerrx_,"PVerrx/D");
  bmmgTree_->Branch("PVerry",&PVerry_,"PVerry/D");
  bmmgTree_->Branch("PVerrz",&PVerrz_,"PVerrz/D");
  bmmgTree_->Branch("PVcovariance",&PVcovariance_, "PVcovariance[9]/D");
  bmmgTree_->Branch("PVndof",&PVndof_,"PVndof/D");
  bmmgTree_->Branch("PVrho",&PVrho_,"PVrho/D");
  bmmgTree_->Branch("isBS",&isBS_,"isBS/I");
  bmmgTree_->Branch("BSx",&BSx_,"BSx/D");
  bmmgTree_->Branch("BSy",&BSy_,"BSy/D");
  bmmgTree_->Branch("BSz",&BSz_,"BSz/D");
  bmmgTree_->Branch("BSdx",&BSdx_,"BSdx/D");
  bmmgTree_->Branch("BSdy",&BSdy_,"BSdy/D");
  bmmgTree_->Branch("BSdz",&BSdz_,"BSdz/D");
  bmmgTree_->Branch("BSdxdz",&BSdxdz_,"BSdxdz/D");
  bmmgTree_->Branch("BSdydz",&BSdydz_,"BSdydz/D");
  bmmgTree_->Branch("BSsigmaZ",&BSsigmaZ_,"BSsigmaZ/D");
  bmmgTree_->Branch("BSdsigmaZ",&BSdsigmaZ_,"BSdsigmaZ/D");
  bmmgTree_->Branch("dedxTrk",&dedxTrk_,"dedxTrk/D");
  bmmgTree_->Branch("errdedxTrk",&errdedxTrk_,"errdedxTrk/D");
  bmmgTree_->Branch("numdedxTrk",&numdedxTrk_,"numdedxTrk/I");
  bmmgTree_->Branch("triggerbit_HLT_DoubleMu4_LowMass_Displaced", &triggerbit_HLT_DoubleMu4_LowMass_Displaced_,"triggerbit_HLT_DoubleMu4_LowMass_Displaced/I");
  bmmgTree_->Branch("triggerbit_HLT_DoubleMu4_LowMass", &triggerbit_HLT_DoubleMu4_LowMass_,"triggerbit_HLT_DoubleMu4_LowMass/I");
  bmmgTree_->Branch("triggerbit_HLT_DoubleMu4_3_Bs", &triggerbit_HLT_DoubleMu4_3_Bs_,"triggerbit_HLT_DoubleMu4_3_Bs/I");
  bmmgTree_->Branch("triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG", &triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_,"triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG/I");
  bmmgTree_->Branch("triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG", &triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_,"triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG/I");
  bmmgTree_->Branch("triggerbit_HLTDimuon4JpsiDisplaced", &triggerbit_HLTDimuon4JpsiDisplaced_,"triggerbit_HLTDimuon4JpsiDisplaced_/I");
  bmmgTree_->Branch("triggerbit_HLTDimuon4JpsiNoVertexing",&triggerbit_HLTDimuon4JpsiNoVertexing_,"triggerbit_HLTDimuon4JpsiNoVertexing_/I");
  bmmgTree_->Branch("triggerbit_HLTDimuon4JpsiTrkTrkDisplaced",&triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_,"triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_/I");
  bmmgTree_->Branch("mu1Pt_beffit", &mu1Pt_beffit_,"mu1Pt_beffit/D");
  bmmgTree_->Branch("mu1Pz_beffit", &mu1Pz_beffit_,"mu1Pz_beffit/D");
  bmmgTree_->Branch("mu1Eta_beffit", &mu1Eta_beffit_,"mu1Eta_beffit/D");
  bmmgTree_->Branch("mu1Phi_beffit", &mu1Phi_beffit_,"mu1Phi_beffit/D");
  bmmgTree_->Branch("mu1Energy_beffit", &mu1Energy_beffit_,"mu1Energy_beffit/D");
  bmmgTree_->Branch("mu2Pt_beffit", &mu2Pt_beffit_,"mu2Pt_beffit/D");
  bmmgTree_->Branch("mu2Pz_beffit", &mu2Pz_beffit_,"mu2Pz_beffit/D");
  bmmgTree_->Branch("mu2Eta_beffit", &mu2Eta_beffit_,"mu2Eta_beffit/D");
  bmmgTree_->Branch("mu2Phi_beffit", &mu2Phi_beffit_,"mu2Phi_beffit/D");
  bmmgTree_->Branch("mu2Energy_beffit", &mu2Energy_beffit_,"mu2Energy_beffit/D");
  bmmgTree_->Branch("mu1SoftID", &mu1SoftID_,"mu1SoftID/I");
  bmmgTree_->Branch("mu2SoftID", &mu2SoftID_,"mu2SoftID/I");
  bmmgTree_->Branch("MuonPairDR", &MuonPairDR_,"MuonPairDR/D");
  bmmgTree_->Branch("Mu1TrkBSDxy", &Mu1TrkBSDxy_,"Mu1TrkBSDxy/D");
  bmmgTree_->Branch("Mu1TrkBSDz", &Mu1TrkBSDz_,"Mu1TrkBSDz/D");
  bmmgTree_->Branch("Mu2TrkBSDxy", &Mu2TrkBSDxy_,"Mu2TrkBSDxy/D");
  bmmgTree_->Branch("Mu2TrkBSDz", &Mu2TrkBSDz_,"Mu2TrkBSDz/D");
  bmmgTree_->Branch("Mu1PixelHits", &Mu1PixelHits_,"Mu1PixelHits/I");
  bmmgTree_->Branch("Mu1TrackerHits", &Mu1TrackerHits_,"Mu1TrackerHits/I");
  bmmgTree_->Branch("Mu1isGood", &Mu1isGood_,"Mu1isGood/I");
  bmmgTree_->Branch("Mu1InnerTrkHighQuality", &Mu1InnerTrkHighQuality_,"Mu1InnerTrkHighQuality/I");
  bmmgTree_->Branch("Mu2PixelHits", &Mu2PixelHits_,"Mu2PixelHits/I");
  bmmgTree_->Branch("Mu2TrackerHits", &Mu2TrackerHits_,"Mu2TrackerHits/I");
  bmmgTree_->Branch("Mu2isGood", &Mu2isGood_,"Mu2isGood/I");
  bmmgTree_->Branch("Mu2InnerTrkHighQuality", &Mu2InnerTrkHighQuality_,"Mu2InnerTrkHighQuality/I");
  bmmgTree_->Branch("mu1MVAScore", &mu1MVAScore_,"mu1MVAScore/D");
  bmmgTree_->Branch("mu2MVAScore", &mu2MVAScore_,"mu2MVAScore/D");
  bmmgTree_->Branch("DiMuonM_beffit", &DiMuonM_beffit_,"DiMuonM_beffit/D");
  bmmgTree_->Branch("DiMuonEta_beffit", &DiMuonEta_beffit_,"DiMuonEta_beffit/D");
  bmmgTree_->Branch("DiMuonPhi_beffit", &DiMuonPhi_beffit_,"DiMuonPhi_beffit/D");
  bmmgTree_->Branch("DiMuonPt_beffit", &DiMuonPt_beffit_,"DiMuonPt_beffit/D");
  bmmgTree_->Branch("DiMuon_vtxProb", &DiMuon_vtxProb_,"DiMuon_vtxProb/D");
  bmmgTree_->Branch("DiMuon_CosineAlpha", &DiMuon_CosineAlpha_,"DiMuon_CosineAlpha/D");
  bmmgTree_->Branch("DiMuon_DCA", &DiMuon_DCA_,"DiMuon_DCA/D");
  bmmgTree_->Branch("DiMuon_Chi2pv_KVFvtx", &DiMuon_Chi2pv_KVFvtx_,"DiMuon_Chi2pv_KVFvtx/D");
  bmmgTree_->Branch("DiMuon_Mahalanobis", &DiMuon_Mahalanobis_,"DiMuon_Mahalanobis/D");
  bmmgTree_->Branch("DiMuon_ResonanceType", &DiMuon_ResonanceType_, "DiMuon_ResonanceType/I");
  bmmgTree_->Branch("DiMuon_Lxy", &DiMuon_Lxy_,"DiMuon_Lxy/D");
  bmmgTree_->Branch("DiMuon_Lxyerr", &DiMuon_Lxyerr_,"DiMuon_Lxyerr/D");
  bmmgTree_->Branch("DiMuon_LxyOverPt", &DiMuon_LxyOverPt_,"DiMuon_LxyOverPt/D");
  bmmgTree_->Branch("DiMuon_mu1Cat_alone", &DiMuon_mu1Cat_alone_,"DiMuon_mu1Cat_alone/I");
  bmmgTree_->Branch("DiMuon_mu2Cat_alone", &DiMuon_mu2Cat_alone_,"DiMuon_mu2Cat_alone/I");
  bmmgTree_->Branch("DiMuon_mu1nPixHits_alone", &DiMuon_mu1nPixHits_alone_,"DiMuon_mu1nPixHits_alone/I");
  bmmgTree_->Branch("DiMuon_mu2nPixHits_alone", &DiMuon_mu2nPixHits_alone_,"DiMuon_mu2nPixHits_alone/I");
  bmmgTree_->Branch("DiMuon_NumberOfCandidates", &DiMuon_NumberOfCandidates_,"DiMuon_NumberOfCandidates/I");

// Tripple object converted photon vertex variables
bmmgTree_->Branch("FourvectorBsMass_mmconvg", &FourvectorBsMass_mmconvg_, "FourvectorBsMass_mmconvg/D");
bmmgTree_->Branch("FourvectorBsPt_mmconvg", &FourvectorBsPt_mmconvg_, "FourvectorBsPt_mmconvg/D");
bmmgTree_->Branch("FourvectorBsEta_mmconvg", &FourvectorBsEta_mmconvg_, "FourvectorBsEta_mmconvg/D");
bmmgTree_->Branch("FourvectorBsPhi_mmconvg", &FourvectorBsPhi_mmconvg_, "FourvectorBsPhi_mmconvg/D");

bmmgTree_->Branch("VertexfitBsMass_mmconvg", &VertexfitBsMass_mmconvg_, "VertexfitBsMass_mmconvg/D");
bmmgTree_->Branch("VertexfitBsPt_mmconvg", &VertexfitBsPt_mmconvg_, "VertexfitBsPt_mmconvg/D");
bmmgTree_->Branch("VertexfitBsEta_mmconvg", &VertexfitBsEta_mmconvg_, "VertexfitBsEta_mmconvg/D");
bmmgTree_->Branch("VertexfitBsPhi_mmconvg", &VertexfitBsPhi_mmconvg_, "VertexfitBsPhi_mmconvg/D");
bmmgTree_->Branch("VertexfitBsPz_mmconvg", &VertexfitBsPz_mmconvg_, "VertexfitBsPz_mmconvg/D");

bmmgTree_->Branch("VertexfitBsSVx_mmconvg", &VertexfitBsSVx_mmconvg_, "VertexfitBsSVx_mmconvg/D");
bmmgTree_->Branch("VertexfitBsSVy_mmconvg", &VertexfitBsSVy_mmconvg_, "VertexfitBsSVy_mmconvg/D");
bmmgTree_->Branch("VertexfitBsSVz_mmconvg", &VertexfitBsSVz_mmconvg_, "VertexfitBsSVz_mmconvg/D");

bmmgTree_->Branch("FourvectorConvGammaPt_mmconvg", &FourvectorConvGammaPt_mmconvg_, "FourvectorConvGammaPt_mmconvg/D");
bmmgTree_->Branch("FourvectorConvGammaEta_mmconvg", &FourvectorConvGammaEta_mmconvg_, "FourvectorConvGammaEta_mmconvg/D");
bmmgTree_->Branch("FourvectorConvGammaPhi_mmconvg", &FourvectorConvGammaPhi_mmconvg_, "FourvectorConvGammaPhi_mmconvg/D");

bmmgTree_->Branch("VertexfitPVxRefitCosTheta_mmconvg", &VertexfitPVxRefitCosTheta_mmconvg_, "VertexfitPVxRefitCosTheta_mmconvg/D");
bmmgTree_->Branch("VertexfitPVyRefitCosTheta_mmconvg", &VertexfitPVyRefitCosTheta_mmconvg_, "VertexfitPVyRefitCosTheta_mmconvg/D");
bmmgTree_->Branch("VertexfitPVzRefitCosTheta_mmconvg", &VertexfitPVzRefitCosTheta_mmconvg_, "VertexfitPVzRefitCosTheta_mmconvg/D");

bmmgTree_->Branch("VertexfitPVxRefitClosestZ_mmconvg", &VertexfitPVxRefitClosestZ_mmconvg_, "VertexfitPVxRefitClosestZ_mmconvg/D");
bmmgTree_->Branch("VertexfitPVyRefitClosestZ_mmconvg", &VertexfitPVyRefitClosestZ_mmconvg_, "VertexfitPVyRefitClosestZ_mmconvg/D");
bmmgTree_->Branch("VertexfitPVzRefitClosestZ_mmconvg", &VertexfitPVzRefitClosestZ_mmconvg_, "VertexfitPVzRefitClosestZ_mmconvg/D");

bmmgTree_->Branch("VertexfitBsCt3DPVClosestZ_mmconvg", &VertexfitBsCt3DPVClosestZ_mmconvg_, "VertexfitBsCt3DPVClosestZ_mmconvg/D");
bmmgTree_->Branch("VertexfitBsCt2DPVClosestZ_mmconvg", &VertexfitBsCt2DPVClosestZ_mmconvg_, "VertexfitBsCt2DPVClosestZ_mmconvg/D");
bmmgTree_->Branch("VertexfitBsCt3DPVCosTheta_mmconvg", &VertexfitBsCt3DPVCosTheta_mmconvg_, "VertexfitBsCt3DPVCosTheta_mmconvg/D");
bmmgTree_->Branch("VertexfitBsCt2DPVCosTheta_mmconvg", &VertexfitBsCt2DPVCosTheta_mmconvg_, "VertexfitBsCt2DPVCosTheta_mmconvg/D");

bmmgTree_->Branch("VertexfitBsCtErr2D_mmconvg", &VertexfitBsCtErr2D_mmconvg_, "VertexfitBsCtErr2D_mmconvg/D");
bmmgTree_->Branch("VertexfitBsCtErr3D_mmconvg", &VertexfitBsCtErr3D_mmconvg_, "VertexfitBsCtErr3D_mmconvg/D");

bmmgTree_->Branch("VertexfitBsDist3D_mmconvg", &VertexfitBsDist3D_mmconvg_, "VertexfitBsDist3D_mmconvg/D");
bmmgTree_->Branch("VertexfitBsDist3DErr_mmconvg", &VertexfitBsDist3DErr_mmconvg_, "VertexfitBsDist3DErr_mmconvg/D");
bmmgTree_->Branch("VertexfitBsDist2D_mmconvg", &VertexfitBsDist2D_mmconvg_, "VertexfitBsDist2D_mmconvg/D");
bmmgTree_->Branch("VertexfitBsDist2DErr_mmconvg", &VertexfitBsDist2DErr_mmconvg_, "VertexfitBsDist2DErr_mmconvg/D");

bmmgTree_->Branch("BsVtxProb_mmconvg", &BsVtxProb_mmconvg_, "BsVtxProb_mmconvg/D");
bmmgTree_->Branch("BsCt2D_mmconvg", &BsCt2D_mmconvg_, "BsCt2D_mmconvg/D");
bmmgTree_->Branch("BsCt3D_mmconvg", &BsCt3D_mmconvg_, "BsCt3D_mmconvg/D");

// Tripple object reco photon vertex variables
bmmgTree_->Branch("FourvectorBsMass_mmrecog", &FourvectorBsMass_mmrecog_, "FourvectorBsMass_mmrecog/D");
bmmgTree_->Branch("FourvectorBsPt_mmrecog", &FourvectorBsPt_mmrecog_, "FourvectorBsPt_mmrecog/D");
bmmgTree_->Branch("FourvectorBsEta_mmrecog", &FourvectorBsEta_mmrecog_, "FourvectorBsEta_mmrecog/D");
bmmgTree_->Branch("FourvectorBsPhi_mmrecog", &FourvectorBsPhi_mmrecog_, "FourvectorBsPhi_mmrecog/D");

bmmgTree_->Branch("VertexfitBsMass_mmrecog", &VertexfitBsMass_mmrecog_, "VertexfitBsMass_mmrecog/D");
bmmgTree_->Branch("VertexfitBsPt_mmrecog", &VertexfitBsPt_mmrecog_, "VertexfitBsPt_mmrecog/D");
bmmgTree_->Branch("VertexfitBsEta_mmrecog", &VertexfitBsEta_mmrecog_, "VertexfitBsEta_mmrecog/D");
bmmgTree_->Branch("VertexfitBsPhi_mmrecog", &VertexfitBsPhi_mmrecog_, "VertexfitBsPhi_mmrecog/D");
bmmgTree_->Branch("VertexfitBsPz_mmrecog", &VertexfitBsPz_mmrecog_, "VertexfitBsPz_mmrecog/D");

bmmgTree_->Branch("VertexfitBsSVx_mmrecog", &VertexfitBsSVx_mmrecog_, "VertexfitBsSVx_mmrecog/D");
bmmgTree_->Branch("VertexfitBsSVy_mmrecog", &VertexfitBsSVy_mmrecog_, "VertexfitBsSVy_mmrecog/D");
bmmgTree_->Branch("VertexfitBsSVz_mmrecog", &VertexfitBsSVz_mmrecog_, "VertexfitBsSVz_mmrecog/D");

bmmgTree_->Branch("FourvectorGammaPt_mmrecog", &FourvectorGammaPt_mmrecog_, "FourvectorGammaPt_mmrecog/D");
bmmgTree_->Branch("FourvectorGammaEta_mmrecog", &FourvectorGammaEta_mmrecog_, "FourvectorGammaEta_mmrecog/D");
bmmgTree_->Branch("FourvectorGammaPhi_mmrecog", &FourvectorGammaPhi_mmrecog_, "FourvectorGammaPhi_mmrecog/D");

bmmgTree_->Branch("VertexfitPVxRefitCosTheta_mmrecog", &VertexfitPVxRefitCosTheta_mmrecog_, "VertexfitPVxRefitCosTheta_mmrecog/D");
bmmgTree_->Branch("VertexfitPVyRefitCosTheta_mmrecog", &VertexfitPVyRefitCosTheta_mmrecog_, "VertexfitPVyRefitCosTheta_mmrecog/D");
bmmgTree_->Branch("VertexfitPVzRefitCosTheta_mmrecog", &VertexfitPVzRefitCosTheta_mmrecog_, "VertexfitPVzRefitCosTheta_mmrecog/D");

bmmgTree_->Branch("VertexfitPVxRefitClosestZ_mmrecog", &VertexfitPVxRefitClosestZ_mmrecog_, "VertexfitPVxRefitClosestZ_mmrecog/D");
bmmgTree_->Branch("VertexfitPVyRefitClosestZ_mmrecog", &VertexfitPVyRefitClosestZ_mmrecog_, "VertexfitPVyRefitClosestZ_mmrecog/D");
bmmgTree_->Branch("VertexfitPVzRefitClosestZ_mmrecog", &VertexfitPVzRefitClosestZ_mmrecog_, "VertexfitPVzRefitClosestZ_mmrecog/D");

bmmgTree_->Branch("VertexfitBsCt3DPVClosestZ_mmrecog", &VertexfitBsCt3DPVClosestZ_mmrecog_, "VertexfitBsCt3DPVClosestZ_mmrecog/D");
bmmgTree_->Branch("VertexfitBsCt2DPVClosestZ_mmrecog", &VertexfitBsCt2DPVClosestZ_mmrecog_, "VertexfitBsCt2DPVClosestZ_mmrecog/D");
bmmgTree_->Branch("VertexfitBsCt3DPVCosTheta_mmrecog", &VertexfitBsCt3DPVCosTheta_mmrecog_, "VertexfitBsCt3DPVCosTheta_mmrecog/D");
bmmgTree_->Branch("VertexfitBsCt2DPVCosTheta_mmrecog", &VertexfitBsCt2DPVCosTheta_mmrecog_, "VertexfitBsCt2DPVCosTheta_mmrecog/D");

bmmgTree_->Branch("VertexfitBsCtErr2D_mmrecog", &VertexfitBsCtErr2D_mmrecog_, "VertexfitBsCtErr2D_mmrecog/D");
bmmgTree_->Branch("VertexfitBsCtErr3D_mmrecog", &VertexfitBsCtErr3D_mmrecog_, "VertexfitBsCtErr3D_mmrecog/D");

bmmgTree_->Branch("VertexfitBsDist3D_mmrecog", &VertexfitBsDist3D_mmrecog_, "VertexfitBsDist3D_mmrecog/D");
bmmgTree_->Branch("VertexfitBsDist3DErr_mmrecog", &VertexfitBsDist3DErr_mmrecog_, "VertexfitBsDist3DErr_mmrecog/D");
bmmgTree_->Branch("VertexfitBsDist2D_mmrecog", &VertexfitBsDist2D_mmrecog_, "VertexfitBsDist2D_mmrecog/D");
bmmgTree_->Branch("VertexfitBsDist2DErr_mmrecog", &VertexfitBsDist2DErr_mmrecog_, "VertexfitBsDist2DErr_mmrecog/D");

bmmgTree_->Branch("BsVtxProb_mmrecog", &BsVtxProb_mmrecog_, "BsVtxProb_mmrecog/D");
bmmgTree_->Branch("BsCt2D_mmrecog", &BsCt2D_mmrecog_, "BsCt2D_mmrecog/D");
bmmgTree_->Branch("BsCt3D_mmrecog", &BsCt3D_mmrecog_, "BsCt3D_mmrecog/D");


bmmgTree_->Branch("DeltaRPhoton1DiMuon_mmrecog", &DeltaRPhoton1DiMuon_mmrecog_, "DeltaRPhoton1DiMuon_mmrecog/D");
bmmgTree_->Branch("DeltaRPhoton2DiMuon_mmrecog", &DeltaRPhoton2DiMuon_mmrecog_, "DeltaRPhoton2DiMuon_mmrecog/D");

bmmgTree_->Branch("BsHelicity_mmrecog", &BsHelicity_mmrecog_, "BsHelicity_mmrecog/D");
bmmgTree_->Branch("BsCoplanarity_mmrecog", &BsCoplanarity_mmrecog_, "BsCoplanarity_mmrecog/D");


bmmgTree_->Branch("DeltaRPhoton1DiMuon_mmrecog", &DeltaRPhoton1DiMuon_mmrecog_, "DeltaRPhoton1DiMuon_mmrecog/D");
bmmgTree_->Branch("DeltaRPhoton2DiMuon_mmrecog", &DeltaRPhoton2DiMuon_mmrecog_, "DeltaRPhoton2DiMuon_mmrecog/D");

bmmgTree_->Branch("BsHelicity_mmrecog", &BsHelicity_mmrecog_, "BsHelicity_mmrecog/D");
bmmgTree_->Branch("BsCoplanarity_mmrecog", &BsCoplanarity_mmrecog_, "BsCoplanarity_mmrecog/D");


// Tetra object converted photon vertex variables
bmmgTree_->Branch("FourvectorBsMass_mmconvgg", &FourvectorBsMass_mmconvgg_, "FourvectorBsMass_mmconvgg/D");
bmmgTree_->Branch("FourvectorBsPt_mmconvgg", &FourvectorBsPt_mmconvgg_, "FourvectorBsPt_mmconvgg/D");
bmmgTree_->Branch("FourvectorBsEta_mmconvgg", &FourvectorBsEta_mmconvgg_, "FourvectorBsEta_mmconvgg/D");
bmmgTree_->Branch("FourvectorBsPhi_mmconvgg", &FourvectorBsPhi_mmconvgg_, "FourvectorBsPhi_mmconvgg/D");

bmmgTree_->Branch("VertexfitBsMass_mmconvgg", &VertexfitBsMass_mmconvgg_, "VertexfitBsMass_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsPt_mmconvgg", &VertexfitBsPt_mmconvgg_, "VertexfitBsPt_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsEta_mmconvgg", &VertexfitBsEta_mmconvgg_, "VertexfitBsEta_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsPhi_mmconvgg", &VertexfitBsPhi_mmconvgg_, "VertexfitBsPhi_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsPz_mmconvgg", &VertexfitBsPz_mmconvgg_, "VertexfitBsPz_mmconvgg/D");

bmmgTree_->Branch("VertexfitBsSVx_mmconvgg", &VertexfitBsSVx_mmconvgg_, "VertexfitBsSVx_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsSVy_mmconvgg", &VertexfitBsSVy_mmconvgg_, "VertexfitBsSVy_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsSVz_mmconvgg", &VertexfitBsSVz_mmconvgg_, "VertexfitBsSVz_mmconvgg/D");

bmmgTree_->Branch("FourvectorConvGammaPt_mmconvgg", &FourvectorConvGammaPt_mmconvgg_, "FourvectorConvGammaPt_mmconvgg/D");
bmmgTree_->Branch("FourvectorConvGammaEta_mmconvgg", &FourvectorConvGammaEta_mmconvgg_, "FourvectorConvGammaEta_mmconvgg/D");
bmmgTree_->Branch("FourvectorConvGammaPhi_mmconvgg", &FourvectorConvGammaPhi_mmconvgg_, "FourvectorConvGammaPhi_mmconvgg/D");

bmmgTree_->Branch("VertexfitPVxRefitCosTheta_mmconvgg", &VertexfitPVxRefitCosTheta_mmconvgg_, "VertexfitPVxRefitCosTheta_mmconvgg/D");
bmmgTree_->Branch("VertexfitPVyRefitCosTheta_mmconvgg", &VertexfitPVyRefitCosTheta_mmconvgg_, "VertexfitPVyRefitCosTheta_mmconvgg/D");
bmmgTree_->Branch("VertexfitPVzRefitCosTheta_mmconvgg", &VertexfitPVzRefitCosTheta_mmconvgg_, "VertexfitPVzRefitCosTheta_mmconvgg/D");

bmmgTree_->Branch("VertexfitPVxRefitClosestZ_mmconvgg", &VertexfitPVxRefitClosestZ_mmconvgg_, "VertexfitPVxRefitClosestZ_mmconvgg/D");
bmmgTree_->Branch("VertexfitPVyRefitClosestZ_mmconvgg", &VertexfitPVyRefitClosestZ_mmconvgg_, "VertexfitPVyRefitClosestZ_mmconvgg/D");
bmmgTree_->Branch("VertexfitPVzRefitClosestZ_mmconvgg", &VertexfitPVzRefitClosestZ_mmconvgg_, "VertexfitPVzRefitClosestZ_mmconvgg/D");

bmmgTree_->Branch("VertexfitBsCt3DPVClosestZ_mmconvgg", &VertexfitBsCt3DPVClosestZ_mmconvgg_, "VertexfitBsCt3DPVClosestZ_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsCt2DPVClosestZ_mmconvgg", &VertexfitBsCt2DPVClosestZ_mmconvgg_, "VertexfitBsCt2DPVClosestZ_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsCt3DPVCosTheta_mmconvgg", &VertexfitBsCt3DPVCosTheta_mmconvgg_, "VertexfitBsCt3DPVCosTheta_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsCt2DPVCosTheta_mmconvgg", &VertexfitBsCt2DPVCosTheta_mmconvgg_, "VertexfitBsCt2DPVCosTheta_mmconvgg/D");

bmmgTree_->Branch("VertexfitBsCtErr2D_mmconvgg", &VertexfitBsCtErr2D_mmconvgg_, "VertexfitBsCtErr2D_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsCtErr3D_mmconvgg", &VertexfitBsCtErr3D_mmconvgg_, "VertexfitBsCtErr3D_mmconvgg/D");

bmmgTree_->Branch("VertexfitBsDist3D_mmconvgg", &VertexfitBsDist3D_mmconvgg_, "VertexfitBsDist3D_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsDist3DErr_mmconvgg", &VertexfitBsDist3DErr_mmconvgg_, "VertexfitBsDist3DErr_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsDist2D_mmconvgg", &VertexfitBsDist2D_mmconvgg_, "VertexfitBsDist2D_mmconvgg/D");
bmmgTree_->Branch("VertexfitBsDist2DErr_mmconvgg", &VertexfitBsDist2DErr_mmconvgg_, "VertexfitBsDist2DErr_mmconvgg/D");

bmmgTree_->Branch("BsVtxProb_mmconvgg", &BsVtxProb_mmconvgg_, "BsVtxProb_mmconvgg/D");
bmmgTree_->Branch("BsCt2D_mmconvgg", &BsCt2D_mmconvgg_, "BsCt2D_mmconvgg/D");
bmmgTree_->Branch("BsCt3D_mmconvgg", &BsCt3D_mmconvgg_, "BsCt3D_mmconvgg/D");


bmmgTree_->Branch("DeltaRPhoton1DiMuon_mmconvgg", &DeltaRPhoton1DiMuon_mmconvgg_, "DeltaRPhoton1DiMuon_mmconvgg/D");
bmmgTree_->Branch("DeltaRPhoton2DiMuon_mmconvgg", &DeltaRPhoton2DiMuon_mmconvgg_, "DeltaRPhoton2DiMuon_mmconvgg/D");

bmmgTree_->Branch("BsHelicity_mmconvgg", &BsHelicity_mmconvgg_, "BsHelicity_mmconvgg/D");
bmmgTree_->Branch("BsCoplanarity_mmconvgg", &BsCoplanarity_mmconvgg_, "BsCoplanarity_mmconvgg/D");


bmmgTree_->Branch("DeltaRPhoton1DiMuon_mmconvgg", &DeltaRPhoton1DiMuon_mmconvgg_, "DeltaRPhoton1DiMuon_mmconvgg/D");
bmmgTree_->Branch("DeltaRPhoton2DiMuon_mmconvgg", &DeltaRPhoton2DiMuon_mmconvgg_, "DeltaRPhoton2DiMuon_mmconvgg/D");

bmmgTree_->Branch("BsHelicity_mmconvgg", &BsHelicity_mmconvgg_, "BsHelicity_mmconvgg/D");
bmmgTree_->Branch("BsCoplanarity_mmconvgg", &BsCoplanarity_mmconvgg_, "BsCoplanarity_mmconvgg/D");

// Tetra object reco photon vertex variables
bmmgTree_->Branch("FourvectorBsMass_mmrecogg", &FourvectorBsMass_mmrecogg_, "FourvectorBsMass_mmrecogg/D");
bmmgTree_->Branch("FourvectorBsPt_mmrecogg", &FourvectorBsPt_mmrecogg_, "FourvectorBsPt_mmrecogg/D");
bmmgTree_->Branch("FourvectorBsEta_mmrecogg", &FourvectorBsEta_mmrecogg_, "FourvectorBsEta_mmrecogg/D");
bmmgTree_->Branch("FourvectorBsPhi_mmrecogg", &FourvectorBsPhi_mmrecogg_, "FourvectorBsPhi_mmrecogg/D");

bmmgTree_->Branch("VertexfitBsMass_mmrecogg", &VertexfitBsMass_mmrecogg_, "VertexfitBsMass_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsPt_mmrecogg", &VertexfitBsPt_mmrecogg_, "VertexfitBsPt_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsEta_mmrecogg", &VertexfitBsEta_mmrecogg_, "VertexfitBsEta_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsPhi_mmrecogg", &VertexfitBsPhi_mmrecogg_, "VertexfitBsPhi_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsPz_mmrecogg", &VertexfitBsPz_mmrecogg_, "VertexfitBsPz_mmrecogg/D");

bmmgTree_->Branch("VertexfitBsSVx_mmrecogg", &VertexfitBsSVx_mmrecogg_, "VertexfitBsSVx_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsSVy_mmrecogg", &VertexfitBsSVy_mmrecogg_, "VertexfitBsSVy_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsSVz_mmrecogg", &VertexfitBsSVz_mmrecogg_, "VertexfitBsSVz_mmrecogg/D");

bmmgTree_->Branch("FourvectorGammaPt_mmrecogg", &FourvectorGammaPt_mmrecogg_, "FourvectorGammaPt_mmrecogg/D");
bmmgTree_->Branch("FourvectorGammaEta_mmrecogg", &FourvectorGammaEta_mmrecogg_, "FourvectorGammaEta_mmrecogg/D");
bmmgTree_->Branch("FourvectorGammaPhi_mmrecogg", &FourvectorGammaPhi_mmrecogg_, "FourvectorGammaPhi_mmrecogg/D");

bmmgTree_->Branch("VertexfitPVxRefitCosTheta_mmrecogg", &VertexfitPVxRefitCosTheta_mmrecogg_, "VertexfitPVxRefitCosTheta_mmrecogg/D");
bmmgTree_->Branch("VertexfitPVyRefitCosTheta_mmrecogg", &VertexfitPVyRefitCosTheta_mmrecogg_, "VertexfitPVyRefitCosTheta_mmrecogg/D");
bmmgTree_->Branch("VertexfitPVzRefitCosTheta_mmrecogg", &VertexfitPVzRefitCosTheta_mmrecogg_, "VertexfitPVzRefitCosTheta_mmrecogg/D");

bmmgTree_->Branch("VertexfitPVxRefitClosestZ_mmrecogg", &VertexfitPVxRefitClosestZ_mmrecogg_, "VertexfitPVxRefitClosestZ_mmrecogg/D");
bmmgTree_->Branch("VertexfitPVyRefitClosestZ_mmrecogg", &VertexfitPVyRefitClosestZ_mmrecogg_, "VertexfitPVyRefitClosestZ_mmrecogg/D");
bmmgTree_->Branch("VertexfitPVzRefitClosestZ_mmrecogg", &VertexfitPVzRefitClosestZ_mmrecogg_, "VertexfitPVzRefitClosestZ_mmrecogg/D");

bmmgTree_->Branch("VertexfitBsCt3DPVClosestZ_mmrecogg", &VertexfitBsCt3DPVClosestZ_mmrecogg_, "VertexfitBsCt3DPVClosestZ_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsCt2DPVClosestZ_mmrecogg", &VertexfitBsCt2DPVClosestZ_mmrecogg_, "VertexfitBsCt2DPVClosestZ_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsCt3DPVCosTheta_mmrecogg", &VertexfitBsCt3DPVCosTheta_mmrecogg_, "VertexfitBsCt3DPVCosTheta_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsCt2DPVCosTheta_mmrecogg", &VertexfitBsCt2DPVCosTheta_mmrecogg_, "VertexfitBsCt2DPVCosTheta_mmrecogg/D");

bmmgTree_->Branch("VertexfitBsCtErr2D_mmrecogg", &VertexfitBsCtErr2D_mmrecogg_, "VertexfitBsCtErr2D_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsCtErr3D_mmrecogg", &VertexfitBsCtErr3D_mmrecogg_, "VertexfitBsCtErr3D_mmrecogg/D");

bmmgTree_->Branch("VertexfitBsDist3D_mmrecogg", &VertexfitBsDist3D_mmrecogg_, "VertexfitBsDist3D_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsDist3DErr_mmrecogg", &VertexfitBsDist3DErr_mmrecogg_, "VertexfitBsDist3DErr_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsDist2D_mmrecogg", &VertexfitBsDist2D_mmrecogg_, "VertexfitBsDist2D_mmrecogg/D");
bmmgTree_->Branch("VertexfitBsDist2DErr_mmrecogg", &VertexfitBsDist2DErr_mmrecogg_, "VertexfitBsDist2DErr_mmrecogg/D");

bmmgTree_->Branch("BsVtxProb_mmrecogg", &BsVtxProb_mmrecogg_, "BsVtxProb_mmrecogg/D");
bmmgTree_->Branch("BsCt2D_mmrecogg", &BsCt2D_mmrecogg_, "BsCt2D_mmrecogg/D");
bmmgTree_->Branch("BsCt3D_mmrecogg", &BsCt3D_mmrecogg_, "BsCt3D_mmrecogg/D");


bmmgTree_->Branch("DeltaRPhoton1DiMuon_mmrecogg", &DeltaRPhoton1DiMuon_mmrecogg_, "DeltaRPhoton1DiMuon_mmrecogg/D");
bmmgTree_->Branch("DeltaRPhoton2DiMuon_mmrecogg", &DeltaRPhoton2DiMuon_mmrecogg_, "DeltaRPhoton2DiMuon_mmrecogg/D");

bmmgTree_->Branch("BsHelicity_mmrecogg", &BsHelicity_mmrecogg_, "BsHelicity_mmrecogg/D");
bmmgTree_->Branch("BsCoplanarity_mmrecogg", &BsCoplanarity_mmrecogg_, "BsCoplanarity_mmrecogg/D");


bmmgTree_->Branch("DeltaRPhoton1DiMuon_mmrecogg", &DeltaRPhoton1DiMuon_mmrecogg_, "DeltaRPhoton1DiMuon_mmrecogg/D");
bmmgTree_->Branch("DeltaRPhoton2DiMuon_mmrecogg", &DeltaRPhoton2DiMuon_mmrecogg_, "DeltaRPhoton2DiMuon_mmrecogg/D");

bmmgTree_->Branch("BsHelicity_mmrecogg", &BsHelicity_mmrecogg_, "BsHelicity_mmrecogg/D");
bmmgTree_->Branch("BsCoplanarity_mmrecogg", &BsCoplanarity_mmrecogg_, "BsCoplanarity_mmrecogg/D");

// mu mu k k
bmmgTree_->Branch("FourvectorBsMass_mmkk", &FourvectorBsMass_mmkk_, "FourvectorBsMass_mmkk/D");
bmmgTree_->Branch("FourvectorBsPt_mmkk", &FourvectorBsPt_mmkk_, "FourvectorBsPt_mmkk/D");
bmmgTree_->Branch("FourvectorBsEta_mmkk", &FourvectorBsEta_mmkk_, "FourvectorBsEta_mmkk/D");
bmmgTree_->Branch("FourvectorBsPhi_mmkk", &FourvectorBsPhi_mmkk_, "FourvectorBsPhi_mmkk/D");

bmmgTree_->Branch("VertexfitBsMass_mmkk", &VertexfitBsMass_mmkk_, "VertexfitBsMass_mmkk/D");
bmmgTree_->Branch("VertexfitBsPt_mmkk", &VertexfitBsPt_mmkk_, "VertexfitBsPt_mmkk/D");
bmmgTree_->Branch("VertexfitBsEta_mmkk", &VertexfitBsEta_mmkk_, "VertexfitBsEta_mmkk/D");
bmmgTree_->Branch("VertexfitBsPhi_mmkk", &VertexfitBsPhi_mmkk_, "VertexfitBsPhi_mmkk/D");
bmmgTree_->Branch("VertexfitBsPz_mmkk", &VertexfitBsPz_mmkk_, "VertexfitBsPz_mmkk/D");

bmmgTree_->Branch("VertexfitBsSVx_mmkk", &VertexfitBsSVx_mmkk_, "VertexfitBsSVx_mmkk/D");
bmmgTree_->Branch("VertexfitBsSVy_mmkk", &VertexfitBsSVy_mmkk_, "VertexfitBsSVy_mmkk/D");
bmmgTree_->Branch("VertexfitBsSVz_mmkk", &VertexfitBsSVz_mmkk_, "VertexfitBsSVz_mmkk/D");

bmmgTree_->Branch("VertexfitPVxRefitCosTheta_mmkk", &VertexfitPVxRefitCosTheta_mmkk_, "VertexfitPVxRefitCosTheta_mmkk/D");
bmmgTree_->Branch("VertexfitPVyRefitCosTheta_mmkk", &VertexfitPVyRefitCosTheta_mmkk_, "VertexfitPVyRefitCosTheta_mmkk/D");
bmmgTree_->Branch("VertexfitPVzRefitCosTheta_mmkk", &VertexfitPVzRefitCosTheta_mmkk_, "VertexfitPVzRefitCosTheta_mmkk/D");

bmmgTree_->Branch("VertexfitPVxRefitClosestZ_mmkk", &VertexfitPVxRefitClosestZ_mmkk_, "VertexfitPVxRefitClosestZ_mmkk/D");
bmmgTree_->Branch("VertexfitPVyRefitClosestZ_mmkk", &VertexfitPVyRefitClosestZ_mmkk_, "VertexfitPVyRefitClosestZ_mmkk/D");
bmmgTree_->Branch("VertexfitPVzRefitClosestZ_mmkk", &VertexfitPVzRefitClosestZ_mmkk_, "VertexfitPVzRefitClosestZ_mmkk/D");

bmmgTree_->Branch("VertexfitBsCt3DPVClosestZ_mmkk", &VertexfitBsCt3DPVClosestZ_mmkk_, "VertexfitBsCt3DPVClosestZ_mmkk/D");
bmmgTree_->Branch("VertexfitBsCt2DPVClosestZ_mmkk", &VertexfitBsCt2DPVClosestZ_mmkk_, "VertexfitBsCt2DPVClosestZ_mmkk/D");
bmmgTree_->Branch("VertexfitBsCt3DPVCosTheta_mmkk", &VertexfitBsCt3DPVCosTheta_mmkk_, "VertexfitBsCt3DPVCosTheta_mmkk/D");
bmmgTree_->Branch("VertexfitBsCt2DPVCosTheta_mmkk", &VertexfitBsCt2DPVCosTheta_mmkk_, "VertexfitBsCt2DPVCosTheta_mmkk/D");

bmmgTree_->Branch("VertexfitBsCtErr2D_mmkk", &VertexfitBsCtErr2D_mmkk_, "VertexfitBsCtErr2D_mmkk/D");
bmmgTree_->Branch("VertexfitBsCtErr3D_mmkk", &VertexfitBsCtErr3D_mmkk_, "VertexfitBsCtErr3D_mmkk/D");

bmmgTree_->Branch("VertexfitBsDist3D_mmkk", &VertexfitBsDist3D_mmkk_, "VertexfitBsDist3D_mmkk/D");
bmmgTree_->Branch("VertexfitBsDist3DErr_mmkk", &VertexfitBsDist3DErr_mmkk_, "VertexfitBsDist3DErr_mmkk/D");
bmmgTree_->Branch("VertexfitBsDist2D_mmkk", &VertexfitBsDist2D_mmkk_, "VertexfitBsDist2D_mmkk/D");
bmmgTree_->Branch("VertexfitBsDist2DErr_mmkk", &VertexfitBsDist2DErr_mmkk_, "VertexfitBsDist2DErr_mmkk/D");

bmmgTree_->Branch("BsVtxProb_mmkk", &BsVtxProb_mmkk_, "BsVtxProb_mmkk/D");
bmmgTree_->Branch("BsCt2D_mmkk", &BsCt2D_mmkk_, "BsCt2D_mmkk/D");
bmmgTree_->Branch("BsCt3D_mmkk", &BsCt3D_mmkk_, "BsCt3D_mmkk/D");

bmmgTree_->Branch("DeltaRKaon1DiMuon_mmkk", &DeltaRKaon1DiMuon_mmkk_, "DeltaRKaon1DiMuon_mmkk/D");
bmmgTree_->Branch("DeltaRKaon2DiMuon_mmkk", &DeltaRKaon2DiMuon_mmkk_, "DeltaRKaon2DiMuon_mmkk/D");

bmmgTree_->Branch("BsHelicity_mmkk", &BsHelicity_mmkk_, "BsHelicity_mmkk/D");
bmmgTree_->Branch("BsCoplanarity_mmkk", &BsCoplanarity_mmkk_, "BsCoplanarity_mmkk/D");


/////
bmmgTree_->Branch("DiMuonMass_Jpsi", &DiMuonMass_Jpsi_, "DiMuonMass_Jpsi/D");
bmmgTree_->Branch("DiMuonEta_Jpsi", &DiMuonEta_Jpsi_, "DiMuonEta_Jpsi/D");
bmmgTree_->Branch("DiMuonPhi_Jpsi", &DiMuonPhi_Jpsi_, "DiMuonPhi_Jpsi/D");
bmmgTree_->Branch("DiMuonPt_Jpsi", &DiMuonPt_Jpsi_, "DiMuonPt_Jpsi/D");

bmmgTree_->Branch("DiMuonMass_Phi", &DiMuonMass_Phi_, "DiMuonMass_Phi/D");
bmmgTree_->Branch("DiMuonEta_Phi", &DiMuonEta_Phi_, "DiMuonEta_Phi/D");
bmmgTree_->Branch("DiMuonPhi_Phi", &DiMuonPhi_Phi_, "DiMuonPhi_Phi/D");
bmmgTree_->Branch("DiMuonPt_Phi", &DiMuonPt_Phi_, "DiMuonPt_Phi/D");

bmmgTree_->Branch("DiMuonMass_Kstar0", &DiMuonMass_Kstar0_, "DiMuonMass_Kstar0/D");
bmmgTree_->Branch("DiMuonEta_Kstar0", &DiMuonEta_Kstar0_, "DiMuonEta_Kstar0/D");
bmmgTree_->Branch("DiMuonPhi_Kstar0", &DiMuonPhi_Kstar0_, "DiMuonPhi_Kstar0/D");
bmmgTree_->Branch("DiMuonPt_Kstar0", &DiMuonPt_Kstar0_, "DiMuonPt_Kstar0/D");

bmmgTree_->Branch("DiMuonMass_NoBound", &DiMuonMass_NoBound_, "DiMuonMass_NoBound/D");
bmmgTree_->Branch("DiMuonEta_NoBound", &DiMuonEta_NoBound_, "DiMuonEta_NoBound/D");
bmmgTree_->Branch("DiMuonPhi_NoBound", &DiMuonPhi_NoBound_, "DiMuonPhi_NoBound/D");
bmmgTree_->Branch("DiMuonPt_NoBound", &DiMuonPt_NoBound_, "DiMuonPt_NoBound/D");




  bmmgTree_->Branch("BsM_beffit", &BsM_beffit_,"BsM_beffit/D");
  bmmgTree_->Branch("BsEta_beffit", &BsEta_beffit_,"BsEta_beffit/D");
  bmmgTree_->Branch("BsPhi_beffit", &BsPhi_beffit_,"BsPhi_beffit/D");
  bmmgTree_->Branch("BsPt_beffit", &BsPt_beffit_,"BsPt_beffit/D");
  bmmgTree_->Branch("Bs_vtxProb", &Bs_vtxProb_,"Bs_vtxProb/D");
  bmmgTree_->Branch("BsCt3D", &BsCt3D_,"BsCt3D/D");
  bmmgTree_->Branch("BsCt2D", &BsCt2D_,"BsCt2D/D");
  bmmgTree_->Branch("BsCt2DBS", &BsCt2DBS_,"BsCt2DBS/D");
  bmmgTree_->Branch("BdCt2DBS", &BdCt2DBS_,"BdCt2DBS/D");
  bmmgTree_->Branch("BsCtMPV", &BsCtMPV_,"BsCtMPV/D");
  bmmgTree_->Branch("BsCt3Drefit", &BsCt3Drefit_,"BsCt3Drefit/D");
  bmmgTree_->Branch("BsCt2Drefit", &BsCt2Drefit_,"BsCt2Drefit/D");
  bmmgTree_->Branch("BsCtMPVrefit", &BsCtMPVrefit_,"BsCtMPVrefit/D");
  bmmgTree_->Branch("BsCtErr", &BsCtErr_,"BsCtErr/D");
  bmmgTree_->Branch("BsCtErr3D", &BsCtErr3D_,"BsCtErr3D/D");
  bmmgTree_->Branch("BsCtErr2D", &BsCtErr2D_,"BsCtErr2D/D");
  bmmgTree_->Branch("BsCtErr2DBS", &BsCtErr2DBS_,"BsCtErr2DBS/D");
  bmmgTree_->Branch("BsCtErr2DClosestZ", &BsCtErr2DClosestZ_,"BsCtErr2DClosestZ/D");
  bmmgTree_->Branch("BdCtErr2DBS", &BdCtErr2DBS_,"BdCtErr2DBS/D");
  bmmgTree_->Branch("BsCtErr2D2", &BsCtErr2D2_,"BsCtErr2D2/D");
  bmmgTree_->Branch("BsCtErrMPV", &BsCtErrMPV_,"BsCtErrMPV/D");
  bmmgTree_->Branch("BsCtErr3Drefit", &BsCtErr3Drefit_,"BsCtErr3Drefit/D");
  bmmgTree_->Branch("BsCtErr2Drefit", &BsCtErr2Drefit_,"BsCtErr2Drefit/D");
  bmmgTree_->Branch("BsCtErrMPVrefit", &BsCtErrMPVrefit_,"BsCtErrMPVrefit/D");
  bmmgTree_->Branch("BsCtErr2DBSOld", &BsCtErr2DBSOld_,"BsCtErr2DBSOld/D"); 
  bmmgTree_->Branch("BsCtErr2DClosestZOld", &BsCtErr2DClosestZOld_,"BsCtErr2DClosestZOld/D");
  bmmgTree_->Branch("BsCt2DPVClosestZOld", &BsCt2DPVClosestZOld_,"BsCt2DPVClosestZOld/D");
  bmmgTree_->Branch("BsCtErr2DOld", &BsCtErr2DOld_,"BsCtErr2DOld/D");
  bmmgTree_->Branch("BsCt2DOld", &BsCt2DOld_,"BsCt2DOld/D");
  bmmgTree_->Branch("HadronMass_fromVertexFitConPhoton", &HadronMass_fromVertexFitConPhoton_,"HadronMass_fromVertexFitConPhoton/D");
  bmmgTree_->Branch("HadronMass_fromVertexFitRecoPhoton", &HadronMass_fromVertexFitRecoPhoton_,"HadronMass_fromVertexFitRecoPhoton/D");
  bmmgTree_->Branch("vertexTypeFlag", &vertexTypeFlag_,"vertexTypeFlag/I");
  bmmgTree_->Branch("PFECal_SC_Eta", &PFECal_SC_Eta_,"PFECal_SC_Eta/D");
  bmmgTree_->Branch("PFECal_SC_Phi", &PFECal_SC_Phi_,"PFECal_SC_Phi/D");
  bmmgTree_->Branch("PFECal_SC_EtaWidth", &PFECal_SC_EtaWidth_,"PFECal_SC_EtaWidth/D");
  bmmgTree_->Branch("PFECal_SC_PhiWidth", &PFECal_SC_PhiWidth_,"PFECal_SC_PhiWidth/D");
  bmmgTree_->Branch("PFECal_SC_ClusterEnergy", &PFECal_SC_ClusterEnergy_,"PFECal_SC_ClusterEnergy/D");
  bmmgTree_->Branch("PFECAL_RecHit_RawId", &PFECAL_RecHit_RawId_,"PFECAL_RecHit_RawId/i");
  bmmgTree_->Branch("PFECAL_RecHit_Subdet", &PFECAL_RecHit_Subdet_,"PFECAL_RecHit_Subdet/I");
  bmmgTree_->Branch("PFECAL_RecHit_Energy", &PFECAL_RecHit_Energy_,"PFECAL_RecHit_Energy/D");
  bmmgTree_->Branch("PFECAL_RecHit_Time", &PFECAL_RecHit_Time_,"PFECAL_RecHit_Time/D");
  bmmgTree_->Branch("PFECAL_RecHit_X", &PFECAL_RecHit_X_,"PFECAL_RecHit_X/D");
  bmmgTree_->Branch("PFECAL_RecHit_Y", &PFECAL_RecHit_Y_,"PFECAL_RecHit_Y/D");
  bmmgTree_->Branch("PFECAL_RecHit_Z", &PFECAL_RecHit_Z_,"PFECAL_RecHit_Z/D");
  bmmgTree_->Branch("PFECAL_RecHit_PVDistance", &PFECAL_RecHit_PVDistance_,"PFECAL_RecHit_PVDistance/D");
  bmmgTree_->Branch("PFECAL_RecHit_TOF", &PFECAL_RecHit_TOF_,"PFECAL_RecHit_TOF/D");
  bmmgTree_->Branch("PFECAL_RecHit_CorrectedTime", &PFECAL_RecHit_CorrectedTime_,"PFECAL_RecHit_CorrectedTime/D");
  bmmgTree_->Branch("PFECAL_RecHit_IsWeird", &PFECAL_RecHit_IsWeird_,"PFECAL_RecHit_IsWeird/O");
  bmmgTree_->Branch("PFECAL_RecHit_IsProblematic", &PFECAL_RecHit_IsProblematic_,"PFECAL_RecHit_IsProblematic/O");
  bmmgTree_->Branch("PFECAL_RecHit_PoorReco", &PFECAL_RecHit_PoorReco_,"PFECAL_RecHit_PoorReco/O");
  bmmgTree_->Branch("PFECAL_RecHit_EB_ieta", &PFECAL_RecHit_EB_ieta_,"PFECAL_RecHit_EB_ieta/I");
  bmmgTree_->Branch("PFECAL_RecHit_EB_iphi", &PFECAL_RecHit_EB_iphi_,"PFECAL_RecHit_EB_iphi/I");
  bmmgTree_->Branch("PFECAL_RecHit_EE_ix", &PFECAL_RecHit_EE_ix_,"PFECAL_RecHit_EE_ix/D");
  bmmgTree_->Branch("PFECAL_RecHit_EE_iy", &PFECAL_RecHit_EE_iy_,"PFECAL_RecHit_EE_iy/D");
  bmmgTree_->Branch("PFECAL_RecHit_EE_zside", &PFECAL_RecHit_EE_zside_,"PFECAL_RecHit_EE_zside/D");

  bmmgTree_->Branch("photonMultiplicity", &photonMultiplicity_,"photonMultiplicity/I"); 
  bmmgTree_->Branch("isFourBody",&isFourBody_,"isFourBody/I");
  bmmgTree_->Branch("photonPt",&photonPt_,"photonPt[2]/D");
  bmmgTree_->Branch("photonEta",&photonEta_,"photonEta[2]/D");
  bmmgTree_->Branch("photonPhi",&photonPhi_,"photonPhi[2]/D");
  bmmgTree_->Branch("photonEnergy",&photonEnergy_,"photonEnergy[2]/D");
  bmmgTree_->Branch("photonET",&photonET_,"photonET[2]/D");
  bmmgTree_->Branch("photonTrkIso",&photonTrkIso_,"photonTrkIso[2]/D");
  bmmgTree_->Branch("photonEcalIso",&photonEcalIso_,"photonEcalIso[2]/D");
  bmmgTree_->Branch("photonHcalIso",&photonHcalIso_,"photonHcalIso[2]/D");
  bmmgTree_->Branch("photonCaloIso",&photonCaloIso_,"photonCaloIso[2]/D");
  bmmgTree_->Branch("photonSSSigmaiEtaiEta",&photonSSSigmaiEtaiEta_,"photonSSSigmaiEtaiEta[2]/D");
  bmmgTree_->Branch("photonSSSigmaiEtaiPhi",&photonSSSigmaiEtaiPhi_,"photonSSSigmaiEtaiPhi[2]/D");
  bmmgTree_->Branch("photonSSSigmaiPhiiPhi",&photonSSSigmaiPhiiPhi_,"photonSSSigmaiPhiiPhi[2]/D");
  bmmgTree_->Branch("photonSSSigmaEtaEta",&photonSSSigmaEtaEta_,"photonSSSigmaEtaEta[2]/D");
  bmmgTree_->Branch("photonSSe1x5",&photonSSe1x5_,"photonSSe1x5[2]/D");
  bmmgTree_->Branch("photonSSe2x5",&photonSSe2x5_,"photonSSe2x5[2]/D");
  bmmgTree_->Branch("photonSSe3x3",&photonSSe3x3_,"photonSSe3x3[2]/D");
  bmmgTree_->Branch("photonSSe5x5",&photonSSe5x5_,"photonSSe5x5[2]/D");
  bmmgTree_->Branch("photonSShcalDepth1OverEcal",&photonSShcalDepth1OverEcal_,"photonSShcalDepth1OverEcal[2]/D");
  bmmgTree_->Branch("photonSShcalDepth2OverEcal",&photonSShcalDepth2OverEcal_,"photonSShcalDepth2OverEcal[2]/D");
  bmmgTree_->Branch("photonSShcalDepth1OverEcalBc",&photonSShcalDepth1OverEcalBc_,"photonSShcalDepth1OverEcalBc[2]/D");
  bmmgTree_->Branch("photonSShcalDepth2OverEcalBc",&photonSShcalDepth2OverEcalBc_,"photonSShcalDepth2OverEcalBc[2]/D");
  bmmgTree_->Branch("photonSShcalOverEcal",&photonSShcalOverEcal_,"photonSShcalOverEcal[7][2]/D");
  bmmgTree_->Branch("photonSShcalOverEcalBc",&photonSShcalOverEcalBc_,"photonSShcalOverEcalBc[7][2]/D");
  bmmgTree_->Branch("photonSSmaxEnergyXtal",&photonSSmaxEnergyXtal_,"photonSSmaxEnergyXtal[2]/D");
  bmmgTree_->Branch("photonSSeffSigmaRR",&photonSSeffSigmaRR_,"photonSSeffSigmaRR[2]/D");
  bmmgTree_->Branch("photonSCEnergy",&photonSCEnergy_,"photonSCEnergy[2]/D");
  bmmgTree_->Branch("photonSCRawEnergy",&photonSCRawEnergy_,"photonSCRawEnergy[2]/D");
  bmmgTree_->Branch("photonSCPreShowerEP1",&photonSCPreShowerEP1_,"photonSCPreShowerEP1[2]/D");
  bmmgTree_->Branch("photonSCPreShowerEP2",&photonSCPreShowerEP2_,"photonSCPreShowerEP2[2]/D");
  bmmgTree_->Branch("photonSCEta",&photonSCEta_,"photonSCEta[2]/D");
  bmmgTree_->Branch("photonSCPhi",&photonSCPhi_,"photonSCPhi[2]/D");
  bmmgTree_->Branch("photonSCEtaWidth",&photonSCEtaWidth_,"photonSCEtaWidth[2]/D");
  bmmgTree_->Branch("photonSCPhiWidth",&photonSCPhiWidth_,"photonSCPhiWidth[2]/D");
  bmmgTree_->Branch("photonSCBrem",&photonSCBrem_,"photonSCBrem[2]/D");
  bmmgTree_->Branch("photonSCR9",&photonSCR9_,"photonSCR9[2]/D");
  bmmgTree_->Branch("photonSCHadTowOverEm",&photonSCHadTowOverEm_,"photonSCHadTowOverEm[2]/D");


  bmmgTree_->Branch("pfCandMultiplicity",&pfCandMultiplicity_,"pfCandMultiplicity/I");
  bmmgTree_->Branch("pfCandPt",pfCandPt_,"pfCandPt[2]/D");
  bmmgTree_->Branch("pfCandEta",pfCandEta_,"pfCandEta[2]/D");
  bmmgTree_->Branch("pfCandPhi",pfCandPhi_,"pfCandPhi[2]/D");
  bmmgTree_->Branch("pfCandEnergy",pfCandEnergy_,"pfCandEnergy[2]/D");
  bmmgTree_->Branch("pfCandET",pfCandET_,"pfCandET[2]/D");
  bmmgTree_->Branch("pfCandMass",pfCandMass_,"pfCandMass[2]/D");
  bmmgTree_->Branch("pfCandCharge",pfCandCharge_,"pfCandCharge[2]/I");
  bmmgTree_->Branch("pfCandEcalEnergy",pfCandEcalEnergy_,"pfCandEcalEnergy[2]/D"); 
  bmmgTree_->Branch("pfCandRawEcalEnergy",pfCandRawEcalEnergy_,"pfCandRawEcalEnergy[2]/D");
  bmmgTree_->Branch("pfCandHcalEnergy",pfCandHcalEnergy_,"pfCandHcalEnergy[2]/D");
  bmmgTree_->Branch("pfCandRawHcalEnergy",pfCandRawHcalEnergy_,"pfCandRawHcalEnergy[2]/D");
  bmmgTree_->Branch("pfCandHoEnergy",pfCandHoEnergy_,"pfCandHoEnergy[2]/D");
  bmmgTree_->Branch("pfCandRawHoEnergy",pfCandRawHoEnergy_,"pfCandRawHoEnergy[2]/D");
  bmmgTree_->Branch("pfCandTime",pfCandTime_,"pfCandTime[2]/D");
  bmmgTree_->Branch("pfCandTrkIso",pfCandTrkIso_,"pfCandTrkIso[2]/D");
  bmmgTree_->Branch("pfCandEcalIso",pfCandEcalIso_,"pfCandEcalIso[2]/D");
  bmmgTree_->Branch("pfCandHcalIso",pfCandHcalIso_,"pfCandHcalIso[2]/D");
  bmmgTree_->Branch("pfCandCaloIso",pfCandCaloIso_,"pfCandCaloIso[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonPt",pfCandRefPhotonPt_,"pfCandRefPhotonPt[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonEta",pfCandRefPhotonEta_,"pfCandRefPhotonEta[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonPhi",pfCandRefPhotonPhi_,"pfCandRefPhotonPhi[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonEnergy",pfCandRefPhotonEnergy_,"pfCandRefPhotonEnergy[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonET",pfCandRefPhotonET_,"pfCandRefPhotonET[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonSigmaIEtaIEta",pfCandRefPhotonSigmaIEtaIEta_,"pfCandRefPhotonSigmaIEtaIEta[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonSigmaIEtaIPhi",pfCandRefPhotonSigmaIEtaIPhi_,"pfCandRefPhotonSigmaIEtaIPhi[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonSigmaIPhiIPhi",pfCandRefPhotonSigmaIPhiIPhi_,"pfCandRefPhotonSigmaIPhiIPhi[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonSigmaEtaEta",pfCandRefPhotonSigmaEtaEta_,"pfCandRefPhotonSigmaEtaEta[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonE1x5",pfCandRefPhotonE1x5_,"pfCandRefPhotonE1x5[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonE2x5",pfCandRefPhotonE2x5_,"pfCandRefPhotonE2x5[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonE3x3",pfCandRefPhotonE3x3_,"pfCandRefPhotonE3x3[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonE5x5",pfCandRefPhotonE5x5_,"pfCandRefPhotonE5x5[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonHcalDepth1OverEcal",pfCandRefPhotonHcalDepth1OverEcal_,"pfCandRefPhotonHcalDepth1OverEcal[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonHcalDepth2OverEcal",pfCandRefPhotonHcalDepth2OverEcal_,"pfCandRefPhotonHcalDepth2OverEcal[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonHcalDepth1OverEcalBc",pfCandRefPhotonHcalDepth1OverEcalBc_,"pfCandRefPhotonHcalDepth1OverEcalBc[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonHcalDepth2OverEcalBc",pfCandRefPhotonHcalDepth2OverEcalBc_,"pfCandRefPhotonHcalDepth2OverEcalBc[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScEnergy",pfCandRefPhotonScEnergy_,"pfCandRefPhotonScEnergy[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScRawEnergy",pfCandRefPhotonScRawEnergy_,"pfCandRefPhotonScRawEnergy[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScEta",pfCandRefPhotonScEta_,"pfCandRefPhotonScEta[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScPhi",pfCandRefPhotonScPhi_,"pfCandRefPhotonScPhi[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScEnergy",pfCandRefPhotonScEnergy_,"pfCandRefPhotonScEnergy[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScEtaWidth",pfCandRefPhotonScEtaWidth_,"pfCandRefPhotonScEtaWidth[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScPhiWidth",pfCandRefPhotonScPhiWidth_,"pfCandRefPhotonScPhiWidth[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonScBrem",pfCandRefPhotonScBrem_,"pfCandRefPhotonScBrem[2]/D");  
  bmmgTree_->Branch("pfCandRefPhotonR9",pfCandRefPhotonR9_,"pfCandRefPhotonR9[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonHadTowOverEm",pfCandRefPhotonHadTowOverEm_,"pfCandRefPhotonHadTowOverEm[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonMaxEnergyXtal",pfCandRefPhotonMaxEnergyXtal_,"pfCandRefPhotonMaxEnergyXtal[2]/D");
  bmmgTree_->Branch("pfCandRefPhotonEffSigmaRR",pfCandRefPhotonEffSigmaRR_,"pfCandRefPhotonEffSigmaRR[2]/D");  
  bmmgTree_->Branch("pfCandRefPhotonHcalOverEcal",pfCandRefPhotonHcalOverEcal_,"pfCandRefPhotonHcalOverEcal[7][2]/D");
  bmmgTree_->Branch("pfCandRefPhotonHcalOverEcalBc",pfCandRefPhotonHcalOverEcalBc_,"pfCandRefPhotonHcalOverEcalBc[7][2]/D");

  bmmgTree_->Branch("DiGammaM_alone",&DiGammaM_alone_,"DiGammaM_alone/D");
  bmmgTree_->Branch("DiGammaEta_alone",&DiGammaEta_alone_,"DiGammaEta_alone/D");
  bmmgTree_->Branch("DiGammaPhi_alone",&DiGammaPhi_alone_,"DiGammaPhi_alone/D");
  bmmgTree_->Branch("DiGammaPt_alone",&DiGammaPt_alone_,"DiGammaPt_alone/D");
  bmmgTree_->Branch("Bsmass_recommg", &Bsmass_recommg_, "Bsmass_recommg/D");
  bmmgTree_->Branch("Bshelicity_recommg", &Bshelicity_recommg_, "Bshelicity_recommg/D");
  bmmgTree_->Branch("Bscoplanarity_recommg", &Bscoplanarity_recommg_, "Bscoplanarity_recommg/D");
  bmmgTree_->Branch("Bspt_recommg", &Bspt_recommg_, "Bspt_recommg/D");
  bmmgTree_->Branch("Bseta_recommg", &Bseta_recommg_, "Bseta_recommg/D");
  bmmgTree_->Branch("Bsphi_recommg", &Bsphi_recommg_, "Bsphi_recommg/D"); 
  bmmgTree_->Branch("Bsmass_recommgg", &Bsmass_recommgg_, "Bsmass_recommgg/D");
  bmmgTree_->Branch("Bshelicity_recommgg", &Bshelicity_recommgg_, "Bshelicity_recommgg/D");
  bmmgTree_->Branch("Bscoplanarity_recommgg", &Bscoplanarity_recommgg_, "Bscoplanarity_recommgg/D");
  bmmgTree_->Branch("Bspt_recommgg", &Bspt_recommgg_, "Bspt_recommgg/D");
  bmmgTree_->Branch("Bseta_recommgg", &Bseta_recommgg_, "Bseta_recommgg/D");
  bmmgTree_->Branch("Bsphi_recommgg", &Bsphi_recommgg_, "Bsphi_recommgg/D");
  bmmgTree_->Branch("BmesonType", &BmesonType_, "BmesonType/I");


  
  bmmgTree_->Branch("electronMultiplicity",&electronMultiplicity_,"electronMultiplicity/D");
  bmmgTree_->Branch("costheta",&costheta_,"costheta/D");
  bmmgTree_->Branch("phi",&phi_,"phi/D");
  bmmgTree_->Branch("cospsi",&cospsi_,"cospsi/D");
  bmmgTree_->Branch("AngleBsDecayLength",&AngleBsDecayLength_,"AngleBsDecayLength/D");

  
  
  bmmgTree_->Branch("JpsiGenPVz_",&JpsiGenPVz_,"JpsiGenPVz/D");
  bmmgTree_->Branch("JpsiGenPVy_",&JpsiGenPVy_,"JpsiGenPVy/D");
  bmmgTree_->Branch("JpsiGenPVx_",&JpsiGenPVx_,"JpsiGenPVx/D");
  bmmgTree_->Branch("JpsiGenPt_",&JpsiGenPt_,"JpsiGenPt/D");
  bmmgTree_->Branch("JpsiGenLxy_",&JpsiGenLxy_,"JpsiGenLxy/D");
  bmmgTree_->Branch("JpsiGenLxyOld_",&JpsiGenLxyOld_,"JpsiGenLxyOld/D");
  bmmgTree_->Branch("JpsiGenLxyOverPt_",&JpsiGenLxyOverPt_,"JpsiGenLxyOverPt/D");
  bmmgTree_->Branch("JpsiGenNumberOfCandidates_",&JpsiGenNumberOfCandidates_,"JpsiGenNumberOfCandidates/D");


  bmmgTree_->Branch("GenMotherID", &GenMotherID_,"GenMotherID/I");
  bmmgTree_->Branch("GenB_pt", &GenB_pt_,"GenB_pt/D");
  bmmgTree_->Branch("GenB_eta", &GenB_eta_,"GenB_eta/D");
  bmmgTree_->Branch("GenB_phi", &GenB_phi_,"GenB_phi/D");
  bmmgTree_->Branch("GenB_mass", &GenB_mass_,"GenB_mass/D");
  bmmgTree_->Branch("GenBLxy", &GenBLxy_,"GenBLxy/D");
  bmmgTree_->Branch("GenBct", &GenBct_,"GenBct/D");
  bmmgTree_->Branch("GenBctErr", &GenBctErr_,"GenBctErr/D");
  bmmgTree_->Branch("GenMuPlus_pt", &GenMuPlus_pt_,"GenMuPlus_pt/D");
  bmmgTree_->Branch("GenMuPlus_eta", &GenMuPlus_eta_,"GenMuPlus_eta/D");
  bmmgTree_->Branch("GenMuPlus_phi", &GenMuPlus_phi_,"GenMuPlus_phi/D");
  bmmgTree_->Branch("GenMuPlus_E", &GenMuPlus_E_,"GenMuPlus_E/D");
  bmmgTree_->Branch("GenMuMinus_pt", &GenMuMinus_pt_,"GenMuMinus_pt/D");
  bmmgTree_->Branch("GenMuMinus_eta", &GenMuMinus_eta_,"GenMuMinus_eta/D");
  bmmgTree_->Branch("GenMuMinus_phi", &GenMuMinus_phi_,"GenMuMinus_phi/D");
  bmmgTree_->Branch("GenMuMinus_E", &GenMuMinus_E_,"GenMuMinus_E/D");
  bmmgTree_->Branch("GenDimuon_mass", &GenDimuon_mass_,"GenDimuon_mass/D");
  bmmgTree_->Branch("GenDimuon_pt", &GenDimuon_pt_,"GenDimuon_pt/D");
  bmmgTree_->Branch("GenNPhotons", &GenNPhotons_,"GenNPhotons/I");
  bmmgTree_->Branch("GenGamma_pt", GenGamma_pt_, "GenGamma_pt[4]/D");
  bmmgTree_->Branch("GenGamma_eta", GenGamma_eta_, "GenGamma_eta[4]/D");
  bmmgTree_->Branch("GenGamma_phi", GenGamma_phi_, "GenGamma_phi[4]/D");
  bmmgTree_->Branch("GenGamma_E", GenGamma_E_, "GenGamma_E[4]/D");
  bmmgTree_->Branch("GenGamma_origin", GenGamma_origin_, "GenGamma_origin[4]/I");
  bmmgTree_->Branch("GenGamma_motherId", GenGamma_motherId_, "GenGamma_motherId[4]/I");
  bmmgTree_->Branch("GenGamma_dRmu1", GenGamma_dRmu1_, "GenGamma_dRmu1[4]/D");
  bmmgTree_->Branch("GenGamma_dRmu2", GenGamma_dRmu2_, "GenGamma_dRmu2[4]/D");    
  bmmgTree_->Branch("GenHelicity3", &GenHelicity3_,"GenHelicity3/D");
  bmmgTree_->Branch("GenCoplanarity3", &GenCoplanarity3_,"GenCoplanarity3/D");
  bmmgTree_->Branch("GenCosTheta_l", &GenCosTheta_l_,"GenCosTheta_l/D");
  bmmgTree_->Branch("GenTriBodyMass", &GenTriBodyMass_,"GenTriBodyMass/D");
  bmmgTree_->Branch("GenHelicity4", &GenHelicity4_,"GenHelicity4/D");
  bmmgTree_->Branch("GenCoplanarity4", &GenCoplanarity4_,"GenCoplanarity4/D");
  bmmgTree_->Branch("GenFourBodyMass", &GenFourBodyMass_,"GenFourBodyMass/D");
  bmmgTree_->Branch("GenDiphoton_mass", &GenDiphoton_mass_,"GenDiphoton_mass/D");
  bmmgTree_->Branch("GenDiphoton_pt", &GenDiphoton_pt_,"GenDiphoton_pt/D");
  bmmgTree_->Branch("GenHasJpsi", &GenHasJpsi_,"GenHasJpsi/O");
  bmmgTree_->Branch("GenJpsi_mass", &GenJpsi_mass_,"GenJpsi_mass/D");
  bmmgTree_->Branch("GenJpsi_pt", &GenJpsi_pt_,"GenJpsi_pt/D");
  bmmgTree_->Branch("GenHasNeutralMeson", &GenHasNeutralMeson_,"GenHasNeutralMeson/O");
  bmmgTree_->Branch("GenNeutralMesonId", &GenNeutralMesonId_,"GenNeutralMesonId/I");
  bmmgTree_->Branch("GenDecayMode", &GenDecayMode_,"GenDecayMode/I");
  bmmgTree_->Branch("GenIs3Body", &GenIs3Body_,"GenIs3Body/O");
  bmmgTree_->Branch("GenIs4Body", &GenIs4Body_,"GenIs4Body/O");
  bmmgTree_->Branch("GenIsHardProcess", &GenIsHardProcess_,"GenIsHardProcess/O");
  bmmgTree_->Branch("GenIsSignal", &GenIsSignal_,"GenIsSignal/O");  
  
  bmmgTree_->Branch("Mu1_isMatched", &Mu1_isMatched_,"Mu1_isMatched/O");
  bmmgTree_->Branch("Mu1_genPdgId", &Mu1_genPdgId_,"Mu1_genPdgId/I");
  bmmgTree_->Branch("Mu1_genMotherPdgId", &Mu1_genMotherPdgId_,"Mu1_genMotherPdgId/I");
  bmmgTree_->Branch("Mu1_genGM  otherPdgId", &Mu1_genGMotherPdgId_,"Mu1_genGMotherPdgId/I");
  bmmgTree_->Branch("Mu1_genDeltaR", &Mu1_genDeltaR_,"Mu1_genDeltaR/F");
  bmmgTree_->Branch("Mu1_genDeltaPt", &Mu1_genDeltaPt_,"Mu1_genDeltaPt/F");
  bmmgTree_->Branch("Mu2_isMatched", &Mu2_isMatched_,"Mu2_isMatched/O");
  bmmgTree_->Branch("Mu2_genPdgId", &Mu2_genPdgId_,"Mu2_genPdgId/I");
  bmmgTree_->Branch("Mu2_genMotherPdgId", &Mu2_genMotherPdgId_,"Mu2_genMotherPdgId/I");
  bmmgTree_->Branch("Mu2_genGMotherPdgId", &Mu2_genGMotherPdgId_,"Mu2_genGMotherPdgId/I");
  bmmgTree_->Branch("Mu2_genDeltaR", &Mu2_genDeltaR_,"Mu2_genDeltaR/F");
  bmmgTree_->Branch("Mu2_genDeltaPt", &Mu2_genDeltaPt_,"Mu2_genDeltaPt/F");
  bmmgTree_->Branch("DiMuon_isMatched", &DiMuon_isMatched_,"DiMuon_isMatched/O");
  bmmgTree_->Branch("Photon_isMatched", Photon_isMatched_, "Photon_isMatched[4]/O");
  bmmgTree_->Branch("Photon_genPdgId", Photon_genPdgId_, "Photon_genPdgId[4]/I");
  bmmgTree_->Branch("Photon_genMotherPdgId", Photon_genMotherPdgId_, "Photon_genMotherPdgId[4]/I");
  bmmgTree_->Branch("Photon_genGMotherPdgId", Photon_genGMotherPdgId_, "Photon_genGMotherPdgId[4]/I");
  bmmgTree_->Branch("Photon_genDeltaR", Photon_genDeltaR_, "Photon_genDeltaR[4]/F");
  bmmgTree_->Branch("Photon_genDeltaPt", Photon_genDeltaPt_, "Photon_genDeltaPt[4]/F");
  bmmgTree_->Branch("Photon_truthOrigin", Photon_truthOrigin_, "Photon_truthOrigin[4]/I");
  bmmgTree_->Branch("NMatchedPhotons", &NMatchedPhotons_,"NMatchedPhotons/I");
  bmmgTree_->Branch("Topology_isMatched", &Topology_isMatched_,"Topology_isMatched/O");
  bmmgTree_->Branch("Topology_mu1Matched", &Topology_mu1Matched_,"Topology_mu1Matched/O");
  bmmgTree_->Branch("Topology_mu2Matched", &Topology_mu2Matched_,"Topology_mu2Matched/O");
  bmmgTree_->Branch("Topology_photon1Matched", &Topology_photon1Matched_,"Topology_photon1Matched/O");
  bmmgTree_->Branch("Topology_photon2Matched", &Topology_photon2Matched_,"Topology_photon2Matched/O");
  bmmgTree_->Branch("Topology_decayModeMatched", &Topology_decayModeMatched_,"Topology_decayModeMatched/O");
  bmmgTree_->Branch("IsSignalMatched", &IsSignalMatched_,"IsSignalMatched/O");
  bmmgTree_->Branch("IsMuonsFromJpsi", &IsMuonsFromJpsi_,"IsMuonsFromJpsi/O");
  bmmgTree_->Branch("IsMuonsDirectFromBs", &IsMuonsDirectFromBs_,"IsMuonsDirectFromBs/O");


}

RadiativeRootTree::~RadiativeRootTree()
{}

void RadiativeRootTree::writeFile()
{
  bmmgFile_->Write();
  bmmgFile_->Close();

}
void RadiativeRootTree::resetEntries()
{
	
	runNumber_          = -9999999;
	eventNumber_        = -9999999;
	lumiSection_        = -9999999;
	PUinteraction_      = -9999999;
	PUTrueinteraction_  = -9999999;
	NVerticesbeforecut_ = -9999999;
	NVerticesaftercut_  = -9999999;
	NTracksPVbeforecut_ = -9999999;
	BSx_                = -9999999;
	BSy_                = -9999999;
	BSz_                = -9999999;
	BSdx_               = -9999999;
  BSdy_               = -9999999;
	BSdz_               = -9999999;
	BSdxdz_             = -9999999;
	BSdydz_             = -9999999;
	BSsigmaZ_           = -9999999;
	BSdsigmaZ_          = -9999999;
	PVx_                = -9999999;
	PVy_                = -9999999;
	PVz_                = -9999999;
	PVerrx_             = -9999999;
	PVerry_             = -9999999;
	PVerrz_             = -9999999;
	PVndof_             = -9999999;
	PVrho_              = -9999999;
	costheta_           = -9999999;
	phi_                = -9999999;
	cospsi_             = -9999999;
	AngleBsDecayLength_ = -9999999;
	isBS_               = -9999999;
	isPV_               = -9999999;
	dedxTrk_            = -9999999;
	errdedxTrk_         = -9999999;
	numdedxTrk_         = -9999999;
	triggerbit_HLT_DoubleMu4_LowMass_Displaced_            = -9999999;
	triggerbit_HLT_DoubleMu4_3_Bs_                         = -9999999;
	triggerbit_HLT_DoubleMu4_LowMass_                      = -9999999;
	triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_            = -9999999;
	triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_  = -9999999;
	triggerbit_HLTDimuon4JpsiDisplaced_                    = -9999999;
	triggerbit_HLTDimuon4JpsiNoVertexing_                  = -9999999;
	triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_              = -9999999;
  mu1Pt_beffit_ = -9999999;
	mu1Pz_beffit_ = -9999999;
	mu1Eta_beffit_ = -9999999;
	mu1Phi_beffit_ = -9999999;
  mu1Energy_beffit_ = -9999999;
	mu2Pt_beffit_ = -9999999;
	mu2Pz_beffit_ = -9999999;
	mu2Eta_beffit_ = -9999999;
	mu2Phi_beffit_ = -9999999;
  mu2Energy_beffit_ = -9999999;
	mu1SoftID_ = -9999999;
	mu2SoftID_ = -9999999;
  MuonPairDR_ = -9999999;
  Mu1TrkBSDxy_ = -9999999;
  Mu1TrkBSDz_ = -9999999;
  Mu2TrkBSDxy_ = -9999999;
  Mu2TrkBSDz_ = -9999999;
  Mu1PixelHits_ = -9999999;
  Mu1TrackerHits_ = -9999999;
  Mu1isGood_ = -9999999;
  Mu1InnerTrkHighQuality_ = -9999999;
  Mu2PixelHits_ = -9999999;
  Mu2TrackerHits_ = -9999999;
  Mu2isGood_ = -9999999;
  Mu2InnerTrkHighQuality_ = -9999999;
  mu1MVAScore_ = -9999999;
  mu2MVAScore_ = -9999999;
	DiMuonM_beffit_ = -9999999;
	DiMuonEta_beffit_ = -9999999;
	DiMuonPhi_beffit_ = -9999999;
	DiMuonPt_beffit_ = -9999999;
	DiMuon_vtxProb_ = -9999999;
	DiMuon_CosineAlpha_ = -9999999;
	DiMuon_DCA_ = -9999999;
	DiMuon_Chi2pv_KVFvtx_ = -9999999;
	DiMuon_Mahalanobis_ = -9999999;
	DiMuon_ResonanceType_ = -9999999;
  DiMuon_Chi2pv_KVFvtx_ = -9999999;
  DiMuon_Lxy_ = -9999999;
  DiMuon_Lxyerr_ = -9999999;
  DiMuon_LxyOverPt_ = -9999999;
  DiMuon_mu1Cat_alone_ = -9999999;
  DiMuon_mu2Cat_alone_ = -9999999;




// ---- Four-vector Bs ----
FourvectorBsMass_mmconvg_ = -9999999;
FourvectorBsPt_mmconvg_   = -9999999;
FourvectorBsEta_mmconvg_  = -9999999;
FourvectorBsPhi_mmconvg_  = -9999999;

// ---- Vertex-fit Bs kinematics ----
VertexfitBsMass_mmconvg_ = -9999999;
VertexfitBsPt_mmconvg_   = -9999999;
VertexfitBsEta_mmconvg_  = -9999999;
VertexfitBsPhi_mmconvg_  = -9999999;
VertexfitBsPz_mmconvg_   = -9999999;

// ---- Secondary vertex ----
VertexfitBsSVx_mmconvg_ = -9999999;
VertexfitBsSVy_mmconvg_ = -9999999;
VertexfitBsSVz_mmconvg_ = -9999999;

// ---- Converted photon four-vector ----
FourvectorConvGammaPt_mmconvg_  = -9999999;
FourvectorConvGammaEta_mmconvg_ = -9999999;
FourvectorConvGammaPhi_mmconvg_ = -9999999;

// ---- PV refit / cos(theta) ----
VertexfitPVxRefitCosTheta_mmconvg_ = -9999999;
VertexfitPVyRefitCosTheta_mmconvg_ = -9999999;
VertexfitPVzRefitCosTheta_mmconvg_ = -9999999;

// ---- PV refit / closest-Z ----
VertexfitPVxRefitClosestZ_mmconvg_ = -9999999;
VertexfitPVyRefitClosestZ_mmconvg_ = -9999999;
VertexfitPVzRefitClosestZ_mmconvg_ = -9999999;

// ---- Proper decay lengths ----
VertexfitBsCt3DPVClosestZ_mmconvg_ = -9999999;
VertexfitBsCt2DPVClosestZ_mmconvg_ = -9999999;
VertexfitBsCt3DPVCosTheta_mmconvg_ = -9999999;
VertexfitBsCt2DPVCosTheta_mmconvg_ = -9999999;

// ---- Lifetime errors ----
VertexfitBsCtErr2D_mmconvg_ = -9999999;
VertexfitBsCtErr3D_mmconvg_ = -9999999;

// ---- Distances and times (3D) ----
VertexfitBsDist3D_mmconvg_     = -9999999;
VertexfitBsDist3DErr_mmconvg_  = -9999999;
VertexfitBsTime3D_mmconvg_     = -9999999;
VertexfitBsTime3DErr_mmconvg_  = -9999999;

// ---- Distances and times (2D) ----
VertexfitBsDist2D_mmconvg_     = -9999999;
VertexfitBsDist2DErr_mmconvg_  = -9999999;
VertexfitBsTime2D_mmconvg_     = -9999999;
VertexfitBsTime2DErr_mmconvg_  = -9999999;

// ---- Global Bs observables ----
BsVtxProb_mmconvg_ = -9999999;
BsCt2D_mmconvg_    = -9999999;
BsCt3D_mmconvg_    = -9999999;

// ---- Angular / topology ----
DeltaRPhoton1DiMuon_mmconvg_ = -9999999;
DeltaRPhoton2DiMuon_mmconvg_ = -9999999;

BsHelicity_mmconvg_    = -9999999;
BsCoplanarity_mmconvg_ = -9999999;

// ---- Reco photon mmrecog ----
FourvectorBsMass_mmrecog_ = -9999999;
FourvectorBsPt_mmrecog_   = -9999999;
FourvectorBsEta_mmrecog_  = -9999999;
FourvectorBsPhi_mmrecog_  = -9999999;

VertexfitBsMass_mmrecog_ = -9999999;
VertexfitBsPt_mmrecog_   = -9999999;
VertexfitBsEta_mmrecog_  = -9999999;
VertexfitBsPhi_mmrecog_  = -9999999;
VertexfitBsPz_mmrecog_   = -9999999;

VertexfitBsSVx_mmrecog_ = -9999999;
VertexfitBsSVy_mmrecog_ = -9999999;
VertexfitBsSVz_mmrecog_ = -9999999;

FourvectorGammaPt_mmrecog_  = -9999999;
FourvectorGammaEta_mmrecog_ = -9999999;
FourvectorGammaPhi_mmrecog_ = -9999999;

VertexfitPVxRefitCosTheta_mmrecog_ = -9999999;
VertexfitPVyRefitCosTheta_mmrecog_ = -9999999;
VertexfitPVzRefitCosTheta_mmrecog_ = -9999999;

VertexfitPVxRefitClosestZ_mmrecog_ = -9999999;
VertexfitPVyRefitClosestZ_mmrecog_ = -9999999;
VertexfitPVzRefitClosestZ_mmrecog_ = -9999999;

VertexfitBsCt3DPVClosestZ_mmrecog_ = -9999999;
VertexfitBsCt2DPVClosestZ_mmrecog_ = -9999999;
VertexfitBsCt3DPVCosTheta_mmrecog_ = -9999999;
VertexfitBsCt2DPVCosTheta_mmrecog_ = -9999999;

VertexfitBsCtErr2D_mmrecog_ = -9999999;
VertexfitBsCtErr3D_mmrecog_ = -9999999;

VertexfitBsDist3D_mmrecog_     = -9999999;
VertexfitBsDist3DErr_mmrecog_  = -9999999;
VertexfitBsDist2D_mmrecog_     = -9999999;
VertexfitBsDist2DErr_mmrecog_  = -9999999;

BsVtxProb_mmrecog_ = -9999999;
BsCt2D_mmrecog_    = -9999999;
BsCt3D_mmrecog_    = -9999999;

DeltaRPhoton1DiMuon_mmrecog_ = -9999999;
DeltaRPhoton2DiMuon_mmrecog_ = -9999999;

BsHelicity_mmrecog_    = -9999999;
BsCoplanarity_mmrecog_ = -9999999;

// ---- Converted photon mmconvgg ----
FourvectorBsMass_mmconvgg_ = -9999999;
FourvectorBsPt_mmconvgg_   = -9999999;
FourvectorBsEta_mmconvgg_  = -9999999;
FourvectorBsPhi_mmconvgg_  = -9999999;

VertexfitBsMass_mmconvgg_ = -9999999;
VertexfitBsPt_mmconvgg_   = -9999999;
VertexfitBsEta_mmconvgg_  = -9999999;
VertexfitBsPhi_mmconvgg_  = -9999999;
VertexfitBsPz_mmconvgg_   = -9999999;

VertexfitBsSVx_mmconvgg_ = -9999999;
VertexfitBsSVy_mmconvgg_ = -9999999;
VertexfitBsSVz_mmconvgg_ = -9999999;

FourvectorConvGammaPt_mmconvgg_  = -9999999;
FourvectorConvGammaEta_mmconvgg_ = -9999999;
FourvectorConvGammaPhi_mmconvgg_ = -9999999;

VertexfitPVxRefitCosTheta_mmconvgg_ = -9999999;
VertexfitPVyRefitCosTheta_mmconvgg_ = -9999999;
VertexfitPVzRefitCosTheta_mmconvgg_ = -9999999;

VertexfitPVxRefitClosestZ_mmconvgg_ = -9999999;
VertexfitPVyRefitClosestZ_mmconvgg_ = -9999999;
VertexfitPVzRefitClosestZ_mmconvgg_ = -9999999;

VertexfitBsCt3DPVClosestZ_mmconvgg_ = -9999999;
VertexfitBsCt2DPVClosestZ_mmconvgg_ = -9999999;
VertexfitBsCt3DPVCosTheta_mmconvgg_ = -9999999;
VertexfitBsCt2DPVCosTheta_mmconvgg_ = -9999999;

VertexfitBsCtErr2D_mmconvgg_ = -9999999;
VertexfitBsCtErr3D_mmconvgg_ = -9999999;

VertexfitBsDist3D_mmconvgg_     = -9999999;
VertexfitBsDist3DErr_mmconvgg_  = -9999999;
VertexfitBsDist2D_mmconvgg_     = -9999999;
VertexfitBsDist2DErr_mmconvgg_  = -9999999;

BsVtxProb_mmconvgg_ = -9999999;
BsCt2D_mmconvgg_    = -9999999;
BsCt3D_mmconvgg_    = -9999999;

DeltaRPhoton1DiMuon_mmconvgg_ = -9999999;
DeltaRPhoton2DiMuon_mmconvgg_ = -9999999;

BsHelicity_mmconvgg_    = -9999999;
BsCoplanarity_mmconvgg_ = -9999999;

// ---- Reco photon mmrecogg ----
FourvectorBsMass_mmrecogg_ = -9999999;
FourvectorBsPt_mmrecogg_   = -9999999;
FourvectorBsEta_mmrecogg_  = -9999999;
FourvectorBsPhi_mmrecogg_  = -9999999;

VertexfitBsMass_mmrecogg_ = -9999999;
VertexfitBsPt_mmrecogg_   = -9999999;
VertexfitBsEta_mmrecogg_  = -9999999;
VertexfitBsPhi_mmrecogg_  = -9999999;
VertexfitBsPz_mmrecogg_   = -9999999;

VertexfitBsSVx_mmrecogg_ = -9999999;
VertexfitBsSVy_mmrecogg_ = -9999999;
VertexfitBsSVz_mmrecogg_ = -9999999;

FourvectorGammaPt_mmrecogg_  = -9999999;
FourvectorGammaEta_mmrecogg_ = -9999999;
FourvectorGammaPhi_mmrecogg_ = -9999999;

VertexfitPVxRefitCosTheta_mmrecogg_ = -9999999;
VertexfitPVyRefitCosTheta_mmrecogg_ = -9999999;
VertexfitPVzRefitCosTheta_mmrecogg_ = -9999999;

VertexfitPVxRefitClosestZ_mmrecogg_ = -9999999;
VertexfitPVyRefitClosestZ_mmrecogg_ = -9999999;
VertexfitPVzRefitClosestZ_mmrecogg_ = -9999999;

VertexfitBsCt3DPVClosestZ_mmrecogg_ = -9999999;
VertexfitBsCt2DPVClosestZ_mmrecogg_ = -9999999;
VertexfitBsCt3DPVCosTheta_mmrecogg_ = -9999999;
VertexfitBsCt2DPVCosTheta_mmrecogg_ = -9999999;

VertexfitBsCtErr2D_mmrecogg_ = -9999999;
VertexfitBsCtErr3D_mmrecogg_ = -9999999;

VertexfitBsDist3D_mmrecogg_     = -9999999;
VertexfitBsDist3DErr_mmrecogg_  = -9999999;
VertexfitBsDist2D_mmrecogg_     = -9999999;
VertexfitBsDist2DErr_mmrecogg_  = -9999999;

BsVtxProb_mmrecogg_ = -9999999;
BsCt2D_mmrecogg_    = -9999999;
BsCt3D_mmrecogg_    = -9999999;

DeltaRPhoton1DiMuon_mmrecogg_ = -9999999;
DeltaRPhoton2DiMuon_mmrecogg_ = -9999999;

BsHelicity_mmrecogg_    = -9999999;
BsCoplanarity_mmrecogg_ = -9999999;

// mu mu k k
FourvectorBsMass_mmkk_ = -9999999;
FourvectorBsPt_mmkk_ = -9999999;
FourvectorBsEta_mmkk_ = -9999999;
FourvectorBsPhi_mmkk_ = -9999999;

VertexfitBsMass_mmkk_ = -9999999;
VertexfitBsPt_mmkk_ = -9999999;
VertexfitBsEta_mmkk_ = -9999999;
VertexfitBsPhi_mmkk_ = -9999999;
VertexfitBsPz_mmkk_ = -9999999;

VertexfitBsSVx_mmkk_ = -9999999;
VertexfitBsSVy_mmkk_ = -9999999;
VertexfitBsSVz_mmkk_ = -9999999;

VertexfitPVxRefitCosTheta_mmkk_ = -9999999;
VertexfitPVyRefitCosTheta_mmkk_ = -9999999;
VertexfitPVzRefitCosTheta_mmkk_ = -9999999;

VertexfitPVxRefitClosestZ_mmkk_ = -9999999;
VertexfitPVyRefitClosestZ_mmkk_ = -9999999;
VertexfitPVzRefitClosestZ_mmkk_ = -9999999;

VertexfitBsCt3DPVClosestZ_mmkk_ = -9999999;
VertexfitBsCt2DPVClosestZ_mmkk_ = -9999999;
VertexfitBsCt3DPVCosTheta_mmkk_ = -9999999;
VertexfitBsCt2DPVCosTheta_mmkk_ = -9999999;

VertexfitBsCtErr2D_mmkk_ = -9999999;
VertexfitBsCtErr3D_mmkk_ = -9999999;

VertexfitBsDist3D_mmkk_ = -9999999;
VertexfitBsDist3DErr_mmkk_ = -9999999;
VertexfitBsDist2D_mmkk_ = -9999999;
VertexfitBsDist2DErr_mmkk_ = -9999999;

BsVtxProb_mmkk_ = -9999999;
BsCt2D_mmkk_ = -9999999;
BsCt3D_mmkk_ = -9999999;

DeltaRKaon1DiMuon_mmkk_ = -9999999;
DeltaRKaon2DiMuon_mmkk_ = -9999999;

BsHelicity_mmkk_ = -9999999;
BsCoplanarity_mmkk_ = -9999999;

DiMuon_mu1nPixHits_alone_ = -9999999;
DiMuon_mu2nPixHits_alone_ = -9999999;
DiMuon_NumberOfCandidates_ = -9999999;

// ===============================
// Reset variables (DiMuon blocks)
// ===============================

// ---- J/psi ----
DiMuonMass_Jpsi_ = -9999999;
DiMuonEta_Jpsi_  = -9999999;
DiMuonPhi_Jpsi_  = -9999999;
DiMuonPt_Jpsi_   = -9999999;

// ---- Phi ----
DiMuonMass_Phi_ = -9999999;
DiMuonEta_Phi_  = -9999999;
DiMuonPhi_Phi_  = -9999999;
DiMuonPt_Phi_   = -9999999;

// ---- K*0 ----
DiMuonMass_Kstar0_ = -9999999;
DiMuonEta_Kstar0_  = -9999999;
DiMuonPhi_Kstar0_  = -9999999;
DiMuonPt_Kstar0_   = -9999999;

// ---- No bound ----
DiMuonMass_NoBound_ = -9999999;
DiMuonEta_NoBound_  = -9999999;
DiMuonPhi_NoBound_  = -9999999;
DiMuonPt_NoBound_   = -9999999;


  BsM_beffit_ = -9999999;
	BsEta_beffit_ = -9999999;
	BsPhi_beffit_ = -9999999;
	BsPt_beffit_ = -9999999;
	Bs_vtxProb_ = -9999999;
  HadronMass_fromVertexFitConPhoton_ = -9999999;
  HadronMass_fromVertexFitRecoPhoton_ = -9999999;
  BsCt3D_ = -9999999;
  BsCt2D_ = -9999999;
  BsCt2DBS_ = -9999999;
  BdCt2DBS_ = -9999999;
  BsCtMPV_ = -9999999;
  BsCt3Drefit_ = -9999999;
  BsCt2Drefit_ = -9999999;
  BsCtMPVrefit_ = -9999999;
  BsCtErr_ = -9999999;
  BsCtErr3D_ = -9999999;
  BsCtErr2D_ = -9999999;
  BsCtErr2DBS_ = -9999999;
  BsCtErr2DClosestZ_ = -9999999;
  BdCtErr2DBS_ = -9999999;
  BsCtErr2D2_ = -9999999;
  BsCtErrMPV_ = -9999999;
  BsCtErr3Drefit_ = -9999999;
  BsCtErr2Drefit_ = -9999999;
  BsCtErrMPVrefit_ = -9999999;
  BsCtErr2DBSOld_ = -9999999;
  BsCtErr2DClosestZOld_ = -9999999;
  BsCt2DPVClosestZOld_ = -9999999;
  BsCtErr2DOld_ = -9999999;
  BsCt2DOld_ = -9999999;
  vertexTypeFlag_     = -9999999;

  PFECal_SC_Eta_ = -9999999;
  PFECal_SC_Phi_ = -9999999;
  PFECal_SC_EtaWidth_ = -9999999;
  PFECal_SC_PhiWidth_ = -9999999;
  PFECal_SC_ClusterEnergy_ = -9999999;
  PFECAL_RecHit_RawId_ = -9999999;
  PFECAL_RecHit_Subdet_ = -9999999;
  PFECAL_RecHit_Energy_ = -9999999;
  PFECAL_RecHit_Time_ = -9999999;
  PFECAL_RecHit_X_ = -9999999;
  PFECAL_RecHit_Y_ = -9999999;
  PFECAL_RecHit_Z_ = -9999999;
  PFECAL_RecHit_PVDistance_ = -9999999;
  PFECAL_RecHit_TOF_ = -9999999;
  PFECAL_RecHit_CorrectedTime_ = -9999999;
  PFECAL_RecHit_IsWeird_ = -9999999;
  PFECAL_RecHit_IsProblematic_ = -9999999;
  PFECAL_RecHit_PoorReco_ = -9999999;
  PFECAL_RecHit_EB_ieta_ = -9999999;
  PFECAL_RecHit_EB_iphi_ = -9999999;
  PFECAL_RecHit_EE_ix_ = -9999999;
  PFECAL_RecHit_EE_iy_ = -9999999;
  PFECAL_RecHit_EE_zside_ = -9999999;

  



	photonMultiplicity_ = -9999999;
  isFourBody_ = -9999999;
  for(size_t j=0; j<2;++j){
	photonPt_[j]           = -9999999;
	photonEta_[j]          = -9999999;
	photonPhi_[j]          = -9999999;
	photonEnergy_[j]       = -9999999;
	photonET_[j]           = -9999999;
	photonTrkIso_[j]       = -9999999;
	photonEcalIso_[j]        = -9999999;
	photonHcalIso_[j]        = -9999999;
	photonCaloIso_[j]        = -9999999;
	photonSSSigmaiEtaiEta_[j]  = -9999999;
	photonSSSigmaiEtaiPhi_[j]  = -9999999;
	photonSSSigmaiPhiiPhi_[j]  = -9999999;
	photonSSSigmaEtaEta_[j]     = -9999999;
	photonSSe1x5_[j]           = -9999999;
	photonSSe2x5_[j]           = -9999999;
	photonSSe3x3_[j]           = -9999999;
	photonSSe5x5_[j]           = -9999999;
	photonSShcalDepth1OverEcal_[j] = -9999999;
	photonSShcalDepth2OverEcal_[j] = -9999999;
	photonSShcalDepth1OverEcalBc_[j] = -9999999;
	photonSShcalDepth2OverEcalBc_[j] = -9999999;
	for(size_t i=0; i<7; ++i){photonSShcalOverEcal_[i][j] = -9999999;}
	for(size_t i=0; i<7; ++i){photonSShcalOverEcalBc_[i][j] = -9999999;}
	photonSSmaxEnergyXtal_[j]  = -9999999;
	photonSSeffSigmaRR_[j]     = -9999999;
	photonSCEnergy_[j]         = -9999999;
	photonSCRawEnergy_[j]      = -9999999;
	photonSCPreShowerEP1_[j]   = -9999999;
	photonSCPreShowerEP2_[j]   = -9999999;
	photonSCEta_[j]            = -9999999;
	photonSCPhi_[j]            = -9999999;
	photonSCEtaWidth_[j]       = -9999999;
	photonSCPhiWidth_[j]       = -9999999;
	photonSCBrem_[j]           = -9999999;
	photonSCR9_[j]             = -9999999;
	photonSCHadTowOverEm_[j]   = -9999999;
  }

  //Resetting the particle flow candidates
  pfCandMultiplicity_ = -9999999; 
  for(size_t j=0; j<2;++j){
    pfCandPt_[j]                = -9999999;   
    pfCandEta_[j]               = -9999999;
    pfCandPhi_[j]               = -9999999;
    pfCandEnergy_[j]            = -9999999;
    pfCandET_[j]                = -9999999;
    pfCandMass_[j]              = -9999999;
    pfCandCharge_[j]            = -9999999;
    pfCandEcalEnergy_[j]        = -9999999;                 
    pfCandRawEcalEnergy_[j]     = -9999999;
    pfCandHcalEnergy_[j]        = -9999999;
    pfCandRawHcalEnergy_[j]     = -9999999;
    pfCandHoEnergy_[j]          = -9999999;
    pfCandRawHoEnergy_[j]       = -9999999;
    pfCandTime_[j]              = -9999999;
    pfCandTrkIso_[j]            = -9999999;
    pfCandEcalIso_[j]           = -9999999;
    pfCandHcalIso_[j]           = -9999999;
    pfCandCaloIso_[j]           = -9999999;       
    pfCandRefPhotonPt_[j]       = -9999999;
    pfCandRefPhotonEta_[j]      = -9999999;
    pfCandRefPhotonPhi_[j]      = -9999999;
    pfCandRefPhotonEnergy_[j]   = -9999999;
    pfCandRefPhotonET_[j]       = -9999999;
    pfCandRefPhotonSigmaIEtaIEta_[j] = -9999999;
    pfCandRefPhotonSigmaIEtaIPhi_[j] = -9999999;
    pfCandRefPhotonSigmaIPhiIPhi_[j] = -9999999;
    pfCandRefPhotonSigmaEtaEta_[j] = -9999999;
    pfCandRefPhotonE1x5_[j]    = -9999999;
    pfCandRefPhotonE2x5_[j]    = -9999999;
    pfCandRefPhotonE3x3_[j]    = -9999999;
    pfCandRefPhotonE5x5_[j]    = -9999999;
    pfCandRefPhotonHcalDepth1OverEcal_[j] = -9999999;
    pfCandRefPhotonHcalDepth2OverEcal_[j] = -9999999;
    pfCandRefPhotonHcalDepth1OverEcalBc_[j] = -9999999;
    pfCandRefPhotonHcalDepth2OverEcalBc_[j] = -9999999;
    pfCandRefPhotonScEnergy_[j] = -9999999;
    pfCandRefPhotonScRawEnergy_[j] = -9999999;
    pfCandRefPhotonScEta_[j]    = -9999999;
    pfCandRefPhotonScPhi_[j]    = -9999999;
    pfCandRefPhotonScEtaWidth_[j] = -9999999;
    pfCandRefPhotonScPhiWidth_[j] = -9999999;
    pfCandRefPhotonScBrem_[j]   = -9999999;
    pfCandRefPhotonR9_[j]       = -9999999;
    pfCandRefPhotonHadTowOverEm_[j] = -9999999;
    pfCandRefPhotonMaxEnergyXtal_[j] = -9999999;
    pfCandRefPhotonEffSigmaRR_[j] = -9999999;
    for(size_t i=0; i<7; ++i){
      pfCandRefPhotonHcalOverEcal_[i][j] = -9999999;
      pfCandRefPhotonHcalOverEcalBc_[i][j] = -9999999;
    }
  }




	DiGammaM_alone_          = -9999999;
	DiGammaEta_alone_        = -9999999;
	DiGammaPhi_alone_        = -9999999;
  DiGammaPt_alone_         = -9999999;
	
	


  Bsmass_recommg_                = -9999999;
  Bshelicity_recommg_            = -9999999;
	Bscoplanarity_recommg_         = -9999999;
  Bspt_recommg_                 = -9999999;
  Bseta_recommg_                = -9999999;
  Bsphi_recommg_                = -9999999;
  Bsmass_recommgg_               = -9999999;
  Bshelicity_recommgg_           = -9999999;
  Bscoplanarity_recommgg_        = -9999999;
  Bspt_recommgg_                = -9999999;
  Bseta_recommgg_               = -9999999;
  Bsphi_recommgg_               = -9999999;
  BmesonType_                   = -9999999;

  
  
	
	electronMultiplicity_   = -9999999;
  pfCandMultiplicity_     = -9999999;
	JpsiGenLxy_             = -9999999;
	JpsiGenPVz_             = -9999999;
	JpsiGenPVy_             = -9999999;
	JpsiGenPVx_             = -9999999;
	JpsiGenPt_              = -9999999;
	JpsiGenLxyOld_          = -9999999;
	JpsiGenLxyOverPt_       = -9999999;
	JpsiGenNumberOfCandidates_ = -9999999;

	for(size_t i=0; i<9;++i){PVcovariance_[i] = -9999999;}


  //Gen Particle information
	GenMotherID_  = -9999999;
	GenB_pt_ = -9999999;
	GenB_eta_ = -9999999;
	GenB_phi_ = -9999999;
	GenB_mass_ = -9999999;
	GenBLxy_ = -9999999;
	GenBct_ = -9999999;
	GenBctErr_ = -9999999;
	GenMuPlus_pt_ = -9999999;
	GenMuPlus_eta_ = -9999999;
	GenMuPlus_phi_ = -9999999;
	GenMuPlus_E_ = -9999999;
	GenMuMinus_pt_ = -9999999;
	GenMuMinus_eta_ = -9999999;
	GenMuMinus_phi_ = -9999999;
	GenMuMinus_E_ = -9999999;
	GenDimuon_mass_ = -9999999;
	GenDimuon_pt_ = -9999999;
	GenNPhotons_ = -9999999;
  for(size_t i=0; i<4;++i){
    GenGamma_pt_[i] = -9999999;
    GenGamma_eta_[i] = -9999999;
    GenGamma_phi_[i] = -9999999;
    GenGamma_E_[i] = -9999999;
    GenGamma_origin_[i] = -9999999;
    GenGamma_motherId_[i] = -9999999;
    GenGamma_dRmu1_[i] = -9999999;
    GenGamma_dRmu2_[i] = -9999999;
  }

  GenHelicity3_ = -9999999;
	GenCoplanarity3_ = -9999999;
	GenCosTheta_l_ = -9999999;
	GenTriBodyMass_ = -9999999;
	GenHelicity4_ = -9999999;
	GenCoplanarity4_ = -9999999;
	GenFourBodyMass_ = -9999999;
	GenDiphoton_mass_ = -9999999;
	GenDiphoton_pt_ = -9999999;
	GenHasJpsi_ = false;
	GenJpsi_mass_ = -9999999;
	GenJpsi_pt_ = -9999999;
	GenHasNeutralMeson_ = -9999999;
	GenNeutralMesonId_ = -9999999;
	GenDecayMode_ = -9999999;
	GenIs3Body_ = false;
	GenIs4Body_ = false;
	GenIsHardProcess_ = false;
	GenIsSignal_ = false;

	//Truth matched 
	Mu1_isMatched_ = -9999999;
	Mu1_genPdgId_ = -9999999;
	Mu1_genMotherPdgId_ = -9999999;
	Mu1_genGMotherPdgId_ = -9999999;
	Mu1_genDeltaR_ = -9999999;
	Mu1_genDeltaPt_ = -9999999;
	Mu2_isMatched_ = -9999999;
	Mu2_genPdgId_ = -9999999;
	Mu2_genMotherPdgId_ = -9999999;
	Mu2_genGMotherPdgId_ = -9999999;
	Mu2_genDeltaR_ = -9999999;
	Mu2_genDeltaPt_ = -9999999;
	DiMuon_isMatched_ = -9999999;
  for (size_t i=0; i<4;++i){
    Photon_isMatched_[i] = -9999999;
    Photon_genPdgId_[i] = -9999999;
    Photon_genMotherPdgId_[i] = -9999999;
    Photon_genGMotherPdgId_[i] = -9999999;
    Photon_genDeltaR_[i] = -9999999;
    Photon_genDeltaPt_[i] = -9999999;
    Photon_truthOrigin_[i] = -9999999;

  }
	NMatchedPhotons_= -9999999;
  Topology_isMatched_ = -9999999;
	Topology_mu1Matched_ = -9999999;
	Topology_mu2Matched_ = -9999999;
	Topology_photon1Matched_ = -9999999;
	Topology_photon2Matched_ = -9999999;
	Topology_decayModeMatched_ = -9999999;
	IsSignalMatched_ = -9999999;
	IsMuonsFromJpsi_ = -9999999;
	IsMuonsDirectFromBs_ = -9999999;
}

void RadiativeRootTree::getDeDx(const double f1, const double f2, const int f3)
{
  dedxTrk_ = f1;
  errdedxTrk_ = f2;
  numdedxTrk_ = f3;
}

void RadiativeRootTree::getVtx(const double aa, const double bb, const double cc, const double dd, const double ee, const double ff,
                                 const double gg, const double hh, const double ii)
{
  BSx_ = aa;
  BSy_ = bb;
  BSz_ = cc;
  PVx_ = dd;
  PVy_ = ee;
  PVz_ = ff;
  PVerrx_ = gg;
  PVerry_ = hh;
  PVerrz_ = ii;
}


void RadiativeRootTree::getAngles(const double aa, const double bb, const double cc, const double dd)
{
  costheta_ = aa;
  phi_ = bb;
  cospsi_ = cc;
  AngleBsDecayLength_ = dd;
}

void RadiativeRootTree::fill()
{
  bmmgTree_->Fill();
}
void RadiativeRootTree::readTree(const std::string filename)
{
  bmmgFile_ = new TFile (filename.c_str(), "READ" );
  bmmgTree_ =  (TTree*) bmmgFile_->Get("BMMGTree");
  setBranchAddresses();
}

void RadiativeRootTree::readTree(std::vector<std::string> filenames){
  TChain * myChain = new TChain("BMMGTree");
  for(std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++){
    myChain->Add( (*it).c_str());
  }
  bmmgTree_ = myChain;
  setBranchAddresses();
}
void RadiativeRootTree::setBranchAddresses(){

  
  bmmgTree_->SetBranchAddress("runNumber", &runNumber_);
  bmmgTree_->SetBranchAddress("eventNumber", &eventNumber_);
  bmmgTree_->SetBranchAddress("lumiSection", &lumiSection_ );
  bmmgTree_->SetBranchAddress("PUinteraction", &PUinteraction_);
  bmmgTree_->SetBranchAddress("PUTrueinteraction", &PUTrueinteraction_);
  bmmgTree_->SetBranchAddress("NVerticesbeforecut",&NVerticesbeforecut_);
  bmmgTree_->SetBranchAddress("NVerticesaftercut",&NVerticesaftercut_);
  bmmgTree_->SetBranchAddress("NTracksPVbeforecut",&NTracksPVbeforecut_);
  bmmgTree_->SetBranchAddress("BSx" , &BSx_ );
  bmmgTree_->SetBranchAddress("BSy" , &BSy_ );
  bmmgTree_->SetBranchAddress("BSz", &BSz_ );
  bmmgTree_->SetBranchAddress("BSdx", &BSdx_ );
  bmmgTree_->SetBranchAddress("BSdy"  , &BSdy_ );
  bmmgTree_->SetBranchAddress("BSdz" , &BSdz_ );
  bmmgTree_->SetBranchAddress("BSdxdz",&BSdxdz_);
  bmmgTree_->SetBranchAddress("BSdydz",&BSdydz_);
  bmmgTree_->SetBranchAddress("BSsigmaZ",&BSsigmaZ_);
  bmmgTree_->SetBranchAddress("BSdsigmaZ",&BSdsigmaZ_);
  bmmgTree_->SetBranchAddress("PVx", &PVx_);
  bmmgTree_->SetBranchAddress("PVy" , &PVy_);
  bmmgTree_->SetBranchAddress("PVz", &PVz_);
  bmmgTree_->SetBranchAddress("PVerrx", &PVerrx_);
  bmmgTree_->SetBranchAddress("PVerry", &PVerry_);
  bmmgTree_->SetBranchAddress("PVerrz", &PVerrz_);
  bmmgTree_->SetBranchAddress("PVrho", &PVrho_);
  bmmgTree_->SetBranchAddress("PVndof", &PVndof_);
  bmmgTree_->SetBranchAddress("PVcovariance", &PVcovariance_);
  bmmgTree_->SetBranchAddress("isPV", &isPV_);
  bmmgTree_->SetBranchAddress("isBS" , &isBS_);
  bmmgTree_->SetBranchAddress("dedxTrk", &dedxTrk_);
  bmmgTree_->SetBranchAddress("errdedxTrk", &errdedxTrk_);
  bmmgTree_->SetBranchAddress("numdedxTrk", &numdedxTrk_);
  bmmgTree_->SetBranchAddress("costheta", &costheta_);
  bmmgTree_->SetBranchAddress("triggerbit_HLT_DoubleMu4_LowMass_Displaced", &triggerbit_HLT_DoubleMu4_LowMass_Displaced_);
  bmmgTree_->SetBranchAddress("triggerbit_HLT_DoubleMu4_LowMass", &triggerbit_HLT_DoubleMu4_LowMass_);
  bmmgTree_->SetBranchAddress("triggerbit_HLT_DoubleMu4_3_Bs", &triggerbit_HLT_DoubleMu4_3_Bs_);
  bmmgTree_->SetBranchAddress("triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG", &triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_);
  bmmgTree_->SetBranchAddress("triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG", &triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_);
  bmmgTree_->SetBranchAddress("triggerbit_HLTDimuon4JpsiDisplaced",&triggerbit_HLTDimuon4JpsiDisplaced_);
  bmmgTree_->SetBranchAddress("triggerbit_HLTDimuon4JpsiNoVertexing",&triggerbit_HLTDimuon4JpsiNoVertexing_);
  bmmgTree_->SetBranchAddress("triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_",&triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_);
  bmmgTree_->SetBranchAddress("mu1Pt_beffit", &mu1Pt_beffit_);
  bmmgTree_->SetBranchAddress("mu1Pz_beffit", &mu1Pz_beffit_);
  bmmgTree_->SetBranchAddress("mu1Eta_beffit", &mu1Eta_beffit_);
  bmmgTree_->SetBranchAddress("mu1Phi_beffit", &mu1Phi_beffit_);
  bmmgTree_->SetBranchAddress("mu1Energy_beffit", &mu1Energy_beffit_);
  bmmgTree_->SetBranchAddress("mu2Pt_beffit", &mu2Pt_beffit_);
  bmmgTree_->SetBranchAddress("mu2Pz_beffit", &mu2Pz_beffit_);
  bmmgTree_->SetBranchAddress("mu2Eta_beffit", &mu2Eta_beffit_);
  bmmgTree_->SetBranchAddress("mu2Phi_beffit", &mu2Phi_beffit_);
  bmmgTree_->SetBranchAddress("mu2Energy_beffit", &mu2Energy_beffit_);
  bmmgTree_->SetBranchAddress("mu1SoftID", &mu1SoftID_);
  bmmgTree_->SetBranchAddress("mu2SoftID", &mu2SoftID_);
  bmmgTree_->SetBranchAddress("MuonPairDR", &MuonPairDR_);
  bmmgTree_->SetBranchAddress("Mu1TrkBSDxy", &Mu1TrkBSDxy_);
  bmmgTree_->SetBranchAddress("Mu1TrkBSDz", &Mu1TrkBSDz_);
  bmmgTree_->SetBranchAddress("Mu2TrkBSDxy", &Mu2TrkBSDxy_);
  bmmgTree_->SetBranchAddress("Mu2TrkBSDz", &Mu2TrkBSDz_);
  bmmgTree_->SetBranchAddress("Mu1PixelHits", &Mu1PixelHits_);
  bmmgTree_->SetBranchAddress("Mu1TrackerHits", &Mu1TrackerHits_);
  bmmgTree_->SetBranchAddress("Mu1isGood", &Mu1isGood_);
  bmmgTree_->SetBranchAddress("Mu1InnerTrkHighQuality", &Mu1InnerTrkHighQuality_);
  bmmgTree_->SetBranchAddress("Mu2PixelHits", &Mu2PixelHits_);
  bmmgTree_->SetBranchAddress("Mu2TrackerHits", &Mu2TrackerHits_);
  bmmgTree_->SetBranchAddress("Mu2isGood", &Mu2isGood_);
  bmmgTree_->SetBranchAddress("Mu2InnerTrkHighQuality", &Mu2InnerTrkHighQuality_);
  bmmgTree_->SetBranchAddress("mu1MVAScore", &mu1MVAScore_);
  bmmgTree_->SetBranchAddress("mu2MVAScore", &mu2MVAScore_);
  bmmgTree_->SetBranchAddress("DiMuonM_beffit", &DiMuonM_beffit_);
  bmmgTree_->SetBranchAddress("DiMuonEta_beffit", &DiMuonEta_beffit_);
  bmmgTree_->SetBranchAddress("DiMuonPhi_beffit", &DiMuonPhi_beffit_);
  bmmgTree_->SetBranchAddress("DiMuonPt_beffit", &DiMuonPt_beffit_);
  bmmgTree_->SetBranchAddress("DiMuon_vtxProb", &DiMuon_vtxProb_);
  bmmgTree_->SetBranchAddress("DiMuon_CosineAlpha", &DiMuon_CosineAlpha_);
  bmmgTree_->SetBranchAddress("DiMuon_DCA", &DiMuon_DCA_);
  bmmgTree_->SetBranchAddress("DiMuon_Chi2pv_KVFvtx", &DiMuon_Chi2pv_KVFvtx_);
  bmmgTree_->SetBranchAddress("DiMuon_Mahalanobis", &DiMuon_Mahalanobis_);
  bmmgTree_->SetBranchAddress("DiMuon_ResonanceType", &DiMuon_ResonanceType_);
  bmmgTree_->SetBranchAddress("DiMuon_Lxy", &DiMuon_Lxy_);
  bmmgTree_->SetBranchAddress("DiMuon_Lxyerr", &DiMuon_Lxyerr_);
  bmmgTree_->SetBranchAddress("DiMuon_LxyOverPt", &DiMuon_LxyOverPt_);
  bmmgTree_->SetBranchAddress("DiMuon_mu1Cat_alone", &DiMuon_mu1Cat_alone_);
  bmmgTree_->SetBranchAddress("DiMuon_mu2Cat_alone", &DiMuon_mu2Cat_alone_);


  bmmgTree_->SetBranchAddress("FourvectorBsMass_mmconvg", &FourvectorBsMass_mmconvg_);
bmmgTree_->SetBranchAddress("FourvectorBsPt_mmconvg", &FourvectorBsPt_mmconvg_);
bmmgTree_->SetBranchAddress("FourvectorBsEta_mmconvg", &FourvectorBsEta_mmconvg_);
bmmgTree_->SetBranchAddress("FourvectorBsPhi_mmconvg", &FourvectorBsPhi_mmconvg_);

bmmgTree_->SetBranchAddress("VertexfitBsMass_mmconvg", &VertexfitBsMass_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsPt_mmconvg", &VertexfitBsPt_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsEta_mmconvg", &VertexfitBsEta_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsPhi_mmconvg", &VertexfitBsPhi_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsPz_mmconvg", &VertexfitBsPz_mmconvg_);

bmmgTree_->SetBranchAddress("VertexfitBsSVx_mmconvg", &VertexfitBsSVx_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsSVy_mmconvg", &VertexfitBsSVy_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsSVz_mmconvg", &VertexfitBsSVz_mmconvg_);

bmmgTree_->SetBranchAddress("FourvectorConvGammaPt_mmconvg", &FourvectorConvGammaPt_mmconvg_);
bmmgTree_->SetBranchAddress("FourvectorConvGammaEta_mmconvg", &FourvectorConvGammaEta_mmconvg_);
bmmgTree_->SetBranchAddress("FourvectorConvGammaPhi_mmconvg", &FourvectorConvGammaPhi_mmconvg_);


bmmgTree_->SetBranchAddress("VertexfitPVxRefitCosTheta_mmconvg", &VertexfitPVxRefitCosTheta_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitCosTheta_mmconvg", &VertexfitPVyRefitCosTheta_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitCosTheta_mmconvg", &VertexfitPVzRefitCosTheta_mmconvg_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitClosestZ_mmconvg", &VertexfitPVxRefitClosestZ_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitClosestZ_mmconvg", &VertexfitPVyRefitClosestZ_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitClosestZ_mmconvg", &VertexfitPVzRefitClosestZ_mmconvg_);

bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVClosestZ_mmconvg", &VertexfitBsCt3DPVClosestZ_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVClosestZ_mmconvg", &VertexfitBsCt2DPVClosestZ_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVCosTheta_mmconvg", &VertexfitBsCt3DPVCosTheta_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVCosTheta_mmconvg", &VertexfitBsCt2DPVCosTheta_mmconvg_);

bmmgTree_->SetBranchAddress("VertexfitBsCtErr2D_mmconvg", &VertexfitBsCtErr2D_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsCtErr3D_mmconvg", &VertexfitBsCtErr3D_mmconvg_);

bmmgTree_->SetBranchAddress("VertexfitBsDist3D_mmconvg", &VertexfitBsDist3D_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist3DErr_mmconvg", &VertexfitBsDist3DErr_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2D_mmconvg", &VertexfitBsDist2D_mmconvg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2DErr_mmconvg", &VertexfitBsDist2DErr_mmconvg_);

bmmgTree_->SetBranchAddress("BsVtxProb_mmconvg", &BsVtxProb_mmconvg_);
bmmgTree_->SetBranchAddress("BsCt2D_mmconvg", &BsCt2D_mmconvg_);
bmmgTree_->SetBranchAddress("BsCt3D_mmconvg", &BsCt3D_mmconvg_);


bmmgTree_->SetBranchAddress("DeltaRPhoton1DiMuon_mmconvg", &DeltaRPhoton1DiMuon_mmconvg_);
bmmgTree_->SetBranchAddress("DeltaRPhoton2DiMuon_mmconvg", &DeltaRPhoton2DiMuon_mmconvg_);

bmmgTree_->SetBranchAddress("BsHelicity_mmconvg", &BsHelicity_mmconvg_);
bmmgTree_->SetBranchAddress("BsCoplanarity_mmconvg", &BsCoplanarity_mmconvg_);

bmmgTree_->SetBranchAddress("FourvectorBsMass_mmrecog", &FourvectorBsMass_mmrecog_);
bmmgTree_->SetBranchAddress("FourvectorBsPt_mmrecog", &FourvectorBsPt_mmrecog_);
bmmgTree_->SetBranchAddress("FourvectorBsEta_mmrecog", &FourvectorBsEta_mmrecog_);
bmmgTree_->SetBranchAddress("FourvectorBsPhi_mmrecog", &FourvectorBsPhi_mmrecog_);

bmmgTree_->SetBranchAddress("VertexfitBsMass_mmrecog", &VertexfitBsMass_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsPt_mmrecog", &VertexfitBsPt_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsEta_mmrecog", &VertexfitBsEta_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsPhi_mmrecog", &VertexfitBsPhi_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsPz_mmrecog", &VertexfitBsPz_mmrecog_);

bmmgTree_->SetBranchAddress("VertexfitBsSVx_mmrecog", &VertexfitBsSVx_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsSVy_mmrecog", &VertexfitBsSVy_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsSVz_mmrecog", &VertexfitBsSVz_mmrecog_);

bmmgTree_->SetBranchAddress("FourvectorGammaPt_mmrecog", &FourvectorGammaPt_mmrecog_);
bmmgTree_->SetBranchAddress("FourvectorGammaEta_mmrecog", &FourvectorGammaEta_mmrecog_);
bmmgTree_->SetBranchAddress("FourvectorGammaPhi_mmrecog", &FourvectorGammaPhi_mmrecog_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitCosTheta_mmrecog", &VertexfitPVxRefitCosTheta_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitCosTheta_mmrecog", &VertexfitPVyRefitCosTheta_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitCosTheta_mmrecog", &VertexfitPVzRefitCosTheta_mmrecog_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitClosestZ_mmrecog", &VertexfitPVxRefitClosestZ_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitClosestZ_mmrecog", &VertexfitPVyRefitClosestZ_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitClosestZ_mmrecog", &VertexfitPVzRefitClosestZ_mmrecog_);

bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVClosestZ_mmrecog", &VertexfitBsCt3DPVClosestZ_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVClosestZ_mmrecog", &VertexfitBsCt2DPVClosestZ_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVCosTheta_mmrecog", &VertexfitBsCt3DPVCosTheta_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVCosTheta_mmrecog", &VertexfitBsCt2DPVCosTheta_mmrecog_);

bmmgTree_->SetBranchAddress("VertexfitBsCtErr2D_mmrecog", &VertexfitBsCtErr2D_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsCtErr3D_mmrecog", &VertexfitBsCtErr3D_mmrecog_);

bmmgTree_->SetBranchAddress("VertexfitBsDist3D_mmrecog", &VertexfitBsDist3D_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsDist3DErr_mmrecog", &VertexfitBsDist3DErr_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2D_mmrecog", &VertexfitBsDist2D_mmrecog_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2DErr_mmrecog", &VertexfitBsDist2DErr_mmrecog_);

bmmgTree_->SetBranchAddress("BsVtxProb_mmrecog", &BsVtxProb_mmrecog_);
bmmgTree_->SetBranchAddress("BsCt2D_mmrecog", &BsCt2D_mmrecog_);
bmmgTree_->SetBranchAddress("BsCt3D_mmrecog", &BsCt3D_mmrecog_);

bmmgTree_->SetBranchAddress("DeltaRPhoton1DiMuon_mmrecog", &DeltaRPhoton1DiMuon_mmrecog_);
bmmgTree_->SetBranchAddress("DeltaRPhoton2DiMuon_mmrecog", &DeltaRPhoton2DiMuon_mmrecog_);

bmmgTree_->SetBranchAddress("BsHelicity_mmrecog", &BsHelicity_mmrecog_);
bmmgTree_->SetBranchAddress("BsCoplanarity_mmrecog", &BsCoplanarity_mmrecog_);

bmmgTree_->SetBranchAddress("FourvectorBsMass_mmconvgg", &FourvectorBsMass_mmconvgg_);
bmmgTree_->SetBranchAddress("FourvectorBsPt_mmconvgg", &FourvectorBsPt_mmconvgg_);
bmmgTree_->SetBranchAddress("FourvectorBsEta_mmconvgg", &FourvectorBsEta_mmconvgg_);
bmmgTree_->SetBranchAddress("FourvectorBsPhi_mmconvgg", &FourvectorBsPhi_mmconvgg_);

bmmgTree_->SetBranchAddress("VertexfitBsMass_mmconvgg", &VertexfitBsMass_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsPt_mmconvgg", &VertexfitBsPt_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsEta_mmconvgg", &VertexfitBsEta_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsPhi_mmconvgg", &VertexfitBsPhi_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsPz_mmconvgg", &VertexfitBsPz_mmconvgg_);

bmmgTree_->SetBranchAddress("VertexfitBsSVx_mmconvgg", &VertexfitBsSVx_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsSVy_mmconvgg", &VertexfitBsSVy_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsSVz_mmconvgg", &VertexfitBsSVz_mmconvgg_);

bmmgTree_->SetBranchAddress("FourvectorConvGammaPt_mmconvgg", &FourvectorConvGammaPt_mmconvgg_);
bmmgTree_->SetBranchAddress("FourvectorConvGammaEta_mmconvgg", &FourvectorConvGammaEta_mmconvgg_);
bmmgTree_->SetBranchAddress("FourvectorConvGammaPhi_mmconvgg", &FourvectorConvGammaPhi_mmconvgg_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitCosTheta_mmconvgg", &VertexfitPVxRefitCosTheta_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitCosTheta_mmconvgg", &VertexfitPVyRefitCosTheta_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitCosTheta_mmconvgg", &VertexfitPVzRefitCosTheta_mmconvgg_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitClosestZ_mmconvgg", &VertexfitPVxRefitClosestZ_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitClosestZ_mmconvgg", &VertexfitPVyRefitClosestZ_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitClosestZ_mmconvgg", &VertexfitPVzRefitClosestZ_mmconvgg_);

bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVClosestZ_mmconvgg", &VertexfitBsCt3DPVClosestZ_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVClosestZ_mmconvgg", &VertexfitBsCt2DPVClosestZ_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVCosTheta_mmconvgg", &VertexfitBsCt3DPVCosTheta_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVCosTheta_mmconvgg", &VertexfitBsCt2DPVCosTheta_mmconvgg_);

bmmgTree_->SetBranchAddress("VertexfitBsCtErr2D_mmconvgg", &VertexfitBsCtErr2D_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsCtErr3D_mmconvgg", &VertexfitBsCtErr3D_mmconvgg_);

bmmgTree_->SetBranchAddress("VertexfitBsDist3D_mmconvgg", &VertexfitBsDist3D_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist3DErr_mmconvgg", &VertexfitBsDist3DErr_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2D_mmconvgg", &VertexfitBsDist2D_mmconvgg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2DErr_mmconvgg", &VertexfitBsDist2DErr_mmconvgg_);

bmmgTree_->SetBranchAddress("BsVtxProb_mmconvgg", &BsVtxProb_mmconvgg_);
bmmgTree_->SetBranchAddress("BsCt2D_mmconvgg", &BsCt2D_mmconvgg_);
bmmgTree_->SetBranchAddress("BsCt3D_mmconvgg", &BsCt3D_mmconvgg_);

bmmgTree_->SetBranchAddress("DeltaRPhoton1DiMuon_mmconvgg", &DeltaRPhoton1DiMuon_mmconvgg_);
bmmgTree_->SetBranchAddress("DeltaRPhoton2DiMuon_mmconvgg", &DeltaRPhoton2DiMuon_mmconvgg_);

bmmgTree_->SetBranchAddress("BsHelicity_mmconvgg", &BsHelicity_mmconvgg_);
bmmgTree_->SetBranchAddress("BsCoplanarity_mmconvgg", &BsCoplanarity_mmconvgg_);

bmmgTree_->SetBranchAddress("FourvectorBsMass_mmrecogg", &FourvectorBsMass_mmrecogg_);
bmmgTree_->SetBranchAddress("FourvectorBsPt_mmrecogg", &FourvectorBsPt_mmrecogg_);
bmmgTree_->SetBranchAddress("FourvectorBsEta_mmrecogg", &FourvectorBsEta_mmrecogg_);
bmmgTree_->SetBranchAddress("FourvectorBsPhi_mmrecogg", &FourvectorBsPhi_mmrecogg_);

bmmgTree_->SetBranchAddress("VertexfitBsMass_mmrecogg", &VertexfitBsMass_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsPt_mmrecogg", &VertexfitBsPt_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsEta_mmrecogg", &VertexfitBsEta_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsPhi_mmrecogg", &VertexfitBsPhi_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsPz_mmrecogg", &VertexfitBsPz_mmrecogg_);

bmmgTree_->SetBranchAddress("VertexfitBsSVx_mmrecogg", &VertexfitBsSVx_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsSVy_mmrecogg", &VertexfitBsSVy_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsSVz_mmrecogg", &VertexfitBsSVz_mmrecogg_);

bmmgTree_->SetBranchAddress("FourvectorGammaPt_mmrecogg", &FourvectorGammaPt_mmrecogg_);
bmmgTree_->SetBranchAddress("FourvectorGammaEta_mmrecogg", &FourvectorGammaEta_mmrecogg_);
bmmgTree_->SetBranchAddress("FourvectorGammaPhi_mmrecogg", &FourvectorGammaPhi_mmrecogg_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitCosTheta_mmrecogg", &VertexfitPVxRefitCosTheta_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitCosTheta_mmrecogg", &VertexfitPVyRefitCosTheta_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitCosTheta_mmrecogg", &VertexfitPVzRefitCosTheta_mmrecogg_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitClosestZ_mmrecogg", &VertexfitPVxRefitClosestZ_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitClosestZ_mmrecogg", &VertexfitPVyRefitClosestZ_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitClosestZ_mmrecogg", &VertexfitPVzRefitClosestZ_mmrecogg_);

bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVClosestZ_mmrecogg", &VertexfitBsCt3DPVClosestZ_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVClosestZ_mmrecogg", &VertexfitBsCt2DPVClosestZ_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVCosTheta_mmrecogg", &VertexfitBsCt3DPVCosTheta_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVCosTheta_mmrecogg", &VertexfitBsCt2DPVCosTheta_mmrecogg_);

bmmgTree_->SetBranchAddress("VertexfitBsCtErr2D_mmrecogg", &VertexfitBsCtErr2D_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsCtErr3D_mmrecogg", &VertexfitBsCtErr3D_mmrecogg_);

bmmgTree_->SetBranchAddress("VertexfitBsDist3D_mmrecogg", &VertexfitBsDist3D_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist3DErr_mmrecogg", &VertexfitBsDist3DErr_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2D_mmrecogg", &VertexfitBsDist2D_mmrecogg_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2DErr_mmrecogg", &VertexfitBsDist2DErr_mmrecogg_);

bmmgTree_->SetBranchAddress("BsVtxProb_mmrecogg", &BsVtxProb_mmrecogg_);
bmmgTree_->SetBranchAddress("BsCt2D_mmrecogg", &BsCt2D_mmrecogg_);
bmmgTree_->SetBranchAddress("BsCt3D_mmrecogg", &BsCt3D_mmrecogg_);

bmmgTree_->SetBranchAddress("DeltaRPhoton1DiMuon_mmrecogg", &DeltaRPhoton1DiMuon_mmrecogg_);
bmmgTree_->SetBranchAddress("DeltaRPhoton2DiMuon_mmrecogg", &DeltaRPhoton2DiMuon_mmrecogg_);

bmmgTree_->SetBranchAddress("BsHelicity_mmrecogg", &BsHelicity_mmrecogg_);
bmmgTree_->SetBranchAddress("BsCoplanarity_mmrecogg", &BsCoplanarity_mmrecogg_);

// mu mu k k
bmmgTree_->SetBranchAddress("FourvectorBsMass_mmkk", &FourvectorBsMass_mmkk_);
bmmgTree_->SetBranchAddress("FourvectorBsPt_mmkk", &FourvectorBsPt_mmkk_);
bmmgTree_->SetBranchAddress("FourvectorBsEta_mmkk", &FourvectorBsEta_mmkk_);
bmmgTree_->SetBranchAddress("FourvectorBsPhi_mmkk", &FourvectorBsPhi_mmkk_);

bmmgTree_->SetBranchAddress("VertexfitBsMass_mmkk", &VertexfitBsMass_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsPt_mmkk", &VertexfitBsPt_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsEta_mmkk", &VertexfitBsEta_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsPhi_mmkk", &VertexfitBsPhi_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsPz_mmkk", &VertexfitBsPz_mmkk_);

bmmgTree_->SetBranchAddress("VertexfitBsSVx_mmkk", &VertexfitBsSVx_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsSVy_mmkk", &VertexfitBsSVy_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsSVz_mmkk", &VertexfitBsSVz_mmkk_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitCosTheta_mmkk", &VertexfitPVxRefitCosTheta_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitCosTheta_mmkk", &VertexfitPVyRefitCosTheta_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitCosTheta_mmkk", &VertexfitPVzRefitCosTheta_mmkk_);

bmmgTree_->SetBranchAddress("VertexfitPVxRefitClosestZ_mmkk", &VertexfitPVxRefitClosestZ_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitPVyRefitClosestZ_mmkk", &VertexfitPVyRefitClosestZ_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitPVzRefitClosestZ_mmkk", &VertexfitPVzRefitClosestZ_mmkk_);

bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVClosestZ_mmkk", &VertexfitBsCt3DPVClosestZ_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVClosestZ_mmkk", &VertexfitBsCt2DPVClosestZ_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsCt3DPVCosTheta_mmkk", &VertexfitBsCt3DPVCosTheta_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsCt2DPVCosTheta_mmkk", &VertexfitBsCt2DPVCosTheta_mmkk_);

bmmgTree_->SetBranchAddress("VertexfitBsCtErr2D_mmkk", &VertexfitBsCtErr2D_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsCtErr3D_mmkk", &VertexfitBsCtErr3D_mmkk_);

bmmgTree_->SetBranchAddress("VertexfitBsDist3D_mmkk", &VertexfitBsDist3D_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsDist3DErr_mmkk", &VertexfitBsDist3DErr_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2D_mmkk", &VertexfitBsDist2D_mmkk_);
bmmgTree_->SetBranchAddress("VertexfitBsDist2DErr_mmkk", &VertexfitBsDist2DErr_mmkk_);

bmmgTree_->SetBranchAddress("BsVtxProb_mmkk", &BsVtxProb_mmkk_);
bmmgTree_->SetBranchAddress("BsCt2D_mmkk", &BsCt2D_mmkk_);
bmmgTree_->SetBranchAddress("BsCt3D_mmkk", &BsCt3D_mmkk_);

bmmgTree_->SetBranchAddress("DeltaRKaon1DiMuon_mmkk", &DeltaRKaon1DiMuon_mmkk_);
bmmgTree_->SetBranchAddress("DeltaRKaon2DiMuon_mmkk", &DeltaRKaon2DiMuon_mmkk_);

bmmgTree_->SetBranchAddress("BsHelicity_mmkk", &BsHelicity_mmkk_);
bmmgTree_->SetBranchAddress("BsCoplanarity_mmkk", &BsCoplanarity_mmkk_);


bmmgTree_->SetBranchAddress("DiMuonMass_Jpsi", &DiMuonMass_Jpsi_);
bmmgTree_->SetBranchAddress("DiMuonEta_Jpsi", &DiMuonEta_Jpsi_);
bmmgTree_->SetBranchAddress("DiMuonPhi_Jpsi", &DiMuonPhi_Jpsi_);
bmmgTree_->SetBranchAddress("DiMuonPt_Jpsi", &DiMuonPt_Jpsi_);

bmmgTree_->SetBranchAddress("DiMuonMass_Phi", &DiMuonMass_Phi_);
bmmgTree_->SetBranchAddress("DiMuonEta_Phi", &DiMuonEta_Phi_);
bmmgTree_->SetBranchAddress("DiMuonPhi_Phi", &DiMuonPhi_Phi_);
bmmgTree_->SetBranchAddress("DiMuonPt_Phi", &DiMuonPt_Phi_);

bmmgTree_->SetBranchAddress("DiMuonMass_Kstar0", &DiMuonMass_Kstar0_);
bmmgTree_->SetBranchAddress("DiMuonEta_Kstar0", &DiMuonEta_Kstar0_);
bmmgTree_->SetBranchAddress("DiMuonPhi_Kstar0", &DiMuonPhi_Kstar0_);
bmmgTree_->SetBranchAddress("DiMuonPt_Kstar0", &DiMuonPt_Kstar0_);

bmmgTree_->SetBranchAddress("DiMuonMass_NoBound", &DiMuonMass_NoBound_);
bmmgTree_->SetBranchAddress("DiMuonEta_NoBound", &DiMuonEta_NoBound_);
bmmgTree_->SetBranchAddress("DiMuonPhi_NoBound", &DiMuonPhi_NoBound_);
bmmgTree_->SetBranchAddress("DiMuonPt_NoBound", &DiMuonPt_NoBound_);


  bmmgTree_->SetBranchAddress("BsM_beffit", &BsM_beffit_);
  bmmgTree_->SetBranchAddress("BsEta_beffit", &BsEta_beffit_);
  bmmgTree_->SetBranchAddress("BsPhi_beffit", &BsPhi_beffit_);
  bmmgTree_->SetBranchAddress("BsPt_beffit", &BsPt_beffit_);
  bmmgTree_->SetBranchAddress("HadronMass_fromVertexFitConPhoton", &HadronMass_fromVertexFitConPhoton_);
  bmmgTree_->SetBranchAddress("HadronMass_fromVertexFitRecoPhoton", &HadronMass_fromVertexFitRecoPhoton_);
  bmmgTree_->SetBranchAddress("Bs_vtxProb", &Bs_vtxProb_);
  bmmgTree_->SetBranchAddress("BsCt3D", &BsCt3D_);
  bmmgTree_->SetBranchAddress("BsCt2D", &BsCt2D_);
  bmmgTree_->SetBranchAddress("BsCt2DBS", &BsCt2DBS_);
  bmmgTree_->SetBranchAddress("BdCt2DBS", &BdCt2DBS_);
  bmmgTree_->SetBranchAddress("BsCtMPV", &BsCtMPV_);
  bmmgTree_->SetBranchAddress("BsCt3Drefit", &BsCt3Drefit_);
  bmmgTree_->SetBranchAddress("BsCt2Drefit", &BsCt2Drefit_);
  bmmgTree_->SetBranchAddress("BsCtMPVrefit", &BsCtMPVrefit_);
  bmmgTree_->SetBranchAddress("BsCtErr", &BsCtErr_);
  bmmgTree_->SetBranchAddress("BsCtErr3D", &BsCtErr3D_);
  bmmgTree_->SetBranchAddress("BsCtErr2D", &BsCtErr2D_);
  bmmgTree_->SetBranchAddress("BsCtErr2DBS", &BsCtErr2DBS_);
  bmmgTree_->SetBranchAddress("BsCtErr2DClosestZ", &BsCtErr2DClosestZ_);
  bmmgTree_->SetBranchAddress("BdCtErr2DBS", &BdCtErr2DBS_);
  bmmgTree_->SetBranchAddress("BsCtErr2D2", &BsCtErr2D2_);
  bmmgTree_->SetBranchAddress("BsCtErrMPV", &BsCtErrMPV_);
  bmmgTree_->SetBranchAddress("BsCtErr3Drefit", &BsCtErr3Drefit_);
  bmmgTree_->SetBranchAddress("BsCtErr2Drefit", &BsCtErr2Drefit_);
  bmmgTree_->SetBranchAddress("BsCtErrMPVrefit", &BsCtErrMPVrefit_);
  bmmgTree_->SetBranchAddress("BsCtErr2DBSOld", &BsCtErr2DBSOld_);
  bmmgTree_->SetBranchAddress("BsCtErr2DClosestZOld", &BsCtErr2DClosestZOld_);
  bmmgTree_->SetBranchAddress("BsCt2DPVClosestZOld", &BsCt2DPVClosestZOld_);
  bmmgTree_->SetBranchAddress("BsCtErr2DOld", &BsCtErr2DOld_);
  bmmgTree_->SetBranchAddress("BsCt2DOld", &BsCt2DOld_);
  bmmgTree_->SetBranchAddress("vertexTypeFlag_", &vertexTypeFlag_ );

  bmmgTree_->SetBranchAddress("PFECal_SC_Eta", &PFECal_SC_Eta_);
  bmmgTree_->SetBranchAddress("PFECal_SC_Phi", &PFECal_SC_Phi_);
  bmmgTree_->SetBranchAddress("PFECal_SC_EtaWidth", &PFECal_SC_EtaWidth_);
  bmmgTree_->SetBranchAddress("PFECal_SC_PhiWidth", &PFECal_SC_PhiWidth_);
  bmmgTree_->SetBranchAddress("PFECal_SC_ClusterEnergy", &PFECal_SC_ClusterEnergy_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_RawId", &PFECAL_RecHit_RawId_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_Subdet", &PFECAL_RecHit_Subdet_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_Energy", &PFECAL_RecHit_Energy_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_Time", &PFECAL_RecHit_Time_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_X", &PFECAL_RecHit_X_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_Y", &PFECAL_RecHit_Y_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_Z", &PFECAL_RecHit_Z_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_PVDistance", &PFECAL_RecHit_PVDistance_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_TOF", &PFECAL_RecHit_TOF_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_CorrectedTime", &PFECAL_RecHit_CorrectedTime_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_IsWeird", &PFECAL_RecHit_IsWeird_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_IsProblematic", &PFECAL_RecHit_IsProblematic_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_PoorReco", &PFECAL_RecHit_PoorReco_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_EB_ieta", &PFECAL_RecHit_EB_ieta_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_EB_iphi", &PFECAL_RecHit_EB_iphi_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_EE_ix", &PFECAL_RecHit_EE_ix_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_EE_iy", &PFECAL_RecHit_EE_iy_);
  bmmgTree_->SetBranchAddress("PFECAL_RecHit_EE_zside", &PFECAL_RecHit_EE_zside_);

  bmmgTree_->SetBranchAddress("photonMultiplicity", &photonMultiplicity_ );
  bmmgTree_->SetBranchAddress("isFourBody", &isFourBody_ );
  bmmgTree_->SetBranchAddress("photonPt", &photonPt_ );
  bmmgTree_->SetBranchAddress("photonEta", &photonEta_ );
  bmmgTree_->SetBranchAddress("photonPhi", &photonPhi_ );
  bmmgTree_->SetBranchAddress("photonEnergy", &photonEnergy_ );
  bmmgTree_->SetBranchAddress("photonET", &photonET_ );
  bmmgTree_->SetBranchAddress("photonTrkIso", &photonTrkIso_);
  bmmgTree_->SetBranchAddress("photonEcalIso", &photonEcalIso_);
  bmmgTree_->SetBranchAddress("photonHcalIso", &photonHcalIso_);
  bmmgTree_->SetBranchAddress("photonCaloIso", &photonCaloIso_);
  bmmgTree_->SetBranchAddress("photonSSSigmaiEtaiEta", &photonSSSigmaiEtaiEta_);
  bmmgTree_->SetBranchAddress("photonSSSigmaiEtaiPhi", &photonSSSigmaiEtaiPhi_);
  bmmgTree_->SetBranchAddress("photonSSSigmaiPhiiPhi", &photonSSSigmaiPhiiPhi_);
  bmmgTree_->SetBranchAddress("photonSSSigmaEtaEta", &photonSSSigmaEtaEta_);
  bmmgTree_->SetBranchAddress("photonSSe1x5", &photonSSe1x5_);
  bmmgTree_->SetBranchAddress("photonSSe2x5", &photonSSe2x5_);
  bmmgTree_->SetBranchAddress("photonSSe3x3", &photonSSe3x3_);
  bmmgTree_->SetBranchAddress("photonSSe5x5", &photonSSe5x5_);
  bmmgTree_->SetBranchAddress("photonSShcalDepth1OverEcal", &photonSShcalDepth1OverEcal_);
  bmmgTree_->SetBranchAddress("photonSShcalDepth2OverEcal", &photonSShcalDepth2OverEcal_);
  bmmgTree_->SetBranchAddress("photonSShcalDepth1OverEcalBc", &photonSShcalDepth1OverEcalBc_);
  bmmgTree_->SetBranchAddress("photonSShcalDepth2OverEcalBc", &photonSShcalDepth2OverEcalBc_);
  bmmgTree_->SetBranchAddress("photonSShcalOverEcal", &photonSShcalOverEcal_);
  bmmgTree_->SetBranchAddress("photonSShcalOverEcalBc", &photonSShcalOverEcalBc_);
  bmmgTree_->SetBranchAddress("photonSSmaxEnergyXtal", &photonSSmaxEnergyXtal_);
  bmmgTree_->SetBranchAddress("photonSSeffSigmaRR", &photonSSeffSigmaRR_);
  bmmgTree_->SetBranchAddress("photonSCEnergy", &photonSCEnergy_);
  bmmgTree_->SetBranchAddress("photonSCRawEnergy", &photonSCRawEnergy_);
  bmmgTree_->SetBranchAddress("photonSCPreShowerEP1", &photonSCPreShowerEP1_);
  bmmgTree_->SetBranchAddress("photonSCPreShowerEP2", &photonSCPreShowerEP2_);
  bmmgTree_->SetBranchAddress("photonSCEta", &photonSCEta_);
  bmmgTree_->SetBranchAddress("photonSCPhi", &photonSCPhi_);
  bmmgTree_->SetBranchAddress("photonSCEtaWidth", &photonSCEtaWidth_);
  bmmgTree_->SetBranchAddress("photonSCPhiWidth", &photonSCPhiWidth_);
  bmmgTree_->SetBranchAddress("photonSCBrem", &photonSCBrem_);
  bmmgTree_->SetBranchAddress("photonSCR9", &photonSCR9_);
  bmmgTree_->SetBranchAddress("photonSCHadTowOverEm", &photonSCHadTowOverEm_);
  
  bmmgTree_->SetBranchAddress("pfCandMultiplicity", &pfCandMultiplicity_ );
  bmmgTree_->SetBranchAddress("pfCandPt", pfCandPt_ );
  bmmgTree_->SetBranchAddress("pfCandEta", pfCandEta_ );
  bmmgTree_->SetBranchAddress("pfCandPhi", pfCandPhi_ );
  bmmgTree_->SetBranchAddress("pfCandEnergy", pfCandEnergy_ );    
  bmmgTree_->SetBranchAddress("pfCandET", pfCandET_ );
  bmmgTree_->SetBranchAddress("pfCandMass", pfCandMass_ );
  bmmgTree_->SetBranchAddress("pfCandCharge", pfCandCharge_ );
  bmmgTree_->SetBranchAddress("pfCandEcalEnergy", pfCandEcalEnergy_ );
  bmmgTree_->SetBranchAddress("pfCandRawEcalEnergy", pfCandRawEcalEnergy_ );
  bmmgTree_->SetBranchAddress("pfCandHcalEnergy", pfCandHcalEnergy_ );  
  bmmgTree_->SetBranchAddress("pfCandRawHcalEnergy", pfCandRawHcalEnergy_ );
  bmmgTree_->SetBranchAddress("pfCandHoEnergy", pfCandHoEnergy_ );
  bmmgTree_->SetBranchAddress("pfCandRawHoEnergy", pfCandRawHoEnergy_ );
  bmmgTree_->SetBranchAddress("pfCandTime", pfCandTime_ );
  bmmgTree_->SetBranchAddress("pfCandTrkIso", pfCandTrkIso_ );
  bmmgTree_->SetBranchAddress("pfCandEcalIso", pfCandEcalIso_ );  
  bmmgTree_->SetBranchAddress("pfCandHcalIso", pfCandHcalIso_ );
  bmmgTree_->SetBranchAddress("pfCandCaloIso", pfCandCaloIso_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonPt", pfCandRefPhotonPt_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonEta", pfCandRefPhotonEta_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonPhi", pfCandRefPhotonPhi_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonEnergy", pfCandRefPhotonEnergy_ );  
  bmmgTree_->SetBranchAddress("pfCandRefPhotonET", pfCandRefPhotonET_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonSigmaIEtaIEta", pfCandRefPhotonSigmaIEtaIEta_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonSigmaIEtaIPhi", pfCandRefPhotonSigmaIEtaIPhi_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonSigmaIPhiIPhi", pfCandRefPhotonSigmaIPhiIPhi_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonSigmaEtaEta", pfCandRefPhotonSigmaEtaEta_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonE1x5", pfCandRefPhotonE1x5_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonE2x5", pfCandRefPhotonE2x5_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonE3x3", pfCandRefPhotonE3x3_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonE5x5", pfCandRefPhotonE5x5_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonHcalDepth1OverEcal", pfCandRefPhotonHcalDepth1OverEcal_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonHcalDepth2OverEcal", pfCandRefPhotonHcalDepth2OverEcal_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonHcalDepth1OverEcalBc", pfCandRefPhotonHcalDepth1OverEcalBc_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonHcalDepth2OverEcalBc", pfCandRefPhotonHcalDepth2OverEcalBc_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonScEnergy", pfCandRefPhotonScEnergy_ );  
  bmmgTree_->SetBranchAddress("pfCandRefPhotonScRawEnergy", pfCandRefPhotonScRawEnergy_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonScEta", pfCandRefPhotonScEta_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonScPhi", pfCandRefPhotonScPhi_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonScEtaWidth", pfCandRefPhotonScEtaWidth_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonScPhiWidth", pfCandRefPhotonScPhiWidth_ );  
  bmmgTree_->SetBranchAddress("pfCandRefPhotonScBrem", pfCandRefPhotonScBrem_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonR9", pfCandRefPhotonR9_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonHadTowOverEm", pfCandRefPhotonHadTowOverEm_ );  
  bmmgTree_->SetBranchAddress("pfCandRefPhotonMaxEnergyXtal", pfCandRefPhotonMaxEnergyXtal_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonEffSigmaRR", pfCandRefPhotonEffSigmaRR_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonHcalOverEcal", pfCandRefPhotonHcalOverEcal_ );
  bmmgTree_->SetBranchAddress("pfCandRefPhotonHcalOverEcalBc", pfCandRefPhotonHcalOverEcalBc_ );  



  bmmgTree_->SetBranchAddress("DiGammaM_alone",     &DiGammaM_alone_);
  bmmgTree_->SetBranchAddress("DiGammaEta_alone",   &DiGammaEta_alone_);
  bmmgTree_->SetBranchAddress("DiGammaPhi_alone",   &DiGammaPhi_alone_);
  bmmgTree_->SetBranchAddress("DiGammaPt_alone",    &DiGammaPt_alone_);
  

  bmmgTree_->SetBranchAddress("Bsmass_recommg",  &Bsmass_recommg_);
  bmmgTree_->SetBranchAddress("Bshelicity_recommg",  &Bshelicity_recommg_);
  bmmgTree_->SetBranchAddress("Bscoplanarity_recommg",  &Bscoplanarity_recommg_);
  bmmgTree_->SetBranchAddress("Bspt_recommg",    &Bspt_recommg_);
  bmmgTree_->SetBranchAddress("Bseta_recommg",   &Bseta_recommg_);
  bmmgTree_->SetBranchAddress("Bsphi_recommg",   &Bsphi_recommg_);
  bmmgTree_->SetBranchAddress("Bsmass_recommgg",  &Bsmass_recommgg_);
  bmmgTree_->SetBranchAddress("Bshelicity_recommgg",  &Bshelicity_recommgg_);
  bmmgTree_->SetBranchAddress("Bscoplanarity_recommgg",  &Bscoplanarity_recommgg_);
  bmmgTree_->SetBranchAddress("Bspt_recommgg",    &Bspt_recommgg_);
  bmmgTree_->SetBranchAddress("Bseta_recommgg",   &Bseta_recommgg_);
  bmmgTree_->SetBranchAddress("Bsphi_recommgg",   &Bsphi_recommgg_);
  bmmgTree_->SetBranchAddress("BmesonType", &BmesonType_);

 

  bmmgTree_->SetBranchAddress("electronMultiplicity", &electronMultiplicity_);
  bmmgTree_->SetBranchAddress("JpsiGenPVx", &JpsiGenPVx_);
  bmmgTree_->SetBranchAddress("JpsiGenPVy", &JpsiGenPVy_);
  bmmgTree_->SetBranchAddress("JpsiGenPVz", &JpsiGenPVz_);
  bmmgTree_->SetBranchAddress("JpsiGenLxy", &JpsiGenLxy_);
  bmmgTree_->SetBranchAddress("JpsiGenPt", &JpsiGenPt_);
  bmmgTree_->SetBranchAddress("JpsiGenLxyOld", &JpsiGenLxyOld_);
  bmmgTree_->SetBranchAddress("JpsiGenLxyOverPt", &JpsiGenLxyOverPt_);
  bmmgTree_->SetBranchAddress("JpsiGenNumberOfCandidates", &JpsiGenNumberOfCandidates_);





  // --- Gen Particle information ---
bmmgTree_->SetBranchAddress("GenMotherID", &GenMotherID_);
bmmgTree_->SetBranchAddress("GenB_pt", &GenB_pt_);
bmmgTree_->SetBranchAddress("GenB_eta", &GenB_eta_);
bmmgTree_->SetBranchAddress("GenB_phi", &GenB_phi_);
bmmgTree_->SetBranchAddress("GenB_mass", &GenB_mass_);
bmmgTree_->SetBranchAddress("GenBLxy", &GenBLxy_);
bmmgTree_->SetBranchAddress("GenBct", &GenBct_);
bmmgTree_->SetBranchAddress("GenBctErr", &GenBctErr_);
bmmgTree_->SetBranchAddress("GenMuPlus_pt", &GenMuPlus_pt_);
bmmgTree_->SetBranchAddress("GenMuPlus_eta", &GenMuPlus_eta_);
bmmgTree_->SetBranchAddress("GenMuPlus_phi", &GenMuPlus_phi_);
bmmgTree_->SetBranchAddress("GenMuPlus_E", &GenMuPlus_E_);
bmmgTree_->SetBranchAddress("GenMuMinus_pt", &GenMuMinus_pt_);
bmmgTree_->SetBranchAddress("GenMuMinus_eta", &GenMuMinus_eta_);
bmmgTree_->SetBranchAddress("GenMuMinus_phi", &GenMuMinus_phi_);
bmmgTree_->SetBranchAddress("GenMuMinus_E", &GenMuMinus_E_);
bmmgTree_->SetBranchAddress("GenDimuon_mass", &GenDimuon_mass_);
bmmgTree_->SetBranchAddress("GenDimuon_pt", &GenDimuon_pt_);
bmmgTree_->SetBranchAddress("GenNPhotons", &GenNPhotons_);

// Photons arrays
bmmgTree_->SetBranchAddress("GenGamma_pt_", &GenGamma_pt_);
bmmgTree_->SetBranchAddress("GenGamma_eta_", &GenGamma_eta_);
bmmgTree_->SetBranchAddress("GenGamma_phi_", &GenGamma_phi_);
bmmgTree_->SetBranchAddress("GenGamma_E_", &GenGamma_E_);
bmmgTree_->SetBranchAddress("GenGamma_origin_", &GenGamma_origin_);
bmmgTree_->SetBranchAddress("GenGamma_motherId_", &GenGamma_motherId_);
bmmgTree_->SetBranchAddress("GenGamma_dRmu1_", &GenGamma_dRmu1_);
bmmgTree_->SetBranchAddress("GenGamma_dRmu2_", &GenGamma_dRmu2_);


// Angular variables
bmmgTree_->SetBranchAddress("GenHelicity3", &GenHelicity3_);
bmmgTree_->SetBranchAddress("GenCoplanarity3", &GenCoplanarity3_);
bmmgTree_->SetBranchAddress("GenCosTheta_l", &GenCosTheta_l_);
bmmgTree_->SetBranchAddress("GenTriBodyMass", &GenTriBodyMass_);
bmmgTree_->SetBranchAddress("GenHelicity4", &GenHelicity4_);
bmmgTree_->SetBranchAddress("GenCoplanarity4", &GenCoplanarity4_);
bmmgTree_->SetBranchAddress("GenFourBodyMass", &GenFourBodyMass_);
bmmgTree_->SetBranchAddress("GenDiphoton_mass", &GenDiphoton_mass_);
bmmgTree_->SetBranchAddress("GenDiphoton_pt", &GenDiphoton_pt_);

// Resonances
bmmgTree_->SetBranchAddress("GenHasJpsi", &GenHasJpsi_);
bmmgTree_->SetBranchAddress("GenJpsi_mass", &GenJpsi_mass_);
bmmgTree_->SetBranchAddress("GenJpsi_pt", &GenJpsi_pt_);
bmmgTree_->SetBranchAddress("GenHasNeutralMeson", &GenHasNeutralMeson_);
bmmgTree_->SetBranchAddress("GenNeutralMesonId", &GenNeutralMesonId_);

// Decay mode / flags
bmmgTree_->SetBranchAddress("GenDecayMode", &GenDecayMode_);
bmmgTree_->SetBranchAddress("GenIs3Body", &GenIs3Body_);
bmmgTree_->SetBranchAddress("GenIs4Body", &GenIs4Body_);
bmmgTree_->SetBranchAddress("GenIsHardProcess", &GenIsHardProcess_);
bmmgTree_->SetBranchAddress("GenIsSignal", &GenIsSignal_);

// --- Truth matched ---
bmmgTree_->SetBranchAddress("Mu1_isMatched", &Mu1_isMatched_);
bmmgTree_->SetBranchAddress("Mu1_genPdgId", &Mu1_genPdgId_);
bmmgTree_->SetBranchAddress("Mu1_genMotherPdgId", &Mu1_genMotherPdgId_);
bmmgTree_->SetBranchAddress("Mu1_genGMotherPdgId", &Mu1_genGMotherPdgId_);
bmmgTree_->SetBranchAddress("Mu1_genDeltaR", &Mu1_genDeltaR_);
bmmgTree_->SetBranchAddress("Mu1_genDeltaPt", &Mu1_genDeltaPt_);
bmmgTree_->SetBranchAddress("Mu2_isMatched", &Mu2_isMatched_);
bmmgTree_->SetBranchAddress("Mu2_genPdgId", &Mu2_genPdgId_);
bmmgTree_->SetBranchAddress("Mu2_genMotherPdgId", &Mu2_genMotherPdgId_);
bmmgTree_->SetBranchAddress("Mu2_genGMotherPdgId", &Mu2_genGMotherPdgId_);
bmmgTree_->SetBranchAddress("Mu2_genDeltaR", &Mu2_genDeltaR_);
bmmgTree_->SetBranchAddress("Mu2_genDeltaPt", &Mu2_genDeltaPt_);
bmmgTree_->SetBranchAddress("DiMuon_isMatched", &DiMuon_isMatched_);

// Photon truth arrays
bmmgTree_->SetBranchAddress("Photon_isMatched_", &Photon_isMatched_);
bmmgTree_->SetBranchAddress("Photon_genPdgId_", &Photon_genPdgId_);
bmmgTree_->SetBranchAddress("Photon_genMotherPdgId_", &Photon_genMotherPdgId_);
bmmgTree_->SetBranchAddress("Photon_genGMotherPdgId_", &Photon_genGMotherPdgId_);
bmmgTree_->SetBranchAddress("Photon_genDeltaR_", &Photon_genDeltaR_);
bmmgTree_->SetBranchAddress("Photon_genDeltaPt_", &Photon_genDeltaPt_);
bmmgTree_->SetBranchAddress("Photon_truthOrigin_", &Photon_truthOrigin_);
// Remaining flags
bmmgTree_->SetBranchAddress("NMatchedPhotons", &NMatchedPhotons_);
bmmgTree_->SetBranchAddress("Topology_isMatched", &Topology_isMatched_);
bmmgTree_->SetBranchAddress("Topology_mu1Matched", &Topology_mu1Matched_);
bmmgTree_->SetBranchAddress("Topology_mu2Matched", &Topology_mu2Matched_);
bmmgTree_->SetBranchAddress("Topology_photon1Matched", &Topology_photon1Matched_);
bmmgTree_->SetBranchAddress("Topology_photon2Matched", &Topology_photon2Matched_);
bmmgTree_->SetBranchAddress("Topology_decayModeMatched", &Topology_decayModeMatched_);
bmmgTree_->SetBranchAddress("IsSignalMatched", &IsSignalMatched_);
bmmgTree_->SetBranchAddress("IsMuonsFromJpsi", &IsMuonsFromJpsi_);
bmmgTree_->SetBranchAddress("IsMuonsDirectFromBs", &IsMuonsDirectFromBs_);




}

