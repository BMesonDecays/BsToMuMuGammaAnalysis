#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TripleObjectVertexFit.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "RecoVertex/VertexPrimitives/interface/VertexException.h"

namespace {
    // transverse distance PV -> SV and its significance; the SV and PV covariances are added
    std::pair<double, double> lxyAndSignificance(const GlobalPoint& sv, const GlobalError& svErr, const reco::Vertex& pv)
    {
        const double dx = sv.x() - pv.x();
        const double dy = sv.y() - pv.y();
        const double lxy = std::hypot(dx, dy);
        if (lxy <= 0) return {lxy, -9999.0};

        const double cxx = svErr.cxx() + pv.covariance(0, 0);
        const double cxy = svErr.cyx() + pv.covariance(0, 1);
        const double cyy = svErr.cyy() + pv.covariance(1, 1);
        const double variance = (dx * dx * cxx + 2 * dx * dy * cxy + dy * dy * cyy) / (lxy * lxy);
        return {lxy, variance > 0 ? lxy / std::sqrt(variance) : -9999.0};
    }

    // cosine of the angle between the PV -> SV direction and the momentum p (3D, or transverse if is2D)
    double cosAlpha(const GlobalPoint& sv, const reco::Vertex& pv, const GlobalVector& p, bool is2D)
    {
        const double dx = sv.x() - pv.x();
        const double dy = sv.y() - pv.y();
        const double dz = is2D ? 0. : sv.z() - pv.z();
        const double pz = is2D ? 0. : p.z();
        const double norm = std::sqrt(dx * dx + dy * dy + dz * dz) * std::sqrt(p.x() * p.x() + p.y() * p.y() + pz * pz);
        if (norm <= 0) return -9999.0;
        return (dx * p.x() + dy * p.y() + dz * pz) / norm;
    }
}


TripleObjectVertexFit::TripleObjectVertexFit() {}
    
template <typename Muon, typename PhotonCollection>
TripleObjectVertexFit::MmgFitResults TripleObjectVertexFit::fitMmgVertices(
    const std::pair< Muon, Muon >& muonPair,
    const PhotonCollection& photons,
    const std::vector<float>& photonMVAs,
    const std::vector<reco::Vertex>& PVs,
    const EcalClusterLazyTools& lazyTools,
    const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
    const MagneticField& bField,
    const TransientTrackBuilder& transientTrackBuilder,
    const GenDecayChain* genChain)

{
    KinematicConstrainedFit fitter; // One fitter for the whole "fitMmgVertex" function

    const auto& muon1 = muonPair.first;
    const auto& muon2 = muonPair.second;

    ReferenceResonance::ResonanceDetails resonanceDetails = ReferenceResonance::findResonances(muon1, muon2);

    reco::TrackRef muTrack1 = muon1.track();
    reco::TrackRef muTrack2 = muon2.track();
    reco::TransientTrack muonTT1 (muTrack1, &bField);
    reco::TransientTrack muonTT2 (muTrack2, &bField);
    std::vector<reco::TransientTrack> muonTTs = {muonTT1, muonTT2};

    // dimuon-mass constrained Kinematic Fit of two muons
    // vertex fits can throw on numerically singular matrices: treat this as a failed fit
    bool dimuonFitSuccess = false;
    try {
        dimuonFitSuccess = fitter.fitDimuon(muonTTs, nominalMuonMass, resonanceDetails);
    } catch (const VertexException& e) {
        edm::LogInfo("TripleObjectVertexFit") << "Dimuon fit threw VertexException: " << e.message();
    }
    if (!dimuonFitSuccess)	return MmgFitResults();

    GlobalVector kinDimuonMomentum = fitter.getDimuon()->currentState().globalMomentum();
    double kinDimuonEnergy = fitter.getDimuon()->currentState().kinematicParameters().energy();
    GlobalPoint SV_point = fitter.getDimuonVtx()->position();
    GlobalError SV_error = fitter.getDimuonVtx()->error();    
    math::XYZPoint SV_mathPoint = math::XYZPoint( SV_point.x(), SV_point.y(), SV_point.z() );

    // select photons based on deltaR with the dimuon. Photon's vertex is set on the SV (dimuon vertex).
    PhotonSelector photonSelector;
    auto photonSelection = photonSelector.selectRecoPhotons( photons, fitter.getDimuon(), SV_mathPoint, 0.5);
    std::vector<size_t> photonIndices = photonSelection.first;
    std::vector<double> photonDeltaRs = photonSelection.second;

    // prepare the output struct "tripleVertexResults"
    MmgFitResults tripleVertexResults(photonIndices.size());    // one entry to each vector for each photon

    // loop over photons
    for (size_t i = 0; i < photonIndices.size(); ++i)
    {   
        size_t photonIdx = photonIndices.at(i);
        reco::Photon photon = photons.at(photonIdx);
        photon.setVertex(SV_mathPoint);     // IMPORTANT

        TLorentzVector photonLV, dimuonLV;
        photonLV.SetPtEtaPhiE(photon.pt(), photon.eta(), photon.phi(), photon.energy());
        dimuonLV.SetPxPyPzE( kinDimuonMomentum.x(), kinDimuonMomentum.y(), kinDimuonMomentum.z(), kinDimuonEnergy );
        TLorentzVector photonDimuonLV = photonLV + dimuonLV;

        // photon and mu mu gamma variables: filled for every selected photon
        tripleVertexResults.photonDeltaRs.at(i) = photonDeltaRs.at(i);
        tripleVertexResults.photonPt.at(i)      = photon.pt();
        tripleVertexResults.photonEta.at(i)     = photon.eta();
        tripleVertexResults.photonPhi.at(i)     = photon.phi();
        tripleVertexResults.photonEnergy.at(i)  = photon.energy();
        tripleVertexResults.photonMVA.at(i)     = photonMVAs.at(photonIdx);
        if (genChain) {
            // photon direction is taken from the SV (setVertex above)
            auto photonGenMatch = genChain->matchPhotonToChain(photon);
            if (photonGenMatch.isMatched) tripleVertexResults.photonGenDeltaR.at(i) = photonGenMatch.deltaR;
        }

        tripleVertexResults.photonDimuonMass.at(i) = photonDimuonLV.M();
        tripleVertexResults.photonDimuonPt.at(i)   = photonDimuonLV.Pt();
        tripleVertexResults.photonDimuonEta.at(i)  = photonDimuonLV.Eta();
        tripleVertexResults.photonDimuonPhi.at(i)  = photonDimuonLV.Phi();

        // find the best PV (lowest DCA to the mu mu gamma direction from the SV); used in the Bs fit
        GlobalVector photonDimuon_momentum = GlobalVector(photonDimuonLV.Px(), photonDimuonLV.Py(), photonDimuonLV.Pz());
        int lowestDCAIndex = BeamSpotAndVertex::LowestDCAIndex( PVs, SV_point, photonDimuon_momentum );
        reco::Vertex PVlowestDCA = PVs.at( lowestDCAIndex );

        // PV variables: filled for every selected photon
        tripleVertexResults.pvIndex.at(i)   = lowestDCAIndex;
        tripleVertexResults.pvX.at(i)       = PVlowestDCA.x();
        tripleVertexResults.pvY.at(i)       = PVlowestDCA.y();
        tripleVertexResults.pvZ.at(i)       = PVlowestDCA.z();
        tripleVertexResults.pvErrX.at(i)    = PVlowestDCA.xError();
        tripleVertexResults.pvErrY.at(i)    = PVlowestDCA.yError();
        tripleVertexResults.pvErrZ.at(i)    = PVlowestDCA.zError();
        tripleVertexResults.pvCovXX.at(i)   = PVlowestDCA.covariance(0, 0);
        tripleVertexResults.pvCovXY.at(i)   = PVlowestDCA.covariance(0, 1);
        tripleVertexResults.pvCovXZ.at(i)   = PVlowestDCA.covariance(0, 2);
        tripleVertexResults.pvCovYY.at(i)   = PVlowestDCA.covariance(1, 1);
        tripleVertexResults.pvCovYZ.at(i)   = PVlowestDCA.covariance(1, 2);
        tripleVertexResults.pvCovZZ.at(i)   = PVlowestDCA.covariance(2, 2);
        tripleVertexResults.pvNdof.at(i)    = PVlowestDCA.ndof();
        tripleVertexResults.pvRho.at(i)     = PVlowestDCA.position().Rho();
        tripleVertexResults.pvNTracks.at(i) = PVlowestDCA.nTracks();
        tripleVertexResults.mu1IsTightSelPV.at(i) = muon::isTightMuon(muon1, PVlowestDCA);
        tripleVertexResults.mu2IsTightSelPV.at(i) = muon::isTightMuon(muon2, PVlowestDCA);

        // mu mu gamma displacement and pointing w.r.t. this PV (photon assigned to the dimuon vertex)
        auto [photonDimuonLxy, photonDimuonLxySig] = lxyAndSignificance(SV_point, SV_error, PVlowestDCA);
        tripleVertexResults.photonDimuonLxy.at(i)        = photonDimuonLxy;
        tripleVertexResults.photonDimuonLxySig.at(i)     = photonDimuonLxySig;
        tripleVertexResults.photonDimuonCosAlpha.at(i)   = cosAlpha(SV_point, PVlowestDCA, photonDimuon_momentum, false);
        tripleVertexResults.photonDimuonCosAlpha2D.at(i) = cosAlpha(SV_point, PVlowestDCA, photonDimuon_momentum, true);

        // Bs variables: filled only if the mass window and the Bs fit are passed
        MassLimits m_lim;
        if (photonDimuonLV.M() < m_lim.BsMassCutLower || photonDimuonLV.M() > m_lim.BsMassCutUpper) continue;

        // the shower position covariance needs the supercluster crystals; in 2024 MINIAOD ~5% of slimmedPhotons
        // carry a supercluster without hitsAndFractions (EcalClusterLazyTools would throw "InvalidCluster")
        if (photon.superCluster()->hitsAndFractions().empty()) continue;

        // prepare the photon for a kinematic fit
        GlobalVector photon_momentum (photon.px(), photon.py(), photon.pz());
        TrackCharge photon_charge = 0;
        FreeTrajectoryState photonFTS = FreeTrajectoryState(SV_point, photon_momentum, photon_charge, &bField);

        auto covPtr = std::make_unique<TMatrixD>( lazyTools.covariancesXYZ( *photon.superCluster() ) );
        AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
        AlgebraicVector6 diagonal(1e6, 1e6, 1e6, 1e6, 1e6, 1e6);
        photonCov.SetDiagonal(diagonal);
        CartesianTrajectoryError photonErr(photonCov);
        photonFTS.setCartesianError(photonErr);

        reco::TransientTrack photonTT = transientTrackBuilder.build( photonFTS );
        if ( !photonTT.isValid() ) continue;
        std::vector<reco::TransientTrack> photonTTs = {photonTT};

        // perform the Bs kinematic fit
        bool bsFitSuccess = false;
        try {
            bsFitSuccess = fitter.TrippleObjectVertexFitRecoPhoton(
                muonTTs, photonTTs, resonanceDetails, std::vector<reco::Photon>({photon}), covPtr.get(), PVlowestDCA, bField, transientTrackBuilder);
        } catch (const VertexException& e) {
            // e.g. "BasicSingleVertexState::could not invert weight matrix" (seen in 2024C data)
            edm::LogInfo("TripleObjectVertexFit") << "Bs fit threw VertexException: " << e.message();
        }
        if ( !bsFitSuccess ) continue;
        RefCountedKinematicParticle bsMeson = fitter.getBhadron();
        if (!bsMeson)   continue;

        const auto& bsState = bsMeson->currentState();
        const auto bsMomentum = bsState.globalMomentum();
        const auto bsParameters = bsState.kinematicParameters();

        // Bs variables
        tripleVertexResults.bsVtxProb.at(i) = fitter.getBhadronProb();
        if (fitter.getVertex()) {
            const GlobalPoint bsVtxPoint = fitter.getVertex()->position();
            tripleVertexResults.bsVtxX.at(i) = bsVtxPoint.x();
            tripleVertexResults.bsVtxY.at(i) = bsVtxPoint.y();
            tripleVertexResults.bsVtxZ.at(i) = bsVtxPoint.z();

            auto [bsLxy, bsLxySig] = lxyAndSignificance(bsVtxPoint, fitter.getVertex()->error(), PVlowestDCA);
            tripleVertexResults.bsLxy.at(i)    = bsLxy;
            tripleVertexResults.bsLxySig.at(i) = bsLxySig;

            const double bsL3D = std::hypot(bsVtxPoint.x() - PVlowestDCA.x(), bsVtxPoint.y() - PVlowestDCA.y(), bsVtxPoint.z() - PVlowestDCA.z());
            if (bsMomentum.mag() > 0) tripleVertexResults.bsCt.at(i) = bsL3D * bsParameters.mass() / bsMomentum.mag();
        }
        tripleVertexResults.bsMass.at(i) = bsParameters.mass();
        tripleVertexResults.bsEnergy.at(i) = bsParameters.energy();
        tripleVertexResults.bsPt.at(i) = bsMomentum.perp();
        tripleVertexResults.bsEta.at(i) = bsMomentum.eta();
        tripleVertexResults.bsPhi.at(i) = bsMomentum.phi();
        tripleVertexResults.bsFitSuccessful.at(i) = true;

    } // loop over photons


    // dimuon variables
    tripleVertexResults.resonanceFlag = static_cast<int>(resonanceDetails.resonanceFlag); 
    tripleVertexResults.dimuonVtxProb = fitter.getDimuonProb();
    tripleVertexResults.dimuonMass = resonanceDetails.mass;
    tripleVertexResults.dimuonFitMass = fitter.getDimuon()->currentState().mass();
    tripleVertexResults.dimuonEta  = kinDimuonMomentum.eta();
    tripleVertexResults.dimuonPhi  = kinDimuonMomentum.phi();
    tripleVertexResults.dimuonPt   = kinDimuonMomentum.perp();
    if (genChain && genChain->info().isValid) {
        // separates the photon energy resolution from the dimuon one
        TLorentzVector dimuonLV;
        dimuonLV.SetPxPyPzE( kinDimuonMomentum.x(), kinDimuonMomentum.y(), kinDimuonMomentum.z(), kinDimuonEnergy );
        tripleVertexResults.dimuonGenPhotonMass = (dimuonLV + genChain->info().signalPhotonP4).M();
    }


    return tripleVertexResults;

}

// Explicit instantiations (the template is defined in this .cc file, so used combinations must be listed here)
// pair of references: keeps pat::Muon overrides (embedded tracks in MINIAOD) instead of slicing to reco::Muon
template TripleObjectVertexFit::MmgFitResults TripleObjectVertexFit::fitMmgVertices<const reco::Muon&, edm::View<reco::Photon>>(
    const std::pair<const reco::Muon&, const reco::Muon&>&, const edm::View<reco::Photon>&, const std::vector<float>&, const std::vector<reco::Vertex>&,
    const EcalClusterLazyTools&, const BeamSpotAndVertex::BSAndVtxVariables&, const MagneticField&, const TransientTrackBuilder&, const GenDecayChain*);
// gen-matched photons only (MC study)
template TripleObjectVertexFit::MmgFitResults TripleObjectVertexFit::fitMmgVertices<const reco::Muon&, std::vector<reco::Photon>>(
    const std::pair<const reco::Muon&, const reco::Muon&>&, const std::vector<reco::Photon>&, const std::vector<float>&, const std::vector<reco::Vertex>&,
    const EcalClusterLazyTools&, const BeamSpotAndVertex::BSAndVtxVariables&, const MagneticField&, const TransientTrackBuilder&, const GenDecayChain*);
