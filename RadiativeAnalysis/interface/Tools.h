#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_Tools_H
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_Tools_H

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include <vector>
#include <set>
#include "TMath.h"
#include "Math/Vector4D.h"

namespace Tools{

    // find the smallest distance of closest approach of a momentum line to the best (closest) PV, (SV point needed)
    // return two points: best PV position and the PCA point
    std::vector<math::XYZPoint> points_PV_pca (const std::vector<reco::Vertex> & primaryVertices,
                                                math::XYZPoint svPosition,
                                                math::XYZVector momentum)
    {
        double minDistance = 999.0;
        const reco::Vertex* bestPV_ptr = &primaryVertices.at(0);
        math::XYZPoint bestPCA = primaryVertices.at(0).position();

        for (const auto & pv : primaryVertices)
        {
            const math::XYZPoint & pvPosition = pv.position();
            double s = ((pvPosition - svPosition).Dot(momentum)) / momentum.Mag2();
            math::XYZPoint tempPCA = svPosition + s*momentum;
            double tempDistance = TMath::Sqrt((pvPosition-tempPCA).Mag2());

            if (tempDistance < minDistance)
            {
                minDistance = tempDistance;
                bestPV_ptr = &pv;
                bestPCA = tempPCA;
            }
        }

        std::vector<math::XYZPoint> outputVector;
        outputVector.push_back(bestPV_ptr->position());
        outputVector.push_back(bestPCA);
        return outputVector;
    }


    // same as above, but returns the reco::Vertex
    const reco::Vertex & bestPV (const std::vector<reco::Vertex> & primaryVertices,
                                math::XYZPoint svPosition, math::XYZVector momentum)
    {
        double minDistance = 999.0;
        const reco::Vertex* bestPV_ptr = &primaryVertices.at(0);
        math::XYZPoint bestPCA = primaryVertices.at(0).position();

        for (const auto & pv : primaryVertices)
        {
            const math::XYZPoint & pvPosition = pv.position();
            double s = ((pvPosition - svPosition).Dot(momentum)) / momentum.Mag2();
            math::XYZPoint tempPCA = svPosition + s*momentum;
            double tempDistance = TMath::Sqrt((pvPosition-tempPCA).Mag2());

            if (tempDistance < minDistance)
            {
                minDistance = tempDistance;
                bestPV_ptr = &pv;
                bestPCA = tempPCA;
            }
        }

        return *bestPV_ptr;
    }


    // XY displacement for two reco::Vertex, with uncertainty
    struct displacementXY
    {
        math::XYZVector vector;
        double error;
    };

    Tools::displacementXY getDisplXY (const reco::Vertex & v1, const reco::Vertex & v2)
    {
        Tools::displacementXY displ;
        math::XYZVector displ3DVec = v1.position() - v2.position();
        math::XYZVector displ2DVec (displ3DVec.x(), displ3DVec.y(), 0);
        displ.vector = displ2DVec;

        double error2 = 0;
        error2 += TMath::Sq(2*(v1.x() - v2.x())) * (v1.xError()*v1.xError() + v2.xError()*v2.xError());
        error2 += TMath::Sq(2*(v1.y() - v2.y())) * (v1.yError()*v1.yError() + v2.yError()*v2.yError());
        error2 += 2*2*(v1.x() - v2.x())*2*(v1.y() - v2.y()) * (v1.covariance(0,1) + v2.covariance(0,1));

        error2 /= 4*(displ2DVec.Mag2());
        displ.error = TMath::Sqrt(error2);

        return displ;
    }

    // XY displacement for a KinematicVertex and a reco::Vertex, with uncertainty
    Tools::displacementXY getDisplXY (const KinematicVertex & v1, const reco::Vertex & v2)
    {
        Tools::displacementXY displ;

        GlobalPoint v1Position = v1.position();
        math::XYZPoint v1PositionPoint (v1Position.x(),v1Position.y(),v1Position.z());

        const AlgebraicSymMatrix33 v1CovMatrix = v1.error().matrix();

        math::XYZVector displ3DVec = v1PositionPoint - v2.position();
        math::XYZVector displ2DVec (displ3DVec.x(), displ3DVec.y(), 0);
        displ.vector = displ2DVec;

        double error2 = 0;
        error2 += TMath::Sq(2*(v1Position.x() - v2.x())) * (v1CovMatrix.At(0,0) + v2.xError()*v2.xError());
        error2 += TMath::Sq(2*(v1Position.y() - v2.y())) * (v1CovMatrix.At(1,1) + v2.yError()*v2.yError());
        error2 += 2*2*(v1Position.x() - v2.x())*2*(v1Position.y() - v2.y()) * (v1CovMatrix(0,1) + v2.covariance(0,1));

        error2 /= 4*(displ2DVec.Mag2());
        displ.error = TMath::Sqrt(error2);

        return displ;
    }

    // Lxy from the beam spot for a reco::Vertex
    // neglect beam spot position error (compared to muons vertex positon error)
    std::vector<double> beamSpotVertexLxy(const reco::BeamSpot& beamSpot, const reco::Vertex& vtx)
    {
      const math::XYZPoint bSpotPoint = beamSpot.position(vtx.z());
      math::XYZVector displ3DVec = vtx.position() - bSpotPoint;
      math::XYZVector displ2DVec (displ3DVec.x(), displ3DVec.y(), 0);
      double sig2_d2 = 4*std::pow(vtx.x()-bSpotPoint.x(), 2)*vtx.covariance(0,0);
      sig2_d2 += 4*std::pow(vtx.y()-bSpotPoint.y(), 2)*vtx.covariance(1,1);
      sig2_d2 += 2*2*2*(vtx.x()-bSpotPoint.x())*(vtx.y()-bSpotPoint.y())*vtx.covariance(0,1);

      double lXY_vtx_bSpot = TMath::Sqrt(displ2DVec.Mag2());
      double lXY_vtx_bSpot_error = TMath::Sqrt(sig2_d2) / (2*lXY_vtx_bSpot);
    
      std::vector<double> outVector = {lXY_vtx_bSpot, lXY_vtx_bSpot_error};
      return outVector;
    }
    // the same for a KinematicVertex
    std::vector<double> beamSpotVertexLxy(const reco::BeamSpot& beamSpot, const KinematicVertex& vtx)
    {
        GlobalPoint vtxPosGP = vtx.position();
        math::XYZPoint vtxPosPoint (vtxPosGP.x(),vtxPosGP.y(),vtxPosGP.z());
        const AlgebraicSymMatrix33 vtxCovMatrix = vtx.error().matrix();

        const math::XYZPoint bSpotPoint = beamSpot.position(vtxPosPoint.z());
        math::XYZVector displ3DVec = vtxPosPoint - bSpotPoint;
        math::XYZVector displ2DVec (displ3DVec.x(), displ3DVec.y(), 0);
        double sig2_d2 = 4*std::pow(vtxPosPoint.x()-bSpotPoint.x(), 2)*vtxCovMatrix(0,0);
        sig2_d2 += 4*std::pow(vtxPosPoint.y()-bSpotPoint.y(), 2)*vtxCovMatrix(1,1);
        sig2_d2 += 2*2*2*(vtxPosPoint.x()-bSpotPoint.x())*(vtxPosPoint.y()-bSpotPoint.y())*vtxCovMatrix(0,1);

        double lXY_vtx_bSpot = TMath::Sqrt(displ2DVec.Mag2());
        double lXY_vtx_bSpot_error = TMath::Sqrt(sig2_d2) / (2*lXY_vtx_bSpot);
    
        std::vector<double> outVector = {lXY_vtx_bSpot, lXY_vtx_bSpot_error};
        return outVector;
    }
    


    // find first produced B0s in MC sample
    const reco::Candidate* findFirstB0s(const reco::Candidate* partB0s)
    {
        if (partB0s->numberOfMothers() > 1)   return partB0s; //first B0s/B0s_bar in the B0s-B0s_bar state

        const reco::Candidate* mother = partB0s->mother(0);
        if (std::abs(mother->pdgId()) != 531)  return partB0s; //first B0s produced from non-B0s

        return findFirstB0s(mother);
    }

    // find the point on the beamspot line (bSPoint) closest to a given point (point)
    math::XYZPoint bSpotClosestPoint(math::XYZPoint point, const reco::BeamSpot& bSpot)
    {
        math::XYZVector bSpotDirection = math::XYZVector(bSpot.dxdz(), bSpot.dydz(), 1.0);
        bSpotDirection = bSpotDirection.unit();
        const math::XYZPoint bSpotRefPoint = bSpot.position();

        math::XYZVector point_bSpotRefPoint = point - bSpotRefPoint;
        math::XYZPoint bSPoint = bSpotRefPoint + bSpotDirection*(point_bSpotRefPoint.Dot(bSpotDirection));

        return bSPoint;
    }

    // find the (closest) distance between two lines in 3D
    double closestDistance(const math::XYZPoint &refPoint1, const math::XYZVector &dirVector1,
                                                const math::XYZPoint &refPoint2, const math::XYZVector &dirVector2)
    {
        math::XYZVector dirUnit1 = dirVector1.Unit();
        math::XYZVector dirUnit2 = dirVector2.Unit();
        math::XYZVector closestDistDirection = dirUnit1.Cross(dirUnit2);    //direction of the vector connecting the two desired closest points

        double closestDistance = std::abs(closestDistDirection.Dot(refPoint2 - refPoint1)); //unsigned (closest) distance between two lines
        
        return closestDistance;
    }


    // check whether the same decay (same PDG ids)
    bool isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {    
        if (dec1.size() != dec2.size()) {
            return false; 
        }

        std::set<int> dec1Set(dec1.begin(), dec1.end());
        std::set<int> dec2Set(dec2.begin(), dec2.end());

        return dec1Set == dec2Set;
    }


    // check if no other packed candidate is compatible with the two muons vertex (get the maximal probability)
    double getMaxCompatibility (const std::vector<pat::PackedCandidate> & packedCandidates,
                                reco::TrackRef mu1Track, reco::TrackRef mu2Track, const MagneticField & field,
                                std::vector<reco::TransientTrack> & trackTTs)
    {
        KalmanVertexFitter kvf(false);
      double maxVertexProb = 0.0;
      for (std::vector<pat::PackedCandidate>::const_iterator icand = packedCandidates.begin(); icand < packedCandidates.end(); icand++)
      {
        if (! icand->hasTrackDetails()) continue;
        const reco::Track candTrack = icand->pseudoTrack();

        // deltaR check to eliminate the already used two muons
        if(std::min(reco::deltaR(candTrack,*mu1Track),reco::deltaR(candTrack,*mu2Track))<0.0003) continue;

        reco::TransientTrack candTT = reco::TransientTrack(candTrack, &field);
        trackTTs.push_back(candTT);
        reco::Vertex v3part (TransientVertex(kvf.vertex(trackTTs)));
        double prob3part = TMath::Prob(v3part.chi2(),v3part.ndof());
        if (prob3part > maxVertexProb)
          maxVertexProb = prob3part;
        
        trackTTs.pop_back();
      }
      return maxVertexProb;
    }

    // for PFCandidates
    double getMaxCompatibility (const std::vector<reco::PFCandidate> & packedCandidates,
                                reco::TrackRef mu1Track, reco::TrackRef mu2Track, const MagneticField & field,
                                std::vector<reco::TransientTrack> & trackTTs)
    {
        KalmanVertexFitter kvf(false);
      double maxVertexProb = 0.0;
      for (std::vector<reco::PFCandidate>::const_iterator icand = packedCandidates.begin(); icand < packedCandidates.end(); icand++)
      {
        if (icand->bestTrack() == nullptr) continue;
        const reco::Track* candTrack = icand->bestTrack();

        // deltaR check to eliminate the already used two muons
        if(std::min(reco::deltaR(*candTrack,*mu1Track),reco::deltaR(*candTrack,*mu2Track))<0.0003) continue;

        reco::TransientTrack candTT = reco::TransientTrack(*candTrack, &field);
        trackTTs.push_back(candTT);
        reco::Vertex v3part (TransientVertex(kvf.vertex(trackTTs)));
        double prob3part = TMath::Prob(v3part.chi2(),v3part.ndof());
        if (prob3part > maxVertexProb)
          maxVertexProb = prob3part;
        
        trackTTs.pop_back();
      }
      return maxVertexProb;
    }


    // pT balance at the PV
    std::vector<double> getPtBalanceVars (const reco::Vertex & bestPV, const reco::Vertex & bestPVmod,
        math::XYZTLorentzVector & fittedDimuonLV, math::XYZTLorentzVector & photonLV, math::XYZTLorentzVector & photonModLV)
    {
        math::XYZTLorentzVector candBsLV = fittedDimuonLV + photonLV;
        math::XYZTLorentzVector candBsModLV = fittedDimuonLV + photonModLV;

        math::XYZVector pSumAtPV (0.0,0.0,0.0);
        for (auto iTrackRef = bestPV.tracks_begin(); iTrackRef < bestPV.tracks_end(); iTrackRef++)
        {
            pSumAtPV += iTrackRef->get()->innerMomentum();
        }

        // for PV selected with modified photon energy
        math::XYZVector pSumAtPVmod (0.0,0.0,0.0);    
        for (auto iTrackRef = bestPVmod.tracks_begin(); iTrackRef < bestPVmod.tracks_end(); iTrackRef++)
        {
        pSumAtPVmod += iTrackRef->get()->innerMomentum();
        }
        
        // get the missing transverse momentum
        math::XYZVector pTMissAtPV (-pSumAtPV.x(),-pSumAtPV.y(),0.0);
        math::XYZVector pTMissAtPVmod (-pSumAtPVmod.x(),-pSumAtPVmod.y(),0.0);

        // calculate direction compatibility with the dimuon and candBs (dimuon+photon)
        double cosDimuonPtMiss = pTMissAtPV.unit().Dot(fittedDimuonLV.Vect()) / fittedDimuonLV.Pt();
        double cosDimuonPtMissMod = pTMissAtPVmod.unit().Dot(fittedDimuonLV.Vect()) / fittedDimuonLV.Pt();

        double cosCandBsPtMiss = pTMissAtPV.unit().Dot(candBsLV.Vect()) / candBsLV.Pt();
        double cosCandBsPtMissMod = pTMissAtPVmod.unit().Dot(candBsModLV.Vect()) / candBsModLV.Pt();

        // check if the dimuon and photon are consistent with the missing transverse momentum
        math::XYZVector pTMissMinusDimuon = pTMissAtPV - fittedDimuonLV.Vect();
        math::XYZVector pTMissModMinusDimuon = pTMissAtPVmod - fittedDimuonLV.Vect();

        math::XYZVector pTMissMinusDimuonMinusPhoton = pTMissMinusDimuon - photonLV.Vect();
        math::XYZVector pTMissMinusDimuonMinusPhotonMod = pTMissModMinusDimuon - photonModLV.Vect();

        
        std::vector<double> ptBalanceVars = {pTMissAtPV.Rho(),pTMissAtPVmod.Rho(),
            cosDimuonPtMiss,cosDimuonPtMissMod,cosCandBsPtMiss,cosCandBsPtMissMod,
            pTMissMinusDimuon.Rho(),pTMissModMinusDimuon.Rho(),
            pTMissMinusDimuonMinusPhoton.Rho(),pTMissMinusDimuonMinusPhotonMod.Rho()};

        return ptBalanceVars;
    }

    


}

#endif