#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/TripleObjectVertexFit.h"


TripleObjectVertexFit::TripleObjectVertexFit() {}
    
template <typename Muon, typename PhotonCollection>
TripleObjectVertexFit::MmgFitResults TripleObjectVertexFit::fitMmgVertices(
    const std::pair< Muon, Muon >& muonPair,
    const PhotonCollection& photons,
    const std::vector<reco::Vertex>& PVs,
    const EcalClusterLazyTools& lazyTools,
    const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
    const MagneticField& bField,
    const TransientTrackBuilder& transientTrackBuilder)
        
{
    // if (muonPair.size() < 2 || photons.size() < 1){
    //     edm::LogWarning("TrippleObjectVertexFit") << "Insufficient number of input particles.";
    //     return {};
    // }
    // else if (muonPair.size() > 2){
    //     edm::LogInfo("TrippleObjectVertexFit") << "Only the first two muons will be used.";
    // }

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
    bool dimuonFitSuccess = fitter.fitDimuon(muonTTs, nominalMuonMass, resonanceDetails);
    if (!dimuonFitSuccess)	return {};

    GlobalVector kinDimuonMomentum = fitter.getDimuon()->currentState().globalMomentum();
    double kinDimuonEnergy = fitter.getDimuon()->currentState().kinematicParameters().energy();
    GlobalPoint SV_point = fitter.getDimuonVtx()->position();    
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
        
        MassLimits m_lim;
        if (photonDimuonLV.M() < m_lim.BsMassCutLower || photonDimuonLV.M() > m_lim.BsMassCutUpper) continue;

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

        // find the best PV
        GlobalVector photonDimuon_momentum = GlobalVector(photonDimuonLV.Px(), photonDimuonLV.Py(), photonDimuonLV.Pz());
        int lowestDCAIndex = BeamSpotAndVertex::LowestDCAIndex( PVs, SV_point, photonDimuon_momentum );
        reco::Vertex PVlowestDCA = PVs.at( lowestDCAIndex );

        // perform the Bs kinematic fit
        bool bsFitSuccess = fitter.TrippleObjectVertexFitRecoPhoton(
            muonTTs, photonTTs, resonanceDetails, std::vector<reco::Photon>({photon}), covPtr.get(), PVlowestDCA, bField, transientTrackBuilder);
        if ( !bsFitSuccess ) continue;
        RefCountedKinematicParticle bsMeson = fitter.getBhadron();
        if (!bsMeson)   continue;

        const auto& bsState = bsMeson->currentState();
        const auto bsMomentum = bsState.globalMomentum();
        const auto bsParameters = bsState.kinematicParameters();

        // photon variables
        tripleVertexResults.photonDeltaRs.at(i) = photonDeltaRs.at(i);
        tripleVertexResults.photonPt.at(i)      = photon.pt();
        tripleVertexResults.photonEta.at(i)     = photon.eta();
        tripleVertexResults.photonPhi.at(i)     = photon.phi();
        tripleVertexResults.photonEnergy.at(i)  = photon.energy();

        tripleVertexResults.photonDimuonMass.at(i) = photonDimuonLV.M();
        tripleVertexResults.photonDimuonPt.at(i)   = photonDimuonLV.Pt();
        tripleVertexResults.photonDimuonEta.at(i)  = photonDimuonLV.Eta();
        tripleVertexResults.photonDimuonPhi.at(i)  = photonDimuonLV.Phi();

        // Bs variables
        tripleVertexResults.bsVtxProb.at(i) = fitter.getBhadronProb();
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
    tripleVertexResults.dimuonMass = fitter.getDimuon()->currentState().mass();
    tripleVertexResults.dimuonEta  = kinDimuonMomentum.eta();
    tripleVertexResults.dimuonPhi  = kinDimuonMomentum.phi();
    tripleVertexResults.dimuonPt   = kinDimuonMomentum.perp();
    

    return tripleVertexResults;

}

// Explicit instantiations (the template is defined in this .cc file, so used combinations must be listed here)
template TripleObjectVertexFit::MmgFitResults TripleObjectVertexFit::fitMmgVertices<reco::Muon, edm::View<reco::Photon>>(
    const std::pair<reco::Muon, reco::Muon>&, const edm::View<reco::Photon>&, const std::vector<reco::Vertex>&,
    const EcalClusterLazyTools&, const BeamSpotAndVertex::BSAndVtxVariables&, const MagneticField&, const TransientTrackBuilder&);
