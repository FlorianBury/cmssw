// -*- C++ -*-
//
// Package:    Phase2TrackerValidateSimHit
// Class:      Phase2TrackerValidateSimHit
// 
/**\class Phase2TrackerValidateSimHit Phase2TrackerValidateSimHit.cc 

 Description: Test pixel digis. 

*/
//
// Author: Suchandra Dutta, Suvankar Roy Chowdhury, Subir Sarkar
// Date: January 29, 2016
//
// system include files
#include <memory>
#include "Phase2TrackerValidateSimHit.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESWatcher.h"


#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h" 
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "Geometry/CommonDetUnit/interface/TrackerGeomDet.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetType.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"

#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/CrossingFrame/interface/MixCollection.h"
// DQM Histograming
#include "DQMServices/Core/interface/MonitorElement.h"
#include "TF1.h"
#include "TTree.h"

#include "CLHEP/Units/GlobalSystemOfUnits.h"
#include "CLHEP/Units/GlobalPhysicalConstants.h"

// 
// constructors 
//
Phase2TrackerValidateSimHit::Phase2TrackerValidateSimHit(const edm::ParameterSet& iConfig) :
  config_(iConfig),
  pixelFlag_(config_.getParameter<bool >("PixelPlotFillingFlag")),
  geomType_(config_.getParameter<std::string>("GeometryType")),
  pSimHitSrc_(config_.getParameter<std::vector<edm::InputTag> >("PSimHitSource")),
  tParticleSrc_(config_.getParameter<edm::InputTag>("TrackingTruthSource")),
  tParticleToken_(consumes<std::vector<TrackingParticle> >(tParticleSrc_)),
  GeVperElectron(3.61E-09) // 1 electron(3.61eV, 1keV(277e, mod 9/06 d.k.
{
  for(const auto& itag : pSimHitSrc_) simHitTokens_.push_back(consumes< edm::PSimHitContainer >(itag));

  edm::LogInfo("Phase2TrackerValidateSimHit") << ">>> Construct Phase2TrackerValidateSimHit ";
  std::cout << ">>> Construct Phase2TrackerValidateSimHit " <<std::endl;


  amplWindow = config_.getParameter<edm::ParameterSet>("AmplTimeH").getParameter<int32_t>("Nxbins");
  toaWindow = config_.getParameter<edm::ParameterSet>("TimeOfArrivalH").getParameter<int32_t>("Nxbins");
  nEvent = 0;
}

//
// destructor
//
Phase2TrackerValidateSimHit::~Phase2TrackerValidateSimHit() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  edm::LogInfo("Phase2TrackerValidateSimHit")<< ">>> Destroy Phase2TrackerValidateSimHit ";
  std::cout << ">>> Destroy Phase2TrackerValidateSimHit " << std::endl;
}
//
// -- DQM Begin Run 
//
void Phase2TrackerValidateSimHit::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
   edm::LogInfo("Phase2TrackerValidateSimHit")<< "Initialize Phase2TrackerValidateSimHit ";
   std::cout << "Initialize Phase2TrackerValidateSimHit " <<std::endl;
}
//
// -- Analyze
//
void Phase2TrackerValidateSimHit::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  // Tracker Topology 
  iSetup.get<TrackerTopologyRcd>().get(tTopoHandle_);

  edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;
  
  if (theTkDigiGeomWatcher.check(iSetup)) {
    iSetup.get<TrackerDigiGeometryRecord>().get(geomType_, geomHandle_);
  }  
  if (!geomHandle_.isValid()) return;
 
  edm::ESHandle<GeometricDet> rDD;
  iSetup.get<IdealGeometryRecord>().get(geomType_,rDD);
  for (auto const& det_u : geomHandle_->detUnits()) {
    const DetId detId(det_u->geographicalId().rawId());
    if (detId.det() != DetId::Detector::Tracker) continue;
    bool pass = false;
    if (pixelFlag_) {
      if (isPixel(detId)) pass = true;
    } else {
      if (!isPixel(detId)) pass = true;
    }
    if (!pass) continue;
    const GeomDet *geomDet = geomHandle_.product()->idToDet(detId);
    if (!geomDet) continue;
    GlobalPoint gp = geomDet->surface().toGlobal(det_u->topology().localPosition(MeasurementPoint(0.0, 0.0)));
    GeometryXYPositionMap->Fill(gp.x()*10., gp.y()*10.);   
    GeometryRZPositionMap->Fill(gp.z()*10., std::hypot(gp.x(),gp.y())*10.);   
    //    std::cout << "Detector Id " << det_u->geographicalId().rawId() << " Position (X, Y , Z,  R) "<< gp.x()*10 << ", " 
    //	      << gp.y()*10.<< ",  " << gp.z()*10. << ", " << std::hypot(gp.x(),gp.y())*10. << std::endl;
  }
  iEvent.getByToken(tParticleToken_, tParticleHandle_);
  if (tParticleHandle_.isValid()) {
    /// Loop over TrackingParticles
    std::vector<TrackingParticle>::const_iterator iterTPart;
    /*    std::cout << " ================================================== " << std::endl;
    unsigned int ival = 0;
    for (iterTPart = tParticleHandle_->begin(); iterTPart != tParticleHandle_->end(); ++iterTPart) {
      /// Make the pointer to the TrackingParticle
      std::cout << ival++ << ". BX #" << iterTPart->eventId().bunchCrossing() << " Event # " << iterTPart->eventId().event() << " Track Id " << iterTPart->g4Tracks()[0].trackId() << " PDG Id " << iterTPart->pdgId()  << " Pt " << iterTPart->pt() << std::endl;
    }
    std::cout << " ================================================== " << std::endl;*/
  }
  const TrackerTopology* tTopo = tTopoHandle_.product();
  const TrackerGeometry* tGeom = geomHandle_.product();  

  for (const auto& itoken : simHitTokens_) {
    edm::Handle<edm::PSimHitContainer> simHitHandle;
    iEvent.getByToken(itoken, simHitHandle);
    if (!simHitHandle.isValid()) continue;
    const edm::PSimHitContainer& simHits = (*simHitHandle.product());
    for(edm::PSimHitContainer::const_iterator isim = simHits.begin(); isim != simHits.end(); ++isim){
      int tkid = (*isim).trackId();
      if (tkid <= 0) continue;
      const PSimHit& simHit = (*isim);
      
      unsigned int rawid = simHit.detUnitId();
      const DetId detId(rawid);
      if (detId.det() != DetId::Detector::Tracker) continue;
      
      float dZ = (*isim).entryPoint().z() - (*isim).exitPoint().z();  
      if (fabs(dZ) <= 0.01) continue;
      
      int layer = -1;
      if (pixelFlag_) {
	if (detId.subdetId() == PixelSubdetector::PixelBarrel || 
	    detId.subdetId() == PixelSubdetector::PixelEndcap) layer = tTopo->getITPixelLayerNumber(rawid);
      } else layer = tTopo->getOTLayerNumber(rawid);
      if (rawid < 437000000) std::cout << " rawid " << rawid << " layer " << layer << std::endl;
      if (layer < 0) continue;
      auto pos = layerMEs.find(layer);
      if (pos == layerMEs.end()) continue;
      SHitMEs& local_mes = pos->second;
      const GeomDet *geomDet = tGeom->idToDet(detId);
      if (!geomDet) continue;
      Global3DPoint pdPos = geomDet->surface().toGlobal(isim->localPosition());
      //if (simHit.eventId().bunchCrossing() == -12)  std::cout << " BX # " << simHit.eventId().bunchCrossing() << " Event number " << simHit.eventId().event() << " Raw Id "<< rawid << " SimTrack Id  " << tkid << " Time Of Flight " << simHit.timeOfFlight()  << " Z pos " << pdPos.z() << std::endl;
      std::cout << " BX # " << simHit.eventId().bunchCrossing() << " Event number " << simHit.eventId().event() << " Raw Id "<< rawid << " SimTrack Id  " << tkid << " Time Of Flight " << simHit.timeOfFlight()  << " Z pos " << pdPos.z() << std::endl;

      SimulatedXYPositionMap->Fill(pdPos.x()*10., pdPos.y()*10.);   
      SimulatedRZPositionMap->Fill(pdPos.z()*10., std::hypot(pdPos.x(),pdPos.y())*10.);   

      float time_to_detid_ns = pdPos.mag()/(CLHEP::c_light*CLHEP::ns/CLHEP::cm);
      float tof = (*isim).timeOfFlight();
      float tkpt = getSimTrackPt(simHit.eventId(), tkid);
      float corr_tof = tof - time_to_detid_ns;
      float charge = simHit.energyLoss()/GeVperElectron;

      local_mes.TOF->Fill(tof);
      local_mes.CorrTOF->Fill(corr_tof);
      local_mes.Charge->Fill(charge);
      local_mes.ChargeVsTOF->Fill(tof, charge);
      local_mes.TOFVsZPos->Fill(pdPos.z()*10.0, corr_tof);
      local_mes.BXNumber->Fill(simHit.eventId().bunchCrossing());
      local_mes.SimTrackPt->Fill(tkpt);
      local_mes.numberOfHits++;
      std::cout << "Fill time"<<std::endl
    }
  }

  for (auto& ilayer : layerMEs) {
    SHitMEs& local_mes = ilayer.second;
    local_mes.NSimHit->Fill(local_mes.numberOfHits);
    local_mes.numberOfHits = 0;
  }
  nEvent++;
}
  
//
// -- Book Histograms
//
void Phase2TrackerValidateSimHit::bookHistograms(DQMStore::IBooker & ibooker,
		 edm::Run const &  iRun ,
		 edm::EventSetup const &  iSetup ) {

  std::string top_folder = config_.getParameter<std::string>("TopFolderName");
  std::stringstream folder_name;

  ibooker.cd();
  folder_name << top_folder ;
  ibooker.setCurrentFolder(folder_name.str());
 
  edm::LogInfo("Phase2TrackerValidateSimHit")<< " Booking Histograms in : " << folder_name.str();
  std::cout<< " Booking Histograms in : " << folder_name.str()<<std::endl;;
  std::stringstream HistoName;


  edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;

  iSetup.get<TrackerTopologyRcd>().get(tTopoHandle_);
  const TrackerTopology* const tTopo = tTopoHandle_.product();

  iSetup.get<TrackerDigiGeometryRecord>().get(geomType_, geomHandle_);
  if (theTkDigiGeomWatcher.check(iSetup)) {
    const TrackerGeometry* tGeom = geomHandle_.product();  
    

    edm::ParameterSet XYParameters =  config_.getParameter<edm::ParameterSet>("XYPositionMapH");  
    HistoName.str("");
    HistoName << "GeometryXPosVsYPos";
    GeometryXYPositionMap = ibooker.book2D(HistoName.str(), HistoName.str(),
					    XYParameters.getParameter<int32_t>("Nxbins"),
					    XYParameters.getParameter<double>("xmin"),
					    XYParameters.getParameter<double>("xmax"),
					    XYParameters.getParameter<int32_t>("Nybins"),
					    XYParameters.getParameter<double>("ymin"),
					    XYParameters.getParameter<double>("ymax"));
    edm::ParameterSet RZParameters =  config_.getParameter<edm::ParameterSet>("RZPositionMapH");  
    HistoName.str("");
    HistoName << "GeometryRPosVsZPos";
    GeometryRZPositionMap = ibooker.book2D(HistoName.str(), HistoName.str(),
					    RZParameters.getParameter<int32_t>("Nxbins"),
					    RZParameters.getParameter<double>("xmin"),
					    RZParameters.getParameter<double>("xmax"),
					    RZParameters.getParameter<int32_t>("Nybins"),
					    RZParameters.getParameter<double>("ymin"),
					    RZParameters.getParameter<double>("ymax"));
    HistoName.str("");
    HistoName << "SimulatedXPosVsYPos";
    SimulatedXYPositionMap = ibooker.book2D(HistoName.str(), HistoName.str(),
					    XYParameters.getParameter<int32_t>("Nxbins"),
					    XYParameters.getParameter<double>("xmin"),
					    XYParameters.getParameter<double>("xmax"),
					    XYParameters.getParameter<int32_t>("Nybins"),
					    XYParameters.getParameter<double>("ymin"),
					    XYParameters.getParameter<double>("ymax"));
    HistoName.str("");
    HistoName << "SimulatedRPosVsZPos";
    SimulatedRZPositionMap = ibooker.book2D(HistoName.str(), HistoName.str(),
					    RZParameters.getParameter<int32_t>("Nxbins"),
					    RZParameters.getParameter<double>("xmin"),
					    RZParameters.getParameter<double>("xmax"),
					    RZParameters.getParameter<int32_t>("Nybins"),
					    RZParameters.getParameter<double>("ymin"),
					    RZParameters.getParameter<double>("ymax"));

    for (auto const & det_u : tGeom->detUnits()) {
      unsigned int detId_raw = det_u->geographicalId().rawId();
      bookLayerHistos(ibooker,detId_raw, tTopo,pixelFlag_); 
    }
  }

}
//
// -- Book Layer Histograms
//
void Phase2TrackerValidateSimHit::bookLayerHistos(DQMStore::IBooker & ibooker, unsigned int det_id, const TrackerTopology* tTopo, bool flag){ 

  int layer;
  if (flag) layer = tTopo->getITPixelLayerNumber(det_id);
  else layer = tTopo->getOTLayerNumber(det_id);

  if (layer < 0) return;
  std::map<uint32_t, SHitMEs >::iterator pos = layerMEs.find(layer);
  if (pos == layerMEs.end()) {

    std::string top_folder = config_.getParameter<std::string>("TopFolderName");
    std::stringstream folder_name;

    std::ostringstream fname1, fname2, tag;
    if (layer < 100) { 
      fname1 << "Barrel";
      fname2 << "Layer_" << layer;    
    } else {
      int side = layer/100;
      int idisc = layer - side*100; 
      fname1 << "EndCap_Side_" << side; 
      fname2 << "Disc_" << idisc;       
    }
   
    ibooker.cd();
    folder_name << top_folder << "/"<< fname1.str() << "/" << fname2.str() ;
    edm::LogInfo("Phase2TrackerValidateSimHit")<< " Booking Histograms in : " << folder_name.str();
    std::cout<< " Booking Histograms in : " << folder_name.str()<<std::endl;

    ibooker.setCurrentFolder(folder_name.str());

    std::ostringstream HistoName;    


    SHitMEs local_mes;


    HistoName.str("");
    edm::ParameterSet parNhit      =  config_.getParameter<edm::ParameterSet>("NumerOfHitsH");
    edm::ParameterSet parTOF       =  config_.getParameter<edm::ParameterSet>("TimeOfFlightH");
    edm::ParameterSet parCharge    =  config_.getParameter<edm::ParameterSet>("ChargeH");
    edm::ParameterSet parZPos      =  config_.getParameter<edm::ParameterSet>("ZPosH");
    edm::ParameterSet parBxNum     =  config_.getParameter<edm::ParameterSet>("BXNumberH");
    edm::ParameterSet parTOA     =  config_.getParameter<edm::ParameterSet>("TimeOfArrivalH");
    edm::ParameterSet parAmplTime  =  config_.getParameter<edm::ParameterSet>("AmplTimeH");
    edm::ParameterSet simTkPt =  config_.getParameter<edm::ParameterSet>("SimTrackPtH");

    HistoName.str("");
    HistoName << "NumberOfSimHitsPerDet_"<< fname2.str();
    local_mes.NSimHit = ibooker.book1D(HistoName.str(), HistoName.str(),
					      parNhit.getParameter<int32_t>("Nxbins"),
					      parNhit.getParameter<double>("xmin"),
					      parNhit.getParameter<double>("xmax"));

    HistoName.str("");
    HistoName << "TimeOfFlight_"<< fname2.str();
    local_mes.TOF = ibooker.book1D(HistoName.str(),HistoName.str(),
						       parTOF.getParameter<int32_t>("Nxbins"),
						       parTOF.getParameter<double>("xmin"),
						       parTOF.getParameter<double>("xmax"));
    HistoName.str("");
    HistoName << "CorrectedTimeOfFlight_"<< fname2.str();
    local_mes.CorrTOF = ibooker.book1D(HistoName.str(), HistoName.str(),
						       parTOF.getParameter<int32_t>("Nxbins"),
						       parTOF.getParameter<double>("xmin"),
						       parTOF.getParameter<double>("xmax"));

    HistoName.str("");
    HistoName << "Charge_"<< fname2.str();
    local_mes.Charge = ibooker.book1D(HistoName.str(), HistoName.str(), 
				      parCharge.getParameter<int32_t>("Nxbins"),
				      parCharge.getParameter<double>("xmin"),
				      parCharge.getParameter<double>("xmax"));

    HistoName.str("");
    HistoName << "ChargeVsTimeOfFlight_"<< fname2.str();
    local_mes.ChargeVsTOF = ibooker.book2D(HistoName.str(), HistoName.str(), 
					       parTOF.getParameter<int32_t>("Nxbins"),
					       parTOF.getParameter<double>("xmin"),
					       parTOF.getParameter<double>("xmax"),
					       parCharge.getParameter<int32_t>("Nxbins"),
					       parCharge.getParameter<double>("xmin"),
					       parCharge.getParameter<double>("xmax"));

    HistoName.str("");
    HistoName << "TimeOfFlightVsZpos_"<< fname2.str();
    local_mes.TOFVsZPos = ibooker.book2D(HistoName.str(), HistoName.str(),
						       parZPos.getParameter<int32_t>("Nxbins"),
						       parZPos.getParameter<double>("xmin"),
						       parZPos.getParameter<double>("xmax"),
						       parTOF.getParameter<int32_t>("Nxbins"),
						       parTOF.getParameter<double>("xmin"),
						       parTOF.getParameter<double>("xmax"));

    HistoName.str("");
    HistoName << "BXNumber_"<< fname2.str();
    local_mes.BXNumber = ibooker.book1D(HistoName.str(), HistoName.str(),
						      parBxNum.getParameter<int32_t>("Nxbins"),
						      parBxNum.getParameter<double>("xmin"),
                                  		      parBxNum.getParameter<double>("xmax"));
    HistoName.str("");
    HistoName << "DelayVsTimeOfArrival_"<< fname2.str();
    local_mes.TOAVsAmplTime = ibooker.bookProfile2D(HistoName.str(), HistoName.str(),
						    parTOA.getParameter<int32_t>("Nxbins"),
						    parTOA.getParameter<double>("xmin"),
						    parTOA.getParameter<double>("xmax"),
						    parAmplTime.getParameter<int32_t>("Nxbins"),
						    parAmplTime.getParameter<double>("xmin"),
						    parAmplTime.getParameter<double>("xmax"),0.0,0.0);

    HistoName.str("");
    HistoName << "SimTrackPt_"<< fname2.str();
    local_mes.SimTrackPt = ibooker.book1D(HistoName.str(), HistoName.str(),
					  simTkPt.getParameter<int32_t>("Nxbins"),
				          simTkPt.getParameter<double>("xmin"),
				          simTkPt.getParameter<double>("xmax"));

    local_mes.numberOfHits = 0;
    layerMEs.insert(std::make_pair(layer, local_mes)); 

  } 

}
float Phase2TrackerValidateSimHit::getSimTrackPt(EncodedEventId event_id, unsigned int tk_id) {
  static int ival = 0;
  if (!tParticleHandle_.isValid()) return -1;
  std::vector<TrackingParticle>::const_iterator iterTPart;
  for (iterTPart = tParticleHandle_->begin(); iterTPart != tParticleHandle_->end(); ++iterTPart) {
    /// Make the pointer to the TrackingParticle
    if (iterTPart->eventId() != event_id) continue;
    //    if (ival == 0) std::cout << " Inside Loop  BX #" << iterTPart->eventId().bunchCrossing() << " Event # " << iterTPart->eventId().event() << " Track Id " << iterTPart->g4Tracks()[0].trackId() << " PDG Id " << iterTPart->pdgId()  << " Pt " << iterTPart->pt() << std::endl;
      
    /// Loop over SimTracks inside TrackingParticle
    std::vector<SimTrack>::const_iterator iterSimTrack;
    for (iterSimTrack = iterTPart->g4Tracks().begin(); iterSimTrack != iterTPart->g4Tracks().end(); ++iterSimTrack) {
      if (iterSimTrack->trackId() == tk_id) return iterSimTrack->momentum().Pt();
    }
  }
  ival++;
  return -1;
}
bool Phase2TrackerValidateSimHit::isPixel(const DetId& detId) {
  return (detId.subdetId() == PixelSubdetector::PixelBarrel ||  detId.subdetId() == PixelSubdetector::PixelEndcap);
}
//define this as a plug-in
DEFINE_FWK_MODULE(Phase2TrackerValidateSimHit);
