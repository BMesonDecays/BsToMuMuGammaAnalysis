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
		 int    runNumber_;
                 int    PUinteraction_;
		 int    PUTrueinteraction_;
                 unsigned int eventNumber_;
                 int    lumiSection_;
		 int    isPV_;
		 int    NVerticesbeforecut_;
		 int    NVerticesaftercut_;
		 int    NTracksPVbeforecut_;
		 int    isBS_;
		 double BSx_ ;
		 double BSy_ ;
		 double BSz_ ;
		 double BSdx_ ;
		 double BSdy_ ;
		 double BSdz_ ;
		 double BSdydz_;
		 double BSdxdz_;
		 double BSsigmaZ_;
		 double BSdsigmaZ_;
		 double PVx_ ;
		 double PVy_ ;
		 double PVz_ ;
		 double PVerrx_ ;
		 double PVerry_ ;
		 double PVerrz_ ;
		 double PVndof_;
		 double PVrho_;
		 double dedxTrk_;
		 double errdedxTrk_;
		 int    numdedxTrk_;
		 int    triggerbit_HLT_DoubleMu4_LowMass_Displaced_;
		 int    triggerbit_HLT_DoubleMu4_LowMass_;
		 int    triggerbit_HLT_DoubleMu4_3_Bs_;
		 int    triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_;
		 int    triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_;
		 int    triggerbit_HLTDimuon4JpsiDisplaced_;
		 int    triggerbit_HLTDimuon4JpsiNoVertexing_;
		 int    triggerbit_HLTDimuon4JpsiTrkTrkDisplaced_;
		 int    photonMultiplicity_;
                 double photonPt_;
                 double photonEta_;
                 double photonPhi_;
		 double photonEnergy_;
		 double photonET_;
		 double photonTrkIso_;
		 double photonEcalIso_;
		 double photonHcalIso_;
		 double photonCaloIso_;
		 double photonSSSigmaiEtaiEta_;
		 double photonSSSigmaiEtaiPhi_;
		 double photonSSSigmaiPhiiPhi_;
		 double photonSSSigmaEtaEta_;
		 double photonSSe1x5_;
		 double photonSSe2x5_;
		 double photonSSe3x3_;
		 double photonSSe5x5_;
		 double photonSShcalDepth1OverEcal_;
		 double photonSShcalDepth2OverEcal_;
		 double photonSShcalDepth1OverEcalBc_;
		 double photonSShcalDepth2OverEcalBc_;
		 double photonSShcalOverEcal_[7];
		 double photonSShcalOverEcalBc_[7];
		 double photonSSmaxEnergyXtal_;
		 double photonSSeffSigmaRR_;
		 double photonSCEnergy_;
		 double photonSCRawEnergy_;
		 double photonSCPreShowerEP1_;
		 double photonSCPreShowerEP2_;
		 double photonSCEta_;
		 double photonSCPhi_;
		 double photonSCEtaWidth_;
		 double photonSCPhiWidth_;
		 double photonSCBrem_;
		 double photonSCR9_;
		 double photonSCHadTowOverEm_;
		 double PiZeroM_alone_;
		 double PiZeroEta_alone_;
		 double PiZeroPhi_alone_;
		 double PiZeroPt_alone_;
		 double EtaMesonM_alone_;
		 double EtaMesonEta_alone_;
		 double EtaMesonPhi_alone_;
		 double EtaMesonPt_alone_;
		 double EtaPrimeM_alone_;
		 double EtaPrimeEta_alone_;
		 double EtaPrimePhi_alone_;
		 double EtaPrimePt_alone_;


		 double K1Pt_beffit_;
                 double K1Pz_beffit_;
                 double K1Eta_beffit_;
                 double K1Phi_beffit_;
                 double K2Pt_beffit_;
                 double K2Pz_beffit_;
                 double K2Eta_beffit_;
                 double K2Phi_beffit_;

		 double PhiM_beffit_;
		 double PhiEta_beffit_;
		 double PhiPhi_beffit_;
		 double PhiPt_beffit_;

		 double BsPhiGammaM_beffit_;
		 double BsPhiGammaEta_beffit_;
		 double BsPhiGammaPhi_beffit_;
		 double BsPhiGammaPt_beffit_;
		 double BsPhiGamma_vtxProb_;
		 double BsPhiGamma_CosineAlpha_;
		 double BsPhiGamma_KKDCA_;
		 double BsPhiGamma_Chi2pv_KVFvtx_;
		 double BsPhiGamma_Mahalanobis_;
		 double BsPhiGamma_K1Pt_fit_;
		 double BsPhiGamma_K2Pt_fit_;
		 double BsPhiGamma_PhiM_fit_;




		 int    electronMultiplicity_;
		 int    pfCandMultiplicity_;

		 double costheta_;
		 double phi_;
		 double cospsi_;
		 double AngleBsDecayLength_;


                 double JpsiGenLxy_;
                 double JpsiGenPVz_;
                 double JpsiGenPVy_;
                 double JpsiGenPVx_;
		 double JpsiGenPt_;
		 double JpsiGenLxyOld_ ;
		 double JpsiGenLxyOverPt_;
		 int    JpsiGenNumberOfCandidates_;
		 //arrays 
		 double PVcovariance_[9];
		 TFile* bmmgFile_;
		 TTree* bmmgTree_;
};
#endif


