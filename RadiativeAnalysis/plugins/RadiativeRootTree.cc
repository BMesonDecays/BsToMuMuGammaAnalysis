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

