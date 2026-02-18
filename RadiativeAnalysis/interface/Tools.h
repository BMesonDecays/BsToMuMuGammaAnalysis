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

    // Klara's method
    inline math::XYZPoint pca(math::XYZPoint pv, math::XYZPoint sv, math::XYZVector mom){
        //s = (PV - SV) * (mom) / |mom|^2 
        double s = ((pv - sv).Dot(mom)) / mom.Mag2();
        //pca = sv + s*mom
        math::XYZPoint PCA = sv + s*mom;

        return PCA;
    }
    // tutaj znajdowany jest punkt najbliższego podejścia (PCA) dla danego primary i secondary vertex oraz wektora pędu
    // w moim przypadku był to pęd cząstki rozpadającej się w tym secondary vertexie
    // PCA ma być na przedłużeniu kierunku pędu
    // s*|mom| to odległość od SV do PCA; tutaj s jest od razu dzielone przez wartość pędu (|mom|)
    // s powinno wyjść z odpowiednim znakiem
    // do wektora położenia SV dodawany jest wektor łączący SV z PCA: s*mom (gdyby wcześniej s nie było dzielone przez |mom|, tutaj byłoby s*(mom/|mom|))


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


    // same as above, but returns the reco::Vertex*
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


    double displacementError (const reco::Vertex & v1, const reco::Vertex & v2)
    {
        double error2 = 0;
        error2 += TMath::Sq(2*(v1.x() - v2.x())) * (v1.xError()*v1.xError() + v2.xError()*v2.xError());
        error2 += TMath::Sq(2*(v1.y() - v2.y())) * (v1.yError()*v1.yError() + v2.yError()*v2.yError());
        error2 += 2*2*(v1.x() - v2.x())*2*(v1.y() - v2.y()) * (v1.covariance(0,1) + v2.covariance(0,1));

        error2 /= 4*((v1.position() - v2.position()).Mag2());

        return TMath::Sqrt(error2);
    }


    // find the minimal 3D distance to any PV
    double minDistPV (math::XYZPoint point, const std::vector<reco::Vertex> & primaryVertices)
    {
        double minDist = 999.;
        for (auto & pv : primaryVertices)
        {
            math::XYZVector displ = point - pv.position();
            double tempDist = TMath::Sqrt(displ.Mag2());
            if (tempDist < minDist)
                minDist = tempDist;
        }
        
        return minDist;
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


    // UNVERIFIED find two points (one for each line) closest to each other for two lines in 3D
    std::vector<math::XYZPoint> closestPoints(const math::XYZPoint &refPoint1, const math::XYZVector &dirVector1,
                                                const math::XYZPoint &refPoint2, const math::XYZVector &dirVector2)
    {
        math::XYZVector dirUnit1 = dirVector1.Unit();
        math::XYZVector dirUnit2 = dirVector2.Unit();
        math::XYZVector nVector = dirUnit1.Cross(dirUnit2);    //direction of the vector connecting the two desired closest points

        math::XYZVector ref1_ref2 = refPoint2 - refPoint1;
        math::XYZVector dir1CrossN = dirUnit1.Cross(nVector);
        math::XYZVector dir2CrossN = dirUnit2.Cross(nVector);
        double dirUnit1DotDirUnit2 = dirUnit1.Dot(dirUnit2);
        double sign = std::abs(dirUnit1DotDirUnit2)/dirUnit1DotDirUnit2;

        // closestPoint1 = refPoint1 + t1*dirUnit1
        // closestPoint2 = refPoint2 + t2*dirUnit2
        double t1 = sign * (dir2CrossN.Dot(ref1_ref2))/(dirUnit1.Dot(dir2CrossN));
        double t2 = sign * (dir1CrossN.Dot(ref1_ref2))/(dirUnit2.Dot(dir1CrossN));
        math::XYZPoint closestPoint1 = refPoint1 + t1*dirUnit1;
        math::XYZPoint closestPoint2 = refPoint2 + t2*dirUnit2;

        std::vector<math::XYZPoint> outputVector;
        outputVector.push_back(closestPoint1);
        outputVector.push_back(closestPoint2);

        return outputVector;
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


}


#endif