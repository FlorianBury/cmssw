// -*- C++ -*-
//
// Package:    Phase2TrackerBXHistogram
// Class:      Phase2TrackerBXHistogram
// 
/**\class Phase2TrackerBXHistogram Phase2TrackerBXHistogram.cc 

Description: Test pixel digis. 

*/
//
// Author: Suchandra Dutta, Suvankar Roy Chowdhury, Subir Sarkar
// Date: January 29, 2016
//
// system include files
#include <memory>
#include <stdexcept>
#include "Phase2TrackerBXHistogram.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESWatcher.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h" 


#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// Geometry
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "Geometry/CommonDetUnit/interface/TrackerGeomDet.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetType.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"

#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/CrossingFrame/interface/MixCollection.h"

// DQM Histograming
#include "DQMServices/Core/interface/MonitorElement.h"
#include "TF1.h"
#include "TTree.h"

// CLHEP 
#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Units/GlobalSystemOfUnits.h"
#include "CLHEP/Units/GlobalPhysicalConstants.h"

namespace scaling {
    double nFactorial(int n) { return std::tgamma(n + 1); }
    double aScalingConstant(int N, int i) {
        return std::pow(-1, (double)i) * nFactorial(N) * nFactorial(N + 2) /
            (nFactorial(N - i) * nFactorial(N + 2 - i) * nFactorial(i));
    }
}  // namespace



// 
// constructors 
//
Phase2TrackerBXHistogram::Phase2TrackerBXHistogram(const edm::ParameterSet& iConfig) :
    config_(iConfig),
    geomType_(config_.getParameter<std::string>("GeometryType")),
    simTrackSrc_(config_.getParameter<edm::InputTag>("SimTrackSource")),
    pSimHitSrc_(config_.getParameter<std::vector<edm::InputTag> >("PSimHitSource")),
    //tParticleSrc_(config_.getParameter<edm::InputTag>("TrackingTruthSource")),
    simTrackToken_(consumes<edm::SimTrackContainer>(simTrackSrc_)),
    //tParticleToken_(consumes<std::vector<TrackingParticle> >(tParticleSrc_)),
    pulseShapeParameters_(config_.getParameter<std::vector<double>>("PulseShapeParameters")),
    mode_(config_.getParameter<std::string>("Mode")),
    bx_range_(config_.getParameter<int>("BXRange")),
    deadTime_(config_.getParameter<double>("CBCDeadTime")),
    pTCut_(config_.getParameter<double>("PTCut")),
    theTofLowerCut_(config_.getParameter<double>("TofLowerCut")),
    theTofUpperCut_(config_.getParameter<double>("TofUpperCut")),
    offset_min_(config_.getParameter<double>("OffsetMin")),
    offset_max_(config_.getParameter<double>("OffsetMax")),
    offset_step_(config_.getParameter<double>("OffsetStep")),
    offset_emulate_(config_.getParameter<double>("OffsetEmulate")),
    theThresholdInE_Endcap_(config_.getParameter<double>("ThresholdInElectrons_Endcap")),
    theThresholdInE_Barrel_(config_.getParameter<double>("ThresholdInElectrons_Barrel")),
    theThresholdSmearing_Endcap_(config_.getParameter<double>("ThresholdSmearing_Endcap")),
    theThresholdSmearing_Barrel_(config_.getParameter<double>("ThresholdSmearing_Barrel")),
    tof_smearing_(config_.getParameter<double>("TOFSmearing")),
    GeVperElectron(3.61E-09), // 1 electron(3.61eV, 1keV(277e, mod 9/06 d.k.
    verbosity_(config_.getParameter<int>("VerbosityLevel"))
{
    for (const auto& itag : pSimHitSrc_){
        simHitTokens_.push_back(std::make_pair(itag,consumes<edm::PSimHitContainer>(itag)));
    }

    edm::LogInfo("Phase2TrackerBXHistogram") << ">>> Construct Phase2TrackerBXHistogram ";
    if (verbosity_ > 0){
        std::cout << ">>> Construct Phase2TrackerBXHistogram " <<std::endl;
        std::cout << "Mode                        : "<<mode_<<std::endl;
        std::cout << "Threshold (Barrel)          : "<<theThresholdInE_Barrel_<<std::endl;
        std::cout << "Threshold (Endcap)          : "<<theThresholdInE_Endcap_<<std::endl;
        std::cout << "Threshold Smearing (Endcap) : "<<theThresholdSmearing_Endcap_<<std::endl;
        std::cout << "Threshold Smearing (Barrel) : "<<theThresholdSmearing_Barrel_<<std::endl;
        std::cout << "ToF smearing                : "<<tof_smearing_<<std::endl;
        if (mode_ == "emulate")
            std::cout << "Offset for emulation        : "<<offset_emulate_<<std::endl;
    }
    if (mode_ != "scan" && mode_ != "emulate"){
        throw std::invalid_argument("Mode "+mode_+" not understood");
    }

    nEvent = 0;
 
}

//
// destructor
//
Phase2TrackerBXHistogram::~Phase2TrackerBXHistogram() {
    // do anything here that needs to be done at desctruction time
    // (e.g. close files, deallocate resources etc.)
    edm::LogInfo("Phase2TrackerBXHistogram")<< ">>> Destroy Phase2TrackerBXHistogram ";
    if (verbosity_ > 0)
        std::cout << ">>> Destroy Phase2TrackerBXHistogram " << std::endl;
}
//
// -- DQM Begin Run 
//
void Phase2TrackerBXHistogram::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    edm::LogInfo("Phase2TrackerBXHistogram")<< "Initialize Phase2TrackerBXHistogram ";
    if (verbosity_ > 0)
        std::cout << "Initialize Phase2TrackerBXHistogram " <<std::endl;
    offset_scan_.clear();
    if (mode_ == "scan"){
        double offIt = offset_min_;
        offset_scan_.push_back(offIt);
        while (offIt <= offset_max_){
            offIt += offset_step_;
            offset_scan_.push_back(offIt);
        }
    }
    if (mode_ == "emulate"){
        offset_scan_.push_back(offset_emulate_);
    }
    fireRandom_ = true;
}

//
// -- Analyze
//
void Phase2TrackerBXHistogram::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;

    // random generator for smearing 
    Service<RandomNumberGenerator> rng;
    if(!rng.isAvailable()) {
        throw cms::Exception("Configuration")
            << "Phase2TrackerBXHistogram requires the RandomNumberGeneratorService,\n"
            "which is not present in the configuration file. You must add\n"
            "the service in the configuration file or remove the modules that\n"
            "require it.\n";
    }
    CLHEP::HepRandomEngine& eng = rng->getEngine(iEvent.streamID());

    if (fireRandom_){
        smearedThresholdFactors_Endcap_.clear();
        smearedThresholdFactors_Barrel_.clear();
        smearedTofFactors_.clear();

        if (theThresholdSmearing_Endcap_ > 0.){
            smearedThreshold_Endcap_ = std::make_unique<CLHEP::RandFlat>(eng,-theThresholdSmearing_Endcap_,theThresholdSmearing_Endcap_); 
        }
        if (theThresholdSmearing_Barrel_ > 0.){
            smearedThreshold_Barrel_ = std::make_unique<CLHEP::RandFlat>(eng,-theThresholdSmearing_Barrel_,theThresholdSmearing_Barrel_); 
        }
        if (tof_smearing_ > 0.){
            smearedTOF_ = std::make_unique<CLHEP::RandFlat>(eng,-tof_smearing_,tof_smearing_); 
        }
        fireRandom_ = false;
    }
    // Tracker Topology 
    iSetup.get<TrackerTopologyRcd>().get(tTopoHandle_);

    edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;

    if (theTkDigiGeomWatcher.check(iSetup)) {
        iSetup.get<TrackerDigiGeometryRecord>().get(geomType_, geomHandle_);
    }  
    if (!geomHandle_.isValid()) return;

    edm::ESHandle<GeometricDet> rDD;
    iSetup.get<IdealGeometryRecord>().get(geomType_,rDD);
//    iEvent.getByToken(tParticleToken_, tParticleHandle_);
    iEvent.getByToken(simTrackToken_, simTrackHandle_);

//    //    if (tParticleHandle_.isValid()) {
//    if (verbosity_>0){
//        std::cout << "Loop over TrackingParticle particles"<<std::endl;
//        for (std::vector<TrackingParticle>::const_iterator  iterTPart = tParticleHandle_->begin(); iterTPart != tParticleHandle_->end(); ++iterTPart) {
//            unsigned int ival = 0;
//            std::cout << ival++ << ". BX #" << iterTPart->eventId().bunchCrossing() << " Event # " << iterTPart->eventId().event() << " Track Id " << iterTPart->g4Tracks()[0].trackId() << " PDG Id " << iterTPart->pdgId() << " Momentum "<< iterTPart->p() << " Pt " << iterTPart->pt() <<" Eta "<< iterTPart->eta() << " Phi "<< iterTPart->phi() <<std::endl;
//
//        }
//    }
//    //    }
//    else{
//        std::cout <<"TrackingParticle handle not valid" <<std::endl;
//    }

        if (simTrackHandle_.isValid()) { 
            if (verbosity_>0){
                std::cout << "Loop over SimTrack particles"<<std::endl;
                for (edm::SimTrackContainer::const_iterator simTrkItr = simTrackHandle_->begin(); simTrkItr != simTrackHandle_->end();    ++simTrkItr) {
                    unsigned int ival = 0;
                    std::cout << ival++ << ". BX #" << simTrkItr->eventId().bunchCrossing() << " Event # " << simTrkItr->eventId().event() << " Track Id " << simTrkItr->trackId() << " PDG Id " << simTrkItr->type() << " Momentum "<< simTrkItr->momentum().P() << " Pt " << simTrkItr->momentum().Pt() <<" Eta "<< simTrkItr->momentum().Eta() << " Phi "<< simTrkItr->momentum().Phi() <<std::endl;
    
                }
            }
        }
        else{
            std::cout <<"SimTrack handle not valid" <<std::endl;
        }

    const TrackerTopology* tTopo = tTopoHandle_.product();
    const TrackerGeometry* tGeom = geomHandle_.product();  

    for(std::vector<double>::iterator offset = offset_scan_.begin(); offset != offset_scan_.end(); ++offset) {
        if (verbosity_>1){
            std::cout<<"========================================================"<<std::endl;
            std::cout<<"New offset value "<<*offset<<std::endl;
        }
        pulseShapeParameters_[0] = *offset;
        Phase2TrackerBXHistogram::storeSignalShape();
        if (verbosity_>1){
            std::cout << "Pulse shape params : "<<std::endl;
            for(std::vector<double>::iterator psp = pulseShapeParameters_.begin(); psp != pulseShapeParameters_.end(); ++psp) {
                std::cout<<"   "<<*psp<<std::endl;
            }
            std::cout << "Loop over hit "<<std::endl;
        }

//        std::vector<edm::InputTag>::iterator itag = pSimHitSrc_.begin();
//        for (const auto& itoken : simHitTokens_) {
        for (const auto& itoken : simHitTokens_) {
            edm::Handle<edm::PSimHitContainer> simHitHandle;
            iEvent.getByToken(itoken.second, simHitHandle);
//            if (itoken.first.encode() == "FastTimerHitsBarrel")
//                continue;
//            if (itoken.first.encode() == "FastTimerHitsEndcap")
//                continue;
            if (verbosity_>1){
                std::cout << "Hit collection " << itoken.first << std::endl;
            }
            if (!simHitHandle.isValid()){
                if (verbosity_>1){
                    std::cout<<"Not valid token"<<std::endl;
                }
                continue;
            }
            const edm::PSimHitContainer& simHits = (*simHitHandle.product());
            for(edm::PSimHitContainer::const_iterator isim = simHits.begin(); isim != simHits.end(); ++isim){
                int tkid = (*isim).trackId();
                if (verbosity_ > 1)
                    std::cout << "Track id "<<tkid<<std::endl;
                if (tkid <= 0) continue;
                const PSimHit& simHit = (*isim);

                unsigned int rawid = simHit.detUnitId();
                const DetId detId(rawid);
                if (detId.det() != DetId::Detector::Tracker) continue;

                float dZ = (*isim).entryPoint().z() - (*isim).exitPoint().z();  
                if (fabs(dZ) <= 0.01) continue;

                /* check if strip */
                if (!isStrip(detId)){
                    if (verbosity_ > 1){
                        std::cout<<"\tNot a strip hit ("<< rawid <<") -> discarded"<<std::endl;
                    }
                    continue;
                }

                /* Layer and geometry */
                int layer = tTopo->getOTLayerNumber(rawid);
                if (verbosity_ > 1)
                    std::cout << " rawid " << rawid << " layer " << layer << std::endl;
                if (layer < 0) continue;
                const GeomDet *geomDet = tGeom->idToDet(detId);
                if (!geomDet){
                    if (verbosity_>1){
                        std::cout<<"\tGeometry not valid"<<std::endl;
                    }
                    continue;
                }
                Global3DPoint pdPos = geomDet->surface().toGlobal(isim->localPosition());

                int bx_true  = simHit.eventId().bunchCrossing();
                int event = simHit.eventId().event();
                float time_to_detid_ns = pdPos.mag()/(CLHEP::c_light*CLHEP::ns/CLHEP::cm);
                float tof = (*isim).timeOfFlight();
                float tkpt = Phase2TrackerBXHistogram::getSimTrackPt(simHit.eventId(), tkid);
                //float tkpt = 3.;
                float corr_tof = tof - time_to_detid_ns;
                float charge = simHit.energyLoss()/GeVperElectron;

                if (verbosity_>2){
                    std::cout << " BX # " << bx_true << " Event number " << event << " Raw Id "<< rawid << " SimTrack Id  " << tkid << " SimTrack Pt " << tkpt <<" Time Of Flight " << tof  << " X pos " <<  pdPos.x() << " Y pos " <<  pdPos.y() << " Z pos " << pdPos.z() << " R pos " <<  std::hypot(pdPos.x(),pdPos.y())  << " time_to_detid_ns " << time_to_detid_ns <<" tof " << tof << " corr_tof " << corr_tof << " charge " << charge <<std::endl; 
                }

                if (tkpt <= pTCut_){
                    if (verbosity_>1){
                        std::cout <<"PT "<<tkpt<<" below cut at "<<pTCut_<<" -> dismissed"<<std::endl; 
                    }
                    continue;
                }
    
                if (verbosity_>1){
                    std::cout<<"-------------------------"<<std::endl;
                    std::cout<<"New hit on detID "<< detId.rawId() <<" with PT = "<<tkpt<<std::endl;
                }       

                
                int attSampled = 0;
                int attLatched = 0;
                for (int bx = bx_true-bx_range_; bx <= bx_true+bx_range_; bx ++){
                    if (verbosity_>2)
                        std::cout << "True BX = "<< bx_true << " -> Looking at "<<bx<<std::endl;
                    /* Sampled mode */
                    if (Phase2TrackerBXHistogram::select_hit(charge,bx,tof,corr_tof,detId,Phase2TrackerBXHistogram::SampledMode)){
                        offsetBX_[*offset].Sampled->Fill(bx-0.5-1);
                        offsetBXMap_.Sampled->Fill(bx-0.5-1,*offset);
                        if (verbosity_>2)
                            std::cout<<"   Sampled fired"<<std::endl;
                        attSampled++;
                    }
                    /* Latched mode */ 
                    if (Phase2TrackerBXHistogram::select_hit(charge,bx,tof,corr_tof,detId,Phase2TrackerBXHistogram::LatchedMode)){
                        offsetBX_[*offset].Latched->Fill(bx-0.5-1);
                        offsetBXMap_.Latched->Fill(bx-0.5-1,*offset);
                        if (verbosity_>1)
                            std::cout<<"   Latched fired"<<std::endl;
                        attLatched++;
                    }
                    hitsTrueMap_.Sampled->Fill(bx-0.5-1,*offset);
                    hitsTrueMap_.Latched->Fill(bx-0.5-1,*offset);
                }
                attBXMap_.Sampled->Fill(attSampled,*offset);
                attBXMap_.Latched->Fill(attLatched,*offset);
            }
        }
    }


    nEvent++;
}


//
// -- Book Histograms
//
void Phase2TrackerBXHistogram::bookHistograms(DQMStore::IBooker & ibooker,edm::Run const &  iRun ,edm::EventSetup const &  iSetup ) {

    ibooker.cd();

    std::string top_folder = config_.getParameter<std::string>("TopFolderName");
    std::stringstream folder_name;
    folder_name << top_folder << "/" << "Hist2D";
    ibooker.setCurrentFolder(folder_name.str());

    std::stringstream HistoName;


    /* Offset scan */
    HistoName.str("");
    HistoName << "OffsetScanSampled";
    offsetBXMap_.Sampled = ibooker.book2D(HistoName.str(), HistoName.str(),
                                          (2*bx_range_)+1,-static_cast<float>(bx_range_)-0.5,static_cast<float>(bx_range_)+0.5,
                                          offset_scan_.size(),offset_min_-(offset_step_/2),offset_max_+(offset_step_/2));
    HistoName.str("");
    HistoName << "OffsetScanLatched";
    offsetBXMap_.Latched = ibooker.book2D(HistoName.str(), HistoName.str(),
                                          (2*bx_range_)+1,-static_cast<float>(bx_range_)-0.5,static_cast<float>(bx_range_)+0.5,
                                          offset_scan_.size(),offset_min_-(offset_step_/2),offset_max_+(offset_step_/2));

    /* Attribution scan */
    HistoName.str("");
    HistoName << "AttributionScanSampled";
    attBXMap_.Sampled = ibooker.book2D(HistoName.str(), HistoName.str(),
                                       5,-0.5,4.5,
                                       offset_scan_.size(),offset_min_-(offset_step_/2),offset_max_+(offset_step_/2));
    HistoName.str("");
    HistoName << "AttributionScanLatched";
    attBXMap_.Latched = ibooker.book2D(HistoName.str(), HistoName.str(),
                                       5,-0.5,4.5,
                                       offset_scan_.size(),offset_min_-(offset_step_/2),offset_max_+(offset_step_/2));

    /* Efficiency true scan */
    HistoName.str("");
    HistoName << "HitsTrueNumberScanSampled";
    hitsTrueMap_.Sampled = ibooker.book2D(HistoName.str(), HistoName.str(),
                                          (2*bx_range_)+1,-static_cast<float>(bx_range_)-0.5,static_cast<float>(bx_range_)+0.5,
                                          offset_scan_.size(),offset_min_-(offset_step_/2),offset_max_+(offset_step_/2));

    HistoName.str("");
    HistoName << "HitsTrueNumberScanLatched";
    hitsTrueMap_.Latched = ibooker.book2D(HistoName.str(), HistoName.str(),
                                          (2*bx_range_)+1,-static_cast<float>(bx_range_)-0.5,static_cast<float>(bx_range_)+0.5,
                                          offset_scan_.size(),offset_min_-(offset_step_/2),offset_max_+(offset_step_/2));


    for(std::vector<double>::iterator offset = offset_scan_.begin(); offset != offset_scan_.end(); ++offset) {
        HistModes hist_modes = Phase2TrackerBXHistogram::bookBXHistos(ibooker,*offset);
        offsetBX_.insert(std::make_pair(*offset,hist_modes));
    }
}

Phase2TrackerBXHistogram::HistModes Phase2TrackerBXHistogram::bookBXHistos(DQMStore::IBooker & ibooker,double offset){
    //edm::LogInfo("Phase2TrackerBXHistogram")<< " Booking Histograms in : " << folder_name.str();
    std::string top_folder = config_.getParameter<std::string>("TopFolderName");
    std::stringstream folder_name;

    ibooker.cd();
    folder_name << top_folder << "/" << "Hist1D";
    ibooker.setCurrentFolder(folder_name.str());
    if (verbosity_>0){
        std::cout<< "Booking Histograms in : " << folder_name.str()<<"  (offset = "<<offset<<")"<<std::endl;
    }

    std::stringstream HistoName;

    HistModes hist_modes;

    HistoName.str("");
    HistoName << "BXHistogramSampledOffset" << offset; //<<";Relative BX Id";
    hist_modes.Sampled = ibooker.book1D(HistoName.str(), HistoName.str(),
                                        (2*bx_range_)+1,-static_cast<float>(bx_range_)-0.5,static_cast<float>(bx_range_)+0.5);

    HistoName.str("");
    HistoName << "BXHistogramLatchedOffset"  << offset;// <<";Relative BX Id";
    hist_modes.Latched = ibooker.book1D(HistoName.str(), HistoName.str(),
                                        (2*bx_range_)+1,-static_cast<float>(bx_range_)-0.5,static_cast<float>(bx_range_)+0.5);

    return hist_modes;
}



float Phase2TrackerBXHistogram::getSimTrackPt(EncodedEventId event_id, unsigned int tk_id) {  
    for (edm::SimTrackContainer::const_iterator simTrkItr = simTrackHandle_->begin(); simTrkItr != simTrackHandle_->end();    ++simTrkItr) {
        if (simTrkItr->eventId() != event_id)
            continue;
        if (simTrkItr->trackId() == tk_id)
            return simTrkItr->momentum().Pt();
    }
    return -1;
}

// isPixel 
bool Phase2TrackerBXHistogram::isPixel(const DetId& detId) {
    return (detId.subdetId() == PixelSubdetector::PixelBarrel ||  detId.subdetId() == PixelSubdetector::PixelEndcap);
}

bool Phase2TrackerBXHistogram::isStrip(const DetId& detId) {
    return (detId.subdetId() == SiStripSubdetector::TIB || detId.subdetId() == SiStripSubdetector::TID || detId.subdetId() == SiStripSubdetector::TOB || detId.subdetId() == SiStripSubdetector::TEC);
}


//bool Phase2TrackerBXHistogram::select_hit(const PSimHit& hit, double tCorr, double& sigScale) const {
bool Phase2TrackerBXHistogram::select_hit(float charge, int bx, float tof, float tcorr, DetId det_id, int hitDetectionMode){
    if (verbosity_>2)
        std::cout<<"Requesting mode "<<hitDetectionMode<<std::endl;
    bool result = false;

    float theThresholdInE = (det_id.subdetId() == StripSubdetector::TOB) ? theThresholdInE_Barrel_ : theThresholdInE_Endcap_;
    float threshold_smearing = 0.;
    if (det_id.subdetId() == StripSubdetector::TOB && theThresholdSmearing_Barrel_>0.){
        threshold_smearing = smearBarrelThresholdDetId(det_id);
    }
    if (det_id.subdetId() != StripSubdetector::TOB && theThresholdSmearing_Endcap_>0.){
        threshold_smearing = smearEndcapThresholdDetId(det_id);
    }
    if (verbosity_>2)
        std::cout<<"Detid : "<<det_id.rawId()<<" -> Threshold = "<<theThresholdInE<<" + "<<threshold_smearing<<std::endl;
    theThresholdInE += threshold_smearing;

    float tofsmeared = 0.;
    if (tof_smearing_ > 0.){
        tofsmeared = smearToFDetId(det_id);                
    }
    if (verbosity_>2)
        std::cout<<"Detid : "<<det_id.rawId()<<" -> TOF = "<<tof<<" + "<<tofsmeared<<std::endl;
    tof += tofsmeared;
    

    if (hitDetectionMode == Phase2TrackerBXHistogram::SampledMode){
        if (verbosity_>2)
            std::cout<<" -> Sampled mode"<<std::endl;
        //result = select_hit_sampledMode(hit, tCorr, scale);
        result = select_hit_sampledMode(charge,bx,tof,tcorr,det_id,theThresholdInE);
    }
    else if (hitDetectionMode == Phase2TrackerBXHistogram::LatchedMode){
        if (verbosity_>2)
            std::cout<<" -> Latched mode"<<std::endl;
        //result = select_hit_latchedMode(hit, tCorr, scale);
        result = select_hit_latchedMode(charge,bx,tof,tcorr,det_id,theThresholdInE);
    }
    else {
        double toa = tof - tcorr;
        result = (toa > theTofLowerCut_ && toa < theTofUpperCut_);
    }
    return result;
}
//
// -- Select Hits in Sampled Mode
//
bool Phase2TrackerBXHistogram::select_hit_sampledMode(float charge, int bx, float tof, float tcorr, DetId det_id, float threshold) const {
    double toa = tof - tcorr;
    toa -= bx*bx_time;
    double sampling_time = bx_time;
    double sigScale = getSignalScale(sampling_time - toa);

    if (verbosity_>2){
        std::cout<<"\t(ToF) "<<tof<<"\t - (TCorr) "<<tcorr<<"\t= (ToA) "<<toa+bx*bx_time<<" -> (corrected) "<<toa<<std::endl;
        std::cout<<"\tAt time "<<sampling_time-toa<<" :  (scale) "<<sigScale<<" * (charge) "<<charge<< " = "<<sigScale*charge<<" -> Trigger "<<int(sigScale * charge > threshold)<<std::endl;
        if (sigScale * charge > threshold)  std::cout<<"PASSED"<<std::endl;
    }

    return (sigScale * charge > threshold);
}
//
// -- Select Hits in Hit Detection Mode
//
bool Phase2TrackerBXHistogram::select_hit_latchedMode(float charge, int bx, float tof, float tcorr, DetId det_id, float threshold) const {
    float toa = tof - tcorr;
    toa -= bx * bx_time;

    float sampling_time = 0;
    bool lastPulse = true;
    bool aboveThr = false;
    for (float i = deadTime_; i <= bx_time; i++) {
        double sigScale = getSignalScale(sampling_time - toa + i);
        if (verbosity_>2)
            std::cout<<"\tTime "<<sampling_time - toa + i <<" : (scale) "<<sigScale<<" * (charge) "<<charge<< " = "<<sigScale*charge<<" -> Trigger "<<int(sigScale * charge > threshold)<<std::endl;

        aboveThr = (sigScale * charge > threshold);
        if (!lastPulse && aboveThr){
            if (verbosity_>2) std::cout<<"\tPASSED"<<std::endl;
            return true;
        }

        lastPulse = aboveThr;
    }
    return false;
}
double Phase2TrackerBXHistogram::cbc3PulsePolarExpansion(double x) const {
    constexpr size_t max_par = 6;
    if (pulseShapeParameters_.size() < max_par)
        return -1;
    double xOffset = pulseShapeParameters_[0];
    double tau = pulseShapeParameters_[1];
    double r = pulseShapeParameters_[2];
    double theta = pulseShapeParameters_[3];
    int nTerms = static_cast<int>(pulseShapeParameters_[4]);

    double fN = 0;
    double xx = x - xOffset;
    if (xx < 0)
        return 0;

    for (int i = 0; i < nTerms; i++) {
        double angularTerm = 0;
        double temporalTerm = 0;
        double rTerm = std::pow(r, i) / (std::pow(tau, 2. * i) * scaling::nFactorial(i + 2));
        for (int j = 0; j <= i; j++) {
            angularTerm += std::pow(std::cos(theta), (double)(i - j)) * std::pow(std::sin(theta), (double)j);
            temporalTerm += scaling::aScalingConstant(i, j) * std::pow(xx, (double)(i - j)) * std::pow(tau, (double)j);
        }
        double fi = rTerm * angularTerm * temporalTerm;

        fN += fi;
    }
    return fN;
}
double Phase2TrackerBXHistogram::signalShape(double x) const {
    double xOffset = pulseShapeParameters_[0];
    double tau = pulseShapeParameters_[1];
    double maxCharge = pulseShapeParameters_[5];

    double xx = x - xOffset;
    return maxCharge * (std::exp(-xx / tau) * std::pow(xx / tau, 2.) * cbc3PulsePolarExpansion(x));
}
void Phase2TrackerBXHistogram::storeSignalShape() {
    pulseShapeVec_.clear();
    for (size_t i = 0; i < interpolationPoints; i++) {
        float val = i / interpolationStep;

        pulseShapeVec_.push_back(signalShape(val));
    }
}
double Phase2TrackerBXHistogram::getSignalScale(double xval) const {
    double res = 0.0;
    int len = pulseShapeVec_.size();

    if (xval < 0.0 || xval * interpolationStep >= len)
        return res;

    unsigned int lower = std::floor(xval) * interpolationStep;
    unsigned int upper = std::ceil(xval) * interpolationStep;
    for (size_t i = lower + 1; i < upper * interpolationStep; i++) {
        float val = i * 0.1;
        if (val > xval) {
            res = pulseShapeVec_[i - 1];
            break;
        }
    }
    return res;
}

float Phase2TrackerBXHistogram::smearEndcapThresholdDetId(DetId det_id){
    std::map<DetId,float>::iterator smear = smearedThresholdFactors_Endcap_.find(det_id);
    if (smear == smearedThresholdFactors_Endcap_.end()){
        float new_smear = smearedThreshold_Endcap_->fire(); 
        smearedThresholdFactors_Endcap_.insert({det_id,new_smear});
        if (verbosity_ > 2){
            std::cout << "New detid " << det_id.rawId() << " given endcap threshold smearing value of " << new_smear << std::endl;
        }
        return new_smear;
    } else{
        if (verbosity_ > 2){
            std::cout << "Known detid " << det_id.rawId() << " with endcap threshold smearing value of " << smear->second << std::endl;
        }
        return smear->second;
    } 
}
float Phase2TrackerBXHistogram::smearBarrelThresholdDetId(DetId det_id){
    std::map<DetId,float>::iterator smear = smearedThresholdFactors_Barrel_.find(det_id);
    if (smear == smearedThresholdFactors_Barrel_.end()){
        float new_smear = smearedThreshold_Barrel_->fire(); 
        smearedThresholdFactors_Barrel_.insert({det_id,new_smear});
        if (verbosity_ > 2){
            std::cout << "New detid " << det_id.rawId() << " given barrel threshold smearing value of " << new_smear << std::endl;
        }
        return new_smear;
    } else{
        if (verbosity_ > 2){
            std::cout << "Known detid " << det_id.rawId() << " with barrel threshold smearing value of " << smear->second << std::endl;
        }
        return smear->second;
    } 
}

float Phase2TrackerBXHistogram::smearToFDetId(DetId det_id){
    std::map<DetId,float>::iterator smear = smearedTofFactors_.find(det_id);
    if (smear == smearedTofFactors_.end()){
        float new_smear = smearedTOF_->fire(); 
        smearedTofFactors_.insert({det_id,new_smear});
        if (verbosity_ > 2){
            std::cout << "New detid " << det_id.rawId() << " given tof smearing value of " << new_smear << std::endl;
        }
        return new_smear;
    } else{
        if (verbosity_ > 2){
            std::cout << "Known detid " << det_id.rawId() << " with tof smearing value of " << smear->second << std::endl;
        }
        return smear->second;
    } 
}



//define this as a plug-in
DEFINE_FWK_MODULE(Phase2TrackerBXHistogram);
