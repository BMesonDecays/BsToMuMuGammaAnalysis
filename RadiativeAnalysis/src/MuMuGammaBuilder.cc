#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/MuMuGammaBuilder.h"
#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/CandidateObservables.h"

#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicVertex.h"
#include "TMath.h"

#include <cmath>
#include <map>
#include <utility>

namespace bmmg {

  namespace {
    TLorentzVector fittedP4(const RefCountedKinematicParticle& p) {
      const KinematicState& s = p->currentState();
      const GlobalVector m = s.globalMomentum();
      return TLorentzVector(m.x(), m.y(), m.z(), s.kinematicParameters().energy());
    }
  }  // namespace

  std::vector<MMGCandidate> buildMuMuGamma(const std::vector<SelectedMuon>& muons,
                                           const std::vector<DimuonCandidate>& dimuons,
                                           const std::vector<SelectedPhoton>& photons,
                                           const std::vector<reco::Vertex>& pvs,
                                           const EcalClusterLazyTools& lazyTools,
                                           const TransientTrackBuilder& ttBuilder,
                                           const MMGVertexFit& fitter,
                                           const MuMuGammaConfig& config) {
    std::vector<MMGCandidate> candidates;
    std::map<std::pair<int, int>, float> compatibilityCache;   // (dimuon, pv) -> max track compatibility

    for (size_t id = 0; id < dimuons.size(); ++id) {
      const DimuonCandidate& dimu = dimuons[id];
      if (!dimu.vtxValid) continue;
      const SelectedMuon& mu1 = muons[dimu.mu1];
      const SelectedMuon& mu2 = muons[dimu.mu2];

      for (size_t ip = 0; ip < photons.size(); ++ip) {
        const reco::Photon& pho = *photons[ip].photon;
        MMGCandidate c;
        c.dimuon = id;
        c.mu1 = dimu.mu1;
        c.mu2 = dimu.mu2;
        c.photon = ip;
        c.prefitP4 = dimu.p4 + pho.p4();
        if (c.prefitP4.mass() < config.minPrefitMass || c.prefitP4.mass() > config.maxPrefitMass) continue;

        // primary vertex: closest to the line through the dimuon vertex along the pre-fit momentum
        c.pv = lowestDCAVertex(pvs, dimu.vtx, GlobalVector(c.prefitP4.px(), c.prefitP4.py(), c.prefitP4.pz()));
        if (c.pv < 0) continue;
        const reco::Vertex& pv = pvs[c.pv];

        if (mu1.kinParticle && mu2.kinParticle)
          c.fit = fitter.fitCandidate(*mu1.kinParticle, *mu2.kinParticle, pho, lazyTools, &ttBuilder,
                                      GlobalPoint(pv.x(), pv.y(), pv.z()));

        if (c.fit) {
          const RefCountedKinematicParticle& b = c.fit->particle;
          const RefCountedKinematicVertex& v = c.fit->vertex;
          const KinematicState& s = b->currentState();
          const double massVar = s.kinematicParametersError().matrix()(6, 6);
          c.massErr = massVar > 0 ? std::sqrt(massVar) : -1.f;
          // the state of the top particle is at the decay vertex; its 7x7 covariance (x,y,z,px,py,pz,m)
          // gives the SV position, the momentum and their correlations
          const AlgebraicSymMatrix66 cov6 = s.kinematicParametersError().matrix().Sub<AlgebraicSymMatrix66>(0, 0);
          c.flight = flightInfo(pv, s.globalPosition(), s.globalMomentum(), cov6, config.massHypothesis);

          const TLorentzVector bP4 = fittedP4(b);
          const TLorentzVector m1 = fittedP4(c.fit->daughters.at(0));
          const TLorentzVector m2 = fittedP4(c.fit->daughters.at(1));
          const TLorentzVector g = fittedP4(c.fit->daughters.at(2));
          c.helicity = helicityAngle(g, bP4);
          c.coplanarity = coplanarityAngle(m1, m2, g, bP4);
          c.cosThetaL = leptonAngle(m1, m2);   // daughters 0, 1 = mu+ , mu-
          c.isolation = candidateIsolation(
              reco::Candidate::LorentzVector(bP4.Px(), bP4.Py(), bP4.Pz(), bP4.E()), *mu1.muon, *mu2.muon, pv);
        } else {
          const TLorentzVector bP4 = toTLV(c.prefitP4);
          c.helicity = helicityAngle(toTLV(pho.p4()), bP4);
          c.coplanarity = coplanarityAngle(toTLV(mu1.muon->p4()), toTLV(mu2.muon->p4()), toTLV(pho.p4()), bP4);
          c.cosThetaL = leptonAngle(toTLV(mu1.muon->p4()), toTLV(mu2.muon->p4()));
          c.isolation = candidateIsolation(c.prefitP4, *mu1.muon, *mu2.muon, pv);
        }

        if (config.computeTrackCompatibility) {
          const auto key = std::make_pair(static_cast<int>(id), c.pv);
          auto it = compatibilityCache.find(key);
          if (it == compatibilityCache.end())
            it = compatibilityCache.emplace(key, maxTrackCompatibility(mu1.track, mu2.track, pv, ttBuilder)).first;
          c.maxTrackCompatibility = it->second;
        }
        candidates.push_back(std::move(c));
      }
    }
    return candidates;
  }

}  // namespace bmmg
