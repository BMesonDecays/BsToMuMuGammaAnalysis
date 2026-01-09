#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_DecayChainVariables_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_DecayChainVariables_h

struct DecayChainVariables {
    double fittedBmassRecoPhoton = -9999;
    double fittedBmassConvertedPhoton = -9999;
    int mu1softid = -9999;
    int mu2softid = -9999;
    int mu1tightid = -9999;
    int mu2tightid = -9999;
    int mu1globalMuon = -9999;
    int mu2globalMuon = -9999;
    int mu1trkMuon = -9999;
    int mu2trkMuon = -9999;
    double max_Dr1 = -9999;
    double max_Dr2 = -9999;
    double muonpairdr = -9999;
    double mu1trkbsxy = -9999;
    double mu1trkbsz = -9999;
    double mu2trkbsxy = -9999;
    double mu2trkbsz = -9999;
    int mu1pixelhits = -9999;
    int mu2pixelhits = -9999;
    int mu1trackerhits = -9999;
    int mu2trackerhits = -9999;
    int mu1isgood = -9999;
    int mu2isgood = -9999;
    int mu1innertrkhq = -9999;
    int mu2innertrkhq = -9999;

    double dimuonMass = -9999;
    double dimuonEta = -9999;
    double dimuonPhi = -9999;
    double dimuonPt = -9999;

    double dimuonMass_Jpsi = -9999;
    double dimuonEta_Jpsi = -9999;
    double dimuonPhi_Jpsi = -9999;
    double dimuonPt_Jpsi = -9999;

    double dimuonMass_Phi = -9999;
    double dimuonEta_Phi = -9999;
    double dimuonPhi_Phi = -9999;
    double dimuonPt_Phi = -9999;

    double dimuonMass_Kstar0 = -9999;
    double dimuonEta_Kstar0 = -9999;
    double dimuonPhi_Kstar0 = -9999;
    double dimuonPt_Kstar0 = -9999;

    double dimuonMass_NoBound = -9999;
    double dimuonEta_NoBound = -9999;
    double dimuonPhi_NoBound = -9999;
    double dimuonPt_NoBound = -9999;

    int resonanceFlag = -9999;
    double opening_angle = -9999;
    double dimuonvtxprob = -9999;
    double mumudca = -9999;
    double dimuonlxy = -9999;
    double dimuonlxyerr = -9999;
    double dimuonlxyOverPt = -9999;
    double dimuonchi2 = -9999;
    double mahalanobis = -9999;
    double mu1pz = -9999.0, mu1pt = -9999.0, mu1eta = -9999.0, mu1phi = -9999.0, mu1energy = -9999.0;
    double mu2pz = -9999.0, mu2pt = -9999.0, mu2eta = -9999.0, mu2phi = -9999.0, mu2energy = -9999.0;
    int isCowboy = -9999;
    int diMuon_mu1Cat = -9999;
    int diMuon_mu2Cat = -9999;
    int diMuon_mu1PixelHits = -9999;
    int diMuon_mu2PixelHits = -9999;
    /*We must choose one of the two canonical patterns: 1. One candidate = one Fill(), 2. Vector branches for per-candidate quantities, 
    we  cannot preserve both categories inside the same scalar branch.*/ 
    /*I am changing the naming pattern so that we must not be having any issue of reading in the ntuple.
    The price that we'll pay is that nunber variables in the ntuple will be a bit higher. Okay so the naming scheme will be following 
    so 1. Three object converted vertex case : *_mmconvg,
       2. Three object reco vertex case      : *_mmrecog
       3. Four object converted vertex case  : *_mmconvgg,
       4. Four object reco vertex case       : *_mmrecogg
       */

    int vertexFitFlag_mmconvg = -9999;
    int vertexFitFlag_mmconvgg = -9999;
    int vertexFitFlag_mmrecog = -9999;
    int vertexFitFlag_mmrecogg = -9999;
    int vertexFitFlag_refmodmmKK = -9999;


    //Tripple object reco photon vertex variables   
    double fourvectorbsmass_mmrecog = -9999;
    double fourvectorbspt_mmrecog = -9999;
    double fourvectorbseta_mmrecog = -9999;
    double fourvectorbsphi_mmrecog = -9999;
    double vertexfitbsmass_mmrecog = -9999;
    double vertexfitbspt_mmrecog = -9999;
    double vertexfitbseta_mmrecog = -9999;
    double vertexfitbsphi_mmrecog = -9999;
    double vertexfitbspz_mmrecog = -9999;
    double vertexfitbsSVx_mmrecog = -9999;
    double vertexfitbsSVy_mmrecog = -9999;
    double vertexfitbsSVz_mmrecog = -9999;
    double vertexfitPVxrefitcosTheta_mmrecog = -9999;
    double vertexfitPVyrefitcosTheta_mmrecog = -9999;
    double vertexfitPVzrefitcosTheta_mmrecog = -9999;
    double vertexfitPVxrefitclosestZ_mmrecog = -9999;
    double vertexfitPVyrefitclosestZ_mmrecog = -9999;
    double vertexfitPVzrefitclosestZ_mmrecog = -9999;
    double vertexfitBsCt3DPVClosestZ_mmrecog = -9999;
    double vertexfitBsCt2DPVClosestZ_mmrecog = -9999;
    double vertexfitBsCt3DPVCosTheta_mmrecog = -9999;
    double vertexfitBsCt2DPVCosTheta_mmrecog = -9999;
    double vertexfitBsCt2DPVCosThetaOld_mmrecog = -9999;
    double vertexfitBsCt2DBSOld_mmrecog = -9999;
    double vertexfitBsCt2DPVClosestZOld_mmrecog = -9999;
    double vertexfitBsCt2DOld_mmrecog = -9999;
    double vertexfitBsCtErr2DBS_mmrecog = -9999;
    double vertexfitBsCtErr2D_mmrecog = -9999;
    double vertexfitBsCtErr2DClosestZ_mmrecog = -9999;
    double vertexfitBsCtErr2DCostheta_mmrecog = -9999;
    double vertexfitBsCtErr2DCosthetaOld_mmrecog = -9999;
    double vertexfitBsCtErr2DBSOld_mmrecog = -9999;
    double vertexfitBsCtErr2DOld_mmrecog = -9999;
    double vertexfitBsCtErr2DClosestZOld_mmrecog = -9999;
    double vertexfitBsCtErr3D_mmrecog = -9999;
    double vertexfitBsCtErr2D2_mmrecog = -9999;
    double vertexfitBsCtErrMPV_mmrecog = -9999;
    double vertexfitBsCt3Drefit_mmrecog = -9999;
    double vertexfitBsCt2Drefit_mmrecog = -9999;
    double vertexfitBsCtMPVrefit_mmrecog = -9999;
    double vertexfitBsCtErr3Drefit_mmrecog = -9999;
    double vertexfitBsCtErr2Drefit_mmrecog = -9999; 
    double vertexfitBsCtErrMPVrefit_mmrecog = -9999;
    double vertexfitBsLxy_mmrecog = -9999;
    double vertexfitBsLxyErr_mmrecog = -9999;
    double vertexfitBsCt_mmrecog = -9999;
    double vertexfitBsCtErr_mmrecog = -9999;
    double vertexfitBsErrX_mmrecog = -9999;
    double vertexfitBsErrY_mmrecog = -9999;
    double vertexfitBsErrXY_mmrecog = -9999;
    double vertexfitBsDist3d_mmrecog = -9999;
    double vertexfitBsDist3dErr_mmrecog = -9999;
    double vertexfitBsTime3d_mmrecog = -9999;
    double vertexfitBsTime3dErr_mmrecog = -9999;
    double vertexfitBsDist2d_mmrecog = -9999;
    double vertexfitBsDist2dErr_mmrecog = -9999;
    double vertexfitBsTime2d_mmrecog = -9999;
    double vertexfitBsTime2dErr_mmrecog = -9999;
    double bsvtxprob_mmrecog = -9999;
    double bsct2d_mmrecog = -9999;
    double bsct2dbs_mmrecog = -9999;    
    double bsct3d_mmrecog = -9999;
    double deltaRphoton1dimuon_mmrecog = -9999;
    double deltaRphoton2dimuon_mmrecog = -9999;
    double deltaRphoton1photon2_mmrecog = -9999;
    double bshelicity_mmrecog = -9999;
    double bscoplanarity_mmrecog = -9999;







//Tripple object converted photon vertex variables
double fourvectorbsmass_mmconvg = -9999;
double fourvectorbspt_mmconvg = -9999;
double fourvectorbseta_mmconvg = -9999;
double fourvectorbsphi_mmconvg = -9999;
double vertexfitbsmass_mmconvg = -9999;
double vertexfitbspt_mmconvg = -9999;
double vertexfitbseta_mmconvg = -9999;
double vertexfitbsphi_mmconvg = -9999;
double vertexfitbspz_mmconvg = -9999;
double vertexfitbsSVx_mmconvg = -9999;
double vertexfitbsSVy_mmconvg = -9999;
double vertexfitbsSVz_mmconvg = -9999;
double fourvectorconvgammapt_mmconvg = -9999;
double fourvectorconvgammaeta_mmconvg = -9999;
double fourvectorconvgammaphi_mmconvg = -9999;

double vertexfitPVxrefitcosTheta_mmconvg = -9999;
double vertexfitPVyrefitcosTheta_mmconvg = -9999;
double vertexfitPVzrefitcosTheta_mmconvg = -9999;
double vertexfitPVxrefitclosestZ_mmconvg = -9999;
double vertexfitPVyrefitclosestZ_mmconvg = -9999;
double vertexfitPVzrefitclosestZ_mmconvg = -9999;

double vertexfitBsCt3DPVClosestZ_mmconvg = -9999;
double vertexfitBsCt2DPVClosestZ_mmconvg = -9999;
double vertexfitBsCt3DPVCosTheta_mmconvg = -9999;
double vertexfitBsCt2DPVCosTheta_mmconvg = -9999;
double vertexfitBsCt2DPVCosThetaOld_mmconvg = -9999;
double vertexfitBsCt2DPVClosestZOld_mmconvg = -9999;
double vertexfitBsCt2DBSOld_mmconvg = -9999;
double vertexfitBsCt2DOld_mmconvg = -9999;

double vertexfitBsCtErr2DBS_mmconvg = -9999;
double vertexfitBsCtErr2D_mmconvg = -9999;
double vertexfitBsCtErr2DClosestZ_mmconvg = -9999;
double vertexfitBsCtErr2DCostheta_mmconvg = -9999;
double vertexfitBsCtErr2DCosthetaOld_mmconvg = -9999;
double vertexfitBsCtErr2DBSOld_mmconvg = -9999;
double vertexfitBsCtErr2DOld_mmconvg = -9999;
double vertexfitBsCtErr2DClosestZOld_mmconvg = -9999;
double vertexfitBsCtErr3D_mmconvg = -9999;
double vertexfitBsCtErr2D2_mmconvg = -9999;
double vertexfitBsCtErrMPV_mmconvg = -9999;

double vertexfitBsCt3Drefit_mmconvg = -9999;
double vertexfitBsCt2Drefit_mmconvg = -9999;
double vertexfitBsCtMPVrefit_mmconvg = -9999;
double vertexfitBsCtErr3Drefit_mmconvg = -9999;
double vertexfitBsCtErr2Drefit_mmconvg = -9999;
double vertexfitBsCtErrMPVrefit_mmconvg = -9999;

double vertexfitBsLxy_mmconvg = -9999;
double vertexfitBsLxyErr_mmconvg = -9999;
double vertexfitBsCt_mmconvg = -9999;
double vertexfitBsCtErr_mmconvg = -9999;

double vertexfitBsErrX_mmconvg = -9999;
double vertexfitBsErrY_mmconvg = -9999;
double vertexfitBsErrXY_mmconvg = -9999;

double vertexfitBsDist3d_mmconvg = -9999;
double vertexfitBsDist3dErr_mmconvg = -9999;
double vertexfitBsTime3d_mmconvg = -9999;
double vertexfitBsTime3dErr_mmconvg = -9999;

double vertexfitBsDist2d_mmconvg = -9999;
double vertexfitBsDist2dErr_mmconvg = -9999;
double vertexfitBsTime2d_mmconvg = -9999;
double vertexfitBsTime2dErr_mmconvg = -9999;

double bsvtxprob_mmconvg = -9999;
double bsct2d_mmconvg = -9999;
double bsct2dbs_mmconvg = -9999;
double bsct3d_mmconvg = -9999;

double deltaRphoton1dimuon_mmconvg = -9999;
double deltaRphoton2dimuon_mmconvg = -9999;


double bshelicity_mmconvg = -9999;
double bscoplanarity_mmconvg = -9999;


//Four object converted photon vertex variables

double fourvectorbsmass_mmconvgg = -9999;
double fourvectorbspt_mmconvgg = -9999;
double fourvectorbseta_mmconvgg = -9999;
double fourvectorbsphi_mmconvgg = -9999;
double fourvectorconvgammapt_mmconvgg = -9999;
double fourvectorconvgammaeta_mmconvgg = -9999;
double fourvectorconvgammaphi_mmconvgg = -9999;
double vertexfitbsmass_mmconvgg = -9999;
double vertexfitbspt_mmconvgg = -9999;
double vertexfitbseta_mmconvgg = -9999;
double vertexfitbsphi_mmconvgg = -9999;
double vertexfitbspz_mmconvgg = -9999;
double vertexfitbsSVx_mmconvgg = -9999;
double vertexfitbsSVy_mmconvgg = -9999;
double vertexfitbsSVz_mmconvgg = -9999;

double vertexfitPVxrefitcosTheta_mmconvgg = -9999;
double vertexfitPVyrefitcosTheta_mmconvgg = -9999;
double vertexfitPVzrefitcosTheta_mmconvgg = -9999;
double vertexfitPVxrefitclosestZ_mmconvgg = -9999;
double vertexfitPVyrefitclosestZ_mmconvgg = -9999;
double vertexfitPVzrefitclosestZ_mmconvgg = -9999;

double vertexfitBsCt3DPVClosestZ_mmconvgg = -9999;
double vertexfitBsCt2DPVClosestZ_mmconvgg = -9999;
double vertexfitBsCt3DPVCosTheta_mmconvgg = -9999;
double vertexfitBsCt2DPVCosTheta_mmconvgg = -9999;
double vertexfitBsCt2DPVCosThetaOld_mmconvgg = -9999;
double vertexfitBsCt2DPVClosestZOld_mmconvgg = -9999;
double vertexfitBsCt2DBSOld_mmconvgg = -9999;
double vertexfitBsCt2DOld_mmconvgg = -9999;

double vertexfitBsCtErr2DBS_mmconvgg = -9999;
double vertexfitBsCtErr2D_mmconvgg = -9999;
double vertexfitBsCtErr2DClosestZ_mmconvgg = -9999;
double vertexfitBsCtErr2DCostheta_mmconvgg = -9999;
double vertexfitBsCtErr2DCosthetaOld_mmconvgg = -9999;
double vertexfitBsCtErr2DBSOld_mmconvgg = -9999;
double vertexfitBsCtErr2DOld_mmconvgg = -9999;
double vertexfitBsCtErr2DClosestZOld_mmconvgg = -9999;
double vertexfitBsCtErr3D_mmconvgg = -9999;
double vertexfitBsCtErr2D2_mmconvgg = -9999;
double vertexfitBsCtErrMPV_mmconvgg = -9999;

double vertexfitBsCt3Drefit_mmconvgg = -9999;
double vertexfitBsCt2Drefit_mmconvgg = -9999;
double vertexfitBsCtMPVrefit_mmconvgg = -9999;
double vertexfitBsCtErr3Drefit_mmconvgg = -9999;
double vertexfitBsCtErr2Drefit_mmconvgg = -9999;
double vertexfitBsCtErrMPVrefit_mmconvgg = -9999;

double vertexfitBsLxy_mmconvgg = -9999;
double vertexfitBsLxyErr_mmconvgg = -9999;
double vertexfitBsCt_mmconvgg = -9999;
double vertexfitBsCtErr_mmconvgg = -9999;

double vertexfitBsErrX_mmconvgg = -9999;
double vertexfitBsErrY_mmconvgg = -9999;
double vertexfitBsErrXY_mmconvgg = -9999;

double vertexfitBsDist3d_mmconvgg = -9999;
double vertexfitBsDist3dErr_mmconvgg = -9999;
double vertexfitBsTime3d_mmconvgg = -9999;
double vertexfitBsTime3dErr_mmconvgg = -9999;

double vertexfitBsDist2d_mmconvgg = -9999;
double vertexfitBsDist2dErr_mmconvgg = -9999;
double vertexfitBsTime2d_mmconvgg = -9999;
double vertexfitBsTime2dErr_mmconvgg = -9999;

double bsvtxprob_mmconvgg = -9999;
double bsct2d_mmconvgg = -9999;
double bsct2dbs_mmconvgg = -9999;
double bsct3d_mmconvgg = -9999;

double deltaRphoton1dimuon_mmconvgg = -9999;
double deltaRphoton2dimuon_mmconvgg = -9999;
double deltaRphoton1photon2_mmconvgg = -9999;

double bshelicity_mmconvgg = -9999;
double bscoplanarity_mmconvgg = -9999;


//Four object reco photon vertex variables
double fourvectorbsmass_mmrecogg = -9999;
double fourvectorbspt_mmrecogg = -9999;
double fourvectorbseta_mmrecogg = -9999;
double fourvectorbsphi_mmrecogg = -9999;
double vertexfitbsmass_mmrecogg = -9999;
double vertexfitbspt_mmrecogg = -9999;
double vertexfitbseta_mmrecogg = -9999;
double vertexfitbsphi_mmrecogg = -9999;
double vertexfitbspz_mmrecogg = -9999;
double vertexfitbsSVx_mmrecogg = -9999;
double vertexfitbsSVy_mmrecogg = -9999;
double vertexfitbsSVz_mmrecogg = -9999;

double vertexfitPVxrefitcosTheta_mmrecogg = -9999;
double vertexfitPVyrefitcosTheta_mmrecogg = -9999;
double vertexfitPVzrefitcosTheta_mmrecogg = -9999;
double vertexfitPVxrefitclosestZ_mmrecogg = -9999;
double vertexfitPVyrefitclosestZ_mmrecogg = -9999;
double vertexfitPVzrefitclosestZ_mmrecogg = -9999;

double vertexfitBsCt3DPVClosestZ_mmrecogg = -9999;
double vertexfitBsCt2DPVClosestZ_mmrecogg = -9999;
double vertexfitBsCt3DPVCosTheta_mmrecogg = -9999;
double vertexfitBsCt2DPVCosTheta_mmrecogg = -9999;
double vertexfitBsCt2DPVCosThetaOld_mmrecogg = -9999;
double vertexfitBsCt2DPVClosestZOld_mmrecogg = -9999;
double vertexfitBsCt2DBSOld_mmrecogg = -9999;
double vertexfitBsCt2DOld_mmrecogg = -9999;

double vertexfitBsCtErr2DBS_mmrecogg = -9999;
double vertexfitBsCtErr2D_mmrecogg = -9999;
double vertexfitBsCtErr2DClosestZ_mmrecogg = -9999;
double vertexfitBsCtErr2DCostheta_mmrecogg = -9999;
double vertexfitBsCtErr2DCosthetaOld_mmrecogg = -9999;
double vertexfitBsCtErr2DBSOld_mmrecogg = -9999;
double vertexfitBsCtErr2DOld_mmrecogg = -9999;
double vertexfitBsCtErr2DClosestZOld_mmrecogg = -9999;
double vertexfitBsCtErr3D_mmrecogg = -9999;
double vertexfitBsCtErr2D2_mmrecogg = -9999;
double vertexfitBsCtErrMPV_mmrecogg = -9999;

double vertexfitBsCt3Drefit_mmrecogg = -9999;
double vertexfitBsCt2Drefit_mmrecogg = -9999;
double vertexfitBsCtMPVrefit_mmrecogg = -9999;
double vertexfitBsCtErr3Drefit_mmrecogg = -9999;
double vertexfitBsCtErr2Drefit_mmrecogg = -9999;
double vertexfitBsCtErrMPVrefit_mmrecogg = -9999;

double vertexfitBsLxy_mmrecogg = -9999;
double vertexfitBsLxyErr_mmrecogg = -9999;
double vertexfitBsCt_mmrecogg = -9999;
double vertexfitBsCtErr_mmrecogg = -9999;

double vertexfitBsErrX_mmrecogg = -9999;
double vertexfitBsErrY_mmrecogg = -9999;
double vertexfitBsErrXY_mmrecogg = -9999;

double vertexfitBsDist3d_mmrecogg = -9999;
double vertexfitBsDist3dErr_mmrecogg = -9999;
double vertexfitBsTime3d_mmrecogg = -9999;
double vertexfitBsTime3dErr_mmrecogg = -9999;

double vertexfitBsDist2d_mmrecogg = -9999;
double vertexfitBsDist2dErr_mmrecogg = -9999;
double vertexfitBsTime2d_mmrecogg = -9999;
double vertexfitBsTime2dErr_mmrecogg = -9999;

double bsvtxprob_mmrecogg = -9999;
double bsct2d_mmrecogg = -9999;
double bsct2dbs_mmrecogg = -9999;
double bsct3d_mmrecogg = -9999;

double deltaRphoton1dimuon_mmrecogg = -9999;
double deltaRphoton2dimuon_mmrecogg = -9999;
double deltaRphoton1photon2_mmrecogg = -9999;

double bshelicity_mmrecogg = -9999;
double bscoplanarity_mmrecogg = -9999;

   
   
};

#endif
