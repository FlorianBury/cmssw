# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: FourMuExtendedPt_1_200_pythia8_cfi --conditions auto:phase2_realistic_T14 -n 10 --era Phase2C8 --eventcontent FEVTDEBUG --relval 10000,100 -s GEN,SIM --datatier GEN-SIM --beamspot HLLHC --geometry Extended2026D41 --python step1_GEN_SIM_CMSSW111_D41_cfg.py --no_exec

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

#from Configuration.Eras.Era_Phase2C8_cff import Phase2C8 -> Can't find
#process = cms.Process('SIM',Phase2C8)
from Configuration.Eras.Era_Phase2C9_cff import Phase2C9
process = cms.Process('SIM',Phase2C9)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('SimGeneral.MixingModule.cfwriter_cfi')
process.load("SimGeneral.MixingModule.trackingTruthProducerSelection_cfi")
#process.load('Configuration.Geometry.GeometryExtended2026D41Reco_cff')
#process.load('Configuration.Geometry.GeometryExtended2026D41_cff') -> Could not find
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
process.genstepfilter.triggerConditions=cms.vstring("generation_step")
from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic_T14', '') -> Could not find
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic_T15', '')


process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.N),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)



process.source = cms.Source("EmptySource",
   firstEvent = cms.untracked.uint32(1)
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


# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('FourMuExtendedPt_1_200_pythia8_cfi nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Pythia gun 
process.generator = cms.EDFilter("Pythia8PtGun",
    PGunParameters = cms.PSet(
        AddAntiParticle = cms.bool(True),
        MaxEta = cms.double(2.5),
        MaxPhi = cms.double(3.14159265359),
        MaxPt = cms.double(200.0),
        MinEta = cms.double(-2.5),
        MinPhi = cms.double(-3.14159265359),
        MinPt = cms.double(0.9),
            ParticleID = cms.vint32(-13, -13)
    ),
    PythiaParameters = cms.PSet(
        parameterSets = cms.vstring()
    ),
    Verbosity = cms.untracked.int32(0),
    firstRun = cms.untracked.uint32(1),
    psethack = cms.string('Four mu pt 1 to 200')
)



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

process.FEVTDEBUGoutput = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('*')
    ),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string(filename),
    outputCommands = process.FEVTDEBUGEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

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
process.generation_step = cms.Path(process.pgen)
process.simulation_step = cms.Path(process.psim)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.dqm_step = cms.Path(process.digiana_seq * process.dqm_comm)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGoutput_step = cms.EndPath(process.FEVTDEBUGoutput)

# Schedule definition
process.schedule = cms.Schedule(process.generation_step,process.genfiltersummary_step,process.simulation_step,process.dqm_step,process.endjob_step,process.FEVTDEBUGoutput_step)
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)
# filter all path with the production filter sequence
for path in process.paths:
	getattr(process,path).insert(0, process.generator)

