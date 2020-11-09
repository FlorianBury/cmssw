import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
simValid = DQMEDAnalyzer('Phase2TrackerValidateSimHit',
    Verbosity = cms.bool(True),
    TopFolderName = cms.string("Ph2TkPixelDigi"),
    PixelPlotFillingFlag = cms.bool(False),
    NtupleCreationFlag = cms.bool(False),
    PSimHitSource  = cms.VInputTag('g4SimHits:TrackerHitsPixelBarrelLowTof',
                                   'g4SimHits:TrackerHitsPixelBarrelHighTof',
                                   'g4SimHits:TrackerHitsPixelEndcapLowTof',
                                   'g4SimHits:TrackerHitsPixelEndcapHighTof',
                                   'g4SimHits:TrackerHitsTIBLowTof',
                                   'g4SimHits:TrackerHitsTIBHighTof',
                                   'g4SimHits:TrackerHitsTIDLowTof',
                                   'g4SimHits:TrackerHitsTIDHighTof',
                                   'g4SimHits:TrackerHitsTOBLowTof',
                                   'g4SimHits:TrackerHitsTOBHighTof',
                                   'g4SimHits:TrackerHitsTECLowTof',
                                   'g4SimHits:TrackerHitsTECHighTof'),
    TrackingTruthSource = cms.InputTag( "mix","MergedTrackTruth" ),
    GeometryType = cms.string('idealForDigi'),
    XYPositionMapH = cms.PSet(
        Nxbins = cms.int32(100),
        xmin   = cms.double(-50.),
        xmax   = cms.double(50.),
        Nybins = cms.int32(100),
        ymin   = cms.double(-50),
        ymax   = cms.double(50.)
    ),
    RZPositionMapH = cms.PSet(
        Nxbins = cms.int32(10000),
        xmin   = cms.double(-500.0),
        xmax   = cms.double(500.),
        Nybins = cms.int32(65),
        ymin   = cms.double(0.),
        ymax   = cms.double(65.)
    ),

    NumerOfHitsH = cms.PSet(
        Nxbins = cms.int32(100),
        xmin   = cms.double(-0.5),
        xmax   = cms.double(99.5)
    ),
    TimeOfFlightH = cms.PSet(
        Nxbins = cms.int32(401),
        xmin   = cms.double(-300.5),
        xmax   = cms.double(100.5)
    ),
    ChargeH = cms.PSet(
        Nxbins = cms.int32(50),
        xmin   = cms.double(0.0),
        xmax   = cms.double(100000.)
    ),
    ZPosH = cms.PSet(
        Nxbins = cms.int32(6000),
        xmin   = cms.double(-3000.),
        xmax   = cms.double(3000.),        
    ),
    BXNumberH = cms.PSet(
        Nxbins = cms.int32(20),
        xmin   = cms.double(-15.5),
        xmax   = cms.double(4.5) 
    ),
    TimeOfArrivalH = cms.PSet(
        Nxbins = cms.int32(25),
        xmin   = cms.double(0.5),
        xmax   = cms.double(25.5) 
    ),
    AmplTimeH = cms.PSet(
        Nxbins = cms.int32(75),
        xmin   = cms.double(0.5),
        xmax   = cms.double(75.5)
    ),
    SimTrackPtH = cms.PSet(
        Nxbins = cms.int32(51),
        xmin   = cms.double(-0.5),
        xmax   = cms.double(50.5)
    )

)

from Configuration.ProcessModifiers.premix_stage2_cff import premix_stage2
premix_stage2.toModify(simValid,
    OuterTrackerDigiSimLinkSource = "mixData:Phase2OTDigiSimLink",
    InnerPixelDigiSimLinkSource = "mixData:PixelDigiSimLink",
)
