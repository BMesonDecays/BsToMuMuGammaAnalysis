#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/KinematicConstrainedFit.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
using namespace reco;
using namespace edm;
using namespace std;
using namespace pat;
#include <TMath.h>


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
/////////////////////////////////////////////
////////////TripleObjectVertex///////////////
/////////////////////////////////////////////
bool KinematicConstrainedFit::TrippleObjectVertexFit(std::vector<reco::TransientTrack> muonTT, const double muonMass, std::vector<reco::TransientTrack> electronTT, const double eleMass){
        const ParticleMass zero_mass(0);
	float zero_sigma = 1E-6;
        float eleSigma = 1E-6;
	KinematicParticleFactoryFromTransientTrack pFactory;
	std::vector<RefCountedKinematicParticle> PhotonParticles;
        PhotonParticles.push_back(pFactory.particle(electronTT[0], eleMass, float(0), float(0), eleSigma));
        PhotonParticles.push_back(pFactory.particle(electronTT[1], eleMass, float(0), float(0), eleSigma));
        KinematicParticleVertexFitter photonfitter;
        RefCountedKinematicTree photonVertexFitTree;
        photonVertexFitTree = photonfitter.fit(PhotonParticles);
	if (!photonVertexFitTree->isEmpty())return 0;
        edm::ParameterSet pSet;
        pSet.addParameter<double>("maxDistance", 3);
        pSet.addParameter<int>("maxNbrOfIterations", 10000);
        KinematicParticleVertexFitter Fitter(pSet);
        photonVertexFitTree = Fitter.fit(PhotonParticles);
        
	if (photonVertexFitTree->isEmpty()) return 0;
	KinematicParticleFitter csFitterPhoton;
        KinematicConstraint * photon_const = new MassKinematicConstraint(zero_mass, zero_sigma);
	photonVertexFitTree = csFitterPhoton.fit(photon_const, photonVertexFitTree);
	if(photonVertexFitTree->isEmpty()){
		delete photon_const;
		return 0;
	}
	photonVertexFitTree->movePointerToTheTop();
	RefCountedKinematicParticle fittedPhotonBranch = photonVertexFitTree->currentParticle();
	float muon_sigma = 0.0000000001;
	float chi = 0.;
	float ndf = 0.;
	std::vector<RefCountedKinematicParticle> allParticlesMu;
	allParticlesMu.push_back(pFactory.particle (muonTT[0], muonMass, chi, ndf, muon_sigma));
	allParticlesMu.push_back(pFactory.particle (muonTT[1], muonMass, chi, ndf, muon_sigma));
        allParticlesMu.push_back(fittedPhotonBranch);
	myTree_B = Fitter.fit(allParticlesMu);
	if(myTree_B->isEmpty()) {
		delete photon_const;
		return 0;
	}
	myTree_B->movePointerToTheTop();
	bhadron = myTree_B->currentParticle();
	bVertex = myTree_B->currentDecayVertex();
	if (!bVertex->vertexIsValid()) {
		delete photon_const;
		return 0;
	}
	vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), (int)bhadron->degreesOfFreedom());
	mass_Bhadron    = bhadron->currentState().mass();
	delete photon_const;
	return 1;
}
/////////////////////////////////////////////
/////////TetraObjectVertex///////////////////
/////////////////////////////////////////////
bool KinematicConstrainedFit::TetraObjectVertexFit(std::vector<reco::TransientTrack> muonTT, const double muonMass, std::vector<reco::TransientTrack> electronTT, const double eleMass){

	const ParticleMass zero_mass(0);
        float zero_sigma = 1E-6;
        float eleSigma = 1E-6;
        std::vector<RefCountedKinematicParticle> PhotonParticles;
	KinematicParticleFactoryFromTransientTrack pFactory;
        PhotonParticles.push_back(pFactory.particle(electronTT[0], eleMass, float(0), float(0), eleSigma));
        PhotonParticles.push_back(pFactory.particle(electronTT[1], eleMass, float(0), float(0), eleSigma));
        PhotonParticles.push_back(pFactory.particle(electronTT[2], eleMass, float(0), float(0), eleSigma));
        PhotonParticles.push_back(pFactory.particle(electronTT[3], eleMass, float(0), float(0), eleSigma));
	KinematicParticleVertexFitter photonfitter;
        RefCountedKinematicTree photonVertexFitTree;
        photonVertexFitTree = photonfitter.fit(PhotonParticles);
        if (!photonVertexFitTree->isEmpty())return 0;
        edm::ParameterSet pSet;
        pSet.addParameter<double>("maxDistance", 3);
        pSet.addParameter<int>("maxNbrOfIterations", 10000);
        KinematicParticleVertexFitter Fitter(pSet);
        photonVertexFitTree = Fitter.fit(PhotonParticles);

        if (photonVertexFitTree->isEmpty()) return 0;
        KinematicParticleFitter csFitterPhoton;
        KinematicConstraint * photon_const = new MassKinematicConstraint(zero_mass, zero_sigma);
        photonVertexFitTree = csFitterPhoton.fit(photon_const, photonVertexFitTree);
        if(photonVertexFitTree->isEmpty()){
                delete photon_const;
                return 0;
        }
        photonVertexFitTree->movePointerToTheTop();
	RefCountedKinematicParticle fittedPhotonBranch = photonVertexFitTree->currentParticle();
        float muon_sigma = 0.0000000001;
        float chi = 0.;
        float ndf = 0.;
        std::vector<RefCountedKinematicParticle> allParticlesMu;
        allParticlesMu.push_back(pFactory.particle (muonTT[0], muonMass, chi, ndf, muon_sigma));
        allParticlesMu.push_back(pFactory.particle (muonTT[1], muonMass, chi, ndf, muon_sigma));
        allParticlesMu.push_back(fittedPhotonBranch);
        myTree_B = Fitter.fit(allParticlesMu);
        if(myTree_B->isEmpty()) {
                delete photon_const;
                return 0;
        }
        myTree_B->movePointerToTheTop();
        bhadron = myTree_B->currentParticle();
        bVertex = myTree_B->currentDecayVertex();
        if (!bVertex->vertexIsValid()) {
                delete photon_const;
                return 0;
        }
        vtxprob_Bhadron = TMath::Prob(bhadron->chiSquared(), (int)bhadron->degreesOfFreedom());
        mass_Bhadron    = bhadron->currentState().mass();
	delete photon_const;
        return 1;

}
