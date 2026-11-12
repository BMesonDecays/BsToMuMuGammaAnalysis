#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/OutputTree.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

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
    tree_->Branch("mu1Pt", &mu1Pt_, "mu1Pt/D");
    tree_->Branch("mu1Eta", &mu1Eta_, "mu1Eta/D");
    tree_->Branch("mu1Phi", &mu1Phi_, "mu1Phi/D");
    tree_->Branch("mu1Charge", &mu1Charge_, "mu1Charge/I");
    tree_->Branch("mu1IsGlobal", &mu1IsGlobal_, "mu1IsGlobal/I");
    tree_->Branch("mu1IsTracker", &mu1IsTracker_, "mu1IsTracker/I");
    tree_->Branch("mu1IsSoft", &mu1IsSoft_, "mu1IsSoft/I");
    tree_->Branch("mu1IsTight", &mu1IsTight_, "mu1IsTight/I");
    tree_->Branch("mu2Pt", &mu2Pt_, "mu2Pt/D");
    tree_->Branch("mu2Eta", &mu2Eta_, "mu2Eta/D");
    tree_->Branch("mu2Phi", &mu2Phi_, "mu2Phi/D");
    tree_->Branch("mu2Charge", &mu2Charge_, "mu2Charge/I");
    tree_->Branch("mu2IsGlobal", &mu2IsGlobal_, "mu2IsGlobal/I");
    tree_->Branch("mu2IsTracker", &mu2IsTracker_, "mu2IsTracker/I");
    tree_->Branch("mu2IsSoft", &mu2IsSoft_, "mu2IsSoft/I");
    tree_->Branch("mu2IsTight", &mu2IsTight_, "mu2IsTight/I");
    tree_->Branch("DiMuon_KalmanVtxProb", &DiMuon_KalmanVtxProb_, "DiMuon_KalmanVtxProb/D");
    tree_->Branch("maxMuonsVertexComp", &maxMuonsVertexComp_, "maxMuonsVertexComp/D");
    tree_->Branch("mu1GenDeltaR", &mu1GenDeltaR_, "mu1GenDeltaR/D");
    tree_->Branch("mu2GenDeltaR", &mu2GenDeltaR_, "mu2GenDeltaR/D");
    
    // Generator summary: filled once per job in a separate tree
    summaryTree_ = new TTree("summary","Generator summary");
    summaryTree_->Branch("GenXsec", &GenXsec_, "GenXsec/D");
    summaryTree_->Branch("GenXsecErr", &GenXsecErr_, "GenXsecErr/D");
    summaryTree_->Branch("FilterEff", &FilterEff_, "FilterEff/D");
    summaryTree_->Branch("FilterEffErr", &FilterEffErr_, "FilterEffErr/D");
    summaryTree_->Branch("nEvents", &nEvents_, "nEvents/I");
    summaryTree_->Branch("LumiEquivalent", &LumiEquivalent_, "LumiEquivalent/D");
    summaryTree_->Branch("LumiEquivalentErr", &LumiEquivalentErr_, "LumiEquivalentErr/D");
    
    // Generator decay chain (MC study only)
    tree_->Branch("genDecayMode", &genDecayMode_, "genDecayMode/I");
    tree_->Branch("genBPdgId", &genBPdgId_, "genBPdgId/I");
    tree_->Branch("genIsHardProcess", &genIsHardProcess_, "genIsHardProcess/I");
    tree_->Branch("genBPt", &genBPt_, "genBPt/D");
    tree_->Branch("genBEta", &genBEta_, "genBEta/D");
    tree_->Branch("genBPhi", &genBPhi_, "genBPhi/D");
    tree_->Branch("genBMass", &genBMass_, "genBMass/D");

    tree_->Branch("genMuPlusPt", &genMuPlusPt_, "genMuPlusPt/D");
    tree_->Branch("genMuPlusEta", &genMuPlusEta_, "genMuPlusEta/D");
    tree_->Branch("genMuPlusPhi", &genMuPlusPhi_, "genMuPlusPhi/D");
    tree_->Branch("genMuMinusPt", &genMuMinusPt_, "genMuMinusPt/D");
    tree_->Branch("genMuMinusEta", &genMuMinusEta_, "genMuMinusEta/D");
    tree_->Branch("genMuMinusPhi", &genMuMinusPhi_, "genMuMinusPhi/D");
    tree_->Branch("genPhotonPt", &genPhotonPt_, "genPhotonPt/D");
    tree_->Branch("genPhotonEta", &genPhotonEta_, "genPhotonEta/D");
    tree_->Branch("genPhotonPhi", &genPhotonPhi_, "genPhotonPhi/D");
    tree_->Branch("genNFSRPhotons", &genNFSRPhotons_, "genNFSRPhotons/I");
    tree_->Branch("genDimuonMass", &genDimuonMass_, "genDimuonMass/D");
    tree_->Branch("genMumugMass", &genMumugMass_, "genMumugMass/D");

    tree_->Branch("genBProdVtxX", &genBProdVtxX_, "genBProdVtxX/D");
    tree_->Branch("genBProdVtxY", &genBProdVtxY_, "genBProdVtxY/D");
    tree_->Branch("genBProdVtxZ", &genBProdVtxZ_, "genBProdVtxZ/D");
    tree_->Branch("genBDecayVtxX", &genBDecayVtxX_, "genBDecayVtxX/D");
    tree_->Branch("genBDecayVtxY", &genBDecayVtxY_, "genBDecayVtxY/D");
    tree_->Branch("genBDecayVtxZ", &genBDecayVtxZ_, "genBDecayVtxZ/D");
    tree_->Branch("genLxy", &genLxy_, "genLxy/D");
    tree_->Branch("genCt", &genCt_, "genCt/D");

    // Beam spot and primary vertex branches (event level)
    tree_->Branch("NVerticesbeforecut", &NVerticesbeforecut_, "NVerticesbeforecut/I");
    tree_->Branch("NVerticesaftercut", &NVerticesaftercut_, "NVerticesaftercut/I");

    // Primary vertex used in the Bs fit (one entry per photon)
    tree_->Branch("PVindex", &PVindexPtr_);
    tree_->Branch("PVx", &PVxPtr_);
    tree_->Branch("PVy", &PVyPtr_);
    tree_->Branch("PVz", &PVzPtr_);
    tree_->Branch("PVerrx", &PVerrxPtr_);
    tree_->Branch("PVerry", &PVerryPtr_);
    tree_->Branch("PVerrz", &PVerrzPtr_);
    tree_->Branch("PVcovXX", &PVcovXXPtr_);
    tree_->Branch("PVcovXY", &PVcovXYPtr_);
    tree_->Branch("PVcovXZ", &PVcovXZPtr_);
    tree_->Branch("PVcovYY", &PVcovYYPtr_);
    tree_->Branch("PVcovYZ", &PVcovYZPtr_);
    tree_->Branch("PVcovZZ", &PVcovZZPtr_);
    tree_->Branch("PVndof", &PVndofPtr_);
    tree_->Branch("PVrho", &PVrhoPtr_);
    tree_->Branch("PVntracks", &PVntracksPtr_);
    tree_->Branch("mu1IsTightSelPV", &mu1IsTightSelPVPtr_);
    tree_->Branch("mu2IsTightSelPV", &mu2IsTightSelPVPtr_);

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
    tree_->Branch("dimuonFitMass", &dimuonFitMass_, "dimuonFitMass/D");
    tree_->Branch("dimuonEta", &dimuonEta_, "dimuonEta/D");
    tree_->Branch("dimuonPhi", &dimuonPhi_, "dimuonPhi/D");
    tree_->Branch("dimuonPt", &dimuonPt_, "dimuonPt/D");
    tree_->Branch("dimuonGenPhotonMass", &dimuonGenPhotonMass_, "dimuonGenPhotonMass/D");
    
    tree_->Branch("photonDeltaRs", &photonDeltaRsPtr_);
    tree_->Branch("photonPt", &photonPtPtr_);
    tree_->Branch("photonEta", &photonEtaPtr_);
    tree_->Branch("photonPhi", &photonPhiPtr_);
    tree_->Branch("photonEnergy", &photonEnergyPtr_);
    tree_->Branch("photonMVA", &photonMVAPtr_);

    tree_->Branch("photonDimuonMass", &photonDimuonMassPtr_);
    tree_->Branch("photonDimuonPt", &photonDimuonPtPtr_);
    tree_->Branch("photonDimuonEta", &photonDimuonEtaPtr_);
    tree_->Branch("photonDimuonPhi", &photonDimuonPhiPtr_);
    tree_->Branch("photonDimuonLxy", &photonDimuonLxyPtr_);
    tree_->Branch("photonDimuonLxySig", &photonDimuonLxySigPtr_);
    tree_->Branch("photonDimuonCosAlpha", &photonDimuonCosAlphaPtr_);
    tree_->Branch("photonDimuonCosAlpha2D", &photonDimuonCosAlpha2DPtr_);
    tree_->Branch("photonGenDeltaR", &photonGenDeltaRPtr_);

    tree_->Branch("bsVtxProb", &bsVtxProbPtr_);
    tree_->Branch("bsVtxX", &bsVtxXPtr_);
    tree_->Branch("bsVtxY", &bsVtxYPtr_);
    tree_->Branch("bsVtxZ", &bsVtxZPtr_);
    tree_->Branch("bsLxy", &bsLxyPtr_);
    tree_->Branch("bsLxySig", &bsLxySigPtr_);
    tree_->Branch("bsCt", &bsCtPtr_);
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
    
    // Generator decay chain
    genDecayMode_ = -999;
    genBPdgId_ = -999;
    genIsHardProcess_ = -999;
    genBPt_ = -9999.0;
    genBEta_ = -9999.0;
    genBPhi_ = -9999.0;
    genBMass_ = -9999.0;

    genMuPlusPt_ = -9999.0;
    genMuPlusEta_ = -9999.0;
    genMuPlusPhi_ = -9999.0;
    genMuMinusPt_ = -9999.0;
    genMuMinusEta_ = -9999.0;
    genMuMinusPhi_ = -9999.0;
    genPhotonPt_ = -9999.0;
    genPhotonEta_ = -9999.0;
    genPhotonPhi_ = -9999.0;
    genNFSRPhotons_ = -999;
    genDimuonMass_ = -9999.0;
    genMumugMass_ = -9999.0;

    genBProdVtxX_ = -9999.0;
    genBProdVtxY_ = -9999.0;
    genBProdVtxZ_ = -9999.0;
    genBDecayVtxX_ = -9999.0;
    genBDecayVtxY_ = -9999.0;
    genBDecayVtxZ_ = -9999.0;
    genLxy_ = -9999.0;
    genCt_ = -9999.0;

    // Primary-vertex entries (event level)
    NVerticesbeforecut_ = -999;
    NVerticesaftercut_ = -999;

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
    dimuonFitMass_ = -9999.0;
    dimuonEta_ = -9999.0;
    dimuonPhi_ = -9999.0;
    dimuonPt_ = -9999.0;
    dimuonGenPhotonMass_ = -9999.0;

    // Muon kinematics and IDs
    mu1Pt_ = -9999.0;
    mu1Eta_ = -9999.0;
    mu1Phi_ = -9999.0;
    mu1Charge_ = -999;
    mu1IsGlobal_ = -999;
    mu1IsTracker_ = -999;
    mu1IsSoft_ = -999;
    mu1IsTight_ = -999;
    mu2Pt_ = -9999.0;
    mu2Eta_ = -9999.0;
    mu2Phi_ = -9999.0;
    mu2Charge_ = -999;
    mu2IsGlobal_ = -999;
    mu2IsTracker_ = -999;
    mu2IsSoft_ = -999;
    mu2IsTight_ = -999;

    maxMuonsVertexComp_ = -9999.0;

    // Muon gen matching
    mu1GenDeltaR_ = -9999.0;
    mu2GenDeltaR_ = -9999.0;

    // Photon branches
    photonDeltaRs_.clear();
    photonPt_.clear();
    photonEta_.clear();
    photonPhi_.clear();
    photonEnergy_.clear();
    photonMVA_.clear();
    photonDimuonMass_.clear();
    photonDimuonPt_.clear();
    photonDimuonEta_.clear();
    photonDimuonPhi_.clear();
    photonDimuonLxy_.clear();
    photonDimuonLxySig_.clear();
    photonDimuonCosAlpha_.clear();
    photonDimuonCosAlpha2D_.clear();
    photonGenDeltaR_.clear();

    // Primary vertex branches (per photon)
    PVindex_.clear();
    PVx_.clear();
    PVy_.clear();
    PVz_.clear();
    PVerrx_.clear();
    PVerry_.clear();
    PVerrz_.clear();
    PVcovXX_.clear();
    PVcovXY_.clear();
    PVcovXZ_.clear();
    PVcovYY_.clear();
    PVcovYZ_.clear();
    PVcovZZ_.clear();
    PVndof_.clear();
    PVrho_.clear();
    PVntracks_.clear();
    mu1IsTightSelPV_.clear();
    mu2IsTightSelPV_.clear();

    // Bs branches
    bsVtxProb_.clear();
    bsVtxX_.clear();
    bsVtxY_.clear();
    bsVtxZ_.clear();
    bsLxy_.clear();
    bsLxySig_.clear();
    bsCt_.clear();
    bsMass_.clear();
    bsEnergy_.clear();
    bsPt_.clear();
    bsEta_.clear();
    bsPhi_.clear();
    bsFitSuccessful_.clear();
}

void OutputTree::addBSAndVtxObservables(const BeamSpotAndVertex::BSAndVtxVariables& vars)
{
    NVerticesbeforecut_ = vars.pv_multiplicity;
    NVerticesaftercut_ = vars.pv_cutmultiplicity;

    BSx_ = vars.bs_x;
    BSy_ = vars.bs_y;
    BSz_ = vars.bs_z;

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
    dimuonFitMass_ = results.dimuonFitMass;
    dimuonEta_ = results.dimuonEta;
    dimuonPhi_ = results.dimuonPhi;
    dimuonPt_ = results.dimuonPt;
    dimuonGenPhotonMass_ = results.dimuonGenPhotonMass;

    // Vector branches
    photonDeltaRs_.insert(photonDeltaRs_.end(), results.photonDeltaRs.begin(), results.photonDeltaRs.end());
    photonPt_.insert(photonPt_.end(), results.photonPt.begin(), results.photonPt.end());
    photonEta_.insert(photonEta_.end(), results.photonEta.begin(), results.photonEta.end());
    photonPhi_.insert(photonPhi_.end(), results.photonPhi.begin(), results.photonPhi.end());
    photonEnergy_.insert(photonEnergy_.end(), results.photonEnergy.begin(), results.photonEnergy.end());
    photonMVA_.insert(photonMVA_.end(), results.photonMVA.begin(), results.photonMVA.end());

    photonDimuonMass_.insert(photonDimuonMass_.end(), results.photonDimuonMass.begin(), results.photonDimuonMass.end());
    photonDimuonPt_.insert(photonDimuonPt_.end(), results.photonDimuonPt.begin(), results.photonDimuonPt.end());
    photonDimuonEta_.insert(photonDimuonEta_.end(), results.photonDimuonEta.begin(), results.photonDimuonEta.end());
    photonDimuonPhi_.insert(photonDimuonPhi_.end(), results.photonDimuonPhi.begin(), results.photonDimuonPhi.end());
    photonDimuonLxy_.insert(photonDimuonLxy_.end(), results.photonDimuonLxy.begin(), results.photonDimuonLxy.end());
    photonDimuonLxySig_.insert(photonDimuonLxySig_.end(), results.photonDimuonLxySig.begin(), results.photonDimuonLxySig.end());
    photonDimuonCosAlpha_.insert(photonDimuonCosAlpha_.end(), results.photonDimuonCosAlpha.begin(), results.photonDimuonCosAlpha.end());
    photonDimuonCosAlpha2D_.insert(photonDimuonCosAlpha2D_.end(), results.photonDimuonCosAlpha2D.begin(), results.photonDimuonCosAlpha2D.end());
    photonGenDeltaR_.insert(photonGenDeltaR_.end(), results.photonGenDeltaR.begin(), results.photonGenDeltaR.end());

    PVindex_.insert(PVindex_.end(), results.pvIndex.begin(), results.pvIndex.end());
    PVx_.insert(PVx_.end(), results.pvX.begin(), results.pvX.end());
    PVy_.insert(PVy_.end(), results.pvY.begin(), results.pvY.end());
    PVz_.insert(PVz_.end(), results.pvZ.begin(), results.pvZ.end());
    PVerrx_.insert(PVerrx_.end(), results.pvErrX.begin(), results.pvErrX.end());
    PVerry_.insert(PVerry_.end(), results.pvErrY.begin(), results.pvErrY.end());
    PVerrz_.insert(PVerrz_.end(), results.pvErrZ.begin(), results.pvErrZ.end());
    PVcovXX_.insert(PVcovXX_.end(), results.pvCovXX.begin(), results.pvCovXX.end());
    PVcovXY_.insert(PVcovXY_.end(), results.pvCovXY.begin(), results.pvCovXY.end());
    PVcovXZ_.insert(PVcovXZ_.end(), results.pvCovXZ.begin(), results.pvCovXZ.end());
    PVcovYY_.insert(PVcovYY_.end(), results.pvCovYY.begin(), results.pvCovYY.end());
    PVcovYZ_.insert(PVcovYZ_.end(), results.pvCovYZ.begin(), results.pvCovYZ.end());
    PVcovZZ_.insert(PVcovZZ_.end(), results.pvCovZZ.begin(), results.pvCovZZ.end());
    PVndof_.insert(PVndof_.end(), results.pvNdof.begin(), results.pvNdof.end());
    PVrho_.insert(PVrho_.end(), results.pvRho.begin(), results.pvRho.end());
    PVntracks_.insert(PVntracks_.end(), results.pvNTracks.begin(), results.pvNTracks.end());
    mu1IsTightSelPV_.insert(mu1IsTightSelPV_.end(), results.mu1IsTightSelPV.begin(), results.mu1IsTightSelPV.end());
    mu2IsTightSelPV_.insert(mu2IsTightSelPV_.end(), results.mu2IsTightSelPV.begin(), results.mu2IsTightSelPV.end());

    bsVtxProb_.insert(bsVtxProb_.end(), results.bsVtxProb.begin(), results.bsVtxProb.end());
    bsVtxX_.insert(bsVtxX_.end(), results.bsVtxX.begin(), results.bsVtxX.end());
    bsVtxY_.insert(bsVtxY_.end(), results.bsVtxY.begin(), results.bsVtxY.end());
    bsVtxZ_.insert(bsVtxZ_.end(), results.bsVtxZ.begin(), results.bsVtxZ.end());
    bsLxy_.insert(bsLxy_.end(), results.bsLxy.begin(), results.bsLxy.end());
    bsLxySig_.insert(bsLxySig_.end(), results.bsLxySig.begin(), results.bsLxySig.end());
    bsCt_.insert(bsCt_.end(), results.bsCt.begin(), results.bsCt.end());
    bsMass_.insert(bsMass_.end(), results.bsMass.begin(), results.bsMass.end());
    bsEnergy_.insert(bsEnergy_.end(), results.bsEnergy.begin(), results.bsEnergy.end());
    bsPt_.insert(bsPt_.end(), results.bsPt.begin(), results.bsPt.end());
    bsEta_.insert(bsEta_.end(), results.bsEta.begin(), results.bsEta.end());
    bsPhi_.insert(bsPhi_.end(), results.bsPhi.begin(), results.bsPhi.end());
    bsFitSuccessful_.insert(bsFitSuccessful_.end(), results.bsFitSuccessful.begin(), results.bsFitSuccessful.end());
}

void OutputTree::addMuonPair(const reco::Muon& mu1, const reco::Muon& mu2, const reco::Vertex& pv)
{
    mu1Pt_        = mu1.pt();
    mu1Eta_       = mu1.eta();
    mu1Phi_       = mu1.phi();
    mu1Charge_    = mu1.charge();
    mu1IsGlobal_  = mu1.isGlobalMuon();
    mu1IsTracker_ = mu1.isTrackerMuon();
    mu1IsSoft_    = muon::isSoftMuon(mu1, pv);
    mu1IsTight_   = muon::isTightMuon(mu1, pv);

    mu2Pt_        = mu2.pt();
    mu2Eta_       = mu2.eta();
    mu2Phi_       = mu2.phi();
    mu2Charge_    = mu2.charge();
    mu2IsGlobal_  = mu2.isGlobalMuon();
    mu2IsTracker_ = mu2.isTrackerMuon();
    mu2IsSoft_    = muon::isSoftMuon(mu2, pv);
    mu2IsTight_   = muon::isTightMuon(mu2, pv);
}

void OutputTree::addGenInfo(const GenDecayChain::GenInfo& gen)
{
    if (!gen.isValid) return;

    // decay and B meson
    genDecayMode_     = static_cast<int>(gen.decayMode);
    genBPdgId_        = gen.bMesonPdgId;
    genIsHardProcess_ = gen.isHardProcess;
    genBPt_   = gen.Bp4.Pt();
    genBEta_  = gen.Bp4.Eta();
    genBPhi_  = gen.Bp4.Phi();
    genBMass_ = gen.Bp4.M();

    // muons and signal photon
    if (gen.hasMuonPlus) {
        genMuPlusPt_  = gen.muPlusP4.Pt();
        genMuPlusEta_ = gen.muPlusP4.Eta();
        genMuPlusPhi_ = gen.muPlusP4.Phi();
    }
    if (gen.hasMuonMinus) {
        genMuMinusPt_  = gen.muMinusP4.Pt();
        genMuMinusEta_ = gen.muMinusP4.Eta();
        genMuMinusPhi_ = gen.muMinusP4.Phi();
    }
    genPhotonPt_    = gen.signalPhotonP4.Pt();
    genPhotonEta_   = gen.signalPhotonP4.Eta();
    genPhotonPhi_   = gen.signalPhotonP4.Phi();
    genNFSRPhotons_ = gen.nPhotons - 1;     // all chain photons except the signal photon
    genDimuonMass_  = gen.dimuonMass;
    genMumugMass_   = gen.triBodyMass;

    // vertices and decay length
    genBProdVtxX_  = gen.Bvtx.X();
    genBProdVtxY_  = gen.Bvtx.Y();
    genBProdVtxZ_  = gen.Bvtx.Z();
    genBDecayVtxX_ = gen.Bdecayvtx.X();
    genBDecayVtxY_ = gen.Bdecayvtx.Y();
    genBDecayVtxZ_ = gen.Bdecayvtx.Z();

    const TVector3 flight = gen.Bdecayvtx - gen.Bvtx;
    genLxy_ = flight.Perp();
    if (gen.Bp4.P() > 0) genCt_ = flight.Mag() * gen.Bp4.M() / gen.Bp4.P();
}
