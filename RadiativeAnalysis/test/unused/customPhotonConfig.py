import FWCore.ParameterSet.Config as cms

# Configuration for BMMGCustomPhotonProducer
BMMGCustomPhotonProducer = cms.EDProducer("BMMGCustomPhotonProducer",
    # Input collections
    ebSuperClusters = cms.InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALBarrel"),
    eeSuperClusters = cms.InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALEndcapWithPreshower"),
    ebRecHits = cms.InputTag("reducedEcalRecHitsEB"),
    eeRecHits = cms.InputTag("reducedEcalRecHitsEE"),
    hbheRecHits = cms.InputTag("reducedHcalRecHits", "hbhereco"),
    caloTowers = cms.InputTag("caloTowers"),
    
    # Selection parameters
    minPt = cms.double(0.0),           # Minimum pT cut in GeV (0 for low pT photons)
    maxEta = cms.double(2.5),          # Maximum |eta| cut
    minEnergy = cms.double(0.0),       # Minimum energy cut in GeV
    useAllClusters = cms.bool(True),   # Use all clusters or apply additional filtering
    
    # Feature calculation flags
    calculateIsolation = cms.bool(True),      # Calculate isolation variables
    calculateShowerShapes = cms.bool(True),   # Calculate shower shape variables
    
    # Isolation parameters
    isolationConeSize = cms.double(0.3),      # Outer cone size for isolation calculation
    isolationConeInnerSize = cms.double(0.15), # Inner cone size for isolation calculation
    hOverEConeSize = cms.double(0.15),        # Cone size for H/E calculation
    
    # Output collection name
    outputCollectionName = cms.string("")     # Empty string uses default collection name
)

# Alternative configuration for tighter cuts
BMMGCustomPhotonProducerTight = cms.EDProducer("BMMGCustomPhotonProducer",
    # Input collections
    ebSuperClusters = cms.InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALBarrel"),
    eeSuperClusters = cms.InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALEndcapWithPreshower"),
    ebRecHits = cms.InputTag("reducedEcalRecHitsEB"),
    eeRecHits = cms.InputTag("reducedEcalRecHitsEE"),
    hbheRecHits = cms.InputTag("reducedHcalRecHits", "hbhereco"),
    caloTowers = cms.InputTag("caloTowers"),
    
    # Tighter selection parameters
    minPt = cms.double(1.0),           # Minimum pT cut of 1 GeV
    maxEta = cms.double(2.4),          # Slightly tighter eta cut
    minEnergy = cms.double(0.5),       # Minimum energy cut of 0.5 GeV
    useAllClusters = cms.bool(False),  # Apply additional filtering
    
    # Feature calculation flags
    calculateIsolation = cms.bool(True),
    calculateShowerShapes = cms.bool(True),
    
    # Isolation parameters
    isolationConeSize = cms.double(0.3),
    isolationConeInnerSize = cms.double(0.15),
    hOverEConeSize = cms.double(0.15),
    
    # Output collection name
    outputCollectionName = cms.string("tightPhotons")
)

# Configuration for AOD input (different input tags)
BMMGCustomPhotonProducerAOD = cms.EDProducer("BMMGCustomPhotonProducer",
    # Input collections for AOD
    ebSuperClusters = cms.InputTag("correctedHybridSuperClusters"),
    eeSuperClusters = cms.InputTag("correctedMulti5x5SuperClustersWithPreshower"),
    ebRecHits = cms.InputTag("reducedEcalRecHitsEB"),
    eeRecHits = cms.InputTag("reducedEcalRecHitsEE"),
    hbheRecHits = cms.InputTag("reducedHcalRecHits", "hbhereco"),
    caloTowers = cms.InputTag("towerMaker"),
    
    # Selection parameters
    minPt = cms.double(0.0),
    maxEta = cms.double(2.5),
    minEnergy = cms.double(0.0),
    useAllClusters = cms.bool(True),
    
    # Feature calculation flags
    calculateIsolation = cms.bool(True),
    calculateShowerShapes = cms.bool(True),
    
    # Isolation parameters
    isolationConeSize = cms.double(0.3),
    isolationConeInnerSize = cms.double(0.15),
    hOverEConeSize = cms.double(0.15),
    
    # Output collection name
    outputCollectionName = cms.string("customPhotonsAOD")
)

# Configuration for high pT photons only
BMMGCustomPhotonProducerHighPt = cms.EDProducer("BMMGCustomPhotonProducer",
    # Input collections
    ebSuperClusters = cms.InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALBarrel"),
    eeSuperClusters = cms.InputTag("particleFlowSuperClusterECAL", "particleFlowSuperClusterECALEndcapWithPreshower"),
    ebRecHits = cms.InputTag("reducedEcalRecHitsEB"),
    eeRecHits = cms.InputTag("reducedEcalRecHitsEE"),
    hbheRecHits = cms.InputTag("reducedHcalRecHits", "hbhereco"),
    caloTowers = cms.InputTag("caloTowers"),
    
    # High pT selection
    minPt = cms.double(3.5),           # 5 GeV minimum pT
    maxEta = cms.double(2.1),          # Tracker acceptance
    minEnergy = cms.double(5.0),       # 5 GeV minimum energy
    useAllClusters = cms.bool(False),
    
    # Feature calculation flags
    calculateIsolation = cms.bool(True),
    calculateShowerShapes = cms.bool(True),
    
    # Isolation parameters
    isolationConeSize = cms.double(0.4),      # Larger cone for high pT
    isolationConeInnerSize = cms.double(0.2),
    hOverEConeSize = cms.double(0.2),
    
    # Output collection name
    outputCollectionName = cms.string("highPtPhotons")
)

# Example sequence for using the producer
BMMGCustomPhotonSequence = cms.Sequence(
    BMMGCustomPhotonProducer
)

# Example sequence with multiple configurations
BMMGCustomPhotonMultiSequence = cms.Sequence(
    BMMGCustomPhotonProducer *
    BMMGCustomPhotonProducerTight *
    BMMGCustomPhotonProducerHighPt
)
