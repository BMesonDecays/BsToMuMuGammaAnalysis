#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_KinematicConstrainedFit_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_KinematicConstrainedFit_h


#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
//#include "SimTracker/Records/interface/TrackAssociatorRecord.h"
#include "RecoVertex/KinematicFitPrimitives/interface/ParticleMass.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/ParticleKinematicLinearizedTrackStateFactory.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h"
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
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include <memory>
#include <cstddef>
#include <cfloat>
#include <string>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector.h"
#include "TLorentzRotation.h"
#include <iostream>
#include <TMath.h>

class KinematicConstrainedFit{
	public: 
		KinematicConstrainedFit();
		~KinematicConstrainedFit(){}
		bool doFit(std::vector<reco::TransientTrack> t_tracks, const double muonMass, const  double mass1, const double  mass2);
		bool BsToJpsiPhiFit(std::vector<reco::TransientTrack> t_tracks, const double muonMass, const  double mass1, const double  mass2);
		bool TrippleObjectVertexFit(std::vector<reco::TransientTrack> muonTT, const double muonMass, std::vector<reco::TransientTrack> electronTT, const double eleMass);
		bool TetraObjectVertexFit(std::vector<reco::TransientTrack> muonTT, const double muonMass, std::vector<reco::TransientTrack> electronTT, const double eleMass);
		
		double getBhadronProb() {return vtxprob_Bhadron;}
		double getBhadronMass() {return mass_Bhadron;}
		RefCountedKinematicParticle getBhardon()       {return bhadron;}
		RefCountedKinematicVertex   getVertex()        {return bVertex;}
		RefCountedKinematicTree     getBhadronTree()   {return myTree_B;}
		RefCountedKinematicTree     getJpsiTree()      {return myTree_Jpsi;}

	private:
	
		double                              vtxprob_Bhadron;
		double                              mass_Bhadron;
		RefCountedKinematicParticle         bhadron;
		RefCountedKinematicVertex           bVertex;
		RefCountedKinematicTree             myTree_B;
		RefCountedKinematicTree             myTree_Jpsi;
		RefCountedKinematicTree             renewed_BsConstrainedTree;
		
		

};
#endif
