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
		int Mu1PixelHits_ ;
		int Mu1TrackerHits_;
		int Mu1isGood_ ;
		int Mu1InnerTrkHighQuality_ ;  
		int Mu2PixelHits_ ;
		int Mu2TrackerHits_ ;
		int Mu2isGood_ ;
		int Mu2InnerTrkHighQuality_ ;
		double mu1MVAScore_;
		double mu2MVAScore_;

		
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


  //Ecal variables
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
		
		
		//Particle flow candiates (photons)
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
		double Bshelicity_recommg_;
		double Bscoplanarity_recommg_;
		

		

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
		 //arrays 
		 double PVcovariance_[9];
		 TFile* bmmgFile_;
		 TTree* bmmgTree_;
		 
};
#endif


