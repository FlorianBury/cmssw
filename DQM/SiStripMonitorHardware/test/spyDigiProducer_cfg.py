#test configuration for the spy data unpacking code
import sys
import FWCore.ParameterSet.Config as cms
from Configuration.AlCa.GlobalTag import GlobalTag
from FWCore.ParameterSet.VarParsing import VarParsing

process = cms.Process('SPYPROD')

process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# ---- VarParsing ----
options = VarParsing('analysis')
options.register('start', 
                 '0',
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.string,
                 "First event to be processed (default = 0)")
options.register('stop', 
                 'max',
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.string,
                 "Last event to be processed (default = max)")

options.parseArguments()

# ---- Input data ----
# See https://twiki.cern.ch/twiki/bin/viewauth/CMS/FEDSpyChannelData for more spy data.
process.source = cms.Source(
    'PoolSource',
    fileNames = cms.untracked.vstring(
        # Spy data (raw) in edm format, as converted from .dat
#'file:/eos/cms/store/group/dpg_tracker_strip/tracker/Online/store/streamer/SiStripSpy/Commissioning11/321779/run321779.root',
'file:/eos/cms/store/group/dpg_tracker_strip/tracker/Online/store/streamer/SiStripSpy/Commissioning11/321054/run321054.root',
       ),
    eventsToProcess = cms.untracked.VEventRange('321054:'+options.start+'-321054:'+options.stop),
    )

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10000))

# --- Message Logging ---
#process.Tracer = cms.Service('Tracer',indentation = cms.untracked.string('$$'))
process.load('DQM.SiStripCommon.MessageLogger_cfi')
#process.MessageLogger.debugModules = cms.untracked.vstring('')
#process.MessageLogger.suppressInfo = cms.untracked.vstring('')
#process.MessageLogger.suppressWarning = cms.untracked.vstring('')
#process.MessageLogger.suppressDebug = cms.untracked.vstring('')


# --- Conditions data ---
# Find the appropriate Global Tags at
# https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')
process.load("Configuration.Geometry.GeometryRecoDB_cff")

# --- The unpacking configuration ---
process.load('DQM.SiStripMonitorHardware.SiStripSpyUnpacker_cfi')
process.load('DQM.SiStripMonitorHardware.SiStripSpyDigiConverter_cfi')

## * Scope digi settings
process.SiStripSpyUnpacker.FEDIDs = cms.vuint32()                   #use a subset of FEDs or leave empty for all.
process.SiStripSpyUnpacker.InputProductLabel = cms.InputTag('rawDataCollector')
process.SiStripSpyUnpacker.AllowIncompleteEvents = True
process.SiStripSpyUnpacker.StoreCounters = True
process.SiStripSpyUnpacker.StoreScopeRawDigis = cms.bool(True)      # Note - needs to be True for use in other modules.
## * Module digi settings
process.SiStripSpyDigiConverter.InputProductLabel = cms.InputTag('SiStripSpyUnpacker','ScopeRawDigis')
process.SiStripSpyDigiConverter.StorePayloadDigis = True
process.SiStripSpyDigiConverter.StoreReorderedDigis = True
process.SiStripSpyDigiConverter.StoreModuleDigis = True
process.SiStripSpyDigiConverter.StoreAPVAddress = True
process.SiStripSpyDigiConverter.MinDigiRange = 0
process.SiStripSpyDigiConverter.MaxDigiRange = 1024
process.SiStripSpyDigiConverter.MinZeroLight = 0
process.SiStripSpyDigiConverter.MaxZeroLight = 1024
process.SiStripSpyDigiConverter.MinTickHeight = 0
process.SiStripSpyDigiConverter.MaxTickHeight = 1024
process.SiStripSpyDigiConverter.ExpectedPositionOfFirstHeaderBit = 6
process.SiStripSpyDigiConverter.DiscardDigisWithWrongAPVAddress = False

# --- Define the path ---
process.p = cms.Path(
    process.SiStripSpyUnpacker
    *process.SiStripSpyDigiConverter
    )

# --- What to output ---
process.output = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string("/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-"+options.start+"-321054-"+options.stop+".root"),
    #fileName = cms.untracked.string("/afs/cern.ch/user/f/fbury/work/HybridStudy/SpyRawToDigis321779.root"),
    #fileName = cms.untracked.string("SpyRawToDigis321054_TEST.root"),
    outputCommands = cms.untracked.vstring(
       'keep *',
       #'drop *',
       #'drop *_source_*_*',
       #'drop *_TriggerResults__SPYUNPACKTEST',
       #'drop *_*_ScopeRawDigis_*',
       #'drop *_*_Payload_*',
       #'drop *_*_Reordered_*',
       #'drop *_*_VirginRaw_*',
       #'drop *_*_TotalEventCount_*',
       #'drop *_*_L1ACount_*',
       #'drop *_*_APVAddress_*',
       )
    )

process.e = cms.EndPath( process.output )

