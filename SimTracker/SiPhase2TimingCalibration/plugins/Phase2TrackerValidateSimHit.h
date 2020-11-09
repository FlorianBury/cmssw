#ifndef Phase2TrackerValidateSimHit_h
#define Phase2TrackerValidateSimHit_h

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "SimDataFormats/CrossingFrame/interface/CrossingFrame.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DataFormats/DetId/interface/DetId.h"
// DQM Histograming
class MonitorElement;
class PixelDigiSimLink;
class SimTrack;
class SimHit;
class TrackerTopology;
class PixelDigi;
class Phase2TrackerDigi;
class TrackerGeometry;
class TF1;
class TTree;

class Phase2TrackerValidateSimHit : public DQMEDAnalyzer{

public:

  explicit Phase2TrackerValidateSimHit(const edm::ParameterSet&);
  ~Phase2TrackerValidateSimHit() override;
  void bookHistograms(DQMStore::IBooker & ibooker, edm::Run const &  iRun ,
		      edm::EventSetup const &  iSetup ) override;
  void dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
  void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;
  
  bool isPixel(const DetId& detId);

  int bxWindow;
  int amplWindow;

  struct SHitMEs{
    MonitorElement* NSimHit;
    MonitorElement* TOF;
    MonitorElement* CorrTOF;
    MonitorElement* Charge;
    MonitorElement* ChargeVsTOF;
    MonitorElement* TOFVsZPos;
    MonitorElement* BXNumber;
    MonitorElement* TOAVsAmplTime;
    MonitorElement* SimTrackPt;
    int numberOfHits;
  };


private:

  void bookLayerHistos(DQMStore::IBooker & ibooker, unsigned int det_id, const TrackerTopology* tTopo, bool flag); 

  float getSimTrackPt(EncodedEventId event_id, unsigned int tk_id);

  edm::ParameterSet config_;

  MonitorElement* GeometryXYPositionMap;
  MonitorElement* GeometryRZPositionMap;
  MonitorElement* SimulatedXYPositionMap;
  MonitorElement* SimulatedRZPositionMap;
  std::map<unsigned int, SHitMEs> layerMEs;

  bool pixelFlag_;
  std::string geomType_;

  edm::InputTag simTrackSrc_;
  edm::InputTag simVertexSrc_;
  std::vector<edm::InputTag> pSimHitSrc_;
  const edm::InputTag puSummarySrc_;
  edm::InputTag tParticleSrc_;

  const edm::EDGetTokenT<std::vector<TrackingParticle> > tParticleToken_;
  edm::Handle<std::vector<TrackingParticle> > tParticleHandle_;
  std::vector< edm::EDGetTokenT< edm::PSimHitContainer > > simHitTokens_;

  edm::Handle<edm::PSimHitContainer> simHits;
  edm::Handle<edm::SimTrackContainer> simTracks;
  edm::Handle<edm::SimVertexContainer> simVertices;
  edm::ESHandle<TrackerTopology> tTopoHandle_;
  edm::ESHandle<TrackerGeometry> geomHandle_;

  const float GeVperElectron; // 3.7E-09 
  int amplWndow;
  int toaWindow;
  int nEvent;
};
#endif
