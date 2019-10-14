#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/SiStripDigi/interface/SiStripDigi.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH2.h"
#include "TTree.h"
#include <iostream>

class SiStripDigiAndClusterStatsDiff : public edm::stream::EDProducer<>
{
    public:
        SiStripDigiAndClusterStatsDiff(const edm::ParameterSet& conf);
        void produce(edm::Event& evt, const edm::EventSetup& eSetup) override;
    private:
        edm::EDGetTokenT<edmNew::DetSetVector<SiStripCluster>> m_clusterAtoken;
        edm::EDGetTokenT<edmNew::DetSetVector<SiStripCluster>> m_clusterBtoken;
        edm::EDGetTokenT<edmNew::DetSetVector<SiStripDigi>> m_digiAtoken;
        edm::EDGetTokenT<edmNew::DetSetVector<SiStripDigi>> m_digiBtoken;

        TH1F *h_nClusRationDigiA, *h_chargeRationDigiA, *h_widthRationDigiA;
        TH1F *h_nClusRationDigiB, *h_chargeRationDigiB, *h_widthRationDigiB;
        TH2F *h_nClusVSnDigiA, *h_chargeVSnDigiA, *h_widthVSnDigiA; 
        TH2F *h_nClusVSnDigiB, *h_chargeVSnDigiB, *h_widthVSnDigiB; 
        TH2F *h_nClusRationDigiComparison, *h_chargeRationDigiComparison, *h_widthRationDigiComparison;
        //TTree* tree;
        int detId;
        int n_event;
        float chargeA;
        float chargeB;
        float widthA;
        float widthB;

        bool detectInvalidDetIds;
        //double invalidMinCharge = 0;
        //double invalidMaxCharge = 1000000;
        //double invalidMinWidth = 0;
        //double invalidMaxWidth = 1000;
        std::vector<uint32_t> invalidDetIdClusters;
        std::vector<uint32_t> excludedDetId;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SiStripDigiAndClusterStatsDiff);

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

SiStripDigiAndClusterStatsDiff::SiStripDigiAndClusterStatsDiff(const edm::ParameterSet& conf):
    detectInvalidDetIds(conf.getParameter<bool>("detectInvalidDetIds")),
    //invalidMinCharge(conf.getParameter<double>("invalidMinCharge")),
    //invalidMaxCharge(conf.getParameter<double>("invalidMaxCharge")),
    //invalidMinWidth(conf.getParameter<double>("invalidMinWidth")),
    //invalidMaxWidth(conf.getParameter<double>("invalidMaxWidth")),
    excludedDetId(conf.getParameter<std::vector<uint32_t>>("excludedDetId"))
{
    const auto inTagA = conf.getParameter<edm::InputTag>("A");
    const auto inTagB = conf.getParameter<edm::InputTag>("B");
    m_digiAtoken = consumes<edmNew::DetSetVector<SiStripDigi>>(inTagA);
    m_digiBtoken = consumes<edmNew::DetSetVector<SiStripDigi>>(inTagB);
    m_clusterAtoken = consumes<edmNew::DetSetVector<SiStripCluster>>(inTagA);
    m_clusterBtoken = consumes<edmNew::DetSetVector<SiStripCluster>>(inTagB);
    edm::LogInfo("SiStripDigiAndClusterStatsDiff") << "Loading digi and clusters from (A) " << inTagA << " and (B) " << inTagB;
    edm::Service<TFileService> fs;

    h_nClusVSnDigiA = fs->make<TH2F>("nClusVSnDigiA", ("Number of cluster versus number of digis per module in collection "+inTagA.encode()+";Clusters;Digis").c_str(), 60, 0., 60., 300, 0., 300.);
    h_nClusVSnDigiB = fs->make<TH2F>("nClusVSnDigiB", ("Number of cluster versus number of digis per module in collection "+inTagB.encode()+";Clusters;Digis").c_str(), 60, 0., 60., 300, 0., 300.);
    h_nClusRationDigiA = fs->make<TH1F>("nClusRationDigiA", ("Ratio nDigi/nClusters per module in collection "+inTagA.encode()+";nDigi/nClusters").c_str(), 100, 0., 100.);
    h_nClusRationDigiB = fs->make<TH1F>("nClusRationDigiB", ("Ratio nDigi/nClusters per module in collection "+inTagB.encode()+";nDigi/nClusters").c_str(), 100, 0., 100.);


    h_chargeVSnDigiA = fs->make<TH2F>("chargeVSnDigiA", ("Cluster charge versus number of digis per module in collection "+inTagA.encode()+"Cluster charge [ADC];Digis").c_str(), 125, 0., 20000., 300, 0., 300.);
    h_chargeVSnDigiB = fs->make<TH2F>("chargeVSnDigiB", ("Cluster charge versus number of digis per module in collection "+inTagB.encode()+"Cluster charge [ADC];Digis").c_str(), 125, 0., 20000., 300, 0., 300.);
    h_chargeRationDigiA = fs->make<TH1F>("chargeRationDigiA", ("Ratio charge/nDigis per module in collection "+inTagA.encode()+";charge/nDigis").c_str(), 100, 0., 1000.);
    h_chargeRationDigiB = fs->make<TH1F>("chargeRationDigiA", ("Ratio charge/nDigis per module in collection "+inTagA.encode()+";charge/nDigis").c_str(), 100, 0., 1000.);

    h_widthVSnDigiA = fs->make<TH2F>("widthVSnDigiA", ("Cluster width versus number of digis per module in collection "+inTagA.encode()+"Cluster width [Number of strips];Digis").c_str(), 120, 0., 120., 300, 0., 300.);
    h_widthVSnDigiB = fs->make<TH2F>("widthVSnDigiB", ("Cluster width versus number of digis per module in collection "+inTagB.encode()+"Cluster width [Number of strips];Digis").c_str(), 120, 0., 120., 300, 0., 300.);
    h_widthRationDigiA = fs->make<TH1F>("widthRationDigiA", ("Ratio width/nDigis per module in collection "+inTagA.encode()+";width/nDigis").c_str(), 100, 0., 10.);
    h_widthRationDigiB = fs->make<TH1F>("widthRationDigiB", ("Ratio width/nDigis per module in collection "+inTagB.encode()+";width/nDigis").c_str(), 100, 0., 10.);

    h_nClusRationDigiComparison = fs->make<TH2F>("h_nClusRationDigiComparison", ("Ratio nDigi/nClusters per module in collection "+inTagB.encode()+" Versus in collection "+inTagB.encode()+";nDigi/nClusters in A;nDigi/nClusters in B").c_str(), 100, 0., 100., 100, 0., 100.);
    h_chargeRationDigiComparison = fs->make<TH2F>("h_chargeRationDigiComparison", ("Ratio charge/nClusters per module in collection "+inTagB.encode()+" Versus in collection "+inTagB.encode()+";charge/nDigis in A;charge/nDigis in B").c_str(), 100, 0., 100., 1000, 0., 1000.);
    h_widthRationDigiComparison = fs->make<TH2F>("h_widthRationDigiComparison", ("Ratio width/nClusters per module in collection "+inTagB.encode()+" Versus in collection "+inTagB.encode()+";width/nDigis in A;width/nDigis in B").c_str(), 100, 0., 10., 100, 0., 10.);

    //tree = fs->make<TTree>("InvalidDetIdClusters", "InvalidDetIdClusters");
    //tree->Branch("detId", &detId, "detId/i");
    //tree->Branch("n_event", &n_event, "n_event/i");
    //tree->Branch("chargeA", &chargeA, "chargeA/F");
    //tree->Branch("chargeB", &chargeB, "chargeB/F");
    //tree->Branch("widthA", &widthA, "widthA/F");
    //tree->Branch("widthB", &widthB, "widthB/F");

    produces<std::vector<uint32_t>>("invalidDetIdClusters"); 
}

void SiStripDigiAndClusterStatsDiff::produce(edm::Event& evt, const edm::EventSetup& eSetup)
{
    invalidDetIdClusters.clear();

    edm::Handle<edmNew::DetSetVector<SiStripDigi>> digisA;
    evt.getByToken(m_digiAtoken, digisA);
    edm::Handle<edmNew::DetSetVector<SiStripDigi>> digisB;
    evt.getByToken(m_digiBtoken, digisB);
    edm::Handle<edmNew::DetSetVector<SiStripCluster>> clustersA;
    evt.getByToken(m_clusterAtoken, clustersA);
    edm::Handle<edmNew::DetSetVector<SiStripCluster>> clustersB;
    evt.getByToken(m_clusterBtoken, clustersB);

    bool is_excluded = false;

    for ( const auto& digiSetA : *digisA ) { // Loop over digis in A
        /* Check exclusion */
        is_excluded = false;
        // exclude potential detIds 
        for (const auto& exclId: excludedDetId){
            if (digiSetA.id() == exclId) is_excluded = true;
        }
        if (is_excluded) continue;

        const auto i_clusterSetA = clustersA->find(digiSetA.id()); // Find clusters in A at same id

        /* Cluster + Digi plots */
        if (clustersA->end() != i_clusterSetA){
            const auto& clusterSetA = *i_clusterSetA;
            h_nClusVSnDigiA->Fill(digiSetA.size(),clusterSetA.size());
            h_nClusRationDigiA->Fill(digiSetA.size()/clusterSetA.size());
            for ( const SiStripCluster& clA : clusterSetA) { // Loop over clusters in A
                const auto& amps = clA.amplitudes(); 
                const auto charge = clA.charge();
                const auto width = amps.size();
                h_chargeVSnDigiA->Fill(digiSetA.size(),charge);
                h_chargeRationDigiA->Fill(charge/digiSetA.size());
                h_widthVSnDigiA->Fill(digiSetA.size(),width);
                h_widthRationDigiA->Fill(width/digiSetA.size());
            }
        }

        /* Check equivalent in detB */
        const auto i_digiSetB = digisB->find(digiSetA.id()); // Find digis in B at same id
        const auto i_clusterSetB = clustersB->find(digiSetA.id()); // Find clusters in B at same id
        if (clustersB->end() != i_clusterSetB && clustersA->end() != i_clusterSetA){
            if (digisB->end != i_digiSetB){
                const auto& clusterSetA = *i_clusterSetA;
                const auto& clusterSetB = *i_clusterSetB;
                const auto& digiSetB = *i_digiSetB;
                h_nClusRationDigiComparison->Fill(digiSetB.size()/clusterSetB.size(),digiSetA.size()/clusterSetA.size());
            }
        }        

    }
    for ( const auto& digiSetB : *digisB ) { // Loop over digis in B
        /* Check exclusion */
        is_excluded = false;
        // exclude potential detIds 
        for (const auto& exclId: excludedDetId){
            if (digiSetB.id() == exclId) is_excluded = true;
        }
        if (is_excluded) continue;

        const auto i_clusterSetB = clustersB->find(digiSetB.id()); // Find clusters in B at same id
        /* Cluster + Digi plots */
        if (clustersB->end() != i_clusterSetB){
            const auto& clusterSetB = *i_clusterSetB;
            h_nClusVSnDigiB->Fill(digiSetB.size(),clusterSetB.size());
            h_nClusRationDigiB->Fill(digiSetB.size()/clusterSetB.size());
            for ( const SiStripCluster& clB : clusterSetB) { // Loop over clusters in B
                const auto& amps = clB.amplitudes(); 
                const auto charge = clB.charge();
                const auto width = amps.size();
                h_chargeVSnDigiB->Fill(digiSetB.size(),charge);
                h_chargeRationDigiB->Fill(charge/digiSetB.size());
                h_widthVSnDigiB->Fill(digiSetB.size(),width);
                h_widthRationDigiB->Fill(width/digiSetB.size());
            }
        }


    }
    evt.put(std::make_unique<std::vector<uint32_t>>(invalidDetIdClusters), "invalidDetIdClusters"); 
}
