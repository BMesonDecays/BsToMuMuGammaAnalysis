#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_RadiativeRootTree
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_RadiativeRootTree

#include <string>
#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#include <TChain.h>
#include <vector>

class RadiativeRootTree {
public:
	RadiativeRootTree();
	~RadiativeRootTree();
	void resetEntries();
	void writeFile();
	void createTree(const std::string filename);
	void readTree(const std::string filename);
	void readTree(std::vector<std::string> filenames);
	void getAngles(const double aa, const double bb, const double cc, const double dd);
	void getVtx(const double aa, const double bb, const double cc, const double dd, const double ee, const double ff, const double gg, const double hh, const double ii);
	void getDeDx(const double f1, const double f2, const int f3);
	void fill();
	void setBranchAddresses();

public:
	int runNumber_;
	int PUinteraction_;
	int PUTrueinteraction_;
	unsigned int eventNumber_;
	int lumiSection_;
	int isPV_;
	int NVerticesbeforecut_;
	int NVerticesaftercut_;
	int NTracksPVbeforecut_;
	int isBS_;
	double BSx_;
	double BSy_;
	double BSz_;
	double BSdx_;
	double BSdy_;
	double BSdz_;
	double BSdydz_;
	double BSdxdz_;
	double BSsigmaZ_;
	double BSdsigmaZ_;
	double PVx_;
	double PVy_;
	double PVz_;
	double PVerrx_;
	double PVerry_;
	double PVerrz_;
	double PVndof_;
	double PVrho_;
	double dedxTrk_;
	double errdedxTrk_;
	int numdedxTrk_;
	int triggerbit_HLT_DoubleMu4_LowMass_Displaced_;
	int triggerbit_HLT_DoubleMu4_LowMass_;
	int triggerbit_HLT_DoubleMu4_3_Bs_;
	int triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_;
	int triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_;
	int triggerbit_HLTDimuon4JpsiDisplaced_;
	int triggerbit_HLTDimuon4JpsiNoVertexing_;
	int triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_;
	double mu1Pt_beffit_;
	double mu1Pz_beffit_;
	double mu1Eta_beffit_;
	double mu1Phi_beffit_;
	double mu1Energy_beffit_;
	double mu2Pt_beffit_;
	double mu2Pz_beffit_;
	double mu2Eta_beffit_;
	double mu2Phi_beffit_;
	double mu2Energy_beffit_;
	double MuonPairDR_;
	double Mu1TrkBSDxy_;
	double Mu1TrkBSDz_;
	double Mu2TrkBSDxy_;
	double Mu2TrkBSDz_;
	int Mu1PixelHits_;
	int Mu1TrackerHits_;
	int Mu1isGood_;
	int Mu1InnerTrkHighQuality_;
	int Mu2PixelHits_;
	int Mu2TrackerHits_;
	int Mu2isGood_;
	int Mu2InnerTrkHighQuality_;
	double mu1MVAScore_;
	double mu2MVAScore_;

	int nMuons_;
	int nPhotons_;
	int nConversions_;
	int nTracks_;

	double DiMuonM_beffit_;
	double DiMuonEta_beffit_;
	double DiMuonPhi_beffit_;
	double DiMuonPt_beffit_;
	double DiMuon_vtxProb_;
	double DiMuon_CosineAlpha_;
	double DiMuon_DCA_;
	double DiMuon_Chi2pv_KVFvtx_;
	double DiMuon_Mahalanobis_;
	double DiMuon_Lxy_;
	double DiMuon_Lxyerr_;
	double DiMuon_LxyOverPt_;
	int    DiMuon_NumberOfCandidates_;
	int    DiMuon_ResonanceType_;
	int    mu1SoftID_;
	int    mu2SoftID_;
	int    DiMuon_mu1Cat_alone_;
	int    DiMuon_mu2Cat_alone_;
	int    DiMuon_mu1nPixHits_alone_;
	int    DiMuon_mu2nPixHits_alone_;






	// Tripple object converted photon vertex variables

double FourvectorBsMass_mmconvg_;
double FourvectorBsPt_mmconvg_;
double FourvectorBsEta_mmconvg_;
double FourvectorBsPhi_mmconvg_;

double VertexfitBsMass_mmconvg_;
double VertexfitBsPt_mmconvg_;
double VertexfitBsEta_mmconvg_;
double VertexfitBsPhi_mmconvg_;
double VertexfitBsPz_mmconvg_;

double VertexfitBsSVx_mmconvg_;
double VertexfitBsSVy_mmconvg_;
double VertexfitBsSVz_mmconvg_;

double FourvectorConvGammaPt_mmconvg_;
double FourvectorConvGammaEta_mmconvg_;
double FourvectorConvGammaPhi_mmconvg_;

double VertexfitPVxRefitCosTheta_mmconvg_;
double VertexfitPVyRefitCosTheta_mmconvg_;
double VertexfitPVzRefitCosTheta_mmconvg_;

double VertexfitPVxRefitClosestZ_mmconvg_;
double VertexfitPVyRefitClosestZ_mmconvg_;
double VertexfitPVzRefitClosestZ_mmconvg_;

double VertexfitBsCt3DPVClosestZ_mmconvg_;
double VertexfitBsCt2DPVClosestZ_mmconvg_;
double VertexfitBsCt3DPVCosTheta_mmconvg_;
double VertexfitBsCt2DPVCosTheta_mmconvg_;
double VertexfitBsCt2DPVCosThetaOld_mmconvg_;
double VertexfitBsCt2DPVClosestZOld_mmconvg_;
double VertexfitBsCt2DBSOld_mmconvg_;
double VertexfitBsCt2DOld_mmconvg_;

double VertexfitBsCtErr2DBS_mmconvg_;
double VertexfitBsCtErr2D_mmconvg_;
double VertexfitBsCtErr2DClosestZ_mmconvg_;
double VertexfitBsCtErr2DCosTheta_mmconvg_;
double VertexfitBsCtErr2DCosThetaOld_mmconvg_;
double VertexfitBsCtErr2DBSOld_mmconvg_;
double VertexfitBsCtErr2DOld_mmconvg_;
double VertexfitBsCtErr2DClosestZOld_mmconvg_;
double VertexfitBsCtErr3D_mmconvg_;
double VertexfitBsCtErr2D2_mmconvg_;
double VertexfitBsCtErrMPV_mmconvg_;

double VertexfitBsCt3DRefit_mmconvg_;
double VertexfitBsCt2DRefit_mmconvg_;
double VertexfitBsCtMPVRefit_mmconvg_;
double VertexfitBsCtErr3DRefit_mmconvg_;
double VertexfitBsCtErr2DRefit_mmconvg_;
double VertexfitBsCtErrMPVRefit_mmconvg_;

double VertexfitBsLxy_mmconvg_;
double VertexfitBsLxyErr_mmconvg_;
double VertexfitBsCt_mmconvg_;
double VertexfitBsCtErr_mmconvg_;

double VertexfitBsErrX_mmconvg_;
double VertexfitBsErrY_mmconvg_;
double VertexfitBsErrXY_mmconvg_;

double VertexfitBsDist3D_mmconvg_;
double VertexfitBsDist3DErr_mmconvg_;
double VertexfitBsTime3D_mmconvg_;
double VertexfitBsTime3DErr_mmconvg_;

double VertexfitBsDist2D_mmconvg_;
double VertexfitBsDist2DErr_mmconvg_;
double VertexfitBsTime2D_mmconvg_;
double VertexfitBsTime2DErr_mmconvg_;

double BsVtxProb_mmconvg_;
double BsCt2D_mmconvg_;
double BsCt2DBS_mmconvg_;
double BsCt3D_mmconvg_;

double DeltaRPhoton1DiMuon_mmconvg_;
double DeltaRPhoton2DiMuon_mmconvg_;

double BsHelicity_mmconvg_;
double BsCoplanarity_mmconvg_;

	// Tripple object reco photon vertex variables

double FourvectorBsMass_mmrecog_;
double FourvectorBsPt_mmrecog_;
double FourvectorBsEta_mmrecog_;
double FourvectorBsPhi_mmrecog_;

double VertexfitBsMass_mmrecog_;
double VertexfitBsPt_mmrecog_;
double VertexfitBsEta_mmrecog_;
double VertexfitBsPhi_mmrecog_;
double VertexfitBsPz_mmrecog_;

double VertexfitBsSVx_mmrecog_;
double VertexfitBsSVy_mmrecog_;
double VertexfitBsSVz_mmrecog_;

double FourvectorGammaPt_mmrecog_;
double FourvectorGammaEta_mmrecog_;
double FourvectorGammaPhi_mmrecog_;

double VertexfitPVxRefitCosTheta_mmrecog_;
double VertexfitPVyRefitCosTheta_mmrecog_;
double VertexfitPVzRefitCosTheta_mmrecog_;

double VertexfitPVxRefitClosestZ_mmrecog_;
double VertexfitPVyRefitClosestZ_mmrecog_;
double VertexfitPVzRefitClosestZ_mmrecog_;

double VertexfitBsCt3DPVClosestZ_mmrecog_;
double VertexfitBsCt2DPVClosestZ_mmrecog_;
double VertexfitBsCt3DPVCosTheta_mmrecog_;
double VertexfitBsCt2DPVCosTheta_mmrecog_;
double VertexfitBsCt2DPVCosThetaOld_mmrecog_;
double VertexfitBsCt2DPVClosestZOld_mmrecog_;
double VertexfitBsCt2DBSOld_mmrecog_;
double VertexfitBsCt2DOld_mmrecog_;

double VertexfitBsCtErr2DBS_mmrecog_;
double VertexfitBsCtErr2D_mmrecog_;
double VertexfitBsCtErr2DClosestZ_mmrecog_;
double VertexfitBsCtErr2DCosTheta_mmrecog_;
double VertexfitBsCtErr2DCosThetaOld_mmrecog_;
double VertexfitBsCtErr2DBSOld_mmrecog_;
double VertexfitBsCtErr2DOld_mmrecog_;
double VertexfitBsCtErr2DClosestZOld_mmrecog_;
double VertexfitBsCtErr3D_mmrecog_;
double VertexfitBsCtErr2D2_mmrecog_;
double VertexfitBsCtErrMPV_mmrecog_;

double VertexfitBsCt3DRefit_mmrecog_;
double VertexfitBsCt2DRefit_mmrecog_;
double VertexfitBsCtMPVRefit_mmrecog_;
double VertexfitBsCtErr3DRefit_mmrecog_;
double VertexfitBsCtErr2DRefit_mmrecog_;
double VertexfitBsCtErrMPVRefit_mmrecog_;

double VertexfitBsLxy_mmrecog_;
double VertexfitBsLxyErr_mmrecog_;
double VertexfitBsCt_mmrecog_;
double VertexfitBsCtErr_mmrecog_;

double VertexfitBsErrX_mmrecog_;
double VertexfitBsErrY_mmrecog_;
double VertexfitBsErrXY_mmrecog_;

double VertexfitBsDist3D_mmrecog_;
double VertexfitBsDist3DErr_mmrecog_;
double VertexfitBsTime3D_mmrecog_;
double VertexfitBsTime3DErr_mmrecog_;

double VertexfitBsDist2D_mmrecog_;
double VertexfitBsDist2DErr_mmrecog_;
double VertexfitBsTime2D_mmrecog_;
double VertexfitBsTime2DErr_mmrecog_;

double BsVtxProb_mmrecog_;
double BsCt2D_mmrecog_;
double BsCt2DBS_mmrecog_;
double BsCt3D_mmrecog_;

double DeltaRPhoton1DiMuon_mmrecog_;
double DeltaRPhoton2DiMuon_mmrecog_;

double BsHelicity_mmrecog_;
double BsCoplanarity_mmrecog_;

	// Tetra object converted photon vertex variables

double FourvectorBsMass_mmconvgg_;
double FourvectorBsPt_mmconvgg_;
double FourvectorBsEta_mmconvgg_;
double FourvectorBsPhi_mmconvgg_;

double VertexfitBsMass_mmconvgg_;
double VertexfitBsPt_mmconvgg_;
double VertexfitBsEta_mmconvgg_;
double VertexfitBsPhi_mmconvgg_;
double VertexfitBsPz_mmconvgg_;

double VertexfitBsSVx_mmconvgg_;
double VertexfitBsSVy_mmconvgg_;
double VertexfitBsSVz_mmconvgg_;

double FourvectorConvGammaPt_mmconvgg_;
double FourvectorConvGammaEta_mmconvgg_;
double FourvectorConvGammaPhi_mmconvgg_;

double VertexfitPVxRefitCosTheta_mmconvgg_;
double VertexfitPVyRefitCosTheta_mmconvgg_;
double VertexfitPVzRefitCosTheta_mmconvgg_;

double VertexfitPVxRefitClosestZ_mmconvgg_;
double VertexfitPVyRefitClosestZ_mmconvgg_;
double VertexfitPVzRefitClosestZ_mmconvgg_;

double VertexfitBsCt3DPVClosestZ_mmconvgg_;
double VertexfitBsCt2DPVClosestZ_mmconvgg_;
double VertexfitBsCt3DPVCosTheta_mmconvgg_;
double VertexfitBsCt2DPVCosTheta_mmconvgg_;
double VertexfitBsCt2DPVCosThetaOld_mmconvgg_;
double VertexfitBsCt2DPVClosestZOld_mmconvgg_;
double VertexfitBsCt2DBSOld_mmconvgg_;
double VertexfitBsCt2DOld_mmconvgg_;

double VertexfitBsCtErr2DBS_mmconvgg_;
double VertexfitBsCtErr2D_mmconvgg_;
double VertexfitBsCtErr2DClosestZ_mmconvgg_;
double VertexfitBsCtErr2DCosTheta_mmconvgg_;
double VertexfitBsCtErr2DCosThetaOld_mmconvgg_;
double VertexfitBsCtErr2DBSOld_mmconvgg_;
double VertexfitBsCtErr2DOld_mmconvgg_;
double VertexfitBsCtErr2DClosestZOld_mmconvgg_;
double VertexfitBsCtErr3D_mmconvgg_;
double VertexfitBsCtErr2D2_mmconvgg_;
double VertexfitBsCtErrMPV_mmconvgg_;

double VertexfitBsCt3DRefit_mmconvgg_;
double VertexfitBsCt2DRefit_mmconvgg_;
double VertexfitBsCtMPVRefit_mmconvgg_;
double VertexfitBsCtErr3DRefit_mmconvgg_;
double VertexfitBsCtErr2DRefit_mmconvgg_;
double VertexfitBsCtErrMPVRefit_mmconvgg_;

double VertexfitBsLxy_mmconvgg_;
double VertexfitBsLxyErr_mmconvgg_;
double VertexfitBsCt_mmconvgg_;
double VertexfitBsCtErr_mmconvgg_;

double VertexfitBsErrX_mmconvgg_;
double VertexfitBsErrY_mmconvgg_;
double VertexfitBsErrXY_mmconvgg_;

double VertexfitBsDist3D_mmconvgg_;
double VertexfitBsDist3DErr_mmconvgg_;
double VertexfitBsTime3D_mmconvgg_;
double VertexfitBsTime3DErr_mmconvgg_;

double VertexfitBsDist2D_mmconvgg_;
double VertexfitBsDist2DErr_mmconvgg_;
double VertexfitBsTime2D_mmconvgg_;
double VertexfitBsTime2DErr_mmconvgg_;

double BsVtxProb_mmconvgg_;
double BsCt2D_mmconvgg_;
double BsCt2DBS_mmconvgg_;
double BsCt3D_mmconvgg_;

double DeltaRPhoton1DiMuon_mmconvgg_;
double DeltaRPhoton2DiMuon_mmconvgg_;

double BsHelicity_mmconvgg_;
double BsCoplanarity_mmconvgg_;

	// Tetra object reco photon vertex variables

double FourvectorBsMass_mmrecogg_;
double FourvectorBsPt_mmrecogg_;
double FourvectorBsEta_mmrecogg_;
double FourvectorBsPhi_mmrecogg_;

double VertexfitBsMass_mmrecogg_;
double VertexfitBsPt_mmrecogg_;
double VertexfitBsEta_mmrecogg_;
double VertexfitBsPhi_mmrecogg_;
double VertexfitBsPz_mmrecogg_;

double VertexfitBsSVx_mmrecogg_;
double VertexfitBsSVy_mmrecogg_;
double VertexfitBsSVz_mmrecogg_;

double FourvectorGammaPt_mmrecogg_;
double FourvectorGammaEta_mmrecogg_;
double FourvectorGammaPhi_mmrecogg_;

double VertexfitPVxRefitCosTheta_mmrecogg_;
double VertexfitPVyRefitCosTheta_mmrecogg_;
double VertexfitPVzRefitCosTheta_mmrecogg_;

double VertexfitPVxRefitClosestZ_mmrecogg_;
double VertexfitPVyRefitClosestZ_mmrecogg_;
double VertexfitPVzRefitClosestZ_mmrecogg_;

double VertexfitBsCt3DPVClosestZ_mmrecogg_;
double VertexfitBsCt2DPVClosestZ_mmrecogg_;
double VertexfitBsCt3DPVCosTheta_mmrecogg_;
double VertexfitBsCt2DPVCosTheta_mmrecogg_;
double VertexfitBsCt2DPVCosThetaOld_mmrecogg_;
double VertexfitBsCt2DPVClosestZOld_mmrecogg_;
double VertexfitBsCt2DBSOld_mmrecogg_;
double VertexfitBsCt2DOld_mmrecogg_;

double VertexfitBsCtErr2DBS_mmrecogg_;
double VertexfitBsCtErr2D_mmrecogg_;
double VertexfitBsCtErr2DClosestZ_mmrecogg_;
double VertexfitBsCtErr2DCosTheta_mmrecogg_;
double VertexfitBsCtErr2DCosThetaOld_mmrecogg_;
double VertexfitBsCtErr2DBSOld_mmrecogg_;
double VertexfitBsCtErr2DOld_mmrecogg_;
double VertexfitBsCtErr2DClosestZOld_mmrecogg_;
double VertexfitBsCtErr3D_mmrecogg_;
double VertexfitBsCtErr2D2_mmrecogg_;
double VertexfitBsCtErrMPV_mmrecogg_;

double VertexfitBsCt3DRefit_mmrecogg_;
double VertexfitBsCt2DRefit_mmrecogg_;
double VertexfitBsCtMPVRefit_mmrecogg_;
double VertexfitBsCtErr3DRefit_mmrecogg_;
double VertexfitBsCtErr2DRefit_mmrecogg_;
double VertexfitBsCtErrMPVRefit_mmrecogg_;

double VertexfitBsLxy_mmrecogg_;
double VertexfitBsLxyErr_mmrecogg_;
double VertexfitBsCt_mmrecogg_;
double VertexfitBsCtErr_mmrecogg_;

double VertexfitBsErrX_mmrecogg_;
double VertexfitBsErrY_mmrecogg_;
double VertexfitBsErrXY_mmrecogg_;

double VertexfitBsDist3D_mmrecogg_;
double VertexfitBsDist3DErr_mmrecogg_;
double VertexfitBsTime3D_mmrecogg_;
double VertexfitBsTime3DErr_mmrecogg_;

double VertexfitBsDist2D_mmrecogg_;
double VertexfitBsDist2DErr_mmrecogg_;
double VertexfitBsTime2D_mmrecogg_;
double VertexfitBsTime2DErr_mmrecogg_;

double BsVtxProb_mmrecogg_;
double BsCt2D_mmrecogg_;
double BsCt2DBS_mmrecogg_;
double BsCt3D_mmrecogg_;

double DeltaRPhoton1DiMuon_mmrecogg_;
double DeltaRPhoton2DiMuon_mmrecogg_;
double DeltaRPhoton1Photon2_mmrecogg_;

double BsHelicity_mmrecogg_;
double BsCoplanarity_mmrecogg_;

	// mu mu k k

double FourvectorBsMass_mmkk_;
double FourvectorBsPt_mmkk_;
double FourvectorBsEta_mmkk_;
double FourvectorBsPhi_mmkk_;

double VertexfitBsMass_mmkk_;
double VertexfitBsPt_mmkk_;
double VertexfitBsEta_mmkk_;
double VertexfitBsPhi_mmkk_;
double VertexfitBsPz_mmkk_;

double VertexfitBsSVx_mmkk_;
double VertexfitBsSVy_mmkk_;
double VertexfitBsSVz_mmkk_;

double VertexfitPVxRefitCosTheta_mmkk_;
double VertexfitPVyRefitCosTheta_mmkk_;
double VertexfitPVzRefitCosTheta_mmkk_;

double VertexfitPVxRefitClosestZ_mmkk_;
double VertexfitPVyRefitClosestZ_mmkk_;
double VertexfitPVzRefitClosestZ_mmkk_;

double VertexfitBsCt3DPVClosestZ_mmkk_;
double VertexfitBsCt2DPVClosestZ_mmkk_;
double VertexfitBsCt3DPVCosTheta_mmkk_;
double VertexfitBsCt2DPVCosTheta_mmkk_;
double VertexfitBsCt2DPVCosThetaOld_mmkk_;
double VertexfitBsCt2DPVClosestZOld_mmkk_;
double VertexfitBsCt2DBSOld_mmkk_;
double VertexfitBsCt2DOld_mmkk_;

double VertexfitBsCtErr2DBS_mmkk_;
double VertexfitBsCtErr2D_mmkk_;
double VertexfitBsCtErr2DClosestZ_mmkk_;
double VertexfitBsCtErr2DCosTheta_mmkk_;
double VertexfitBsCtErr2DCosThetaOld_mmkk_;
double VertexfitBsCtErr2DBSOld_mmkk_;
double VertexfitBsCtErr2DOld_mmkk_;
double VertexfitBsCtErr2DClosestZOld_mmkk_;
double VertexfitBsCtErr3D_mmkk_;
double VertexfitBsCtErr2D2_mmkk_;
double VertexfitBsCtErrMPV_mmkk_;

double VertexfitBsCt3DRefit_mmkk_;
double VertexfitBsCt2DRefit_mmkk_;
double VertexfitBsCtMPVRefit_mmkk_;
double VertexfitBsCtErr3DRefit_mmkk_;
double VertexfitBsCtErr2DRefit_mmkk_;
double VertexfitBsCtErrMPVRefit_mmkk_;

double VertexfitBsLxy_mmkk_;
double VertexfitBsLxyErr_mmkk_;
double VertexfitBsCt_mmkk_;
double VertexfitBsCtErr_mmkk_;

double VertexfitBsErrX_mmkk_;
double VertexfitBsErrY_mmkk_;
double VertexfitBsErrXY_mmkk_;

double VertexfitBsDist3D_mmkk_;
double VertexfitBsDist3DErr_mmkk_;
double VertexfitBsTime3D_mmkk_;
double VertexfitBsTime3DErr_mmkk_;

double VertexfitBsDist2D_mmkk_;
double VertexfitBsDist2DErr_mmkk_;
double VertexfitBsTime2D_mmkk_;
double VertexfitBsTime2DErr_mmkk_;

double BsVtxProb_mmkk_;
double BsCt2D_mmkk_;
double BsCt2DBS_mmkk_;
double BsCt3D_mmkk_;

double DeltaRKaon1DiMuon_mmkk_;
double DeltaRKaon2DiMuon_mmkk_;

double BsHelicity_mmkk_;
double BsCoplanarity_mmkk_;



double DiMuonMass_Jpsi_;
double DiMuonEta_Jpsi_;
double DiMuonPhi_Jpsi_;
double DiMuonPt_Jpsi_;

double DiMuonMass_Phi_;
double DiMuonEta_Phi_;
double DiMuonPhi_Phi_;
double DiMuonPt_Phi_;

double DiMuonMass_Kstar0_;
double DiMuonEta_Kstar0_;
double DiMuonPhi_Kstar0_;
double DiMuonPt_Kstar0_;

double DiMuonMass_NoBound_;
double DiMuonEta_NoBound_;
double DiMuonPhi_NoBound_;
double DiMuonPt_NoBound_;



	double BsM_beffit_;
	double BsEta_beffit_;
	double BsPhi_beffit_;
	double BsPt_beffit_;
	double Bs_vtxProb_;
	double BsCt_;
	double BsCt3D_;
	double BsCt2D_;
	double BsCt2DBS_;
	double BdCt2DBS_;
	double BdCt2DMC_;
	double BdCt3DMC_;
	double BsCtMPV_;
	double BsCt3Drefit_;
	double BsCt2Drefit_;
	double BsCtMPVrefit_;
	double BsCtErr_;
	double BsCtErr3D_;
	double BsCtErr2D_;
	double BsCtErr2DBS_;
	double BsCtErr2DClosestZ_;
	double BdCtErr2DBS_;
	double BsCtErr2D2_;
	double BsCtErrMPV_;
	double BsCtErr3Drefit_;
	double BsCtErr2Drefit_;
	double BsCtErrMPVrefit_;
	double BsCtErr2DBSOld_;
	double BsCt2DBSOld_;

	double BsCtErr2DClosestZOld_;
	double BsCt2DPVClosestZOld_;
	double BsCtErr2DOld_;
	double BsCt2DOld_;
	double HadronMass_fromVertexFitConPhoton_;
	double HadronMass_fromVertexFitRecoPhoton_;
	int vertexTypeFlag_;

	// Ecal variables
	double PFECal_SC_Eta_;
	double PFECal_SC_Phi_;
	double PFECal_SC_EtaWidth_;
	double PFECal_SC_PhiWidth_;
	double PFECal_SC_ClusterEnergy_;
	uint32_t PFECAL_RecHit_RawId_;
	int PFECAL_RecHit_Subdet_;
	double PFECAL_RecHit_Energy_;
	double PFECAL_RecHit_Time_;
	double PFECAL_RecHit_X_;
	double PFECAL_RecHit_Y_;
	double PFECAL_RecHit_Z_;
	double PFECAL_RecHit_PVDistance_;
	double PFECAL_RecHit_TOF_;
	double PFECAL_RecHit_CorrectedTime_;

	// Flags for problematic RecHits
	bool PFECAL_RecHit_IsWeird_;
	bool PFECAL_RecHit_IsProblematic_;
	bool PFECAL_RecHit_PoorReco_;
	int PFECAL_RecHit_EB_ieta_;
	int PFECAL_RecHit_EB_iphi_;
	double PFECAL_RecHit_EE_ix_;
	double PFECAL_RecHit_EE_iy_;
	double PFECAL_RecHit_EE_zside_;

	int photonMultiplicity_;
	int isFourBody_;
	double photonPt_[2];
	double photonEta_[2];
	double photonPhi_[2];
	double photonEnergy_[2];
	double photonET_[2];
	double photonTrkIso_[2];
	double photonEcalIso_[2];
	double photonHcalIso_[2];
	double photonCaloIso_[2];
	double photonSSSigmaiEtaiEta_[2];
	double photonSSSigmaiEtaiPhi_[2];
	double photonSSSigmaiPhiiPhi_[2];
	double photonSSSigmaEtaEta_[2];
	double photonSSe1x5_[2];
	double photonSSe2x5_[2];
	double photonSSe3x3_[2];
	double photonSSe5x5_[2];
	double photonSShcalDepth1OverEcal_[2];
	double photonSShcalDepth2OverEcal_[2];
	double photonSShcalDepth1OverEcalBc_[2];
	double photonSShcalDepth2OverEcalBc_[2];
	double photonSShcalOverEcal_[7][2];
	double photonSShcalOverEcalBc_[7][2];
	double photonSSmaxEnergyXtal_[2];
	double photonSSeffSigmaRR_[2];
	double photonSCEnergy_[2];
	double photonSCRawEnergy_[2];
	double photonSCPreShowerEP1_[2];
	double photonSCPreShowerEP2_[2];
	double photonSCEta_[2];
	double photonSCPhi_[2];
	double photonSCEtaWidth_[2];
	double photonSCPhiWidth_[2];
	double photonSCBrem_[2];
	double photonSCR9_[2];
	double photonSCHadTowOverEm_[2];

	// Particle flow candidates (photons)
	double pfCandPt_[2];
	double pfCandEta_[2];
	double pfCandPhi_[2];
	double pfCandEnergy_[2];
	double pfCandET_[2];
	double pfCandMass_[2];
	int    pfCandCharge_[2];
	double pfCandEcalEnergy_[2];
	double pfCandRawEcalEnergy_[2];
	double pfCandHcalEnergy_[2];
	double pfCandRawHcalEnergy_[2];
	double pfCandHoEnergy_[2];
	double pfCandRawHoEnergy_[2];
	double pfCandTime_[2];
	double pfCandTrkIso_[2];
	double pfCandEcalIso_[2];
	double pfCandHcalIso_[2];
	double pfCandCaloIso_[2];

	double pfCandRefPhotonPt_[2];
	double pfCandRefPhotonEta_[2];
	double pfCandRefPhotonPhi_[2];
	double pfCandRefPhotonEnergy_[2];
	double pfCandRefPhotonET_[2];
	double pfCandRefPhotonSigmaIEtaIEta_[2];
	double pfCandRefPhotonSigmaIEtaIPhi_[2];
	double pfCandRefPhotonSigmaIPhiIPhi_[2];
	double pfCandRefPhotonSigmaEtaEta_[2];
	double pfCandRefPhotonE1x5_[2];
	double pfCandRefPhotonE2x5_[2];
	double pfCandRefPhotonE3x3_[2];
	double pfCandRefPhotonE5x5_[2];
	double pfCandRefPhotonHcalDepth1OverEcal_[2];
	double pfCandRefPhotonHcalDepth2OverEcal_[2];
	double pfCandRefPhotonHcalDepth1OverEcalBc_[2];
	double pfCandRefPhotonHcalDepth2OverEcalBc_[2];
	double pfCandRefPhotonScEnergy_[2];
	double pfCandRefPhotonScRawEnergy_[2];
	double pfCandRefPhotonScEta_[2];
	double pfCandRefPhotonScPhi_[2];
	double pfCandRefPhotonScEtaWidth_[2];
	double pfCandRefPhotonScPhiWidth_[2];
	double pfCandRefPhotonScBrem_[2];
	double pfCandRefPhotonR9_[2];
	double pfCandRefPhotonHadTowOverEm_[2];
	double pfCandRefPhotonMaxEnergyXtal_[2];
	double pfCandRefPhotonEffSigmaRR_[2];
	double pfCandRefPhotonHcalOverEcal_[7][2];
	double pfCandRefPhotonHcalOverEcalBc_[7][2];

	double DiGammaM_alone_;
	double DiGammaEta_alone_;
	double DiGammaPhi_alone_;
	double DiGammaPt_alone_;

	double Bsmass_recommg_;
	double Bspt_recommg_;
	double Bseta_recommg_;
	double Bsphi_recommg_;
	double Bshelicity_recommg_;
	double Bscoplanarity_recommg_;

	double Bsmass_recommgg_;
	double Bspt_recommgg_;
	double Bseta_recommgg_;
	double Bsphi_recommgg_;
	double Bshelicity_recommgg_;
	double Bscoplanarity_recommgg_;
	int BmesonType_;

	int electronMultiplicity_;
	int pfCandMultiplicity_;

	double costheta_;
	double phi_;
	double cospsi_;
	double AngleBsDecayLength_;
	double JpsiGenLxy_;
	double JpsiGenPVz_;
	double JpsiGenPVy_;
	double JpsiGenPVx_;
	double JpsiGenPt_;
	double JpsiGenLxyOld_;
	double JpsiGenLxyOverPt_;
	int JpsiGenNumberOfCandidates_;

	//Gen Particle information
	int     GenMotherID_;
	double  GenB_pt_;
	double  GenB_eta_;
	double  GenB_phi_;
	double  GenB_mass_;
	double  GenBLxy_;
	double  GenBct_;
	double  GenBctErr_;
	double  GenMuPlus_pt_;
	double  GenMuPlus_eta_;
	double  GenMuPlus_phi_;
	double  GenMuPlus_E_;
	double  GenMuMinus_pt_;
	double  GenMuMinus_eta_;
	double  GenMuMinus_phi_;
	double  GenMuMinus_E_;
	double  GenDimuon_mass_;
	double  GenDimuon_pt_;
	int     GenNPhotons_;
	double  GenGamma_pt_[4];
	double  GenGamma_eta_[4];
	double  GenGamma_phi_[4];
	double  GenGamma_E_[4];
	int     GenGamma_origin_[4];
	int     GenGamma_motherId_[4];
	double  GenGamma_dRmu1_[4];
	double  GenGamma_dRmu2_[4];
	double  GenHelicity3_;
	double  GenCoplanarity3_;
	double  GenCosTheta_l_;
	double  GenTriBodyMass_;
	double  GenHelicity4_;
	double  GenCoplanarity4_;
	double  GenFourBodyMass_;
	double  GenDiphoton_mass_;
	double  GenDiphoton_pt_;
	bool    GenHasJpsi_;
	double  GenJpsi_mass_;
	double  GenJpsi_pt_;
	bool    GenHasNeutralMeson_;
	int     GenNeutralMesonId_;
	int     GenDecayMode_;
	bool    GenIs3Body_;
	bool    GenIs4Body_;
	bool    GenIsHardProcess_;
	bool    GenIsSignal_;


	//Truth matched 
	bool   Mu1_isMatched_;
	int    Mu1_genPdgId_;
	int    Mu1_genMotherPdgId_;
	int    Mu1_genGMotherPdgId_;
	float  Mu1_genDeltaR_;
	float  Mu1_genDeltaPt_;
	bool   Mu2_isMatched_;
	int    Mu2_genPdgId_;
	int    Mu2_genMotherPdgId_;
	int    Mu2_genGMotherPdgId_;
	float  Mu2_genDeltaR_;
	float  Mu2_genDeltaPt_;
	bool   DiMuon_isMatched_;
	bool   Photon_isMatched_[4];
	int    Photon_genPdgId_[4];
	int    Photon_genMotherPdgId_[4];
	int    Photon_genGMotherPdgId_[4];
	float  Photon_genDeltaR_[4];
	float  Photon_genDeltaPt_[4];
	int    Photon_truthOrigin_[4];
	int    NMatchedPhotons_;
	bool   Topology_isMatched_;
	bool   Topology_mu1Matched_;
	bool   Topology_mu2Matched_;
	bool   Topology_photon1Matched_;
	bool   Topology_photon2Matched_;
	bool   Topology_decayModeMatched_;
	bool   IsSignalMatched_;
	bool   IsMuonsFromJpsi_;
	bool   IsMuonsDirectFromBs_;

	// arrays
	double PVcovariance_[9];
	TFile* bmmgFile_;
	TTree* bmmgTree_;
};

#endif
