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

} // namespace VariableDefinitions