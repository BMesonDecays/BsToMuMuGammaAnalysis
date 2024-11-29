#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/RadiativeRootTree.h"
using namespace std;

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
  bmmgTree_->Branch("photonMultiplicity", &photonMultiplicity_,"photonMultiplicity/I");
  bmmgTree_->Branch("photonPt",&photonPt_,"photonPt/D");
  bmmgTree_->Branch("photonEta",&photonEta_,"photonEta/D");
  bmmgTree_->Branch("photonPhi",&photonPhi_,"photonPhi/D");
  bmmgTree_->Branch("photonEnergy",&photonEnergy_,"photonEnergy/D");
  bmmgTree_->Branch("photonET",&photonET_,"photonET/D");
  bmmgTree_->Branch("photonTrkIso",&photonTrkIso_,"photonTrkIso/D");
  bmmgTree_->Branch("photonEcalIso",&photonEcalIso_,"photonEcalIso/D");
  bmmgTree_->Branch("photonHcalIso",&photonHcalIso_,"photonHcalIso/D");
  bmmgTree_->Branch("photonCaloIso",&photonCaloIso_,"photonCaloIso/D");
  bmmgTree_->Branch("photonSSSigmaiEtaiEta",&photonSSSigmaiEtaiEta_,"photonSSSigmaiEtaiEta/D");
  bmmgTree_->Branch("photonSSSigmaiEtaiPhi",&photonSSSigmaiEtaiPhi_,"photonSSSigmaiEtaiPhi/D");
  bmmgTree_->Branch("photonSSSigmaiPhiiPhi",&photonSSSigmaiPhiiPhi_,"photonSSSigmaiPhiiPhi/D");
  bmmgTree_->Branch("photonSSSigmaEtaEta",&photonSSSigmaEtaEta_,"photonSSSigmaEtaEta/D");
  bmmgTree_->Branch("photonSSe1x5",&photonSSe1x5_,"photonSSe1x5/D");
  bmmgTree_->Branch("photonSSe2x5",&photonSSe2x5_,"photonSSe2x5/D");
  bmmgTree_->Branch("photonSSe3x3",&photonSSe3x3_,"photonSSe3x3/D");
  bmmgTree_->Branch("photonSSe5x5",&photonSSe5x5_,"photonSSe5x5/D");
  bmmgTree_->Branch("photonSShcalDepth1OverEcal",&photonSShcalDepth1OverEcal_,"photonSShcalDepth1OverEcal/D");
  bmmgTree_->Branch("photonSShcalDepth2OverEcal",&photonSShcalDepth2OverEcal_,"photonSShcalDepth2OverEcal/D");
  bmmgTree_->Branch("photonSShcalDepth1OverEcalBc",&photonSShcalDepth1OverEcalBc_,"photonSShcalDepth1OverEcal/D");
  bmmgTree_->Branch("photonSShcalDepth2OverEcalBc",&photonSShcalDepth2OverEcalBc_,"photonSShcalDepth2OverEcalBc/D");
  bmmgTree_->Branch("photonSShcalOverEcal",&photonSShcalOverEcal_,"photonSShcalOverEcal[7]/D");
  bmmgTree_->Branch("photonSShcalOverEcalBc",&photonSShcalOverEcalBc_,"photonSShcalOverEcalBc[7]/D");
  bmmgTree_->Branch("photonSSmaxEnergyXtal",&photonSSmaxEnergyXtal_,"photonSSmaxEnergyXtal/D");
  bmmgTree_->Branch("photonSSeffSigmaRR",&photonSSeffSigmaRR_,"photonSSeffSigmaRR/D");
  bmmgTree_->Branch("photonSCEnergy",&photonSCEnergy_,"photonSCEnergy/D");
  bmmgTree_->Branch("photonSCRawEnergy",&photonSCRawEnergy_,"photonSCRawEnergy/D");
  bmmgTree_->Branch("photonSCPreShowerEP1",&photonSCPreShowerEP1_,"photonSCPreShowerEP1/D");
  bmmgTree_->Branch("photonSCPreShowerEP2",&photonSCPreShowerEP2_,"photonSCPreShowerEP2/D");
  bmmgTree_->Branch("photonSCEta",&photonSCEta_,"photonSCEta/D");
  bmmgTree_->Branch("photonSCPhi",&photonSCPhi_,"photonSCPhi/D");
  bmmgTree_->Branch("photonSCEtaWidth",&photonSCEtaWidth_,"photonSCEtaWidth/D");
  bmmgTree_->Branch("photonSCPhiWidth",&photonSCPhiWidth_,"photonSCPhiWidth/D");
  bmmgTree_->Branch("photonSCBrem",&photonSCBrem_,"photonSCBrem/D");
  bmmgTree_->Branch("photonSCR9",&photonSCR9_,"photonSCR9/D");
  bmmgTree_->Branch("photonSCHadTowOverEm",&photonSCHadTowOverEm_,"photonSCHadTowOverEm/D");
  bmmgTree_->Branch("PiZeroM_alone",&PiZeroM_alone_,"PiZeroM_alone/D");
  bmmgTree_->Branch("PiZeroEta_alone",&PiZeroEta_alone_,"PiZeroEta_alone/D");
  bmmgTree_->Branch("PiZeroPhi_alone",&PiZeroPhi_alone_,"PiZeroPhi_alone/D");
  bmmgTree_->Branch("PiZeroPt_alone",&PiZeroPt_alone_,"PiZeroPt_alone/D");
  bmmgTree_->Branch("EtaMesonM_alone", &EtaMesonM_alone_, "EtaMesonM_alone/D");
  bmmgTree_->Branch("EtaMesonEta_alone", &EtaMesonEta_alone_, "EtaMesonEta_alone/D");
  bmmgTree_->Branch("EtaMesonPhi_alone", &EtaMesonPhi_alone_, "EtaMesonPhi_alone/D");
  bmmgTree_->Branch("EtaMesonPt_alone", &EtaMesonPt_alone_, "EtaMesonPt_alone/D");
  bmmgTree_->Branch("EtaPrimeM_alone", &EtaPrimeM_alone_, "EtaPrimeM_alone/D");
  bmmgTree_->Branch("EtaPrimeEta_alone", &EtaPrimeEta_alone_, "EtaPrimeEta_alone/D");
  bmmgTree_->Branch("EtaPrimePhi_alone", &EtaPrimePhi_alone_, "EtaPrimePhi_alone/D");
  bmmgTree_->Branch("EtaPrimePt_alone", &EtaPrimePt_alone_, "EtaPrimePt_alone/D");
  bmmgTree_->Branch("K1Pt_beffit", &K1Pt_beffit_,"K1Pt_beffit/D");
  bmmgTree_->Branch("K1Pz_beffit", &K1Pz_beffit_,"K1Pz_beffit/D");
  bmmgTree_->Branch("K1Eta_beffit", &K1Eta_beffit_,"K1Eta_beffit/D");
  bmmgTree_->Branch("K1Phi_beffit", &K1Phi_beffit_,"K1Phi_beffit/D");
  bmmgTree_->Branch("K2Pt_beffit", &K2Pt_beffit_,"K2Pt_beffit/D");
  bmmgTree_->Branch("K2Pz_beffit", &K2Pz_beffit_,"K2Pz_beffit/D");
  bmmgTree_->Branch("K2Eta_beffit", &K2Eta_beffit_,"K2Eta_beffit/D");
  bmmgTree_->Branch("K2Phi_beffit", &K2Phi_beffit_,"K2Phi_beffit/D");

  bmmgTree_->Branch("PhiM_beffit", &PhiM_beffit_, "PhiM_beffit/D");
  bmmgTree_->Branch("PhiEta_beffit", &PhiEta_beffit_, "PhiEta_beffit/D");
  bmmgTree_->Branch("PhiPhi_beffit", &PhiPhi_beffit_, "PhiPhi_beffit/D");
  bmmgTree_->Branch("PhiPt_beffit", &PhiPt_beffit_, "PhiPt_beffit/D");
  bmmgTree_->Branch("BsPhiGammaM_beffit", &BsPhiGammaM_beffit_, "BsPhiGammaM_beffit/D");
  bmmgTree_->Branch("BsPhiGammaEta_beffit", &BsPhiGammaEta_beffit_, "BsPhiGammaEta_beffit/D");
  bmmgTree_->Branch("BsPhiGammaPhi_beffit", &BsPhiGammaPhi_beffit_, "BsPhiGammaPhi_beffit/D");
  bmmgTree_->Branch("BsPhiGammaPt_beffit", &BsPhiGammaPt_beffit_, "BsPhiGammaPt_beffit/D");
  bmmgTree_->Branch("BsPhiGamma_vtxProb", &BsPhiGamma_vtxProb_, "BsPhiGamma_vtxProb/D");
  bmmgTree_->Branch("BsPhiGamma_CosineAlpha", &BsPhiGamma_CosineAlpha_, "BsPhiGamma_CosineAlpha/D");
  bmmgTree_->Branch("BsPhiGamma_KKDCA", &BsPhiGamma_KKDCA_, "BsPhiGamma_KKDCA/D");
  bmmgTree_->Branch("BsPhiGamma_Chi2pv_KVFvtx", &BsPhiGamma_Chi2pv_KVFvtx_, "BsPhiGamma_Chi2pv_KVFvtx/D");
  bmmgTree_->Branch("BsPhiGamma_Mahalanobis", &BsPhiGamma_Mahalanobis_, "BsPhiGamma_Mahalanobis/D");
  bmmgTree_->Branch("BsPhiGamma_K1Pt_fit", &BsPhiGamma_K1Pt_fit_, "BsPhiGamma_K1Pt_fit/D");
  bmmgTree_->Branch("BsPhiGamma_K2Pt_fit", &BsPhiGamma_K2Pt_fit_, "BsPhiGamma_K2Pt_fit/D");
  bmmgTree_->Branch("BsPhiGamma_PhiM_fit", &BsPhiGamma_PhiM_fit_, "BsPhiGamma_PhiM_fit/D");
  
  bmmgTree_->Branch("electronMultiplicity",&electronMultiplicity_,"electronMultiplicity/D");
  bmmgTree_->Branch("pfCandMultiplicity",&pfCandMultiplicity_,"pfCandMultiplicity/D");
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
	photonMultiplicity_ = -9999999;
	photonPt_           = -9999999;
	photonEta_          = -9999999;
	photonPhi_          = -9999999;
	photonEnergy_       = -9999999;
	photonET_           = -9999999;
	photonTrkIso_       = -9999999;
	photonEcalIso_        = -9999999;
	photonHcalIso_        = -9999999;
	photonCaloIso_        = -9999999;
	photonSSSigmaiEtaiEta_  = -9999999;
	photonSSSigmaiEtaiPhi_  = -9999999;
	photonSSSigmaiPhiiPhi_  = -9999999;
	photonSSSigmaEtaEta_    = -9999999;
	photonSSe1x5_           = -9999999;
	photonSSe2x5_           = -9999999;
	photonSSe3x3_           = -9999999;
	photonSSe5x5_           = -9999999;
	photonSShcalDepth1OverEcal_ = -9999999;
	photonSShcalDepth2OverEcal_ = -9999999;
	photonSShcalDepth1OverEcalBc_ = -9999999;
	photonSShcalDepth2OverEcalBc_ = -9999999;
	for(size_t i=0; i<7; ++i){photonSShcalOverEcal_[i] = -9999999;}
	for(size_t i=0; i<7; ++i){photonSShcalOverEcalBc_[i] = -9999999;}
	photonSSmaxEnergyXtal_  = -9999999;
	photonSSeffSigmaRR_     = -9999999;
	photonSCEnergy_         = -9999999;
	photonSCRawEnergy_      = -9999999;
	photonSCPreShowerEP1_   = -9999999;
	photonSCPreShowerEP2_   = -9999999;
	photonSCEta_            = -9999999;
	photonSCPhi_            = -9999999;
	photonSCEtaWidth_       = -9999999;
	photonSCPhiWidth_       = -9999999;
	photonSCBrem_           = -9999999;
	photonSCR9_             = -9999999;
	photonSCHadTowOverEm_   = -9999999;
	PiZeroM_alone_          = -9999999;
	PiZeroEta_alone_        = -9999999;
	PiZeroPhi_alone_        = -9999999;
	PiZeroPt_alone_         = -9999999;
	EtaMesonM_alone_        = -9999999;
	EtaMesonEta_alone_      = -9999999;
	EtaMesonPhi_alone_      = -9999999;
	EtaMesonPt_alone_       = -9999999;
        EtaPrimeM_alone_        = -9999999;
	EtaPrimeEta_alone_      = -9999999;
	EtaPrimePhi_alone_      = -9999999;
	EtaPrimePt_alone_       = -9999999;


        K1Pt_beffit_ = -9999999;
        K1Pz_beffit_ = -9999999;
        K1Eta_beffit_ = -9999999;
        K1Phi_beffit_ = -9999999;
        K2Pt_beffit_ = -9999999;
        K2Pz_beffit_ = -9999999;
        K2Eta_beffit_ = -9999999;
        K2Phi_beffit_ = -9999999;


	PhiM_beffit_            = -9999999;
        PhiEta_beffit_          = -9999999;
        PhiPhi_beffit_          = -9999999;
        PhiPt_beffit_           = -9999999;
	BsPhiGammaM_beffit_     = -9999999;
        BsPhiGammaEta_beffit_   = -9999999;
        BsPhiGammaPhi_beffit_   = -9999999;
        BsPhiGammaPt_beffit_    = -9999999;
        BsPhiGamma_vtxProb_     = -9999999;
        BsPhiGamma_CosineAlpha_ = -9999999;
        BsPhiGamma_KKDCA_       = -9999999;
        BsPhiGamma_Chi2pv_KVFvtx_ = -9999999;
        BsPhiGamma_Mahalanobis_ = -9999999;
        BsPhiGamma_K1Pt_fit_    = -9999999;
        BsPhiGamma_K2Pt_fit_    = -9999999;
        BsPhiGamma_PhiM_fit_    = -9999999; 
	electronMultiplicity_   = -9999999;
	pfCandMultiplicity_     = -9999999;

	JpsiGenLxy_             = -9999999;
        JpsiGenPVz_             = -9999999;
        JpsiGenPVy_             = -9999999;
        JpsiGenPVx_             = -9999999;
        JpsiGenPt_              = -9999999;
        JpsiGenLxyOld_          = -9999999;
        JpsiGenLxyOverPt_       = -9999999;
        JpsiGenNumberOfCandidates_= -9999999;

	for(size_t i=0; i<9;++i){PVcovariance_[i] = -9999999;}
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
  bmmgTree_->SetBranchAddress("photonMultiplicity", &photonMultiplicity_ );
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
  bmmgTree_->SetBranchAddress("PiZeroM_alone",     &PiZeroM_alone_);
  bmmgTree_->SetBranchAddress("PiZeroEta_alone",   &PiZeroEta_alone_);
  bmmgTree_->SetBranchAddress("PiZeroPhi_alone",   &PiZeroPhi_alone_);
  bmmgTree_->SetBranchAddress("PiZeroPt_alone",    &PiZeroPt_alone_);
  bmmgTree_->SetBranchAddress("EtaMesonM_alone",   &EtaMesonM_alone_);
  bmmgTree_->SetBranchAddress("EtaMesonEta_alone", &EtaMesonEta_alone_);
  bmmgTree_->SetBranchAddress("EtaMesonPhi_alone", &EtaMesonPhi_alone_);
  bmmgTree_->SetBranchAddress("EtaMesonPt_alone",  &EtaMesonPt_alone_);
  bmmgTree_->SetBranchAddress("EtaPrimeM_alone",   &EtaPrimeM_alone_);
  bmmgTree_->SetBranchAddress("EtaPrimeEta_alone", &EtaPrimeEta_alone_);
  bmmgTree_->SetBranchAddress("EtaPrimePhi_alone", &EtaPrimePhi_alone_);
  bmmgTree_->SetBranchAddress("EtaPrimePt_alone",  &EtaPrimePt_alone_);

  bmmgTree_->SetBranchAddress(  "K1Pt_beffit"			  , &K1Pt_beffit_  );
  bmmgTree_->SetBranchAddress(  "K1Pz_beffit"			  , &K1Pz_beffit_  );
  bmmgTree_->SetBranchAddress(  "K1Eta_beffit"			  , &K1Eta_beffit_  );
  bmmgTree_->SetBranchAddress(  "K1Phi_beffit"			  , &K1Phi_beffit_  );
  bmmgTree_->SetBranchAddress(  "K2Pt_beffit"			  , &K2Pt_beffit_  );
  bmmgTree_->SetBranchAddress(  "K2Pz_beffit"			  , &K2Pz_beffit_  );
  bmmgTree_->SetBranchAddress(  "K2Eta_beffit"			  , &K2Eta_beffit_  );
  bmmgTree_->SetBranchAddress(  "K2Phi_beffit"			  , &K2Phi_beffit_  );

  bmmgTree_->SetBranchAddress("PhiM_beffit", &PhiM_beffit_);
  bmmgTree_->SetBranchAddress("PhiEta_beffit", &PhiEta_beffit_);
  bmmgTree_->SetBranchAddress("PhiPhi_beffit", &PhiPhi_beffit_);
  bmmgTree_->SetBranchAddress("PhiPt_beffit", &PhiPt_beffit_);
  bmmgTree_->SetBranchAddress("BsPhiGammaM_beffit", &BsPhiGammaM_beffit_);
  bmmgTree_->SetBranchAddress("BsPhiGammaEta_beffit", &BsPhiGammaEta_beffit_);
  bmmgTree_->SetBranchAddress("BsPhiGammaPhi_beffit", &BsPhiGammaPhi_beffit_);
  bmmgTree_->SetBranchAddress("BsPhiGammaPt_beffit", &BsPhiGammaPt_beffit_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_vtxProb", &BsPhiGamma_vtxProb_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_CosineAlpha", &BsPhiGamma_CosineAlpha_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_KKDCA", &BsPhiGamma_KKDCA_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_Chi2pv_KVFvtx", &BsPhiGamma_Chi2pv_KVFvtx_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_Mahalanobis", &BsPhiGamma_Mahalanobis_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_K1Pt_fit", &BsPhiGamma_K1Pt_fit_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_K2Pt_fit", &BsPhiGamma_K2Pt_fit_);
  bmmgTree_->SetBranchAddress("BsPhiGamma_PhiM_fit", &BsPhiGamma_PhiM_fit_);
  
  bmmgTree_->SetBranchAddress("electronMultiplicity", &electronMultiplicity_);
  bmmgTree_->SetBranchAddress("pfCandMultiplicity", &pfCandMultiplicity_);

  double JpsiGenLxy_;
                 double JpsiGenPVz_;
                 double JpsiGenPVy_;
                 double JpsiGenPVx_;
                 double JpsiGenPt_;
                 double JpsiGenLxyOld_ ;
                 double JpsiGenLxyOverPt_;
                 int    JpsiGenNumberOfCandidates_;

  bmmgTree_->SetBranchAddress("JpsiGenPVx", &JpsiGenPVx_);
  bmmgTree_->SetBranchAddress("JpsiGenPVy", &JpsiGenPVy_);
  bmmgTree_->SetBranchAddress("JpsiGenPVz", &JpsiGenPVz_);
  bmmgTree_->SetBranchAddress("JpsiGenLxy", &JpsiGenLxy_);
  bmmgTree_->SetBranchAddress("JpsiGenPt", &JpsiGenPt_);
  bmmgTree_->SetBranchAddress("JpsiGenLxyOld", &JpsiGenLxyOld_);
  bmmgTree_->SetBranchAddress("JpsiGenLxyOverPt", &JpsiGenLxyOverPt_);
  bmmgTree_->SetBranchAddress("JpsiGenNumberOfCandidates", &JpsiGenNumberOfCandidates_);

}

