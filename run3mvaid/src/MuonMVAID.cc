#include "BsToMuMuGammaAnalysis/run3mvaid/interface/MuonMVAID.h"


// 
// MiniRun3IdProducer is designed to add Run3 mva id to miniaod
//

MuonMVAID::MuonMVAID(const edm::ParameterSet &iConfig):
xgboost_models_( iConfig.getParameter<vector<string>>( "xgboost_models" ) ),
xgboost_variable_names_( iConfig.getParameter<vector<string>>( "xgboost_variable_names" ) )
{
    for (auto model: xgboost_models_)
      softMuonMva_.push_back(XGBooster(edm::FileInPath("BsToMuMuGammaAnalysis/run3mvaid/data/" + model + ".model").fullPath(),
				       edm::FileInPath("BsToMuMuGammaAnalysis/run3mvaid/data/" + model + ".features").fullPath()));
}

void MuonMVAID::fillSoftMva(pat::Muon& mu_cand){
  // "match2_pullDyDz"
  // "match1_pullDyDz"
  // "match2_pullDxDz"
  // "match1_pullDxDz"
  for (unsigned int i=0; i < softMuonMva_.size(); ++i) {
    softMuonMva_.at(i).set("pt",                  mu_cand.pt());
    softMuonMva_.at(i).set("eta",                 mu_cand.eta());
    softMuonMva_.at(i).set("trkValidFrac",        mu_cand.userFloat("trkValidFrac"));
    softMuonMva_.at(i).set("glbTrackProbability", mu_cand.userFloat("glbTrackProbability"));
    softMuonMva_.at(i).set("nLostHitsInner",      mu_cand.userInt(  "nLostHitsInner"));
    softMuonMva_.at(i).set("nLostHitsOuter",      mu_cand.userInt(  "nLostHitsOuter"));
    softMuonMva_.at(i).set("trkKink",             mu_cand.userFloat("trkKink"));
    softMuonMva_.at(i).set("chi2LocalPosition",   mu_cand.userFloat("chi2LocalPosition"));
    softMuonMva_.at(i).set("match2_dX",           mu_cand.userFloat("match2_dX"));
    softMuonMva_.at(i).set("match2_pullX",        mu_cand.userFloat("match2_pullX"));
    softMuonMva_.at(i).set("match1_dX",           mu_cand.userFloat("match1_dX"));
    softMuonMva_.at(i).set("match1_pullX",        mu_cand.userFloat("match1_pullX"));
    softMuonMva_.at(i).set("nPixels",             mu_cand.userInt(  "nPixels"));
    softMuonMva_.at(i).set("nValidHits",          mu_cand.userInt(  "nValidHits"));
    softMuonMva_.at(i).set("nLostHitsOn",         mu_cand.userInt(  "nLostHitsOn"));
    softMuonMva_.at(i).set("match2_dY",           mu_cand.userFloat("match2_dY"));
    softMuonMva_.at(i).set("match2_pullY",        mu_cand.userFloat("match2_pullY"));
    softMuonMva_.at(i).set("match1_dY",           mu_cand.userFloat("match1_dY"));
    softMuonMva_.at(i).set("match1_pullY",        mu_cand.userFloat("match1_pullY"));
    softMuonMva_.at(i).set("match2_pullDyDz",     mu_cand.userFloat("match2_pullDyDz"));
    softMuonMva_.at(i).set("match1_pullDyDz",     mu_cand.userFloat("match1_pullDyDz"));
    softMuonMva_.at(i).set("match2_pullDxDz",     mu_cand.userFloat("match2_pullDxDz"));
    softMuonMva_.at(i).set("match1_pullDxDz",     mu_cand.userFloat("match1_pullDxDz"));
    softMuonMva_.at(i).set("glbNormChi2",         mu_cand.userFloat("glbNormChi2"));
    softMuonMva_.at(i).set("trkLayers",           mu_cand.userInt("trkLayers"));
    softMuonMva_.at(i).set("highPurity",          mu_cand.userInt("highPurity"));
    
    mu_cand.addUserFloat(xgboost_variable_names_.at(i), softMuonMva_.at(i).predict());
  }
}



vector<float> MuonMVAID::produce(const std::vector<pat::Muon>& InputMuons) {

    // Output collection
    auto muons = std::make_unique<std::vector<pat::Muon>>();

    for ( const auto& muon: InputMuons){

      pat::Muon mu_cand = muon;

      mu_cand.addUserFloat("trkKink",             muon.combinedQuality().trkKink);
      mu_cand.addUserFloat("glbTrackProbability", muon.combinedQuality().glbTrackProbability);
      mu_cand.addUserFloat("chi2LocalPosition",   muon.combinedQuality().chi2LocalPosition);
      mu_cand.addUserFloat("chi2LocalMomentum",   muon.combinedQuality().chi2LocalMomentum);
      mu_cand.addUserFloat("trkRelChi2",          muon.combinedQuality().trkRelChi2);
      mu_cand.addUserFloat("staRelChi2",          muon.combinedQuality().staRelChi2);
      mu_cand.addUserFloat("mvaId",               muon.mvaIDValue());
      
      if (muon.isGlobalMuon()){
	mu_cand.addUserFloat("glbNormChi2", muon.globalTrack()->normalizedChi2());
	mu_cand.addUserFloat("staNormChi2", muon.outerTrack()->normalizedChi2());
	mu_cand.addUserInt( "staValidHits", muon.outerTrack()->hitPattern().muonStationsWithValidHits());
	mu_cand.addUserInt("chargeProduct", muon.outerTrack()->charge() * muon.innerTrack()->charge());
      } else {
	mu_cand.addUserFloat("glbNormChi2", 9999.);
	mu_cand.addUserFloat("staNormChi2", 9999.);
	mu_cand.addUserInt( "staValidHits", 0);
	mu_cand.addUserInt("chargeProduct", 0);
      }
      
      if (muon.isTrackerMuon() or muon.isGlobalMuon()){
	mu_cand.addUserFloat("trkValidFrac",  muon.innerTrack()->validFraction());
	mu_cand.addUserFloat("trkNormChi2",   muon.innerTrack()->normalizedChi2());
	
	mu_cand.addUserInt("pixelPattern",    bmm::get_pixel_pattern(muon.innerTrack()->hitPattern()));
	mu_cand.addUserInt("nPixels",         muon.innerTrack()->hitPattern().numberOfValidPixelHits());
	mu_cand.addUserInt("nValidHits",      muon.innerTrack()->hitPattern().numberOfValidTrackerHits());
	mu_cand.addUserInt("nLostHitsInner",  muon.innerTrack()->hitPattern().numberOfLostTrackerHits(reco::HitPattern::MISSING_INNER_HITS));
	mu_cand.addUserInt("nLostHitsOn",     muon.innerTrack()->hitPattern().numberOfLostTrackerHits(reco::HitPattern::TRACK_HITS));
	mu_cand.addUserInt("nLostHitsOuter",  muon.innerTrack()->hitPattern().numberOfLostTrackerHits(reco::HitPattern::MISSING_OUTER_HITS));
	
	mu_cand.addUserInt("trkLayers",           muon.innerTrack()->hitPattern().trackerLayersWithMeasurement());
	mu_cand.addUserInt("trkLostLayersInner",  muon.innerTrack()->hitPattern().trackerLayersWithoutMeasurement(reco::HitPattern::MISSING_INNER_HITS));
	mu_cand.addUserInt("trkLostLayersOn",     muon.innerTrack()->hitPattern().trackerLayersWithoutMeasurement(reco::HitPattern::TRACK_HITS));
	mu_cand.addUserInt("trkLostLayersOuter",  muon.innerTrack()->hitPattern().trackerLayersWithoutMeasurement(reco::HitPattern::MISSING_OUTER_HITS));

	mu_cand.addUserInt("highPurity",   muon.innerTrack()->quality(reco::Track::highPurity));

      } else {
	mu_cand.addUserFloat("trkValidFrac",  0);
	mu_cand.addUserFloat("trkNormChi2",   9999.);
	
	mu_cand.addUserInt("pixelPattern",    0);
	mu_cand.addUserInt("nPixels",         0);
	mu_cand.addUserInt("nValidHits",      0);
	mu_cand.addUserInt("nLostHitsInner",  0);
	mu_cand.addUserInt("nLostHitsOn",     0);
	mu_cand.addUserInt("nLostHitsOuter",  0);
	
	mu_cand.addUserInt("trkLayers",           0);
	mu_cand.addUserInt("trkLostLayersInner",  0);
	mu_cand.addUserInt("trkLostLayersOn",     0);
	mu_cand.addUserInt("trkLostLayersOuter",  0);

	mu_cand.addUserInt("highPurity",   0);

      }
	
      fillMatchInfo(mu_cand, muon);
      fillSoftMva(mu_cand);

      muons->push_back(mu_cand);
    }
    
    vector<float> output;
    for(const auto& mu : *muons)
      output.push_back(mu.userFloat("run3muonmva"));

    return output;
}

vector<float> MuonMVAID::produce(const std::vector<reco::Muon>& InputMuons) {

    // Output collection
    auto muons = std::make_unique<std::vector<pat::Muon>>();

    for ( const auto& muon: InputMuons){

      pat::Muon mu_cand = pat::Muon(muon);

      mu_cand.addUserFloat("trkKink",             muon.combinedQuality().trkKink);
      mu_cand.addUserFloat("glbTrackProbability", muon.combinedQuality().glbTrackProbability);
      mu_cand.addUserFloat("chi2LocalPosition",   muon.combinedQuality().chi2LocalPosition);
      mu_cand.addUserFloat("chi2LocalMomentum",   muon.combinedQuality().chi2LocalMomentum);
      mu_cand.addUserFloat("trkRelChi2",          muon.combinedQuality().trkRelChi2);
      mu_cand.addUserFloat("staRelChi2",          muon.combinedQuality().staRelChi2);
      
      if (muon.isGlobalMuon()){
	mu_cand.addUserFloat("glbNormChi2", muon.globalTrack()->normalizedChi2());
	mu_cand.addUserFloat("staNormChi2", muon.outerTrack()->normalizedChi2());
	mu_cand.addUserInt( "staValidHits", muon.outerTrack()->hitPattern().muonStationsWithValidHits());
	mu_cand.addUserInt("chargeProduct", muon.outerTrack()->charge() * muon.innerTrack()->charge());
      } else {
	mu_cand.addUserFloat("glbNormChi2", 9999.);
	mu_cand.addUserFloat("staNormChi2", 9999.);
	mu_cand.addUserInt( "staValidHits", 0);
	mu_cand.addUserInt("chargeProduct", 0);
      }
      
      if (muon.isTrackerMuon() or muon.isGlobalMuon()){
	mu_cand.addUserFloat("trkValidFrac",  muon.innerTrack()->validFraction());
	mu_cand.addUserFloat("trkNormChi2",   muon.innerTrack()->normalizedChi2());
	
	mu_cand.addUserInt("pixelPattern",    bmm::get_pixel_pattern(muon.innerTrack()->hitPattern()));
	mu_cand.addUserInt("nPixels",         muon.innerTrack()->hitPattern().numberOfValidPixelHits());
	mu_cand.addUserInt("nValidHits",      muon.innerTrack()->hitPattern().numberOfValidTrackerHits());
	mu_cand.addUserInt("nLostHitsInner",  muon.innerTrack()->hitPattern().numberOfLostTrackerHits(reco::HitPattern::MISSING_INNER_HITS));
	mu_cand.addUserInt("nLostHitsOn",     muon.innerTrack()->hitPattern().numberOfLostTrackerHits(reco::HitPattern::TRACK_HITS));
	mu_cand.addUserInt("nLostHitsOuter",  muon.innerTrack()->hitPattern().numberOfLostTrackerHits(reco::HitPattern::MISSING_OUTER_HITS));
	
	mu_cand.addUserInt("trkLayers",           muon.innerTrack()->hitPattern().trackerLayersWithMeasurement());
	mu_cand.addUserInt("trkLostLayersInner",  muon.innerTrack()->hitPattern().trackerLayersWithoutMeasurement(reco::HitPattern::MISSING_INNER_HITS));
	mu_cand.addUserInt("trkLostLayersOn",     muon.innerTrack()->hitPattern().trackerLayersWithoutMeasurement(reco::HitPattern::TRACK_HITS));
	mu_cand.addUserInt("trkLostLayersOuter",  muon.innerTrack()->hitPattern().trackerLayersWithoutMeasurement(reco::HitPattern::MISSING_OUTER_HITS));

	mu_cand.addUserInt("highPurity",   muon.innerTrack()->quality(reco::Track::highPurity));

      } else {
	mu_cand.addUserFloat("trkValidFrac",  0);
	mu_cand.addUserFloat("trkNormChi2",   9999.);
	
	mu_cand.addUserInt("pixelPattern",    0);
	mu_cand.addUserInt("nPixels",         0);
	mu_cand.addUserInt("nValidHits",      0);
	mu_cand.addUserInt("nLostHitsInner",  0);
	mu_cand.addUserInt("nLostHitsOn",     0);
	mu_cand.addUserInt("nLostHitsOuter",  0);
	
	mu_cand.addUserInt("trkLayers",           0);
	mu_cand.addUserInt("trkLostLayersInner",  0);
	mu_cand.addUserInt("trkLostLayersOn",     0);
	mu_cand.addUserInt("trkLostLayersOuter",  0);

	mu_cand.addUserInt("highPurity",   0);

      }
	
      fillMatchInfo(mu_cand, muon);
      fillSoftMva(mu_cand);

      muons->push_back(mu_cand);
    }
    
    vector<float> output;
    for(const auto& mu : *muons)
      output.push_back(mu.userFloat("run3muonmva"));

    return output;
}


const MatchPair&
fgetBetterMatch(const MatchPair& match1, const MatchPair& match2){

  if (match2.first->detector() == MuonSubdetId::DT and
      match1.first->detector() != MuonSubdetId::DT)
    return match2;

  if ( abs(match1.first->x - match1.second->x) >
       abs(match2.first->x - match2.second->x) )
    return match2;
    
  return match1;
}

float fdX(const MatchPair& match){
  if (match.first and match.second->hasPhi())
    return (match.first->x - match.second->x);
  else
    return 9999.;
}

float fpullX(const MatchPair& match){
  if (match.first and match.second->hasPhi())
    return fdX(match) /
      sqrt(pow(match.first->xErr, 2) + pow(match.second->xErr, 2));
  else
    return 9999.;
}

float fpullDxDz(const MatchPair& match){
  if (match.first and match.second->hasPhi())
    return (match.first->dXdZ - match.second->dXdZ) /
           sqrt(pow(match.first->dXdZErr, 2) + pow(match.second->dXdZErr, 2));
  else
    return 9999.;
}

float fdY(const MatchPair& match){
  if (match.first and match.second->hasZed())
    return (match.first->y - match.second->y);
  else
    return 9999.;
}

float fpullY(const MatchPair& match){
  if (match.first and match.second->hasZed())
    return fdY(match) /
      sqrt(pow(match.first->yErr, 2) + pow(match.second->yErr, 2));
  else
    return 9999.;
}

float fpullDyDz(const MatchPair& match){
  if (match.first and match.second->hasZed())
    return (match.first->dYdZ - match.second->dYdZ) /
           sqrt(pow(match.first->dYdZErr, 2) + pow(match.second->dYdZErr, 2));
  else
    return 9999.;
}

void fillMatchInfoForStation(string prefix,
			     pat::Muon& cand,
			     const MatchPair& match){
  cand.addUserFloat(prefix + "_dX",       fdX(match));
  cand.addUserFloat(prefix + "_pullX",    fpullX(match));
  cand.addUserFloat(prefix + "_pullDxDz", fpullDxDz(match));
  cand.addUserFloat(prefix + "_dY",       fdY(match));
  cand.addUserFloat(prefix + "_pullY",    fpullY(match));
  cand.addUserFloat(prefix + "_pullDyDz", fpullDyDz(match));
}

void MuonMVAID::fillMatchInfo(pat::Muon& cand,
				      const pat::Muon& muon){
  // Initiate containter for results
  const int n_stations = 2;
  vector<MatchPair> matches;
  for (unsigned int i=0; i < n_stations; ++i)
    matches.push_back(pair(nullptr, nullptr));

  // Find best matches
  for (auto& chamberMatch : muon.matches()){
    unsigned int station = chamberMatch.station() - 1;
    if (station >= n_stations) continue;

    // Find best segment match.
    // We could consider all segments, but we will restrict to segments
    // that match to this candidate better than to other muon candidates
    for (auto& segmentMatch : chamberMatch.segmentMatches){
      if ( not segmentMatch.isMask(reco::MuonSegmentMatch::BestInStationByDR) ||
	   not segmentMatch.isMask(reco::MuonSegmentMatch::BelongsToTrackByDR) )
	continue;

      // Multiple segment matches are possible in different
      // chambers that are either overlapping or belong to
      // different detectors. We need to select one.
      auto match_pair = MatchPair(&chamberMatch, &segmentMatch);
      
      if (matches[station].first)
	matches[station] = fgetBetterMatch(matches[station], match_pair);
      else
	matches[station] = match_pair;
    }
  }

  // Fill matching information
  fillMatchInfoForStation("match1", cand, matches[0]);
  fillMatchInfoForStation("match2", cand, matches[1]);
}


//  LocalWords:  vertices
