import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

process = cms.Process('HYBRID')#,eras.Run2_HI)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
#process.load('Configuration.EventContent.EventContentHeavyIons_cff')
process.load("Configuration.EventContent.EventContent_cff")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
#process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
###process.load('Configuration.StandardSequences.DigiToRaw_Repack_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("RecoVertex.BeamSpotProducer.BeamSpot_cff")

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    SkipEvent = cms.untracked.vstring('ProductNotFound'),
            
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1) 
)

# Input source
process.source = cms.Source("PoolSource",
    #fileNames = cms.untracked.vstring('file:/afs/cern.ch/user/f/fbury/work/HybridStudy/SpyRawToDigis321054.root'),
    fileNames = cms.untracked.vstring(
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-0-321054-500.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-1001-321054-1500.root', 
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-1501-321054-2000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-2501-321054-3000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-3001-321054-3500.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-3501-321054-4000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-4001-321054-4500.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-4501-321054-5000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-5001-321054-5500.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-501-321054-1000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-5501-321054-6000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-6001-321054-6500.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-6501-321054-7000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-7001-321054-7500.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-7501-321054-8000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-8001-321054-8500.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-8501-321054-9000.root',
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-9001-321054-9500.root',         
        'file:/afs/cern.ch/user/f/fbury/eos/HybridStudy/SpyRawToDigis321054-9501-321054-10000.root',
    #eventsToProcess = cms.untracked.VEventRange('321779:23-321779:23'),
)

process.options = cms.untracked.PSet(

)

# Production Info
#process.configurationMetadata = cms.untracked.PSet(
#    annotation = cms.untracked.string('step1 nevts:1'),
#    name = cms.untracked.string('Applications'),
#    version = cms.untracked.string('$Revision: 1.19 $')
#)

# Output definition

process.RAWoutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('RAW'),
        filterName = cms.untracked.string('')
    ),
    #fileName = cms.untracked.string('~/work/HybridStudy/whatever321054.root'),
    fileName = cms.untracked.string('~/work/HybridStudy/whatever321779.root'),
    #outputCommands = process.RAWEventContent.outputCommands,
    outputCommands = cms.untracked.vstring("drop*"),
    splitLevel = cms.untracked.int32(0)

)

# Additional output definition

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')

from EventFilter.RawDataCollector.rawDataCollectorByLabel_cfi import rawDataCollector
process.load("RecoLocalTracker.SiStripZeroSuppression.SiStripZeroSuppression_cfi")
from RecoLocalTracker.SiStripZeroSuppression.SiStripZeroSuppression_cfi import siStripZeroSuppression
#process.load("EventFilter.SiStripRawToDigi.SiStripDigiToRaw_cfi")
#from EventFilter.SiStripRawToDigi.SiStripDigiToRaw_cfi import SiStripDigiToRaw

##
## WF 1: emulate hybrid, repack, unpack, zero-suppress, repack
##
inputVR = cms.InputTag("SiStripSpyDigiConverter", "VirginRaw")
algo_zsHybridEmu = process.siStripZeroSuppression.Algorithms.clone(
                APVInspectMode = "HybridEmulation",
                APVRestoreMode = "",
                CommonModeNoiseSubtractionMode = 'Median',
                MeanCM = 0,
                DeltaCMThreshold = 20,
                Use10bitsTruncation = True,
            )
process.zsHybridEmu = process.siStripZeroSuppression.clone(  # Raw -> ZS in hybrid -> Digis
    produceRawDigis=True,
    produceHybridFormat=True,
    Algorithms=algo_zsHybridEmu,
    RawDigiProducersList=cms.VInputTag(inputVR),
    storeCM = cms.bool(True),
    produceCalculatedBaseline = cms.bool(True),
    produceBaselinePoints = cms.bool(True),
    )

algo_zsHybrid = process.siStripZeroSuppression.Algorithms.clone(
        APVInspectMode = "Hybrid")
process.zsHybrid = process.siStripZeroSuppression.clone(      # Full software (with inspect and restore) ZS -> digis
    RawDigiProducersList = cms.VInputTag(
        cms.InputTag("zsHybridEmu", "VirginRaw"),
        ),
    Algorithms=algo_zsHybrid,
    produceRawDigis = cms.bool(True),
    forceReadHybridFormat=cms.untracked.bool(True),
    storeCM = cms.bool(True),
    produceCalculatedBaseline = cms.bool(True),
    produceBaselinePoints = cms.bool(True),
    )

##
## WF 2: zero-suppress, repack
##
# ZS:       process.siStripZeroSuppression 
algo_zsClassic = process.siStripZeroSuppression.Algorithms.clone(
                APVInspectMode = "Null",
                APVRestoreMode = "",
                CommonModeNoiseSubtractionMode = 'Median',
                )
process.zsClassic = process.siStripZeroSuppression.clone(      # Without hybrid
        RawDigiProducersList=cms.VInputTag(inputVR),
        Algorithms=algo_zsClassic,
        produceRawDigis = cms.bool(True),
        storeCM = cms.bool(True),
        produceCalculatedBaseline = cms.bool(True),
        produceBaselinePoints = cms.bool(True),
    )

## Hybrid Format analyzer
process.hybridAna = cms.EDAnalyzer("SiStripHybridFormatAnalyzer",
    srcDigis =  cms.InputTag('zsHybrid','VirginRaw'),
    srcAPVCM =  cms.InputTag('zsHybrid','APVCMVirginRaw'),
    nModuletoDisplay = cms.uint32(10000),
    plotAPVCM	= cms.bool(True)
)
process.classicAna = cms.EDAnalyzer("SiStripHybridFormatAnalyzer",
    srcDigis =  cms.InputTag('zsClassic','VirginRaw'),
    srcAPVCM =  cms.InputTag('zsClassic','APVCMVirginRaw'),
    nModuletoDisplay = cms.uint32(10000),
    plotAPVCM	= cms.bool(True)
)   

## Comparison 
excludedDetId = cms.vuint32()
#excludedDetId = cms.vuint32([369174804, 
#                             369137006, 
#                             369140942,
#                             369141870,
#                             369138237,
#                             402674446,
#                             470111728,
#                             369174808,
#                             369141806,
#                             470145008, 
#                             470438384])
                            
process.diffRawZS = cms.EDAnalyzer("SiStripDigiDiff",
        A = cms.InputTag("zsHybrid", "VirginRaw"),
        B = cms.InputTag("zsClassic", "VirginRaw"),
        nDiffToPrint=cms.untracked.uint64(100),
        IgnoreAllZeros=cms.bool(True), ## workaround for packer removing all zero strips for ZS
        TopBitsToIgnore = cms.uint32(0),
        BottomBitsToIgnore = cms.uint32(1),
        )
process.digiStatDiff = cms.EDProducer("SiStripDigiStatsDiff",
        A = cms.InputTag("zsHybrid", "VirginRaw"),
        B = cms.InputTag("zsClassic", "VirginRaw"),
        detectInvalidDetIds=cms.bool(False), 
        invalidMinDigi=cms.double(50),
        invalidMaxDigi=cms.double(60),
        excludedDetId = excludedDetId,
        )

## Clusterizer 
process.load("RecoLocalTracker.SiStripClusterizer.SiStripClusterizer_RealData_cfi")
process.clusterizeZS1 = process.siStripClusters.clone(DigiProducersList=cms.VInputTag(cms.InputTag("zsHybrid", "VirginRaw")))
process.clusterizeZS2 = process.siStripClusters.clone(DigiProducersList=cms.VInputTag(cms.InputTag("zsClassic", "VirginRaw")))
process.clusterStatDiff = cms.EDProducer("SiStripClusterStatsDiff",
        A = cms.InputTag("clusterizeZS1"),
        B = cms.InputTag("clusterizeZS2"),
        detectInvalidDetIds=cms.bool(False), 
        invalidMinCharge=cms.double(0),
        invalidMaxCharge=cms.double(2000),
        invalidMinWidth=cms.double(50),
        invalidMaxWidth=cms.double(55),
        excludedDetId = excludedDetId,
        )


# Baseline Analyzer
process.load("RecoLocalTracker.SiStripZeroSuppression.SiStripBaselineAnalyzer_cfi")
#from RecoLocalTracker.SiStripZeroSuppression.SiStripBaselineAnalyzer_cfi import SiStripBaselineAnalyzer

process.baselineAnalyzerZS1 = process.SiStripBaselineAnalyzer.clone(
    nModuletoDisplay = cms.uint32(100000000),
    plotPedestals = cms.bool(True), ## should work in any case
    plotRawDigi = cms.bool(True), ## will plot raw digis, and do ZS on them (customize by setting Algorithms, like for the ZS)
    plotAPVCM = cms.bool(True), ## if True, pass a CM tag to 'srcAPVCM' (edm::DetSetVector<SiStripProcessedRawDigi>, the ZS will store one under APVCM+tag if storeCM is set to true)
    plotBaseline = cms.bool(True), ## set to true to plot the baseline, also pass srcBaseline then (from ZS with produceCalculatedBaseline=True, under BADAPVBASELINE+tag)
    plotBaselinePoints = cms.bool(True), ## set to true to plot the baseline points, also pass srcBaselinePoints then (from ZS with produceBaselinePoints=True, under BADAPVBASELINEPOINTS+tag)
    plotDigis = cms.bool(False), ## does not do anything
    plotClusters = cms.bool(True), ## would get the clusters from siStripClusters (hardcoded), so you'd need to change the code to add those plots (but it's independent of all the rest)
    useInvalidDetIds = cms.bool(False), ## Get the invalid detIDs from SiStripDigiStatsDiff and SiStripClusterStatsDiff : enable either invalidDetIdDigis or invalidDetIdClusters  Input Tags

    srcProcessedRawDigi = cms.InputTag('zsHybridEmu','VirginRaw'), ## here pass VR (edm::DetSetVector<SiStripRawDigi>), 'processed' is confusing but it's actually VR
    srcAPVCM  =  cms.InputTag('zsHybridEmu','APVCMVirginRaw'),
    srcBaseline =  cms.InputTag('zsHybridEmu','BADAPVBASELINEVirginRaw'),
    srcClusters = cms.InputTag('clusterizeZS1',''),
    invalidDetIdDigis = cms.InputTag('digiStatDiff','invalidDetIdDigis'),
    invalidDetIdClusters = cms.InputTag('clusterStatDiff','invalidDetIdClusters'),
    Algorithms = algo_zsHybridEmu,
)
process.baselineAnalyzerZS2 = process.SiStripBaselineAnalyzer.clone(
    nModuletoDisplay = cms.uint32(100000000),
    plotPedestals = cms.bool(True), ## should work in any case
    plotRawDigi = cms.bool(True), ## will plot raw digis, and do ZS on them (customize by setting Algorithms, like for the ZS)
    plotAPVCM = cms.bool(True), ## if True, pass a CM tag to 'srcAPVCM' (edm::DetSetVector<SiStripProcessedRawDigi>, the ZS will store one under APVCM+tag if storeCM is set to true)
    plotBaseline = cms.bool(True), ## set to true to plot the baseline, also pass srcBaseline then (from ZS with produceCalculatedBaseline=True, under BADAPVBASELINE+tag)
    plotBaselinePoints = cms.bool(True), ## set to true to plot the baseline points, also pass srcBaselinePoints then (from ZS with produceBaselinePoints=True, under BADAPVBASELINEPOINTS+tag)
    plotDigis = cms.bool(False), ## does not do anything
    plotClusters = cms.bool(True), ## would get the clusters from siStripClusters (hardcoded), so you'd need to change the code to add those plots (but it's independent of all the rest)
    useInvalidDetIds = cms.bool(False), ## Get the invalid detIDs from SiStripDigiStatsDiff and SiStripClusterStatsDiff : enable either invalidDetIdDigis or invalidDetIdClusters  Input Tags

    srcProcessedRawDigi = cms.InputTag('zsClassic','VirginRaw'), ## here pass VR (edm::DetSetVector<SiStripRawDigi>), 'processed' is confusing but it's actually VR
    srcAPVCM  =  cms.InputTag('zsClassic','APVCMVirginRaw'),
    srcBaseline =  cms.InputTag('zsClassic','BADAPVBASELINEVirginRaw'),
    srcClusters = cms.InputTag('clusterizeZS2',''),
    invalidDetIdDigis = cms.InputTag('digiStatDiff','invalidDetIdDigis'),
    invalidDetIdClusters = cms.InputTag('clusterStatDiff','invalidDetIdClusters'),
    Algorithms = algo_zsClassic,
)

process.baselineComparator = cms.EDAnalyzer("SiStripBaselineComparator",
    srcClusters = cms.InputTag('clusterizeZS1',''),
    srcClusters2 = cms.InputTag('clusterizeZS2',''),
)

process.hybridBaselineAnalyzer = cms.EDAnalyzer("SiStripHybridBaselineAnalyzer",
    nModuletoDisplay = cms.uint32(100000000),
    plotPedestals = cms.bool(True), ## should work in any case
    plotRawDigi = cms.bool(True), ## will plot raw digis, and do ZS on them (customize by setting Algorithms, like for the ZS)
    plotAPVCM = cms.bool(True), ## if True, pass a CM tag to 'srcAPVCM' (edm::DetSetVector<SiStripProcessedRawDigi>, the ZS will store one under APVCM+tag if storeCM is set to true)
    plotBaseline = cms.bool(True), ## set to true to plot the baseline, also pass srcBaseline then (from ZS with produceCalculatedBaseline=True, under BADAPVBASELINE+tag)
    plotBaselinePoints = cms.bool(True), ## set to true to plot the baseline points, also pass srcBaselinePoints then (from ZS with produceBaselinePoints=True, under BADAPVBASELINEPOINTS+tag)
    plotDigis = cms.bool(False), ## does not do anything
    plotClusters = cms.bool(True), ## would get the clusters from siStripClusters (hardcoded), so you'd need to change the code to add those plots (but it's independent of all the rest)
    useInvalidDetIds = cms.bool(False), ## Get the invalid detIDs from SiStripDigiStatsDiff and SiStripClusterStatsDiff : enable either invalidDetIdDigis or invalidDetIdClusters  Input Tags

    srcVirginRawDigi = inputVR, ## here pass VR (edm::DetSetVector<SiStripRawDigi>), 'processed' is confusing but it's actually VR
    srcZSVirginRawDigi = cms.InputTag('zsHybridEmu','VirginRaw'), ## here pass VR (edm::DetSetVector<SiStripRawDigi>), 'processed' is confusing but it's actually VR
    srcAPVCM  =  cms.InputTag('zsHybridEmu','APVCMVirginRaw'),
    srcBaseline =  cms.InputTag('zsHybridEmu','BADAPVBASELINEVirginRaw'),
    srcBaselineH =  cms.InputTag('zsHybrid','BADAPVBASELINEVirginRaw'),
    srcBaselinePoints =  cms.InputTag('zsHybridEmu','BADAPVBASELINEPOINTSVirginRaw'),
    srcBaselinePointsH =  cms.InputTag('zsHybrid','BADAPVBASELINEPOINTSVirginRaw'),
    srcClusters = cms.InputTag('clusterizeZS1',''),
    invalidDetIdDigis = cms.InputTag('digiStatDiff','invalidDetIdDigis'),
    invalidDetIdClusters = cms.InputTag('clusterStatDiff','invalidDetIdClusters'),
    Algorithms = algo_zsHybridEmu,
)

# RecHit
process.load("RecoLocalTracker.SiStripRecHitConverter.SiStripRecHitConverter_cfi")
from RecoLocalTracker.SiStripRecHitConverter.SiStripRecHitConverter_cfi import siStripMatchedRecHits
process.recHitZS1 = process.siStripMatchedRecHits.clone(
        ClusterProducer    = cms.InputTag('clusterizeZS1'),
)

process.recHitZS2 = process.siStripMatchedRecHits.clone(
        ClusterProducer    = cms.InputTag('clusterizeZS2'),
)

process.readRecHitZS1 = cms.EDAnalyzer("ReadRecHit",
        VerbosityLevel = cms.untracked.int32(0),
        RecHitProducer = cms.string('recHitZS1'),
        rechitsmatched = cms.InputTag('recHitZS1','matchedRecHit'),
        rechitsrphi    = cms.InputTag('recHitZS1','rphiRecHit'),
        rechitsstereo  = cms.InputTag('recHitZS1','stereoRecHit'),
)

process.readRecHitZS2 = cms.EDAnalyzer("ReadRecHit",
        VerbosityLevel = cms.untracked.int32(0),
        RecHitProducer = cms.string('recHitZS2'),
)

# Track reconstruction 
process.load("RecoTracker.Configuration.RecoTracker_cff")

from RecoTracker.TkSeedingLayers.PixelLessLayerPairs_cfi import PixelLessLayerPairs
process.pixelLessLayerPairsZS1 = PixelLessLayerPairs.clone()

process.pixelLessLayerPairsZS1.TIB.matchedRecHits = cms.InputTag("recHitZS1","matchedRecHit")
process.pixelLessLayerPairsZS1.TIB.rphiRecHits    = cms.InputTag("recHitZS1","rphiRecHitUnmatched")
process.pixelLessLayerPairsZS1.TIB.stereoRecHits  = cms.InputTag("recHitZS1","stereoRecHitUnmatched")

process.pixelLessLayerPairsZS1.TID.matchedRecHits = cms.InputTag("recHitZS1","matchedRecHit")
process.pixelLessLayerPairsZS1.TID.rphiRecHits    = cms.InputTag("recHitZS1","rphiRecHitUnmatched")
process.pixelLessLayerPairsZS1.TID.stereoRecHits  = cms.InputTag("recHitZS1","stereoRecHitUnmatched")

process.pixelLessLayerPairsZS1.TEC.matchedRecHits = cms.InputTag("recHitZS1","matchedRecHit")
process.pixelLessLayerPairsZS1.TEC.rphiRecHits    = cms.InputTag("recHitZS1","rphiRecHitUnmatched")
process.pixelLessLayerPairsZS1.TEC.stereoRecHits  = cms.InputTag("recHitZS1","stereoRecHitUnmatched")

from RecoTracker.IterativeTracking.iterativeTk_cff import *
from RecoTracker.IterativeTracking.ElectronSeeds_cff import *
from RecoTracker.DeDx.dedxEstimators_cff import *
process.tracking = cms.Sequence(iterTracking*electronSeedsSeq*doAlldEdXEstimators)


# printcontent 
process.load("FWCore.Modules.printContent_cfi")

# Sequence 
process.DigiToRawZS = cms.Sequence(
        process.zsHybridEmu *  process.zsHybrid * process.zsClassic
        # Analyze #
        #* process.diffRawZS 
        * process.digiStatDiff
        * process.clusterizeZS1 * process.clusterizeZS2
        * process.clusterStatDiff
        # Baseline 
        #* process.baselineAnalyzerZS1 * process.baselineAnalyzerZS2
        #* process.hybridBaselineAnalyzer
        #* process.baselineComparator
        #* process.hybridAna * process.classicAna
        # RecHit 
         * process.recHitZS1 * process.recHitZS2
        # read RecHit #
        #* process.readRecHitZS1 #* process.readRecHitZS2
        # Track reconstruction 
        #* process.pixelLessLayerPairsZS1
        #* process.tracking
        # Printcontent
        * process.printContent
        )

# TFileService output 

process.TFileService = cms.Service("TFileService",
        #fileName = cms.string("/afs/cern.ch/user/f/fbury/work/HybridStudy/diffhistos321054.root"),
        fileName = cms.string("/afs/cern.ch/user/f/fbury/work/HybridStudy/diffhistos321779_recHit.root"),
        closeFileFast = cms.untracked.bool(True),
        )

del process.siStripQualityESProducer.ListOfRecordToMerge[0] # Because spy data is outside of a run

# Path and EndPath definitions
#process.raw2digi_step = cms.Path(process.RawToDigi)
process.DigiToRawZS = cms.Path(process.DigiToRawZS)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.RAWoutput_step = cms.EndPath(process.RAWoutput)

# Schedule definition
#process.schedule = cms.Schedule(process.raw2digi_step,process.DigiToRawZS,process.endjob_step,process.RAWoutput_step)
process.schedule = cms.Schedule(process.DigiToRawZS,process.endjob_step,process.RAWoutput_step)
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# Customisation from command line

# Add early deletion of temporary data products to reduce peak memory need
#from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
#process = customiseEarlyDelete(process)
# End adding early deletion

#process.MessageLogger = cms.Service(
#    "MessageLogger",
#    destinations = cms.untracked.vstring(
#        "log_checkhybrid"
#        ),
#    debugModules = cms.untracked.vstring("diffRawZS", 
#                                         "zsHybridEmu", 
#                                         "zsHybrid", 
#                                         "zsClassic", 
#                                         "siStripZeroSuppression", 
#                                         "StatDiff",
#                                         "clusterizeZS1",
#                                         "clusterizeZS2",
#                                         "clusterStatDiff",
#                                         "recHitZS1",
#                                         "recHitZS2",
#                                         "readRecHitZS1",
#                                         "readRecHitZS2",
#                                         ),
#    categories=cms.untracked.vstring("SiStripZeroSuppression", "SiStripDigiDiff")
#    )
#
