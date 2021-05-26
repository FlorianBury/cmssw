from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing ('analysis')

options.register('N',
                 100,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.int,
                 "Number of events to be processed")
options.register('threshold',
                 5800,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.float,
                 "Value of the threshold")
options.register('thresholdsmearing',
                 0.,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.float,
                 "Value of the threshold smearing")
options.register('tofsmearing',
                 0.,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.float,
                 "Value of the tof smearing")
options.register('mode',
                 'scan',
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.string,
                 "Mode : scan (scan delay values) or emulate (use a random value of delay)") 
options.register('offset',
                 -1.,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.float,
                 "Specific offset value only used in emulate mode, if left to default (-1) will take random value")


options.parseArguments()

import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
process = cms.Process('MIX',eras.Phase2)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mix_POISSON_average_cfi')
process.load('SimGeneral.MixingModule.cfwriter_cfi')
process.load("SimGeneral.MixingModule.trackingTruthProducerSelection_cfi")
process.load('Configuration.Geometry.GeometryExtended2026D49Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2026D49_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Generator_cff')
#process.load('IOMC.EventVertexGenerators.VtxSmearedHLLHC_cfi')
process.load('Configuration.StandardSequences.VtxSmearedNoSmear_cff')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.SimL1Emulator_cff')
process.load('Configuration.StandardSequences.DigiToRaw_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic', '')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.N),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)


# Input source
input_filename = 'file:root://xrootd-cms.infn.it:///store/group/dpg_tracker_upgrade/Digitizer/Production/CMSSW_106X/GEN_SIM/AllEta/SingleMuPt_100_GEN_SIM_1.root'
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
    FailPath = cms.untracked.vstring(),
    IgnoreCompletely = cms.untracked.vstring(),
    Rethrow = cms.untracked.vstring(),
    SkipEvent = cms.untracked.vstring(),
    allowUnscheduled = cms.obsolete.untracked.bool,
    canDeleteEarly = cms.untracked.vstring(),
    emptyRunLumiMode = cms.obsolete.untracked.string,
    eventSetup = cms.untracked.PSet(
        forceNumberOfConcurrentIOVs = cms.untracked.PSet(

        ),
        numberOfConcurrentIOVs = cms.untracked.uint32(1)
    ),
    fileMode = cms.untracked.string('FULLMERGE'),
    forceEventSetupCacheClearOnNewRun = cms.untracked.bool(False),
    makeTriggerResults = cms.obsolete.untracked.bool,
    numberOfConcurrentLuminosityBlocks = cms.untracked.uint32(1),
    numberOfConcurrentRuns = cms.untracked.uint32(1),
    numberOfStreams = cms.untracked.uint32(0),
    numberOfThreads = cms.untracked.uint32(1),
    printDependencies = cms.untracked.bool(False),
    sizeOfStackForThreadsInKB = cms.optional.untracked.uint32,
    throwIfIllegalParameter = cms.untracked.bool(True),
    wantSummary = cms.untracked.bool(False)
)

process.SimpleMemoryCheck = cms.Service("SimpleMemoryCheck",
    oncePerEventMode = cms.untracked.bool(True),
    ignoreTotal = cms.untracked.int32(1)
)


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
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/06D2C06E-A59F-DC49-9836-56B3409B1B79.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/70F5AC85-46E8-8745-A5CF-2B8C47743204.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/974B586D-6800-5B4B-9AEA-78BC8518257F.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/553AFA27-C8EB-F044-9591-57F29D90527A.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/3BFB9814-7F8A-4744-A773-773AC28113C4.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/1E161C34-8F0F-0946-983C-443E75A27265.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/9F12231D-132F-904F-BCC5-D7B70FD4D0D7.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/FB25285C-3225-8046-BFF2-D9A10EB9806C.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/50C9BEE3-A974-7647-878E-7FBD1B4E6F97.root',
  'root://xrootd-cms.infn.it:///store/relval/CMSSW_10_4_0_pre2/RelValMinBias_14TeV/GEN-SIM/103X_upgrade2023_realistic_v2_2023D21noPU-v1/20000/F7EAFA17-64E9-D343-A63F-25484924F49B.root',
                ])
process.mix.mixObjects.mixSH.crossingFrames.extend([
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
        'TrackerHitsTOBLowTof',
        'FastTimerHitsBarrel',
        'FastTimerHitsEndcap',])
process.mix.digitizers.mergedtruth.select.signalOnlyTP = cms.bool(False)
process.trackingParticles.simHitCollections = cms.PSet()
process.mix.digitizers.mergedtruth = cms.PSet(process.trackingParticles)
process.mix.playback = cms.untracked.bool(True)

process.dump = cms.EDAnalyzer("EventContentAnalyzer")


# Output definition
import random
if options.mode == 'scan':
    filename = 'BXHistScan_N_{:d}_threshold{:d}_thresholdsmearing_{:0.1f}_tofsmearing_{:0.1f}_raw'.format(
                    options.N,
                    int(options.threshold),
                    options.thresholdsmearing,
                    options.tofsmearing)
elif options.mode == 'emulate':
    if options.offset == -1.:
        offset_emulate = round(random.random()*50,2)
    else:
        offset_emulate = round(options.offset,2)
        
    filename = 'BXHistEmulateDelay_{:0.2f}_N_{:d}_threshold{:d}_thresholdsmearing_{:0.1f}_tofsmearing_{:0.1f}_raw'.format(
                    offset_emulate,
                    options.N,
                    int(options.threshold),
                    options.thresholdsmearing,
                    options.tofsmearing)
else:
    raise RuntimeError("mode {} argument not understood".format(options.mode))

filename = filename.replace('.','p')+'.root'
print ("Producing file %s"%filename)

process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    splitLevel = cms.untracked.int32(0),
    outputCommands = process.DQMEventContent.outputCommands,
    fileName = cms.untracked.string(filename),
    dataset = cms.untracked.PSet(
        filterName = cms.untracked.string(''),
        dataTier = cms.untracked.string('DQMIO')
    )
)

#process.FEVTDEBUGoutput = cms.OutputModule("PoolOutputModule",
#    SelectEvents = cms.untracked.PSet(
#        SelectEvents = cms.vstring('*')
#    ),
#    dataset = cms.untracked.PSet(
#        dataTier = cms.untracked.string('GEN-SIM'),
#        filterName = cms.untracked.string('')
#    ),
#    fileName = cms.untracked.string(filename),
#    outputCommands = process.FEVTDEBUGEventContent.outputCommands,
#    splitLevel = cms.untracked.int32(0)
#)
#
process.load('Phase2TrackerBXHistogram_cfi')
process.digiana_seq = cms.Sequence(process.timeCalib)

### Modify Hit parameters 
process.timeCalib.ThresholdInElectrons_Barrel = cms.double(options.threshold)
process.timeCalib.ThresholdInElectrons_Endcap = cms.double(options.threshold)
process.timeCalib.ThresholdSmearing_Barrel = cms.double(options.thresholdsmearing)
process.timeCalib.ThresholdSmearing_Endcap = cms.double(options.thresholdsmearing)
process.timeCalib.TOFSmearing = cms.double(options.tofsmearing)
process.timeCalib.Mode = cms.string(options.mode)
if options.mode == 'emulate':
    process.timeCalib.OffsetEmulate = cms.double(offset_emulate)

process.load('IOMC.RandomEngine.IOMC_cff')
process.RandomNumberGeneratorService.generator.initialSeed  = random.randrange(1,10e07)
process.RandomNumberGeneratorService.VtxSmeared.initialSeed = random.randrange(1,10e07)
process.RandomNumberGeneratorService.g4SimHits.initialSeed  = random.randrange(1,10e07)
setattr(process.RandomNumberGeneratorService,'timeCalib',cms.PSet(
                    initialSeed = cms.untracked.uint32(random.randrange(1,10e07)),
                    engineName  = cms.untracked.string('TRandom3'))  
)


process.load('DQMServices.Components.DQMEventInfo_cfi')
process.dqmEnv.subSystemFolder = cms.untracked.string('Ph2TkTB')

process.dqm_comm = cms.Sequence(process.dqmEnv)

# Path and EndPath definitions
process.endjob_step = cms.EndPath(process.endOfProcess)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)
#process.DQMoutput_step = cms.EndPath(process.FEVTDEBUGoutput)

process.dqm_step =  cms.Path(process.digiana_seq * process.dqm_comm )

process.mix_step = cms.Path(process.mix)
# Schedule definition
process.schedule = cms.Schedule(
    process.mix_step,
    process.dqm_step,
    process.DQMoutput_step
)

