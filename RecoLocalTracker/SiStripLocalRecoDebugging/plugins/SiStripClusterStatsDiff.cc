#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH2.h"
#include "TTree.h"

class SiStripClusterStatsDiff : public edm::stream::EDAnalyzer<>
{
    public:
        SiStripClusterStatsDiff(const edm::ParameterSet& conf);
        void analyze(const edm::Event& evt, const edm::EventSetup& eSetup) override;
    private:
        edm::EDGetTokenT<edmNew::DetSetVector<SiStripCluster>> m_digiAtoken;
        edm::EDGetTokenT<edmNew::DetSetVector<SiStripCluster>> m_digiBtoken;
        TH1F* h_nClusA, * h_nClusB, * h_nClusDiff, * h_nClusRelDiff;
        TH1F *h_clusChargeA, *h_clusChargeB, *h_clusWidthA, *h_clusWidthB, *h_clusBaryA, *h_clusBaryB, *h_clusVarA, *h_clusVarB;
        TH2F* h_nClus2D, *h_clusWidthdVsChargeA, *h_clusWidthdVsChargeB;
        TTree* tree;
        int detId;
        int n_event;
        float chargeA;
        float chargeB;
        float widthA;
        float widthB;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SiStripClusterStatsDiff);

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

SiStripClusterStatsDiff::SiStripClusterStatsDiff(const edm::ParameterSet& conf)
{
    const auto inTagA = conf.getParameter<edm::InputTag>("A");
    m_digiAtoken = consumes<edmNew::DetSetVector<SiStripCluster>>(inTagA);
    const auto inTagB = conf.getParameter<edm::InputTag>("B");
    m_digiBtoken = consumes<edmNew::DetSetVector<SiStripCluster>>(inTagB);
    edm::LogInfo("SiStripClusterStatsDiff") << "Loading clusters from (A) " << inTagA << " and (B) " << inTagB;
    edm::Service<TFileService> fs;
    h_nClusA = fs->make<TH1F>("nClusA", ("nClus per module in collection "+inTagA.encode()+"Clusters+").c_str(), 60, 0., 60.);
    h_nClusB = fs->make<TH1F>("nClusB", ("nClus per module in collection "+inTagB.encode()+"Clusters").c_str(), 60, 0., 60.);
    h_nClus2D = fs->make<TH2F>("nClus2D", ("2D distribution of the number of clusters between the collections;"+inTagA.encode()+";"+inTagB.encode()+";Clusters").c_str(), 60., 0., 60., 60., 0., 60.);
    h_nClusDiff = fs->make<TH1F>("nClusDiff", ("Differences in nClus per module between the collections "+inTagA.encode()+" and "+inTagB.encode()+";Clusters").c_str(), 50., -50., 50.);
    h_nClusRelDiff = fs->make<TH1F>("nClusRelDiff", ("Relative differences in nClus per module between the collections "+inTagA.encode()+" and "+inTagB.encode()+" (B-A);Clusters").c_str(), 100., -.1, .1);
    //
    h_clusChargeA = fs->make<TH1F>("clusChargeA", ("Cluster charge in collection "+inTagA.encode()+";Charge [ADC]").c_str(), 125, 0., 20000.);
    h_clusChargeB = fs->make<TH1F>("clusChargeB", ("Cluster charge in collection "+inTagB.encode()+";Charge [ADC]").c_str(), 125, 0., 20000.);
    //h_clusCharge2D = fs->make<TH2F>("clusCharge2D", ("2D distribution of charge per module between the collections;"+inTagA.encode()+";"+inTagB.encode()+";Charge [ADC]").c_str(), 200, 0., 15000.,200, 0., 15000.);
    h_clusWidthA = fs->make<TH1F>("clusWidthA", ("Cluster width in collection "+inTagA.encode()+";Cluster width [Number of strips]").c_str(), 125, 0., 125.);
    h_clusWidthB = fs->make<TH1F>("clusWidthB", ("Cluster width in collection "+inTagA.encode()+";Cluster width [Number of strips]").c_str(), 125, 0., 125.);

    h_clusWidthdVsChargeA = fs->make<TH2F>("clusWidthVsChargeA", ("Cluster width VS cluster charge per module in collection "+inTagA.encode()+";Cluster charge [ADC]; Cluster width [Number of strips]").c_str(), 125, 0., 20000., 125, 0., 125.);
    h_clusWidthdVsChargeB = fs->make<TH2F>("clusWidthVsChargeB", ("Cluster width VS cluster charge per module in collection "+inTagB.encode()+";Cluster charge [ADC]; Cluster width [Number of strips]").c_str(), 125, 0., 20000., 125, 0., 125.);

    h_clusBaryA = fs->make<TH1F>("clusBaryA", ("Cluster barycenter in collection "+inTagA.encode()+";Strip number").c_str(), 128, 0., 6*128.);
    h_clusBaryB = fs->make<TH1F>("clusBaryB", ("Cluster barycenter in collection "+inTagB.encode()+";Strip number").c_str(), 128, 0., 6*128.);
    //h_clusBar2D = fs->make<TH2F>("clusBar2D", ("2D distribution of cluster barycenter per module between the collections;"+inTagA.encode()+";"+inTagB.encode()+";Strip number").c_str(), 128, 0., 6*128.,128, 0., 6*128.)
    h_clusVarA = fs->make<TH1F>("clusVarA", ("Cluster variance in collection "+inTagA.encode()+";Number of strips").c_str(), 100, 0., 1000.);
    h_clusVarB = fs->make<TH1F>("clusVarB", ("Cluster variance in collection "+inTagB.encode()+";Number of strips").c_str(), 100, 0., 1000.);
    //_clusVar2D = fs->make<TH2F>("clusVar2D", ("2D distribution of cluster variance per module between the collections;"+inTagA.encode()+";"+inTagB.encode()+";Number of strips").c_str(), 100, 0., 1000.,100, 0., 1000.)

    tree = fs->make<TTree>("InvalidDetIdClusters", "InvalidDetIdClusters");
    tree->Branch("detId", &detId, "detId/i");
    tree->Branch("n_event", &n_event, "n_event/i");
    tree->Branch("chargeA", &chargeA, "chargeA/F");
    tree->Branch("chargeB", &chargeB, "chargeB/F");
    tree->Branch("widthA", &widthA, "widthA/F");
    tree->Branch("widthB", &widthB, "widthB/F");

}

void SiStripClusterStatsDiff::analyze(const edm::Event& evt, const edm::EventSetup& eSetup)
{
    edm::Handle<edmNew::DetSetVector<SiStripCluster>> digisA;
    evt.getByToken(m_digiAtoken, digisA);
    edm::Handle<edmNew::DetSetVector<SiStripCluster>> digisB;
    evt.getByToken(m_digiBtoken, digisB);

    for ( const auto& dsetA : *digisA ) {
        const auto i_dsetB = digisB->find(dsetA.id());
        h_nClusA->Fill(dsetA.size());
        if ( digisB->end() != i_dsetB ) { // A and B: compare
            const auto& dsetB = *i_dsetB;
            h_nClusDiff->Fill(dsetB.size()-dsetA.size());
            h_nClus2D->Fill(dsetA.size(),dsetB.size());
            h_nClusRelDiff->Fill(.5*(dsetB.size()-dsetA.size())/(dsetB.size()+dsetA.size()));

            //edm::LogWarning("SiStripClusterStatsDiff") << "Different number of clusters for det " << dsetA.id << ": " << dsetA.size() << " (A) versus " << dsetB.size() << " (B)";
            //if (abs(dsetA.size()-dsetB.size()>20){
            //    edm::LogWarning("SiStripClusterStatsDiff") << "Abnormal difference => detId : "<<dsetA.id<<"/"<<dsetB.id; 
            //}
        } else { // A\B
            h_nClusDiff->Fill(-dsetA.size());
            h_nClusRelDiff->Fill(-1.);
            h_nClus2D->Fill(-1,-1);
        }
        for ( const SiStripCluster& clA : dsetA ) {
            const auto& amps = clA.amplitudes();
            float sumx{0.}, sumxsq{0.};
            auto iStrip = clA.firstStrip();
            for ( auto digi : amps ) {
                sumx += iStrip*digi;
                sumxsq += iStrip*iStrip*digi;
            }
            const auto chg = clA.charge();
            h_clusChargeA->Fill(chg);
            h_clusWidthA ->Fill(amps.size());
            h_clusWidthdVsChargeA ->Fill(chg,amps.size());
            h_clusBaryA  ->Fill(sumx/chg);
            h_clusVarA   ->Fill(std::sqrt(sumxsq*chg-sumx*sumx)/chg);

            if ((chg>=10000 && chg<=12000) || (amps.size()>=35 && amps.size()<=60)){
                detId = dsetA.id();
                n_event = evt.id().event();
                chargeA = chg;
                chargeB = 0;
                widthA = amps.size();
                widthB = 0;
                tree->Fill();
            }
        } // End loop over clusters
    } // End loop over digisA
    for ( const auto& dsetB : *digisB ) {
        h_nClusB->Fill(dsetB.size());
        const auto i_dsetA = digisA->find(dsetB.id());
        if ( digisA->end() == i_dsetA ) { // B\A
            h_nClusDiff->Fill(dsetB.size());
            h_nClusRelDiff->Fill(1.);
        }
        for ( const SiStripCluster& clB : dsetB ) {
            const auto& amps = clB.amplitudes();
            float sumx{0.}, sumxsq{0.};
            auto iStrip = clB.firstStrip();
            for ( auto digi : amps ) {
                sumx += iStrip*digi;
                sumxsq += iStrip*iStrip*digi;
            }
            const auto chg = clB.charge();
            h_clusChargeB->Fill(chg);
            h_clusWidthB ->Fill(amps.size());
            h_clusWidthdVsChargeB ->Fill(chg,amps.size());
            h_clusBaryB  ->Fill(sumx/chg);
            h_clusVarB   ->Fill(std::sqrt(sumxsq*chg-sumx*sumx)/chg);

             if ((chg>=10000 && chg<=12000) || (amps.size()>=35 && amps.size()<=60)){
                detId = dsetB.id();
                n_event = evt.id().event();
                chargeA = 0;
                chargeB = chg;
                widthA = 0;
                widthB = amps.size();
                tree->Fill();
            }

        } // End loop over clusters
    } // End loop over digisA 
}
