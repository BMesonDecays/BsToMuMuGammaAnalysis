#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/SCRecHitAccumulator.h"



SCRecHitAccumulator::SCRecHitAccumulator(){}
SCRecHitAccumulator::SCAndRecHitVariables SCRecHitAccumulator::SCAndRecHitObservables(
    const std::vector<reco::SuperCluster>& superclusters,
    const EcalRecHitCollection& recHits,
    const BeamSpotAndVertex::BSAndVtxVariables& bsAndVtxInfo,
    const CaloGeometry& caloGeom) {
        
        SCAndRecHitVariables scrv;
        ROOT::Math::XYZVector pv_pos(bsAndVtxInfo.pv_x, bsAndVtxInfo.pv_y, bsAndVtxInfo.pv_z);
        for (const auto& sc : superclusters) {
            for (auto clstr = sc.clustersBegin(); clstr != sc.clustersEnd(); ++clstr) {
                const reco::CaloCluster* cluster = clstr->get();
                
                const std::vector<std::pair<DetId, float>>& hitsAndFractions = cluster->hitsAndFractions();
                for (const auto& hitPair : hitsAndFractions) {
                    DetId detid = hitPair.first;
                    for (const auto& hit : recHits) {
                        if (hit.id() == detid) {
                            scrv.sc_eta = sc.eta();
                            scrv.sc_phi = sc.phi();
                            scrv.sc_energy = sc.energy();
                            scrv.sc_eta_width = sc.etaWidth();
                            scrv.sc_phi_width = sc.phiWidth();
                            scrv.cluster_energy = cluster->energy();
                            scrv.rechit_rawid = detid.rawId();
                            scrv.rechit_subdet = detid.subdetId();
                            scrv.rechit_energy = hit.energy();
                            scrv.rechit_time = hit.time();
    
                            const GlobalPoint& position = caloGeom.getPosition(detid);
                            scrv.rechit_x = position.x();
                            scrv.rechit_y = position.y();
                            scrv.rechit_z = position.z();
                            ROOT::Math::XYZVector hit_pos(position.x(), position.y(), position.z());
                            ROOT::Math::XYZVector delta = hit_pos - pv_pos;
                            scrv.rechit_pv_distance = delta.R();
                            scrv.rechit_tof = scrv.rechit_pv_distance / c_light_;
                            scrv.rechit_corrected_time = scrv.rechit_time - scrv.rechit_tof;
                            scrv.rechit_is_weird = hit.checkFlag(EcalRecHit::kWeird);
                            scrv.rechit_is_problematic = hit.checkFlags(
                                std::vector<int>{EcalRecHit::kOutOfTime, EcalRecHit::kPoorCalib});
                                scrv.rechit_poor_reco = hit.checkFlagMask(0x1 << EcalRecHit::kPoorReco);
                            //std::cout << "Outputs related to the ECalRecHits SuperCluster ----------------------------\n";
                            //std::cout << "  EcalRecHit: energy = " << hit.energy() << ", time = " << hit.time() << "\n";
                            if (detid.subdetId() == EcalBarrel) {
                                EBDetId ebid(detid);
                                scrv.rechit_EB_ieta = ebid.ieta();
                                scrv.rechit_EB_iphi = ebid.iphi();
                                //std::cout << "  EcalRecHit in the Barrel (ieta, iphi): (" << ebid.ieta() << ", "<< ebid.iphi() << ")\n";
                            } else if (detid.subdetId() == EcalEndcap) {
                                EEDetId eeid(detid);
                                scrv.rechit_EE_ix = eeid.ix();
                                scrv.rechit_EE_iy = eeid.iy();
                                scrv.rechit_EE_zside = eeid.zside();
                                //std::cout << "  EcalRecHit in the EndCap region (ix, iy, zside): (" << eeid.ix() << ", "<< eeid.iy() << ", " << eeid.zside() << ")\n";
                            }
                            /*std::cout << "  Raw ID     : " << detid.rawId() << "\n";
                            std::cout << "  Det        : " << detid.det() << " (should be " << DetId::Ecal
                                      << " for ECAL)\n";
                            std::cout << "  Subdet     : " << detid.subdetId() << " (should be " << EcalBarrel
                                      << " for EB)\n";
                            std::cout << "  RecHit position: x=" << position.x() << ", y=" << position.y()
                                      << ", z=" << position.z() << "\n";
                            std::cout << "  Distance PV to RecHit: " << scrv.rechit_pv_distance << " cm\n";
                            std::cout << "  Photon TOF: " << scrv.rechit_tof << " ns\n";
                            std::cout << "  Corrected time: " << scrv.rechit_corrected_time << " ns\n";*/
    
                            /*if (scrv.rechit_is_weird) {
                                std::cout << "[EcalRecHit] Weird hit detected!" << "\n";
                            }
                            if (scrv.rechit_is_problematic) {
                                std::cout << "[EcalRecHit] Problematic hit with multiple possible issues." <<"\n";
                            }
                            if (scrv.rechit_poor_reco) {
                                std::cout << "[EcalRecHit] Flag mask matched." <<"\n";
                            }*/
    
                            
                        }
                    }
                }
            }
        }
    
        return scrv;
    }

