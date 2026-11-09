#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_OutputTree_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_OutputTree_h

#include <string>
#include <vector>
#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#include <TChain.h>

#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BeamSpotAndVertex.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TripleObjectVertexFit.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/GenChain.h"


class OutputTree {
public:
    OutputTree();
    ~OutputTree();

    void resetAllEntries();
    void resetDecayEntries();

    void createTree(const std::string filename);
    void fill();
    void fillSummary();     // one entry per job: generator cross section, filter efficiency, equivalent lumi
   	void writeFile();

    void addBSAndVtxObservables(const BeamSpotAndVertex::BSAndVtxVariables& vars);
    void addMmgFitResults(const TripleObjectVertexFit::MmgFitResults& results);
    void addGenInfo(const GenDecayChain::GenInfo& gen);     // MC study only
    // muon kinematics and IDs; soft and tight IDs are evaluated w.r.t. pv
    void addMuonPair(const reco::Muon& mu1, const reco::Muon& mu2, const reco::Vertex& pv);

    // Event metadata
    int runNumber_ = -999;
    unsigned int eventNumber_ = 0;
    int lumiSection_ = -999;
    
    // Pileup
    int PUinteraction_ = -999;
    int PUTrueinteraction_ = -999;
    
    // Event multiplicities
    int nMuons_ = -999;
    int nPhotons_ = -999;
    int nTracks_ = -999;
    
    // Muon-pair variables
    double mu1MVAScore_ = -9999.0;
    double mu2MVAScore_ = -9999.0;
    double mu1Pt_ = -9999.0;
    double mu1Eta_ = -9999.0;
    double mu1Phi_ = -9999.0;
    int mu1Charge_ = -999;
    int mu1IsGlobal_ = -999;
    int mu1IsTracker_ = -999;
    int mu1IsSoft_ = -999;
    int mu1IsTight_ = -999;
    double mu2Pt_ = -9999.0;
    double mu2Eta_ = -9999.0;
    double mu2Phi_ = -9999.0;
    int mu2Charge_ = -999;
    int mu2IsGlobal_ = -999;
    int mu2IsTracker_ = -999;
    int mu2IsSoft_ = -999;
    int mu2IsTight_ = -999;
    double DiMuon_KalmanVtxProb_ = -9999.0;
    double mu1GenDeltaR_ = -9999.0;     // MC study only
    double mu2GenDeltaR_ = -9999.0;     // MC study only

    // Trigger bits
    int triggerbit_HLT_DoubleMu4_3_Displaced_Photon4_BsToMMG_ = 0;
    int triggerbit_HLT_DoubleMu4_3_Photon4_BsToMMG_ = 0;
    int triggerbit_HLT_DoubleMu4_LowMass_Displaced_ = 0;
    int triggerbit_HLT_DoubleMu4_3_LowMass_ = 0;
    int triggerbit_HLT_DoubleMu4_3_Bs_ = 0;
    int triggerbit_HLT_DoubleMu4_JpsiTrkTrk_Displaced_ = 0;
    int triggerbit_HLT_DoubleMu4_Jpsi_Displaced_ = 0;
    int triggerbit_HLT_DoubleMu4_Jpsi_NoVertexing_ = 0;
    int triggerbit_HLT_DoubleMu2_Jpsi_LowPt_ = 0;
    int triggerbit_HLT_DoubleMu4_3_Jpsi_ = 0;
    
    // Generator summary
    double GenXsec_ = -9999.0;
    double GenXsecErr_ = -9999.0;
    double FilterEff_ = -9999.0;
    double FilterEffErr_ = -9999.0;
    int nEvents_ = -999;
    double LumiEquivalent_ = -9999.0;
    double LumiEquivalentErr_ = -9999.0;
    
    // Generator decay chain (MC study only; event level)
    int genDecayMode_ = -999;
    int genBPdgId_ = -999;
    int genIsHardProcess_ = -999;
    double genBPt_ = -9999.0;
    double genBEta_ = -9999.0;
    double genBPhi_ = -9999.0;
    double genBMass_ = -9999.0;

    double genMuPlusPt_ = -9999.0;
    double genMuPlusEta_ = -9999.0;
    double genMuPlusPhi_ = -9999.0;
    double genMuMinusPt_ = -9999.0;
    double genMuMinusEta_ = -9999.0;
    double genMuMinusPhi_ = -9999.0;
    double genPhotonPt_ = -9999.0;       // signal photon
    double genPhotonEta_ = -9999.0;
    double genPhotonPhi_ = -9999.0;
    int genNFSRPhotons_ = -999;
    double genDimuonMass_ = -9999.0;
    double genMumugMass_ = -9999.0;

    double genBProdVtxX_ = -9999.0;      // B production vertex
    double genBProdVtxY_ = -9999.0;
    double genBProdVtxZ_ = -9999.0;
    double genBDecayVtxX_ = -9999.0;     // B decay vertex
    double genBDecayVtxY_ = -9999.0;
    double genBDecayVtxZ_ = -9999.0;
    double genLxy_ = -9999.0;            // transverse decay length
    double genCt_ = -9999.0;             // proper decay length: L(3D) * m / p

    // Beam spot and primary vertex variables (event level)
    int NVerticesbeforecut_ = -999;
    int NVerticesaftercut_ = -999;

    double BSx_ = -9999.0;
    double BSy_ = -9999.0;
    double BSz_ = -9999.0;    
    double BSdx_ = -9999.0;
    double BSdy_ = -9999.0;
    double BSdz_ = -9999.0;
    double BSdxdz_ = -9999.0;
    double BSdydz_ = -9999.0;
    double BSsigmaZ_ = -9999.0;
    double BSdsigmaZ_ = -9999.0;

    
    // MmgFitResults: dimuon variables
    int resonanceFlag_ = -999;
    double dimuonVtxProb_ = -9999.0;
    double dimuonMass_ = -9999.0;       // measured
    double dimuonFitMass_ = -9999.0;    // after the dimuon kinematic fit
    double dimuonEta_ = -9999.0;
    double dimuonPhi_ = -9999.0;
    double dimuonPt_ = -9999.0;

    // MmgFitResults: photon variables
    std::vector<double> photonDeltaRs_;
    std::vector<double> photonPt_;
    std::vector<double> photonEta_;
    std::vector<double> photonPhi_;
    std::vector<double> photonEnergy_;
    std::vector<double> photonMVA_;
    std::vector<double> photonDimuonMass_;
    std::vector<double> photonDimuonPt_;
    std::vector<double> photonDimuonEta_;
    std::vector<double> photonDimuonPhi_;
    std::vector<double> photonDimuonLxy_;
    std::vector<double> photonDimuonLxySig_;
    std::vector<double> photonDimuonCosAlpha_;
    std::vector<double> photonDimuonCosAlpha2D_;
    std::vector<double> photonGenDeltaR_;

    // MmgFitResults: primary vertex used in the Bs fit (one entry per photon)
    std::vector<int> PVindex_;
    std::vector<double> PVx_;
    std::vector<double> PVy_;
    std::vector<double> PVz_;
    std::vector<double> PVerrx_;
    std::vector<double> PVerry_;
    std::vector<double> PVerrz_;
    std::vector<double> PVcovXX_;
    std::vector<double> PVcovXY_;
    std::vector<double> PVcovXZ_;
    std::vector<double> PVcovYY_;
    std::vector<double> PVcovYZ_;
    std::vector<double> PVcovZZ_;
    std::vector<double> PVndof_;
    std::vector<double> PVrho_;
    std::vector<int> PVntracks_;

    // MmgFitResults: Bs variables
    std::vector<double> bsVtxProb_;
    std::vector<double> bsVtxX_;
    std::vector<double> bsVtxY_;
    std::vector<double> bsVtxZ_;
    std::vector<double> bsLxy_;
    std::vector<double> bsLxySig_;
    std::vector<double> bsCt_;
    std::vector<double> bsMass_;
    std::vector<double> bsEnergy_;
    std::vector<double> bsPt_;
    std::vector<double> bsEta_;
    std::vector<double> bsPhi_;
    std::vector<bool> bsFitSuccessful_;


private:
    std::vector<double>* photonDeltaRsPtr_ = &photonDeltaRs_;
    std::vector<double>* photonPtPtr_ = &photonPt_;
    std::vector<double>* photonEtaPtr_ = &photonEta_;
    std::vector<double>* photonPhiPtr_ = &photonPhi_;
    std::vector<double>* photonEnergyPtr_ = &photonEnergy_;
    std::vector<double>* photonMVAPtr_ = &photonMVA_;
    std::vector<double>* photonDimuonMassPtr_ = &photonDimuonMass_;
    std::vector<double>* photonDimuonPtPtr_ = &photonDimuonPt_;
    std::vector<double>* photonDimuonEtaPtr_ = &photonDimuonEta_;
    std::vector<double>* photonDimuonPhiPtr_ = &photonDimuonPhi_;
    std::vector<double>* photonDimuonLxyPtr_ = &photonDimuonLxy_;
    std::vector<double>* photonDimuonLxySigPtr_ = &photonDimuonLxySig_;
    std::vector<double>* photonDimuonCosAlphaPtr_ = &photonDimuonCosAlpha_;
    std::vector<double>* photonDimuonCosAlpha2DPtr_ = &photonDimuonCosAlpha2D_;
    std::vector<double>* photonGenDeltaRPtr_ = &photonGenDeltaR_;

    std::vector<int>* PVindexPtr_ = &PVindex_;
    std::vector<double>* PVxPtr_ = &PVx_;
    std::vector<double>* PVyPtr_ = &PVy_;
    std::vector<double>* PVzPtr_ = &PVz_;
    std::vector<double>* PVerrxPtr_ = &PVerrx_;
    std::vector<double>* PVerryPtr_ = &PVerry_;
    std::vector<double>* PVerrzPtr_ = &PVerrz_;
    std::vector<double>* PVcovXXPtr_ = &PVcovXX_;
    std::vector<double>* PVcovXYPtr_ = &PVcovXY_;
    std::vector<double>* PVcovXZPtr_ = &PVcovXZ_;
    std::vector<double>* PVcovYYPtr_ = &PVcovYY_;
    std::vector<double>* PVcovYZPtr_ = &PVcovYZ_;
    std::vector<double>* PVcovZZPtr_ = &PVcovZZ_;
    std::vector<double>* PVndofPtr_ = &PVndof_;
    std::vector<double>* PVrhoPtr_ = &PVrho_;
    std::vector<int>* PVntracksPtr_ = &PVntracks_;

    std::vector<double>* bsVtxProbPtr_ = &bsVtxProb_;
    std::vector<double>* bsVtxXPtr_ = &bsVtxX_;
    std::vector<double>* bsVtxYPtr_ = &bsVtxY_;
    std::vector<double>* bsVtxZPtr_ = &bsVtxZ_;
    std::vector<double>* bsLxyPtr_ = &bsLxy_;
    std::vector<double>* bsLxySigPtr_ = &bsLxySig_;
    std::vector<double>* bsCtPtr_ = &bsCt_;
    std::vector<double>* bsMassPtr_ = &bsMass_;
    std::vector<double>* bsEnergyPtr_ = &bsEnergy_;
    std::vector<double>* bsPtPtr_ = &bsPt_;
    std::vector<double>* bsEtaPtr_ = &bsEta_;
    std::vector<double>* bsPhiPtr_ = &bsPhi_;
    std::vector<bool>* bsFitSuccessfulPtr_ = &bsFitSuccessful_;

    TFile* treeFile_;
    TTree* tree_;
    TTree* summaryTree_;
};

#endif