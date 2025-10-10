from __future__ import print_function
import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.nano_cff import *

def nanoAOD_customizeNanoMuonId(process):
    process.load('addID.run3mvaid.NanoMuonId_cff')
    # Data 
    process.nanoSequence   = cms.Sequence(process.nanoSequence + process.NanoMuonIdSequence + process.NanoMuonIdTables)
    # MC
    process.nanoSequenceMC = cms.Sequence(process.nanoSequenceMC + process.NanoMuonIdMcSequence + process.NanoMuonIdMcTables)

    process.load('PhysicsTools.NanoAOD.muons_cff')
    # process.muonTable.variables.mvaMuID = Var("mvaIDValue()",float,doc="MVA-based ID score ",precision=6)
    return process

