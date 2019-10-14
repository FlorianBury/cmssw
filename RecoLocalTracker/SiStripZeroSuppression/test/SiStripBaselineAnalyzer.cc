// -*- C++ -*-
//
// Package:    SiStripBaselineAnalyzer
// Class:      SiStripBaselineAnalyzer
// 
/**\class SiStripBaselineAnalyzer SiStripBaselineAnalyzer.cc Validation/SiStripAnalyzer/src/SiStripBaselineAnalyzer.cc

Description: <one line class summary>

Implementation:
<Notes on implementation>
*/
//
// Original Author:  Ivan Amos Cali
//         Created:  Mon Jul 28 14:10:52 CEST 2008
//
//


// system include files
#include <memory>
#include <iostream>
#include <map>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/DetSet.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"

#include "DataFormats/SiStripDigi/interface/SiStripProcessedRawDigi.h"
#include "DataFormats/SiStripDigi/interface/SiStripRawDigi.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/SiStripCluster/interface/SiStripClusterCollection.h"

#include "CondFormats/SiStripObjects/interface/SiStripPedestals.h"
#include "CondFormats/DataRecord/interface/SiStripPedestalsRcd.h"

#include "RecoLocalTracker/SiStripZeroSuppression/interface/SiStripPedestalsSubtractor.h"
#include "RecoLocalTracker/SiStripZeroSuppression/interface/SiStripCommonModeNoiseSubtractor.h"
#include "RecoLocalTracker/SiStripZeroSuppression/interface/SiStripRawProcessingFactory.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Utils/interface/TFileDirectory.h"

#include "DataFormats/Provenance/interface/RunLumiEventNumber.h"

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/SiStripDetId/interface/SiStripDetId.h"
#include "DataFormats/DetId/interface/DetId.h"


//ROOT inclusion
#include "TROOT.h"
#include "TFile.h"
#include "TNtuple.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TList.h"
#include "TString.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "THStack.h"
#include "TPRegexp.h"


//
// class decleration
//

class SiStripBaselineAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
    public:
        explicit SiStripBaselineAnalyzer(const edm::ParameterSet&);
        ~SiStripBaselineAnalyzer() override;


    private:
        void beginJob() override ;
        void analyze(const edm::Event&, const edm::EventSetup&) override;
        void endJob() override ;


        std::unique_ptr<SiStripPedestalsSubtractor>   subtractorPed_;
        edm::ESHandle<SiStripPedestals> pedestalsHandle;
        std::vector<int> pedestals;
        uint32_t peds_cache_id;

        bool plotClusters_;
        bool plotBaseline_;
        bool plotBaselinePoints_;
        bool plotRawDigi_;
        bool plotDigis_;
        bool plotAPVCM_;
        bool plotPedestals_;
        bool useInvalidDetIds_;

        edm::EDGetTokenT<edm::DetSetVector<SiStripProcessedRawDigi>> srcBaseline_;
        edm::EDGetTokenT<edm::DetSetVector<SiStripDigi>> srcBaselinePoints_;
        edm::EDGetTokenT<edm::DetSetVector<SiStripRawDigi>> srcProcessedRawDigi_;
        edm::EDGetTokenT<edm::DetSetVector<SiStripProcessedRawDigi>> srcAPVCM_;
        edm::EDGetTokenT<edm::DetSetVector<SiStripDigi>> srcDigis_;
        edm::EDGetTokenT<edmNew::DetSetVector<SiStripCluster>> srcClusters_;
        edm::EDGetTokenT<std::vector<uint32_t>> invalidDetIdDigis_;
        edm::EDGetTokenT<std::vector<uint32_t>> invalidDetIdClusters_;

        edm::Service<TFileService> fs_;

        TH1F* h1BadAPVperEvent_;

        TH1F* h1ProcessedRawDigis_;
        TH1F* h1Baseline_;
        TH1F* h1Clusters_;
        TH1F* h1Pedestals_;	  
        std::map<std::string,std::map<uint32_t,TH1F*>> h1APVCM_;
        //std::map<std::string,std::pair<std::vector<uint32_t>,std::vector<TH1F*>>> h1APVCM_;

        TCanvas* Canvas_;
        std::vector<TH1F> vProcessedRawDigiHisto_;
        std::vector<TH1F> vBaselineHisto_;
        std::vector<TH1F> vBaselinePointsHisto_;
        std::vector<TH1F> vClusterHisto_;

        uint16_t nModuletoDisplay_;
        uint16_t actualModule_;
};


SiStripBaselineAnalyzer::SiStripBaselineAnalyzer(const edm::ParameterSet& conf){
    usesResource("TFileService");

    plotClusters_ = conf.getParameter<bool>( "plotClusters" );
    plotBaseline_ = conf.getParameter<bool>( "plotBaseline" );
    plotBaselinePoints_ = conf.getParameter<bool>( "plotBaselinePoints" );
    plotRawDigi_ = conf.getParameter<bool>( "plotRawDigi" );
    plotAPVCM_ = conf.getParameter<bool>( "plotAPVCM" );
    plotPedestals_ = conf.getParameter<bool>( "plotPedestals" );
    plotDigis_ = conf.getParameter<bool>( "plotDigis" );
    useInvalidDetIds_ = conf.getParameter<bool>( "useInvalidDetIds" );

    if (plotBaseline_)
        srcBaseline_ = consumes<edm::DetSetVector<SiStripProcessedRawDigi>>(conf.getParameter<edm::InputTag>( "srcBaseline" ));
    if (plotBaselinePoints_)
        srcBaselinePoints_ = consumes<edm::DetSetVector<SiStripDigi>>(conf.getParameter<edm::InputTag>("srcBaselinePoints"));
    if (plotRawDigi_)
        srcProcessedRawDigi_ = consumes<edm::DetSetVector<SiStripRawDigi>>(conf.getParameter<edm::InputTag>("srcProcessedRawDigi"));
    if (plotAPVCM_)
        srcAPVCM_ = consumes<edm::DetSetVector<SiStripProcessedRawDigi>>(conf.getParameter<edm::InputTag>("srcAPVCM"));
    if (plotDigis_)
        srcDigis_ = consumes<edm::DetSetVector<SiStripDigi>>(conf.getParameter<edm::InputTag>("srcDigis"));
    if (plotClusters_)
        srcClusters_ = consumes<edmNew::DetSetVector<SiStripCluster>>(conf.getParameter<edm::InputTag>("srcClusters"));
    if (useInvalidDetIds_){
        invalidDetIdDigis_ = consumes<std::vector<uint32_t>>(conf.getParameter<edm::InputTag>("invalidDetIdDigis"));
        invalidDetIdClusters_ = consumes<std::vector<uint32_t>>(conf.getParameter<edm::InputTag>("invalidDetIdClusters")); 
    }

    subtractorPed_ = SiStripRawProcessingFactory::create_SubtractorPed(conf.getParameter<edm::ParameterSet>("Algorithms"));
    nModuletoDisplay_ = conf.getParameter<uint32_t>( "nModuletoDisplay" );

    h1BadAPVperEvent_ = fs_->make<TH1F>("BadAPV/Event","BadAPV/Event", 2001, -0.5, 2000.5);
    h1BadAPVperEvent_->SetXTitle("# Modules with Bad APVs");
    h1BadAPVperEvent_->SetYTitle("Entries");
    h1BadAPVperEvent_->SetLineWidth(2);
    h1BadAPVperEvent_->SetLineStyle(2);

    h1APVCM_["TIB"][1] = fs_->make<TH1F>("APV_CM_TIB_layer_1","APV CM TIB layer 1",  2048, -1023.5, 1023.5);
    h1APVCM_["TIB"][2] = fs_->make<TH1F>("APV_CM_TIB_layer_2","APV CM TIB layer 2",  2048, -1023.5, 1023.5);
    h1APVCM_["TIB"][3] = fs_->make<TH1F>("APV_CM_TIB_layer_3","APV CM TIB layer 3",  2048, -1023.5, 1023.5);
    h1APVCM_["TIB"][4] = fs_->make<TH1F>("APV_CM_TIB_layer_4","APV CM TIB layer 4",  2048, -1023.5, 1023.5);

    h1APVCM_["TOB"][1] = fs_->make<TH1F>("APV_CM_TOB_layer_1","APV CM TOB layer 1",  2048, -1023.5, 1023.5);
    h1APVCM_["TOB"][2] = fs_->make<TH1F>("APV_CM_TOB_layer_2","APV CM TOB layer 2",  2048, -1023.5, 1023.5);
    h1APVCM_["TOB"][3] = fs_->make<TH1F>("APV_CM_TOB_layer_3","APV CM TOB layer 3",  2048, -1023.5, 1023.5);
    h1APVCM_["TOB"][4] = fs_->make<TH1F>("APV_CM_TOB_layer_4","APV CM TOB layer 4",  2048, -1023.5, 1023.5);
    h1APVCM_["TOB"][5] = fs_->make<TH1F>("APV_CM_TOB_layer_5","APV CM TOB layer 5",  2048, -1023.5, 1023.5);
    h1APVCM_["TOB"][6] = fs_->make<TH1F>("APV_CM_TOB_layer_6","APV CM TOB layer 6",  2048, -1023.5, 1023.5);

    h1APVCM_["TID"][1] = fs_->make<TH1F>("APV_CM_TID_layer_1","APV CM TID layer 1",  2048, -1023.5, 1023.5);
    h1APVCM_["TID"][2] = fs_->make<TH1F>("APV_CM_TID_layer_2","APV CM TID layer 2",  2048, -1023.5, 1023.5);
    h1APVCM_["TID"][3] = fs_->make<TH1F>("APV_CM_TID_layer_3","APV CM TID layer 3",  2048, -1023.5, 1023.5);

    h1APVCM_["TEC"][1] = fs_->make<TH1F>("APV_CM_TEC_layer_1","APV CM TEC layer 1",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][2] = fs_->make<TH1F>("APV_CM_TEC_layer_2","APV CM TEC layer 2",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][3] = fs_->make<TH1F>("APV_CM_TEC_layer_3","APV CM TEC layer 3",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][4] = fs_->make<TH1F>("APV_CM_TEC_layer_4","APV CM TEC layer 4",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][5] = fs_->make<TH1F>("APV_CM_TEC_layer_5","APV CM TEC layer 5",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][6] = fs_->make<TH1F>("APV_CM_TEC_layer_6","APV CM TEC layer 6",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][7] = fs_->make<TH1F>("APV_CM_TEC_layer_7","APV CM TEC layer 7",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][8] = fs_->make<TH1F>("APV_CM_TEC_layer_8","APV CM TEC layer 8",  2048, -1023.5, 1023.5);
    h1APVCM_["TEC"][9] = fs_->make<TH1F>("APV_CM_TEC_layer_9","APV CM TEC layer 9",  2048, -1023.5, 1023.5);

    for (auto & subDet : h1APVCM_){
        for (auto & layer : subDet.second){
            layer.second->SetXTitle("APV CM [adc]");
            layer.second->SetYTitle("Entries");
            layer.second->SetLineWidth(2);
            layer.second->SetLineStyle(2);
        }
    }

    //h1APVCM_ = fs_->make<TH1F>("APV CM","APV CM", 2048, -1023.5, 1023.5);
    //h1APVCM_->SetXTitle("APV CM [adc]");
    //h1APVCM_->SetYTitle("Entries");
    //h1APVCM_->SetLineWidth(2);
    //h1APVCM_->SetLineStyle(2);

    h1Pedestals_ = fs_->make<TH1F>("Pedestals","Pedestals", 2048, -1023.5, 1023.5);
    h1Pedestals_->SetXTitle("Pedestals [adc]");
    h1Pedestals_->SetYTitle("Entries");
    h1Pedestals_->SetLineWidth(2);
    h1Pedestals_->SetLineStyle(2);



}


SiStripBaselineAnalyzer::~SiStripBaselineAnalyzer()
{

}

    void
SiStripBaselineAnalyzer::analyze(const edm::Event& e, const edm::EventSetup& es)
{
    using namespace edm;
    if(plotPedestals_&&actualModule_ ==0){
        uint32_t p_cache_id = es.get<SiStripPedestalsRcd>().cacheIdentifier();
        if(p_cache_id != peds_cache_id) {
            es.get<SiStripPedestalsRcd>().get(pedestalsHandle);
            peds_cache_id = p_cache_id;
        }


        std::vector<uint32_t> detIdV;
        pedestalsHandle->getDetIds(detIdV);

        for(uint32_t i=0; i < detIdV.size(); ++i){
            pedestals.clear();
            SiStripPedestals::Range pedestalsRange = pedestalsHandle->getRange(detIdV[i]);
            pedestals.resize((pedestalsRange.second- pedestalsRange.first)*8/10);
            pedestalsHandle->allPeds(pedestals, pedestalsRange);
            for(uint32_t it=0; it < pedestals.size(); ++it) h1Pedestals_->Fill(pedestals[it]);
        }
    }

    if(plotAPVCM_){
        edm::Handle<edm::DetSetVector<SiStripProcessedRawDigi>> moduleCM;
        edm::InputTag CMLabel("siStripZeroSuppression:APVCM");
        e.getByToken(srcAPVCM_, moduleCM);

        edm::ESHandle<TrackerTopology> tTopoHandle;
        es.get<TrackerTopologyRcd>().get(tTopoHandle);
        const TrackerTopology* const tTopo = tTopoHandle.product();

        edm::DetSetVector<SiStripProcessedRawDigi>::const_iterator itCMDetSetV =moduleCM->begin();
        for (; itCMDetSetV != moduleCM->end(); ++itCMDetSetV){  
            edm::DetSet<SiStripProcessedRawDigi>::const_iterator  itCM= itCMDetSetV->begin();
            auto id = DetId(itCMDetSetV->detId());
            std::string subDet = tTopo->print(id).substr(0,3);
            uint32_t layer = tTopo->layer(id);
            for(;itCM != itCMDetSetV->end(); ++itCM){
                h1APVCM_[subDet][layer]->Fill(itCM->adc());
            }
        }
    }

    subtractorPed_->init(es);



    edm::Handle<edm::DetSetVector<SiStripProcessedRawDigi>> moduleBaseline;
    if (plotBaseline_)
        e.getByToken(srcBaseline_, moduleBaseline);

    edm::Handle<edm::DetSetVector<SiStripDigi> > moduleBaselinePoints;
    if (plotBaselinePoints_)
        e.getByToken(srcBaselinePoints_, moduleBaselinePoints);

    edm::Handle<edm::DetSetVector<SiStripRawDigi>> moduleRawDigi;
    if (plotRawDigi_)
        e.getByToken(srcProcessedRawDigi_, moduleRawDigi);

    edm::Handle<edm::DetSetVector<SiStripDigi> >moduleDigis;
    if (plotDigis_)
        e.getByToken(srcDigis_, moduleDigis);

    edm::Handle<edmNew::DetSetVector<SiStripCluster> > clusters;
    if (plotClusters_) {
        e.getByToken(srcClusters_, clusters);
    }
    edm::Handle<std::vector<uint32_t>> detIdDigis;
    edm::Handle<std::vector<uint32_t>> detIdClusters;
    if (useInvalidDetIds_){
        e.getByToken(invalidDetIdDigis_, detIdDigis);
        e.getByToken(invalidDetIdClusters_, detIdClusters);
    }

    char detIds[20];
    char evs[20];
    char runs[20];    


    TFileDirectory sdProcessedRawDigis_= fs_->mkdir("ProcessedRawDigis");
    TFileDirectory sdBaseline_= fs_->mkdir("Baseline");
    TFileDirectory sdBaselinePoints_= fs_->mkdir("BaselinePoints");
    TFileDirectory sdClusters_= fs_->mkdir("Clusters");
    TFileDirectory sdDigis_= fs_->mkdir("Digis");

    uint32_t NBabAPVs = moduleRawDigi->size();     
    std::cout<< "Number of module with HIP in this event: " << NBabAPVs << std::endl;
    h1BadAPVperEvent_->Fill(NBabAPVs);

    edm::DetSetVector<SiStripRawDigi>::const_iterator itRawDigis = moduleRawDigi->begin();
    for (; itRawDigis != moduleRawDigi->end(); ++itRawDigis) {

        if (!useInvalidDetIds_){
            if(actualModule_ > nModuletoDisplay_) return;
        }
        uint32_t detId = itRawDigis->id;

        if (useInvalidDetIds_){
            bool is_invalid = false;
            for (const auto& id: *detIdDigis){
                if (detId==id){
                    is_invalid = true;
                    break;
                }
            }
            for (const auto &id : *detIdClusters){
                if (detId==id){
                    is_invalid = true;
                    break;
                }
            }
            if (!is_invalid) continue;
        }   


        actualModule_++;
        edm::RunNumber_t const run = e.id().run();
        edm::EventNumber_t const event = e.id().event();
        //std::cout << "processing module N: " << actualModule_<< " detId: " << detId << " event: "<< event << std::endl; 



        edm::DetSet<SiStripRawDigi>::const_iterator itRaw = itRawDigis->begin(); 
        //bool restAPV[6] = {false,false,false,false,false,false};
        int strip =0, totADC=0;
        int minAPVRes = 7, maxAPVRes = -1;
        for(;itRaw != itRawDigis->end(); ++itRaw, ++strip){
            float adc = itRaw->adc();
            totADC+= adc;
            if(strip%127 ==0){
                //std::cout << "totADC " << totADC << std::endl;
                int APV = strip/128;
                if(totADC!= 0){
                    //restAPV[APV] = true;
                    totADC =0;
                    if(APV>maxAPVRes) maxAPVRes = APV;
                    if(APV<minAPVRes) minAPVRes = APV;
                }
            }
        }

        uint16_t bins =768;
        float minx = -0.5, maxx=767.5;
        //if(minAPVRes !=7){
        //  minx = minAPVRes * 128 -0.5;
        //  maxx = maxAPVRes * 128 + 127.5;
        //  bins = maxx-minx;
        //}

        sprintf(detIds,"%ul", detId);
        sprintf(evs,"%llu", event);
        sprintf(runs,"%u", run);
        char* dHistoName = Form("Id:%s_run:%s_ev:%s",detIds, runs, evs);
        h1ProcessedRawDigis_ = sdProcessedRawDigis_.make<TH1F>(dHistoName,dHistoName, bins, minx, maxx); 

        if(plotBaseline_){
            h1Baseline_ = sdBaseline_.make<TH1F>(dHistoName,dHistoName, bins, minx, maxx); 
            h1Baseline_->SetXTitle("strip#");
            h1Baseline_->SetYTitle("ADC");
            h1Baseline_->SetMaximum(1024.);
            h1Baseline_->SetMinimum(-300.);
            h1Baseline_->SetLineWidth(2);
            h1Baseline_->SetLineStyle(2);
            h1Baseline_->SetLineColor(2);
        }

        if(plotClusters_){
            h1Clusters_ = sdClusters_.make<TH1F>(dHistoName,dHistoName, bins, minx, maxx);

            h1Clusters_->SetXTitle("strip#");
            h1Clusters_->SetYTitle("ADC");
            h1Clusters_->SetMaximum(1024.);
            h1Clusters_->SetMinimum(-300.);
            h1Clusters_->SetLineWidth(2);
            h1Clusters_->SetLineStyle(2);
            h1Clusters_->SetLineColor(3);
        }

        h1ProcessedRawDigis_->SetXTitle("strip#");  
        h1ProcessedRawDigis_->SetYTitle("ADC");
        h1ProcessedRawDigis_->SetMaximum(1024.);
        h1ProcessedRawDigis_->SetMinimum(-300.);
        h1ProcessedRawDigis_->SetLineWidth(2);

        std::vector<int16_t> ProcessedRawDigis(itRawDigis->size());
        subtractorPed_->subtract( *itRawDigis, ProcessedRawDigis);

        edm::DetSet<SiStripProcessedRawDigi>::const_iterator  itBaseline;
        const bool hasBaseline = ( plotBaseline_ && ( moduleBaseline->find(detId) != moduleBaseline->end() )  );
        if (hasBaseline) {
            itBaseline = (*moduleBaseline)[detId].begin();
        } else if ( plotBaseline_ ) {
            std::cout << "Asked to plot baseline, but it's not there :-(" << std::endl;
        }

        std::vector<int16_t>::const_iterator itProcessedRawDigis;
        strip =0;      
        for(itProcessedRawDigis = ProcessedRawDigis.begin();itProcessedRawDigis != ProcessedRawDigis.end(); ++itProcessedRawDigis){
            //if(restAPV[strip/128]){
            float adc = *itProcessedRawDigis;       
            h1ProcessedRawDigis_->Fill(strip, adc);
            if (hasBaseline) {
                h1Baseline_->Fill(strip, itBaseline->adc()); 
                ++itBaseline;
            }
            //}
            ++strip;
        }	  

        if(plotClusters_){
            edmNew::DetSetVector<SiStripCluster>::const_iterator itClusters = clusters->begin();
            for ( ; itClusters != clusters->end(); ++itClusters ){
                for ( edmNew::DetSet<SiStripCluster>::const_iterator clus =	itClusters->begin(); clus != itClusters->end(); ++clus){
                    if(itClusters->id() == detId){
                        int firststrip = clus->firstStrip();
                        //std::cout << "Found cluster in detId " << detId << " " << firststrip << " " << clus->amplitudes().size() << " -----------------------------------------------" << std::endl;		
                        strip=0;
                        for( auto itAmpl = clus->amplitudes().begin(); itAmpl != clus->amplitudes().end(); ++itAmpl){
                            h1Clusters_->Fill(firststrip+strip, *itAmpl);
                            ++strip;
                        }
                    }              
                }
            }
        }
    }		

}


// ------------ method called once each job just before starting event loop  ------------
void SiStripBaselineAnalyzer::beginJob()
{


    actualModule_ =0;  


}

// ------------ method called once each job just after ending the event loop  ------------
void 
SiStripBaselineAnalyzer::endJob() {

}

//define this as a plug-in
DEFINE_FWK_MODULE(SiStripBaselineAnalyzer);

