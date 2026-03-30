#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
using namespace reco;
using namespace edm;
using namespace std;
using namespace pat;
#include <TMath.h>

// Mass contraints should be set to the pg vbalues not feed from four momenta, which in general has windened resolution -Szymon 
//No resonance case - no constaint  - Szymon 


KinematicConstrainedFit::KinematicConstrainedFit(){}
bool KinematicConstrainedFit::doFit(std::vector<reco::TransientTrack> t_tracks, const double muonMass, const double mass1, const double  mass2){
    
	reco::TransientTrack track_MuP = t_tracks[0];
	reco::TransientTrack track_MuM = t_tracks[1];
	//Creating a KinematicParticleFactory
	KinematicParticleFactoryFromTransientTrack pFactory;
	//The mass of a muon and the insignificant mass sigma to avoid singularities in the covariance matrix.
        float muon_sigma = 0.0000000001;
	//initial chi2 and ndf before kinematic fits. The chi2 of the reconstruction is not considered
	float chi = 0.;
	float ndf = 0.;
	std::vector<RefCountedKinematicParticle> allParticlesMu;
	allParticlesMu.push_back(pFactory.particle (track_MuP, muonMass, chi, ndf, muon_sigma));
	allParticlesMu.push_back(pFactory.particle (track_MuM, muonMass, chi, ndf, muon_sigma));
	KinematicParticleVertexFitter Fitter;
        std::vector<RefCountedKinematicParticle> allParticlesTrk;
	RefCountedKinematicTree BsTree = Fitter.fit(allParticlesMu);
	//if the fit fails, do not consider this as candidate
	if(BsTree->isEmpty()) return 0;
	KinematicParticleFitter constFitter;
	double nominalBsMass = 5.36689;
	double bsMSigma = 0.00019;
	KinematicConstraint * bs_const = new MassKinematicConstraint(nominalBsMass, bsMSigma);
        BsTree = constFitter.fit(bs_const,BsTree);
        myTree_B= BsTree;
        if(BsTree->isEmpty()) {
		delete bs_const;
		return 0;
	} 
	BsTree->movePointerToTheTop();
	RefCountedKinematicParticle Bs_branch = BsTree->currentParticle();
	allParticlesTrk.push_back(Bs_branch);
	myTree_B = Fitter.fit(allParticlesTrk);
	if(myTree_B->isEmpty()) {
		delete bs_const;
		return 0;
       	}
	myTree_B->movePointerToTheTop();
	bhadron = myTree_B->currentParticle();
        bVertex = myTree_B->currentDecayVertex();
	if (!bVertex->vertexIsValid()) {
        delete bs_const;
        return 0;
       }
	vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), (int)bhadron->degreesOfFreedom());
	delete bs_const;
        return 1;
}
////////////////////////////////////////////////
/////////BsToJpsiPhiVertexFit-fromData//////////
////////////////////////////////////////////////
bool KinematicConstrainedFit::BsToJpsiPhiFit(std::vector<reco::TransientTrack> t_tracks, const double muonMass, const double mass1, const double  mass2){
  
  reco::TransientTrack track_MuP = t_tracks[0];
  reco::TransientTrack track_MuM = t_tracks[1];
  reco::TransientTrack track_Kp = t_tracks[2];
  reco::TransientTrack track_Km = t_tracks[3];
  
  KinematicParticleFactoryFromTransientTrack pFactory;
  
  float muon_sigma = 0.0000000001;
  float kaon_sigma = 0.000016;
  //  float pi_sigma = 0.000016;	      
  
  float chi = 0.;
  float ndf = 0.;	    
  
  std::vector<RefCountedKinematicParticle> allParticlesMu;
  allParticlesMu.push_back(pFactory.particle (track_MuP, muonMass, chi, ndf, muon_sigma));
  allParticlesMu.push_back(pFactory.particle (track_MuM, muonMass, chi, ndf, muon_sigma));
  
  
  std::vector<RefCountedKinematicParticle> allParticlesTrk;
  allParticlesTrk.push_back(pFactory.particle (track_Kp, mass1, chi, ndf, kaon_sigma));
  allParticlesTrk.push_back(pFactory.particle (track_Km, mass2, chi, ndf, kaon_sigma));
  
  KinematicParticleVertexFitter Fitter;
  RefCountedKinematicTree JpsiTree = Fitter.fit(allParticlesMu);
  
  if(JpsiTree->isEmpty()) return 0;
  
  KinematicParticleFitter constFitter;
  
  double nominalJpsiMass =  3.096916;
  float jpsiMsigma = 0.00004;
  KinematicConstraint * jpsi_const = new MassKinematicConstraint( nominalJpsiMass, jpsiMsigma);
  
  JpsiTree = constFitter.fit(jpsi_const,JpsiTree);
  myTree_Jpsi=JpsiTree;
 
  if(JpsiTree->isEmpty()) {
    delete jpsi_const;
    return 0;    
  }
  JpsiTree->movePointerToTheTop();
  RefCountedKinematicParticle Jpsi_branch = JpsiTree->currentParticle();
  allParticlesTrk.push_back(Jpsi_branch);
  myTree_B = Fitter.fit(allParticlesTrk);
  if(myTree_B->isEmpty()) {
    delete jpsi_const;
    return 0;
  }
  myTree_B->movePointerToTheTop();
  bhadron = myTree_B->currentParticle();
  bVertex = myTree_B->currentDecayVertex();
  
  if (!bVertex->vertexIsValid()) {
    delete jpsi_const;
    return 0;    
  }
  vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), (int)bhadron->degreesOfFreedom());
  mass_Bhadron    = bhadron->currentState().mass();
  delete jpsi_const;
  return 1;
}
////////////////////////////////////////////////////////////
////////////TripleObjectVertexConvertedPhoton///////////////
////////////////////////////////////////////////////////////
bool KinematicConstrainedFit::TrippleObjectVertexFitConvertedPhoton(
    const std::vector<reco::TransientTrack> muonTT,
    const std::vector<reco::TransientTrack> electronTT,
    ReferenceResonance::ResonanceDetails resonanceDetails,
    reco::Vertex PV, bool verbose)
{
    const ParticleMass zero_mass = 1e-4f; 
     float eleMass = 0.000511;
     float muonMass = 0.10565837;
     float zero_sigma = 1e-4f;
     float eleSigma = 1e-4f;
     float muonSigma = 1e-4f;
     float chi = 1e-4f;
     float ndf = 1e-4f;

    KinematicParticleFactoryFromTransientTrack pFactory;

    // -- Fit photon from e+e−
    std::vector<RefCountedKinematicParticle> photonParticles = {
        pFactory.particle(electronTT[0], eleMass, chi, ndf, eleSigma),
        pFactory.particle(electronTT[1], eleMass, chi, ndf, eleSigma)
    };
    KinematicParticleVertexFitter vertexFitter;
    RefCountedKinematicTree photonVertexFitTree = vertexFitter.fit(photonParticles);
    if(!photonVertexFitTree->isValid()) {
        edm::ParameterSet photonFitParams;
        photonFitParams.addParameter<double>("maxDistance", 3.0);
        photonFitParams.addParameter<int>("maxNbrOfIterations", 10000);
        KinematicParticleVertexFitter vertexFitter2(photonFitParams);
        photonVertexFitTree = vertexFitter2.fit(photonParticles);
    }
    if (!photonVertexFitTree || !photonVertexFitTree->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Photon vertex fit after optimization failed.";
        return false;
    }
    KinematicConstraint *photonConstraint = new MassKinematicConstraint(zero_mass, zero_sigma);
    KinematicParticleFitter csFitterPhoton;
    photonVertexFitTree = csFitterPhoton.fit(photonConstraint, photonVertexFitTree);
    if (!photonVertexFitTree || !photonVertexFitTree->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Photon mass-constrained fit failed.";
        return false;
    }
    photonVertexFitTree->movePointerToTheTop();
    RefCountedKinematicParticle fittedPhoton = photonVertexFitTree->currentParticle();
    std::cout << "Muon0 p converted photon case = " << muonTT[0].track().p() << "\n";
    std::cout << "Muon1 p converted photon case = " << muonTT[1].track().p() << "\n";
    // Combine with muons
    std::vector<RefCountedKinematicParticle> allParticlesGlobal = {
        pFactory.particle(muonTT[0], muonMass, chi, ndf, muonSigma),
        pFactory.particle(muonTT[1], muonMass, chi, ndf, muonSigma),
        fittedPhoton
    };

    // create the constraint
    GlobalPoint vertexPosition(PV.x(), PV.y(), PV.z());
    RefCountedKinematicTree vertexFitTreeGlobal;
    if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::None)
    {
        MultiTrackKinematicConstraint* multiPointingConstraint = new MultiTrackPointingKinematicConstraint(vertexPosition);
        KinematicConstrainedVertexFitter constrainedFitter;
        vertexFitTreeGlobal = constrainedFitter.fit(allParticlesGlobal, multiPointingConstraint);
    }
    else if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Jpsi
            || resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Phi
            || resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::KStar
    )
    {
        double nominalDimuonMass;
        if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Jpsi) nominalDimuonMass = ReferenceResonance::Constants::JpsiMass_PDG;
        else if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Phi) nominalDimuonMass = ReferenceResonance::Constants::PhiMass_PDG;
        else if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::KStar) nominalDimuonMass = ReferenceResonance::Constants::KStar0Mass_PDG;
        
        MultiTrackKinematicConstraint* massConstraint = new TwoTrackMassKinematicConstraint(nominalDimuonMass);
        MultiTrackKinematicConstraint* multiPointingConstraint = new MultiTrackPointingKinematicConstraint(vertexPosition);
        std::vector<MultiTrackKinematicConstraint*> constraints = {massConstraint, multiPointingConstraint};
        MultiTrackKinematicConstraint* combinedConstraint = new CombinedKinematicConstraint(constraints);
        KinematicConstrainedVertexFitter constrainedFitter;
        vertexFitTreeGlobal = constrainedFitter.fit(allParticlesGlobal, combinedConstraint);
    }

    if (!vertexFitTreeGlobal || !vertexFitTreeGlobal->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Full vertex fit with muons failed.";
        return false;
    }
    vertexFitTreeGlobal->movePointerToTheTop();
    bhadron = vertexFitTreeGlobal->currentParticle();
    bVertex = vertexFitTreeGlobal->currentDecayVertex();
    if (!bVertex || !bVertex->vertexIsValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "B vertex is not valid.";
        return false;
    }

    vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), static_cast<int>(bhadron->degreesOfFreedom()));
    mass_Bhadron    = bhadron->currentState().mass();

    return true;
}
////////////////////////////////////////////////////////////
/////////TrippleObjectVertexRecoPhoton//////////////////////
////////////////////////////////////////////////////////////
bool KinematicConstrainedFit::TrippleObjectVertexFitRecoPhoton(
    std::vector<reco::TransientTrack> muonTT,
    std::vector<reco::TransientTrack>photonTT,
    ReferenceResonance::ResonanceDetails resonanceDetails, 
    const std::vector<reco::Photon>& photons, 
    TMatrixD* photonCovMatrix,
    reco::Vertex PV,
    const MagneticField& field,
    const TransientTrackBuilder& theB){
    float muonMass = 0.10565837;
    float muon_sigma = 0.0000000001;
    float chi = 0.;
    float ndf = 0.;
    const ParticleMass photon_mass(0.);
    float photon_sigma = 1E-6;
    if (muonTT.size() < 2 || photons.size() < 1) {
        edm::LogWarning("TrippleObjectVertexFit") << "Insufficient input tracks.";
        return false;
    }
    KinematicParticleFactoryFromTransientTrack pFactory;

    KinematicParticleVertexFitter Fitter;

    std::vector<RefCountedKinematicParticle> photonKinematicParticles;
    photonKinematicParticles.push_back(pFactory.particle (muonTT[0], muonMass, chi, ndf, muon_sigma));
    photonKinematicParticles.push_back(pFactory.particle (muonTT[1], muonMass, chi, ndf, muon_sigma));
    photonKinematicParticles.push_back(pFactory.particle(photonTT[0], photon_mass, float(0), float(0), photon_sigma, &photons[0], photonCovMatrix));
    auto fullVertexFitTree = Fitter.fit(photonKinematicParticles);
    if (!fullVertexFitTree || !fullVertexFitTree->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Full vertex fit with muons failed.";
        return false;
    }


        /////////////////////
        // try global fit
        /////////////////////
        vector<RefCountedKinematicParticle> allParticlesGlobal;
        fullVertexFitTree->movePointerToTheTop();
        fullVertexFitTree->movePointerToTheFirstChild();
        RefCountedKinematicParticle mu1Global = fullVertexFitTree->currentParticle();
        FreeTrajectoryState fts(mu1Global->currentState().globalPosition(), mu1Global->currentState().globalMomentum(), mu1Global->currentState().particleCharge(), &field);
        AlgebraicSymMatrix66 muonCov;
        AlgebraicSymMatrix77 muonCov77 = mu1Global->currentState().kinematicParametersError().matrix();
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 6; j++)
          {
            muonCov(i, j) = muonCov77(i, j);
          }
        }
        CartesianTrajectoryError muonErr(muonCov);
        fts.setCartesianError(muonErr);
        reco::TransientTrack muonTTGlobal = theB.build(fts);
        float sigma = 0.001;
        mu1Global = pFactory.particle(muonTTGlobal, mu1Global->currentState().mass(), float(0), float(0), sigma);
        allParticlesGlobal.push_back(mu1Global);

        fullVertexFitTree->movePointerToTheNextChild();
        RefCountedKinematicParticle mu2Global = fullVertexFitTree->currentParticle();
        FreeTrajectoryState fts2(mu2Global->currentState().globalPosition(), mu2Global->currentState().globalMomentum(), mu2Global->currentState().particleCharge(), &field);
        AlgebraicSymMatrix66 muonCov2;
        AlgebraicSymMatrix77 muonCov77_2 = mu2Global->currentState().kinematicParametersError().matrix();
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 6; j++)
          {
            muonCov2(i, j) = muonCov77_2(i, j);
          }
        }
        CartesianTrajectoryError muonErr2(muonCov2);
        fts2.setCartesianError(muonErr2);
        reco::TransientTrack muonTTGlobal2 = theB.build(fts2);
        mu2Global = pFactory.particle(muonTTGlobal2, mu2Global->currentState().mass(), float(0), float(0), sigma);
        allParticlesGlobal.push_back(mu2Global);

        fullVertexFitTree->movePointerToTheNextChild();
        RefCountedKinematicParticle phoGlobal = fullVertexFitTree->currentParticle();
        FreeTrajectoryState fts3(phoGlobal->currentState().globalPosition(), phoGlobal->currentState().globalMomentum(), phoGlobal->currentState().particleCharge(), &field);
        AlgebraicSymMatrix66 photonCov;
        AlgebraicSymMatrix77 photonCov77 = phoGlobal->currentState().kinematicParametersError().matrix();
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 6; j++)
          {
            photonCov(i, j) = photonCov77(i, j);
          }
        }
        CartesianTrajectoryError photonErr(photonCov);
        fts3.setCartesianError(photonErr);
        reco::TransientTrack phoTTGlobal = theB.build(fts3);
        float sigma3 = 0.001;
        phoGlobal = pFactory.particle(phoTTGlobal, phoGlobal->currentState().mass(), float(0), float(0), sigma3);

        allParticlesGlobal.push_back(phoGlobal);

        fullVertexFitTree->movePointerToTheTop();

        // create the constraint
        GlobalPoint vertexPosition(PV.x(), PV.y(), PV.z());
        RefCountedKinematicTree vertexFitTreeGlobal;
        if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::None)
        {
            MultiTrackKinematicConstraint* multiPointingConstraint = new MultiTrackPointingKinematicConstraint(vertexPosition);
            KinematicConstrainedVertexFitter constrainedFitter;
            vertexFitTreeGlobal = constrainedFitter.fit(allParticlesGlobal, multiPointingConstraint);
        }
        else if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Jpsi
                || resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Phi
                || resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::KStar
        )
        {
            double nominalDimuonMass;
            if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Jpsi) nominalDimuonMass = ReferenceResonance::Constants::JpsiMass_PDG;
            else if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::Phi) nominalDimuonMass = ReferenceResonance::Constants::PhiMass_PDG;
            else if(resonanceDetails.resonanceFlag == ReferenceResonance::ResonanceFlag::KStar) nominalDimuonMass = ReferenceResonance::Constants::KStar0Mass_PDG;
            
            MultiTrackKinematicConstraint* massConstraint = new TwoTrackMassKinematicConstraint(nominalDimuonMass);
            MultiTrackKinematicConstraint* multiPointingConstraint = new MultiTrackPointingKinematicConstraint(vertexPosition);
            std::vector<MultiTrackKinematicConstraint*> constraints = {massConstraint, multiPointingConstraint};
            MultiTrackKinematicConstraint* combinedConstraint = new CombinedKinematicConstraint(constraints);
            KinematicConstrainedVertexFitter constrainedFitter;
            vertexFitTreeGlobal = constrainedFitter.fit(allParticlesGlobal, combinedConstraint);
        }


    if(!vertexFitTreeGlobal || !vertexFitTreeGlobal->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Global fit with pointing constraint failed.";
        return false;
    }

    vertexFitTreeGlobal->movePointerToTheTop();

    bhadron = vertexFitTreeGlobal->currentParticle();
    bVertex = vertexFitTreeGlobal->currentDecayVertex();
    if (!bVertex || !bVertex->vertexIsValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "B vertex is not valid.";
        return false;
    }
    vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), static_cast<int>(bhadron->degreesOfFreedom()));
    mass_Bhadron    = bhadron->currentState().mass();
    // delete dimuon_const;
    return true;

}
////////////////////////////////////////////////////////////
/////////TetraObjectVertexConvertedPhoton///////////////////
////////////////////////////////////////////////////////////
bool KinematicConstrainedFit::TetraObjectVertexFitConvertedPhoton(
    std::vector<reco::TransientTrack> muonTT, const double muonMass, 
    std::vector<reco::TransientTrack> electronTT, const double eleMass){

	const ParticleMass zero_mass(0);
    float zero_sigma = 1E-6;
    float eleSigma = 1E-6;
	float muon_sigma = 0.0000000001;
    float chi = 0.;
    float ndf = 0.;
	double nominalJpsiMass =  3.096916;
	float jpsiMsigma = 0.00004;
	KinematicParticleFactoryFromTransientTrack pFactory;
	std::vector<RefCountedKinematicParticle> allParticlesMu;
    allParticlesMu.push_back(pFactory.particle (muonTT[0], muonMass, chi, ndf, muon_sigma));
    allParticlesMu.push_back(pFactory.particle (muonTT[1], muonMass, chi, ndf, muon_sigma));
	KinematicParticleVertexFitter Fitter;
	RefCountedKinematicTree JpsiTree = Fitter.fit(allParticlesMu);
	if(JpsiTree->isEmpty()) return 0;
	KinematicParticleFitter constFitter;
	KinematicConstraint * jpsi_const = new MassKinematicConstraint( nominalJpsiMass, jpsiMsigma);
	JpsiTree = constFitter.fit(jpsi_const,JpsiTree);
	myTree_Jpsi=JpsiTree;
	if(JpsiTree->isEmpty()) {
		delete jpsi_const;
		return 0;
	}
	JpsiTree->movePointerToTheTop();
	RefCountedKinematicParticle Jpsi_branch = JpsiTree->currentParticle();
	// Fit the first photon
	std::vector<RefCountedKinematicParticle> firstPhoton;
	firstPhoton.push_back(pFactory.particle(electronTT[0], eleMass, float(0), float(0), eleSigma));
	firstPhoton.push_back(pFactory.particle(electronTT[1], eleMass, float(0), float(0), eleSigma));

	KinematicParticleVertexFitter photonfitter1;
	RefCountedKinematicTree photonVertexFitTree_firstPhoton = photonfitter1.fit(firstPhoton);
	if (!photonVertexFitTree_firstPhoton || photonVertexFitTree_firstPhoton->isEmpty()) return 0;

	edm::ParameterSet pSet;
	pSet.addParameter<double>("maxDistance", 3);
	pSet.addParameter<int>("maxNbrOfIterations", 10000);
	KinematicParticleVertexFitter photonfitter1_optimized(pSet);
	photonVertexFitTree_firstPhoton = photonfitter1_optimized.fit(firstPhoton);
	if (!photonVertexFitTree_firstPhoton || photonVertexFitTree_firstPhoton->isEmpty()) return 0;

	KinematicParticleFitter csFitterPhoton_firstPhoton;
	KinematicConstraint* photon_const_firstPhoton = new MassKinematicConstraint(zero_mass, zero_sigma);
	photonVertexFitTree_firstPhoton = csFitterPhoton_firstPhoton.fit(photon_const_firstPhoton, photonVertexFitTree_firstPhoton);
	if (!photonVertexFitTree_firstPhoton || photonVertexFitTree_firstPhoton->isEmpty()) {
   		 delete photon_const_firstPhoton;
   		 return 0;
	}
	photonVertexFitTree_firstPhoton->movePointerToTheTop();
	RefCountedKinematicParticle fittedPhotonBranch_firstPhoton = photonVertexFitTree_firstPhoton->currentParticle();

	// Fit the second photon
	std::vector<RefCountedKinematicParticle> secondPhoton;
	secondPhoton.push_back(pFactory.particle(electronTT[2], eleMass, float(0), float(0), eleSigma));
	secondPhoton.push_back(pFactory.particle(electronTT[3], eleMass, float(0), float(0), eleSigma));

	KinematicParticleVertexFitter photonfitter2;
	RefCountedKinematicTree photonVertexFitTree_secondPhoton = photonfitter2.fit(secondPhoton);
	if (!photonVertexFitTree_secondPhoton || photonVertexFitTree_secondPhoton->isEmpty()) return 0;

	KinematicParticleVertexFitter photonfitter2_optimized(pSet);
	photonVertexFitTree_secondPhoton = photonfitter2_optimized.fit(secondPhoton);
	if (!photonVertexFitTree_secondPhoton || photonVertexFitTree_secondPhoton->isEmpty()) return 0;

	KinematicParticleFitter csFitterPhoton_secondPhoton;
	KinematicConstraint* photon_const_secondPhoton = new MassKinematicConstraint(zero_mass, zero_sigma);
	photonVertexFitTree_secondPhoton = csFitterPhoton_secondPhoton.fit(photon_const_secondPhoton, photonVertexFitTree_secondPhoton);
	if (!photonVertexFitTree_secondPhoton || photonVertexFitTree_secondPhoton->isEmpty()) {
		delete photon_const_secondPhoton;
		return 0;
	}
	photonVertexFitTree_secondPhoton->movePointerToTheTop();
	RefCountedKinematicParticle fittedPhotonBranch_secondPhoton = photonVertexFitTree_secondPhoton->currentParticle();
	std::vector<RefCountedKinematicParticle> allParticlesB;
	allParticlesB.push_back(fittedPhotonBranch_firstPhoton);
	allParticlesB.push_back(fittedPhotonBranch_secondPhoton);
	allParticlesB.push_back(Jpsi_branch);

        myTree_B = Fitter.fit(allParticlesB);
        if(myTree_B->isEmpty()) {
                delete jpsi_const;
                return 0;
        }
        myTree_B->movePointerToTheTop();
        bhadron = myTree_B->currentParticle();
        bVertex = myTree_B->currentDecayVertex();
        if (!bVertex->vertexIsValid()) {
                delete jpsi_const;
                return 0;
        }
        vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), (int)bhadron->degreesOfFreedom());
        mass_Bhadron    = bhadron->currentState().mass();
        delete jpsi_const;
        return 1;

}
////////////////////////////////////////////////////////////
/////////TetraObjectVertexRecoPhoton////////////////////////
/////////////////////////////////////////////////////////////
bool KinematicConstrainedFit::TetraObjectVertexFitRecoPhoton(
    std::vector<reco::TransientTrack> muonTT,
    std::vector<reco::TransientTrack>photonTT,
    const double DiMuonMass, const double DiMuonSigma, 
    const std::vector<reco::Photon>& photons,
    std::vector<TMatrixD>&  photonCovMatrix){
    //    TMatrixD& photonCovMatrix){
    float muonMass = 0.10565837;
    float muon_sigma = 0.0000000001;
    float chi = 0.;
    float ndf = 0.;
    const ParticleMass photon_mass(0.);
    float photon_sigma = 1E-6;
    if (muonTT.size() < 2 || photons.size() < 1) {
        edm::LogWarning("TrippleObjectVertexFit") << "Insufficient input tracks.";
        return false;
    }
    KinematicParticleFactoryFromTransientTrack pFactory;
    std::vector<RefCountedKinematicParticle> allParticlesMu;
    allParticlesMu.push_back(pFactory.particle (muonTT[0], muonMass, chi, ndf, muon_sigma));
    allParticlesMu.push_back(pFactory.particle (muonTT[1], muonMass, chi, ndf, muon_sigma));
    KinematicParticleVertexFitter Fitter;
    RefCountedKinematicTree dimuonTree = Fitter.fit(allParticlesMu);
    if (!dimuonTree || !dimuonTree->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Dimuon vertex fit failed.";
        return false;
    }
    KinematicParticleFitter constFitter;
    KinematicConstraint * dimuon_const = new MassKinematicConstraint(DiMuonMass, DiMuonSigma);
    dimuonTree = constFitter.fit(dimuon_const,dimuonTree);
    if (!dimuonTree || !dimuonTree->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Dimuon vertex fit failed.";
        return false;
    }
    dimuonTree->movePointerToTheTop();
    RefCountedKinematicParticle dimuon = dimuonTree->currentParticle();
    std::vector<RefCountedKinematicParticle> photonKinematicParticles;
    photonKinematicParticles.push_back(pFactory.particle(photonTT[0], photon_mass, float(0), 
    float(0), photon_sigma, &photons[0], &photonCovMatrix[0]));
    photonKinematicParticles.push_back(pFactory.particle(photonTT[1], photon_mass, float(0), 
    float(0), photon_sigma, &photons[1], &photonCovMatrix[1]));
    photonKinematicParticles.push_back(dimuon);
    auto fullVertexFitTree = Fitter.fit(photonKinematicParticles);
    if (!fullVertexFitTree || !fullVertexFitTree->isValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "Full vertex fit with muons failed.";
        return false;
    }
    fullVertexFitTree->movePointerToTheTop();

    bhadron = fullVertexFitTree->currentParticle();
    bVertex = fullVertexFitTree->currentDecayVertex();
    if (!bVertex || !bVertex->vertexIsValid()) {
        edm::LogInfo("TrippleObjectVertexFit") << "B vertex is not valid.";
        return false;
    }
    vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), static_cast<int>(bhadron->degreesOfFreedom()));
    mass_Bhadron    = bhadron->currentState().mass();
    delete dimuon_const;
    return true;

}
