#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/DimuonVertexCompatibility.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexPrimitives/interface/VertexException.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "TMath.h"

#include <cmath>


bool DimuonVertexCompatibility::isSameTrack(const reco::Track& track, const reco::Track& muonTrack)
{
    if (track.charge() != muonTrack.charge())   return false;
    if (muonTrack.pt() <= 0)                    return false;
    if (std::abs(track.pt() - muonTrack.pt()) / muonTrack.pt() >= sameTrackMaxRelDPt)   return false;
    return reco::deltaR(track, muonTrack) < sameTrackMaxDeltaR;
}


double DimuonVertexCompatibility::ipSignificance2(const reco::Track& track, const GlobalPoint& vtx, const GlobalError& vtxErr)
{
    // linearised around the track reference point: no propagation, valid on the mm-cm scale of B decays
    const math::XYZPoint vtxPoint(vtx.x(), vtx.y(), vtx.z());
    const double dxy = track.dxy(vtxPoint);
    const double dz  = track.dz(vtxPoint);
    // vertex error: cxx + cyy overestimates the transverse projection, so fewer tracks are skipped (conservative)
    const double varXY = track.dxyError() * track.dxyError() + vtxErr.cxx() + vtxErr.cyy();
    const double varZ  = track.dzError()  * track.dzError()  + vtxErr.czz();
    if (varXY <= 0 || varZ <= 0)  return 0.;   // no usable errors: never skip
    return dxy * dxy / varXY + dz * dz / varZ;
}


double DimuonVertexCompatibility::maxThirdTrackVtxProb(
    const reco::TransientTrack& muonTT1,
    const reco::TransientTrack& muonTT2,
    const std::vector<reco::TransientTrack>& tracks)
{
    KalmanVertexFitter fitter(false);   // no track refit

    // dimuon vertex: reference point for skipping incompatible tracks
    // (position/error are computed lazily and can throw too, so they are inside the try block)
    GlobalPoint dimuonVtxPos;
    GlobalError dimuonVtxErr;
    try {
        const TransientVertex dimuonVertex = fitter.vertex(std::vector<reco::TransientTrack>{muonTT1, muonTT2});
        if (!dimuonVertex.isValid())  return 0.0;
        dimuonVtxPos = dimuonVertex.position();
        dimuonVtxErr = dimuonVertex.positionError();
    } catch (const VertexException&) {
        return 0.0;     // numerically singular dimuon fit
    }
    const double maxIPSig2 = maxIPSignificance * maxIPSignificance;

    std::vector<reco::TransientTrack> vtxTracks = {muonTT1, muonTT2, muonTT1};  // third slot replaced below

    double maxVtxProb = 0.0;
    for (const auto& trackTT : tracks)
    {
        const reco::Track& track = trackTT.track();
        if (ipSignificance2(track, dimuonVtxPos, dimuonVtxErr) > maxIPSig2)  continue;
        if (isSameTrack(track, muonTT1.track()) || isSameTrack(track, muonTT2.track()))  continue;

        vtxTracks[2] = trackTT;
        double prob = 0.0;
        try {
            TransientVertex vertex = fitter.vertex(vtxTracks);
            if (!vertex.isValid())  continue;
            prob = TMath::Prob(vertex.totalChiSquared(), static_cast<int>(std::lround(vertex.degreesOfFreedom())));
        } catch (const VertexException&) {
            continue;   // numerically singular fit with this track (e.g. a degenerate pseudo-track covariance)
        }
        if (prob > maxVtxProb)  maxVtxProb = prob;
    }
    return maxVtxProb;
}
