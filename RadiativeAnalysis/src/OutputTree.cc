#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/OutputTree.h"

OutputTree::OutputTree()
    : treeFile_(nullptr),
      tree_(nullptr),
      summaryTree_(nullptr)
{
    resetAllEntries();
}

OutputTree::~OutputTree() = default;

void OutputTree::writeFile()
{
    treeFile_->Write();
    treeFile_->Close();
    // delete treeFile_;
}

void OutputTree::fill()
{
    tree_->Fill();
}

void OutputTree::fillSummary()
{
    summaryTree_->Fill();
}

void OutputTree::createTree(const std::string filename)
{
    treeFile_ = new TFile(filename.c_str(), "RECREATE" );
    // int bufsize = 256000;
    tree_ = new TTree("tree","Output tree");    //,bufsize);

    tree_->Branch("runNumber", &runNumber_, "runNumber/I");
    tree_->Branch("eventNumber", &eventNumber_, "eventNumber/i");
    tree_->Branch("lumiSection", &lumiSection_, "lumiSection/I");
    
    tree_->Branch("PUinteraction", &PUinteraction_, "PUinteraction/I");
    tree_->Branch("PUTrueinteraction", &PUTrueinteraction_, "PUTrueinteraction/I");
    
    tree_->Branch("nMuons", &nMuons_, "nMuons/I");
    tree_->Branch("nPhotons", &nPhotons_, "nPhotons/I");
    tree_->Branch("nTracks", &nTracks_, "nTracks/I");
    
    tree_->Branch("mu1MVAScore", &mu1MVAScore_, "mu1MVAScore/D");
    tree_->Branch("mu2MVAScore", &mu2MVAScore_, "mu2MVAScore/D");
    tree_->Branch("DiMuon_KalmanVtxProb", &DiMuon_KalmanVtxProb_, "DiMuon_KalmanVtxProb/D");
    
    // Generator summary: filled once per job in a separate tree
    summaryTree_ = new TTree("summary","Generator summary");
    summaryTree_->Branch("GenXsec", &GenXsec_, "GenXsec/D");
    summaryTree_->Branch("GenXsecErr", &GenXsecErr_, "GenXsecErr/D");
    summaryTree_->Branch("FilterEff", &FilterEff_, "FilterEff/D");
    summaryTree_->Branch("FilterEffErr", &FilterEffErr_, "FilterEffErr/D");
    summaryTree_->Branch("nEvents", &nEvents_, "nEvents/I");
    summaryTree_->Branch("LumiEquivalent", &LumiEquivalent_, "LumiEquivalent/D");
    summaryTree_->Branch("LumiEquivalentErr", &LumiEquivalentErr_, "LumiEquivalentErr/D");
    
    tree_->Branch("JpsiGenNumberOfCandidates", &JpsiGenNumberOfCandidates_, "JpsiGenNumberOfCandidates/I");
    tree_->Branch("JpsiGenPVx", &JpsiGenPVx_, "JpsiGenPVx/D");
    tree_->Branch("JpsiGenPVy", &JpsiGenPVy_, "JpsiGenPVy/D");
    tree_->Branch("JpsiGenPVz", &JpsiGenPVz_, "JpsiGenPVz/D");
    tree_->Branch("JpsiGenLxyOld", &JpsiGenLxyOld_, "JpsiGenLxyOld/D");
    tree_->Branch("JpsiGenLxy", &JpsiGenLxy_, "JpsiGenLxy/D");
    tree_->Branch("JpsiGenLxyOverPt", &JpsiGenLxyOverPt_, "JpsiGenLxyOverPt/D");
    tree_->Branch("JpsiGenPt", &JpsiGenPt_, "JpsiGenPt/D");

    // Beam spot and primary vertex branches
    tree_->Branch("NTracksPVbeforecut", &NTracksPVbeforecut_, "NTracksPVbeforecut/I");    
    tree_->Branch("NVerticesbeforecut", &NVerticesbeforecut_, "NVerticesbeforecut/I");    
    tree_->Branch("NVerticesaftercut", &NVerticesaftercut_, "NVerticesaftercut/I");
    
    tree_->Branch("PVx", &PVx_, "PVx/D");
    tree_->Branch("PVy", &PVy_, "PVy/D");
    tree_->Branch("PVz", &PVz_, "PVz/D");    
    tree_->Branch("PVerrx", &PVerrx_, "PVerrx/D");
    tree_->Branch("PVerry", &PVerry_, "PVerry/D");
    tree_->Branch("PVerrz", &PVerrz_, "PVerrz/D");    
    tree_->Branch("PVcovariance", PVcovariance_, "PVcovariance[9]/D");
    tree_->Branch("PVndof", &PVndof_, "PVndof/D");
    tree_->Branch("PVrho", &PVrho_, "PVrho/D");
    
    tree_->Branch("BSx", &BSx_, "BSx/D");
    tree_->Branch("BSy", &BSy_, "BSy/D");
    tree_->Branch("BSz", &BSz_, "BSz/D");    
    tree_->Branch("BSdx", &BSdx_, "BSdx/D");
    tree_->Branch("BSdy", &BSdy_, "BSdy/D");
    tree_->Branch("BSdz", &BSdz_, "BSdz/D");
    tree_->Branch("BSdxdz", &BSdxdz_, "BSdxdz/D");
    tree_->Branch("BSdydz", &BSdydz_, "BSdydz/D");    
    tree_->Branch("BSsigmaZ", &BSsigmaZ_, "BSsigmaZ/D");
    tree_->Branch("BSdsigmaZ", &BSdsigmaZ_, "BSdsigmaZ/D");
    
    // Dimuon, photon and Bs meson branches
    tree_->Branch("resonanceFlag", &resonanceFlag_, "resonanceFlag/I");
    tree_->Branch("dimuonVtxProb", &dimuonVtxProb_, "dimuonVtxProb/D");
    tree_->Branch("dimuonMass", &dimuonMass_, "dimuonMass/D");
    tree_->Branch("dimuonEta", &dimuonEta_, "dimuonEta/D");
    tree_->Branch("dimuonPhi", &dimuonPhi_, "dimuonPhi/D");
    tree_->Branch("dimuonPt", &dimuonPt_, "dimuonPt/D");
    
    tree_->Branch("photonDeltaRs", &photonDeltaRsPtr_);
    tree_->Branch("photonPt", &photonPtPtr_);
    tree_->Branch("photonEta", &photonEtaPtr_);
    tree_->Branch("photonPhi", &photonPhiPtr_);
    tree_->Branch("photonEnergy", &photonEnergyPtr_);

    tree_->Branch("photonDimuonMass", &photonDimuonMassPtr_);
    tree_->Branch("photonDimuonPt", &photonDimuonPtPtr_);
    tree_->Branch("photonDimuonEta", &photonDimuonEtaPtr_);
    tree_->Branch("photonDimuonPhi", &photonDimuonPhiPtr_);

    tree_->Branch("bsMass", &bsMassPtr_);
    tree_->Branch("bsEnergy", &bsEnergyPtr_);
    tree_->Branch("bsPt", &bsPtPtr_);
    tree_->Branch("bsEta", &bsEtaPtr_);
    tree_->Branch("bsPhi", &bsPhiPtr_);
    tree_->Branch("bsFitSuccessful", &bsFitSuccessfulPtr_);
}


void OutputTree::resetAllEntries()
{
    resetDecayEntries();

    runNumber_ = -999;
    eventNumber_ = 0;
    lumiSection_ = -999;
    
    PUinteraction_ = -999;
    PUTrueinteraction_ = -999;
    
    nMuons_ = -999;
    nPhotons_ = -999;
    nTracks_ = -999;
    
    mu1MVAScore_ = -9999.0;
    mu2MVAScore_ = -9999.0;
    DiMuon_KalmanVtxProb_ = -9999.0;
    
    GenXsec_ = -9999.0;
    GenXsecErr_ = -9999.0;
    FilterEff_ = -9999.0;
    FilterEffErr_ = -9999.0;
    nEvents_ = -999;
    LumiEquivalent_ = -9999.0;
    LumiEquivalentErr_ = -9999.0;
    
    JpsiGenNumberOfCandidates_ = 0;
    JpsiGenPVx_ = -9999.0;
    JpsiGenPVy_ = -9999.0;
    JpsiGenPVz_ = -9999.0;
    JpsiGenLxyOld_ = -9999.0;
    JpsiGenLxy_ = -9999.0;
    JpsiGenLxyOverPt_ = -9999.0;
    JpsiGenPt_ = -9999.0;

    // Primary-vertex entries
    NTracksPVbeforecut_ = -999;
    NVerticesbeforecut_ = -999;
    NVerticesaftercut_ = -999;

    PVx_ = -9999.0;
    PVy_ = -9999.0;
    PVz_ = -9999.0;

    PVerrx_ = -9999.0;
    PVerry_ = -9999.0;
    PVerrz_ = -9999.0;

    for (int i = 0; i < 9; ++i) {
        PVcovariance_[i] = -9999.0;
    }

    PVndof_ = -9999.0;
    PVrho_ = -9999.0;

    // Beam-spot entries
    BSx_ = -9999.0;
    BSy_ = -9999.0;
    BSz_ = -9999.0;

    BSdx_ = -9999.0;
    BSdy_ = -9999.0;
    BSdz_ = -9999.0;
    BSdxdz_ = -9999.0;
    BSdydz_ = -9999.0;
    BSsigmaZ_ = -9999.0;
    BSdsigmaZ_ = -9999.0;
}


void OutputTree::resetDecayEntries()
{
    // Dimuon branches
    resonanceFlag_ = -999;
    dimuonVtxProb_ = -9999.0;
    dimuonMass_ = -9999.0;
    dimuonEta_ = -9999.0;
    dimuonPhi_ = -9999.0;
    dimuonPt_ = -9999.0;

    // Photon branches
    photonDeltaRs_.clear();
    photonPt_.clear();
    photonEta_.clear();
    photonPhi_.clear();
    photonEnergy_.clear();
    photonDimuonMass_.clear();
    photonDimuonPt_.clear();
    photonDimuonEta_.clear();
    photonDimuonPhi_.clear();

    // Bs branches
    bsMass_.clear();
    bsEnergy_.clear();
    bsPt_.clear();
    bsEta_.clear();
    bsPhi_.clear();
    bsFitSuccessful_.clear();
}

void OutputTree::addBSAndVtxObservables(const BeamSpotAndVertex::BSAndVtxVariables& vars)
{
    NTracksPVbeforecut_ = vars.pv_ntracks;
    NVerticesbeforecut_ = vars.pv_multiplicity;
    NVerticesaftercut_ = vars.pv_cutmultiplicity;

    for (int i = 0; i < 9; ++i) {
        PVcovariance_[i] = vars.pv_covariance[i];
    }

    PVndof_ = vars.pv_ndof;
    PVrho_ = vars.pv_rho;

    BSx_ = vars.bs_x;
    BSy_ = vars.bs_y;
    BSz_ = vars.bs_z;

    PVx_ = vars.pv_x;
    PVy_ = vars.pv_y;
    PVz_ = vars.pv_z;

    PVerrx_ = vars.pv_errx;
    PVerry_ = vars.pv_erry;
    PVerrz_ = vars.pv_errz;

    BSdx_ = vars.bs_dx;
    BSdy_ = vars.bs_dy;
    BSdz_ = vars.bs_dz;
    BSsigmaZ_ = vars.bs_sigmaZ;
    BSdsigmaZ_ = vars.bs_dsigmaZ;
    BSdxdz_ = vars.bs_dxdz;
    BSdydz_ = vars.bs_dydz;
}

void OutputTree::addMmgFitResults(
    const TripleObjectVertexFit::MmgFitResults& results)
{
    // Scalar branches
    resonanceFlag_ = results.resonanceFlag;
    dimuonVtxProb_ = results.dimuonVtxProb;
    dimuonMass_ = results.dimuonMass;
    dimuonEta_ = results.dimuonEta;
    dimuonPhi_ = results.dimuonPhi;
    dimuonPt_ = results.dimuonPt;

    // Vector branches
    photonDeltaRs_.insert(photonDeltaRs_.end(), results.photonDeltaRs.begin(), results.photonDeltaRs.end());
    photonPt_.insert(photonPt_.end(), results.photonPt.begin(), results.photonPt.end());
    photonEta_.insert(photonEta_.end(), results.photonEta.begin(), results.photonEta.end());
    photonPhi_.insert(photonPhi_.end(), results.photonPhi.begin(), results.photonPhi.end());
    photonEnergy_.insert(photonEnergy_.end(), results.photonEnergy.begin(), results.photonEnergy.end());

    photonDimuonMass_.insert(photonDimuonMass_.end(), results.photonDimuonMass.begin(), results.photonDimuonMass.end());
    photonDimuonPt_.insert(photonDimuonPt_.end(), results.photonDimuonPt.begin(), results.photonDimuonPt.end());
    photonDimuonEta_.insert(photonDimuonEta_.end(), results.photonDimuonEta.begin(), results.photonDimuonEta.end());
    photonDimuonPhi_.insert(photonDimuonPhi_.end(), results.photonDimuonPhi.begin(), results.photonDimuonPhi.end());

    bsMass_.insert(bsMass_.end(), results.bsMass.begin(), results.bsMass.end());
    bsEnergy_.insert(bsEnergy_.end(), results.bsEnergy.begin(), results.bsEnergy.end());
    bsPt_.insert(bsPt_.end(), results.bsPt.begin(), results.bsPt.end());
    bsEta_.insert(bsEta_.end(), results.bsEta.begin(), results.bsEta.end());
    bsPhi_.insert(bsPhi_.end(), results.bsPhi.begin(), results.bsPhi.end());
    bsFitSuccessful_.insert(bsFitSuccessful_.end(), results.bsFitSuccessful.begin(), results.bsFitSuccessful.end());
}
