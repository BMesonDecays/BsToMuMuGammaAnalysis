#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/VariableDefinitions.h"

namespace VariableDefinitions {

double calculateCt2D(const reco::Vertex& PV,
                     const RefCountedKinematicVertex& bVertex,
                     const RefCountedKinematicParticle& bCand)
{
    MassLimits m_lim;
    VertexDistanceXY d;
    const GlobalPoint mySV = bVertex->position();
    const GlobalError mySVErr = bVertex->error();
    const GlobalPoint refVertex(PV.x(), PV.y(), PV.z());
    const GlobalError refVertexErr = PV.error();
    GlobalVector momentumVec = bCand->currentState().globalMomentum();

    Measurement1D VtxDist = d.distance(VertexState(mySV, mySVErr), VertexState(refVertex, refVertexErr));
    double transmom = std::sqrt(momentumVec.y() * momentumVec.y() + momentumVec.x() * momentumVec.x());
    if (transmom == 0) return -1.0;
    return m_lim.BsPDGMass * VtxDist.value() / transmom;
}

double calculateCt3D(const reco::Vertex& PV,
                     const RefCountedKinematicVertex& bVertex,
                     const RefCountedKinematicParticle& bCand)
{
    MassLimits m_lim;
    VertexDistance3D d;
    const GlobalPoint mySV = bVertex->position();
    const GlobalError mySVErr = bVertex->error();
    const GlobalPoint refVertex(PV.x(), PV.y(), PV.z());
    const GlobalError refVertexErr = PV.error();
    GlobalVector momentumVec = bCand->currentState().globalMomentum();

    Measurement1D VtxDist = d.distance(VertexState(mySV, mySVErr), VertexState(refVertex, refVertexErr));
    double momMag = momentumVec.mag();
    if (momMag == 0) return -1.0;
    return m_lim.BsPDGMass * VtxDist.value() / momMag;
}

double calculateCt2DError(const reco::Vertex& PV,
                          const RefCountedKinematicVertex& bVertex,
                          const RefCountedKinematicParticle& bCand)
{
    MassLimits m_lim;
    VertexDistanceXY d;
    const GlobalPoint mySV = bVertex->position();
    const GlobalError mySVErr = bVertex->error();
    const GlobalPoint refVertex(PV.x(), PV.y(), PV.z());
    const GlobalError refVertexErr = PV.error();
    GlobalVector momentumVec = bCand->currentState().globalMomentum();
    AlgebraicSymMatrix77 momErr = bCand->currentState().kinematicParametersError().matrix();

    Measurement1D VtxDist = d.distance(VertexState(mySV, mySVErr), VertexState(refVertex, refVertexErr));
    double VtxDistErr = VtxDist.error();
    double transmom = std::sqrt(momentumVec.y() * momentumVec.y() + momentumVec.x() * momentumVec.x());

    double dpx = std::sqrt(momErr(3, 3));
    double dpy = std::sqrt(momErr(4, 4));

    double dpt = std::sqrt(std::pow(momentumVec.x() * dpx, 2) + std::pow(momentumVec.y() * dpy, 2)) / transmom;

    if (transmom == 0 || VtxDist.value() == 0) return -1.0;

    double firstTerm2 = std::pow(m_lim.BsPDGMass * VtxDistErr / transmom, 2.);
    double secondTerm2 = std::pow(m_lim.BsPDGMass * VtxDist.value() * dpt / (transmom * transmom), 2.);

    return std::sqrt(firstTerm2 + secondTerm2);
}

double calculateCt3DError(const reco::Vertex& PV,
                          const RefCountedKinematicVertex& bVertex,
                          const RefCountedKinematicParticle& bCand)
{
    MassLimits m_lim;
    VertexDistance3D d;
    const GlobalPoint mySV = bVertex->position();
    const GlobalError mySVErr = bVertex->error();
    const GlobalPoint refVertex(PV.x(), PV.y(), PV.z());
    const GlobalError refVertexErr = PV.error();
    GlobalVector momentumVec = bCand->currentState().globalMomentum();
    AlgebraicSymMatrix77 momErr = bCand->currentState().kinematicParametersError().matrix();

    Measurement1D VtxDist = d.distance(VertexState(mySV, mySVErr), VertexState(refVertex, refVertexErr));
    double VtxDistErr = VtxDist.error();
    double momMag = momentumVec.mag();

    double dpx = std::sqrt(momErr(3, 3));
    double dpy = std::sqrt(momErr(4, 4));
    double dpz = std::sqrt(momErr(5, 5));

    double dmomMag = std::sqrt(std::pow(momentumVec.x() * dpx, 2) + std::pow(momentumVec.y() * dpy, 2) + std::pow(momentumVec.z() * dpz, 2)) / momMag;

    if (momMag == 0 || VtxDist.value() == 0) return -1.0;

    double firstTerm2 = std::pow(m_lim.BsPDGMass * VtxDistErr / momMag, 2.);
    double secondTerm2 = std::pow(m_lim.BsPDGMass * VtxDist.value() * dmomMag / (momMag * momMag), 2.);

    return std::sqrt(firstTerm2 + secondTerm2);
}

double isolation(const reco::Candidate::LorentzVector& candidateP4,
                 const reco::Muon& mu1,
                 const reco::Muon& mu2,
                 reco::Vertex PV,
                 double drMax, double ptMin)
{
    double sumPt = 0.0;
    for (const auto& trk : PV.tracks()) {
        double dR = reco::deltaR(candidateP4, *trk);
        double mu1dR = reco::deltaR(mu1, *trk);
        double mu2dR = reco::deltaR(mu2, *trk);
        // exclude muon tracks
        if (trk->charge() == 0) continue;
        if (trk->pt() < ptMin) continue;
        if (mu1dR < 0.01 || mu2dR < 0.01) continue;
        if (dR > drMax) continue;
        sumPt += trk->pt();
        
    }
    return candidateP4.pt() / (candidateP4.pt() + sumPt);
}


} // namespace VariableDefinitions