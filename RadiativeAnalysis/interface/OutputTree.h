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
    double DiMuon_KalmanVtxProb_ = -9999.0;
    
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
    
    // Generator J/psi variables
    int JpsiGenNumberOfCandidates_ = 0;
    double JpsiGenPVx_ = -9999.0;
    double JpsiGenPVy_ = -9999.0;
    double JpsiGenPVz_ = -9999.0;
    double JpsiGenLxyOld_ = -9999.0;
    double JpsiGenLxy_ = -9999.0;
    double JpsiGenLxyOverPt_ = -9999.0;
    double JpsiGenPt_ = -9999.0;

    // Beam spot and primary vertex variables
    int NTracksPVbeforecut_ = -999;
    int NVerticesbeforecut_ = -999;
    int NVerticesaftercut_ = -999;
    
    double PVx_ = -9999.0;
    double PVy_ = -9999.0;
    double PVz_ = -9999.0;    
    double PVerrx_ = -9999.0;
    double PVerry_ = -9999.0;
    double PVerrz_ = -9999.0;
    double PVcovariance_[9] = {};    
    double PVndof_ = -9999.0;
    double PVrho_ = -9999.0;
    
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
    double dimuonMass_ = -9999.0;
    double dimuonEta_ = -9999.0;
    double dimuonPhi_ = -9999.0;
    double dimuonPt_ = -9999.0;

    // MmgFitResults: photon variables
    std::vector<double> photonDeltaRs_;
    std::vector<double> photonPt_;
    std::vector<double> photonEta_;
    std::vector<double> photonPhi_;
    std::vector<double> photonEnergy_;
    std::vector<double> photonDimuonMass_;
    std::vector<double> photonDimuonPt_;
    std::vector<double> photonDimuonEta_;
    std::vector<double> photonDimuonPhi_;

    // MmgFitResults: Bs variables
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
    std::vector<double>* photonDimuonMassPtr_ = &photonDimuonMass_;
    std::vector<double>* photonDimuonPtPtr_ = &photonDimuonPt_;
    std::vector<double>* photonDimuonEtaPtr_ = &photonDimuonEta_;
    std::vector<double>* photonDimuonPhiPtr_ = &photonDimuonPhi_;

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