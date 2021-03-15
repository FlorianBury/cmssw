# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: step2 --conditions auto:phase2_realistic -s DIGI:pdigi_valid,L1,L1TrackTrigger,DIGI2RAW,HLT:@fake2 --datatier GEN-SIM-DIGI-RAW -n 10 --geometry Extended2023D21 --era Phase2 --eventcontent FEVTDEBUGHLT --no_exec --python step2_DIGI_CMSSW104_D21_cfg.tmpl
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

process = cms.Process('MIXTEST',eras.Phase2)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mix_POISSON_average_cfi')
#process.load('SimGeneral.MixingModule.cfwriter_cfi')
process.load('Configuration.Geometry.GeometryExtended2023D21Reco_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.SimL1Emulator_cff')
process.load('Configuration.StandardSequences.DigiToRaw_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic', '')


# Input source
input_filename = 'file:/eos/cms/store/group/dpg_tracker_upgrade/Digitizer/Production/CMSSW_106X/GEN_SIM/AllEta/SingleMuPt_100_GEN_SIM_1.root'
print input_filename
process.source = cms.Source("PoolSource",
    dropDescendantsOfDroppedBranches = cms.untracked.bool(False),
    fileNames = cms.untracked.vstring(input_filename),
    inputCommands = cms.untracked.vstring(
        'keep *', 
        'drop *_genParticles_*_*', 
        'drop *_genParticlesForJets_*_*', 
        'drop *_kt4GenJets_*_*', 
        'drop *_kt6GenJets_*_*', 
        'drop *_iterativeCone5GenJets_*_*', 
        'drop *_ak4GenJets_*_*', 
        'drop *_ak7GenJets_*_*', 
        'drop *_ak8GenJets_*_*', 
        'drop *_ak4GenJetsNoNu_*_*', 
        'drop *_ak8GenJetsNoNu_*_*', 
        'drop *_genCandidatesForMET_*_*', 
        'drop *_genParticlesForMETAllVisible_*_*', 
        'drop *_genMetCalo_*_*', 
        'drop *_genMetCaloAndNonPrompt_*_*', 
        'drop *_genMetTrue_*_*', 
        'drop *_genMetIC5GenJs_*_*'
    ),
    secondaryFileNames = cms.untracked.vstring()
)

process.options = cms.untracked.PSet(

)
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.SimpleMemoryCheck = cms.Service("SimpleMemoryCheck",
    oncePerEventMode = cms.untracked.bool(True),
    ignoreTotal = cms.untracked.int32(1)
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('step2 nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Customization of Mixing Module
process.mix.input.nbPileupEvents.averageNumber = cms.double(200.0000000)
process.mix.bunchspace = cms.int32(25)
process.mix.minBunch = cms.int32(-12)
process.mix.maxBunch = cms.int32(3)
process.mix.input.fileNames = cms.untracked.vstring([
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/06D2C06E-A59F-DC49-9836-56B3409B1B79.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/70F5AC85-46E8-8745-A5CF-2B8C47743204.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/974B586D-6800-5B4B-9AEA-78BC8518257F.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/553AFA27-C8EB-F044-9591-57F29D90527A.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/3BFB9814-7F8A-4744-A773-773AC28113C4.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/1E161C34-8F0F-0946-983C-443E75A27265.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/9F12231D-132F-904F-BCC5-D7B70FD4D0D7.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/FB25285C-3225-8046-BFF2-D9A10EB9806C.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/50C9BEE3-A974-7647-878E-7FBD1B4E6F97.root',
  '/store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/F7EAFA17-64E9-D343-A63F-25484924F49B.root'])
process.mix.mixObjects.mixSH.crossingFrames.extend(['TrackerHitsTECHighTof', 
        'TrackerHitsPixelBarrelHighTof', 
        'TrackerHitsPixelBarrelLowTof', 
       'TrackerHitsPixelEndcapHighTof', 
        'TrackerHitsPixelEndcapLowTof', 
        'TrackerHitsTECHighTof', 
        'TrackerHitsTECLowTof', 
        'TrackerHitsTIBHighTof', 
        'TrackerHitsTIBLowTof', 
        'TrackerHitsTIDHighTof', 
        'TrackerHitsTIDLowTof', 
        'TrackerHitsTOBHighTof',
        'TrackerHitsTOBLowhTof'])
process.mix.digitizers.mergedtruth.select.signalOnlyTP = cms.bool(False)

process.dump = cms.EDAnalyzer("EventContentAnalyzer")

# Output definition
filename = '/eos/cms/store/group/dpg_tracker_upgrade/Digitizer/Processing/XFrame_CMS106X/MixingTest_PU200_MB_AllEta_step1_1.root'
print filename
process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    splitLevel = cms.untracked.int32(0),
    outputCommands = process.DQMEventContent.outputCommands,
    fileName = cms.untracked.string(filename),
    dataset = cms.untracked.PSet(
        filterName = cms.untracked.string(''),
        dataTier = cms.untracked.string('DQMIO')
    )
)
# root file with ntuple                                                                                                                                                                                         
ntuple_filename = '/eos/cms/store/group/dpg_tracker_upgrade/Digitizer/Processing/XFrame_CMS106X/MixingTest_PU200_MB_AllEta_ntuple_1.root'
process.TFileService = cms.Service("TFileService", fileName = cms.string(ntuple_filename), closeFileFast = cms.untracked.bool(True))


process.load('SimTracker.SiPhase2Digitizer.Phase2TrackerValidateSimHit_cff')
process.digiana_seq = cms.Sequence(process.pixSimValid * process.otSimValid)

process.load('DQMServices.Components.DQMEventInfo_cfi')
process.dqmEnv.subSystemFolder = cms.untracked.string('Ph2TkDigi')

process.dqm_comm = cms.Sequence(process.dqmEnv)

# Path and EndPath definitions
process.endjob_step = cms.EndPath(process.endOfProcess)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)

process.dqm_step =  cms.Path(process.digiana_seq * process.dqm_comm)

process.mix_step = cms.Path(process.mix)
# Schedule definition
process.schedule = cms.Schedule(
    process.mix_step,
    process.dqm_step,
    process.DQMoutput_step
)

