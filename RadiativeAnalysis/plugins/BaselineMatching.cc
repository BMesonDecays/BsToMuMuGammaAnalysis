#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/BaselineMatching.h"


float BaselineMatching::safeDivide(float num, float den) const
{
    return (den != 0.f ? num / den : 9999.f);
}


const BaselineMatching::MatchPair& 
BaselineMatching::getBetterMatch(const MatchPair& m1, const MatchPair& m2) const
{
    // Prefer DT over CSC
    if (m2.first->detector() == MuonSubdetId::DT &&
        m1.first->detector() != MuonSubdetId::DT)
        return m2;

    // Compare |dx|
    float dx1 = std::abs(m1.first->x - m1.second->x);
    float dx2 = std::abs(m2.first->x - m2.second->x);

    if (dx2 < dx1)
        return m2;

    return m1;
}


float BaselineMatching::dX(const MatchPair& m) const
{
    if (m.first && m.second->hasPhi())
        return m.first->x - m.second->x;
    return 9999.f;
}

float BaselineMatching::pullX(const MatchPair& m) const
{
    if (m.first && m.second->hasPhi()) {
        float err2 = std::pow(m.first->xErr, 2) + std::pow(m.second->xErr, 2);
        return safeDivide(dX(m), std::sqrt(err2));
    }
    return 9999.f;
}

float BaselineMatching::pullDxDz(const MatchPair& m) const
{
    if (m.first && m.second->hasPhi()) {
        float num = (m.first->dXdZ - m.second->dXdZ);
        float err2 = std::pow(m.first->dXdZErr, 2) + std::pow(m.second->dXdZErr, 2);
        return safeDivide(num, std::sqrt(err2));
    }
    return 9999.f;
}


float BaselineMatching::dY(const MatchPair& m) const
{
    if (m.first && m.second->hasZed())
        return m.first->y - m.second->y;
    return 9999.f;
}

float BaselineMatching::pullY(const MatchPair& m) const
{
    if (m.first && m.second->hasZed()) {
        float err2 = std::pow(m.first->yErr, 2) + std::pow(m.second->yErr, 2);
        return safeDivide(dY(m), std::sqrt(err2));
    }
    return 9999.f;
}

float BaselineMatching::pullDyDz(const MatchPair& m) const
{
    if (m.first && m.second->hasZed()) {
        float num = (m.first->dYdZ - m.second->dYdZ);
        float err2 = std::pow(m.first->dYdZErr, 2) + std::pow(m.second->dYdZErr, 2);
        return safeDivide(num, std::sqrt(err2));
    }
    return 9999.f;
}
