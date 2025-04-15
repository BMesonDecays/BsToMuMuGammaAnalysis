#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/SCRecHitAccumulator.h"



SCRecHitAccumulator::SCRecHitAccumulator(){}
SCRecHitAccumulator::SCAndRecHitVariables SCRecHitAccumulator::SCAndRecHitObservables(
    const std::vector<reco::SuperCluster>& superclusters,
    const EcalRecHitCollection& recHits,
    const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
    const CaloGeometry& caloGeom) {


        SCAndRecHitVariables scrv;

        // Use vertex position from bsAndVtxInfo
        ROOT::Math::XYZVector pv_pos(bsAndVtxInfo.pv_x, bsAndVtxInfo.pv_y, bsAndVtxInfo.pv_z);
    
        std::cout << "SuperCluster multiplicity: " << superclusters.size() << "\n";
    
        // Process SuperClusters
        for (const auto& sc : superclusters) {
            std::cout << "SuperCluster eta and phi width: ----------------------------------------------- "
                      << sc.etaWidth() << "\t" << sc.phiWidth() << "\n";
    
            // Process clusters
            for (auto clstr = sc.clustersBegin(); clstr != sc.clustersEnd(); ++clstr) {
                const reco::CaloCluster* cluster = clstr->get();
                std::cout << "  Cluster energy: " << cluster->energy() << "\n";
    
                // Get RecHits for this cluster
                const std::vector<std::pair<DetId, float>>& hitsAndFractions = cluster->hitsAndFractions();
                for (const auto& hitPair : hitsAndFractions) {
                    DetId detid = hitPair.first;
    
                    // Find matching RecHit
                    for (const auto& hit : recHits) {
                        if (hit.id() == detid) {
    
                            // SuperCluster variables
                            scrv.sc_eta = sc.eta();
                            scrv.sc_phi = sc.phi();
                            scrv.sc_energy = sc.energy();
                            scrv.sc_eta_width = sc.etaWidth();
                            scrv.sc_phi_width = sc.phiWidth();
    
                            // Cluster variable
                            scrv.cluster_energy = cluster->energy();
    
                            // RecHit variables
                            scrv.rechit_rawid = detid.rawId();
                            scrv.rechit_subdet = detid.subdetId();
                            scrv.rechit_energy = hit.energy();
                            scrv.rechit_time = hit.time();
    
                            const GlobalPoint& position = caloGeom.getPosition(detid);
                            scrv.rechit_x = position.x();
                            scrv.rechit_y = position.y();
                            scrv.rechit_z = position.z();
    
                            // Calculate distance and TOF
                            ROOT::Math::XYZVector hit_pos(position.x(), position.y(), position.z());
                            ROOT::Math::XYZVector delta = hit_pos - pv_pos;
                            scrv.rechit_pv_distance = delta.R();
                            scrv.rechit_tof = scrv.rechit_pv_distance / c_light_;
    
                            // Flags
                            scrv.rechit_is_weird = hit.checkFlag(EcalRecHit::kWeird);
                            scrv.rechit_is_problematic = hit.checkFlags(
                                std::vector<int>{EcalRecHit::kOutOfTime, EcalRecHit::kPoorCalib});
                                scrv.rechit_poor_reco = hit.checkFlagMask(0x1 << EcalRecHit::kPoorReco);
    
                            // Logging
                            std::cout << "Outputs related to the ECalRecHits SuperCluster ----------------------------\n";
                            std::cout << "  EcalRecHit: energy = " << hit.energy() << ", time = " << hit.time() << "\n";
                            if (detid.subdetId() == EcalBarrel) {
                                EBDetId ebid(detid);
                                std::cout << "  EcalRecHit at (ieta, iphi): (" << ebid.ieta() << ", "
                                          << ebid.iphi() << ")\n";
                            } else if (detid.subdetId() == EcalEndcap) {
                                EEDetId eeid(detid);
                                std::cout << "  EcalRecHit at (ix, iy, zside): (" << eeid.ix() << ", "
                                          << eeid.iy() << ", " << eeid.zside() << ")\n";
                            }
                            std::cout << "  Raw ID     : " << detid.rawId() << "\n";
                            std::cout << "  Det        : " << detid.det() << " (should be " << DetId::Ecal
                                      << " for ECAL)\n";
                            std::cout << "  Subdet     : " << detid.subdetId() << " (should be " << EcalBarrel
                                      << " for EB)\n";
                            std::cout << "  RecHit position: x=" << position.x() << ", y=" << position.y()
                                      << ", z=" << position.z() << "\n";
                            std::cout << "  Distance PV to RecHit: " << scrv.rechit_pv_distance << " cm\n";
                            std::cout << "  Photon TOF: " << scrv.rechit_tof << " ns\n";
    
                            if (vars.rechit_is_weird) {
                                edm::LogInfo("EcalRecHit") << "Weird hit detected!";
                            }
                            if (vars.rechit_is_problematic) {
                                edm::LogInfo("EcalRecHit") << "Problematic hit with multiple possible issues.";
                            }
                            if (vars.rechit_poor_reco) {
                                edm::LogInfo("EcalRecHit") << "Flag mask matched.";
                            }
    
                            
                        }
                    }
                }
            }
        }
    
        return scrv;
    }

