#include "TH1F.h"
#include "TCanvas.h"
#include "TObject.h"
#include "TFile.h"
#include "TPaveStats.h"
#include "TGraphErrors.h"
#include "TGaxis.h"
#include "TROOT.h"
#include "TF1.h"
#include "TLegend.h"
#include "TKey.h"
#include "TClass.h"
#include "TPRegexp.h"
#include <TROOT.h>
#include <TStyle.h>
#include <TColor.h>

#include "iostream"
#include "vector"
#include "math.h"
#include "map"
#include "iterator"

/* Load TrackerTopology */
#include "/cvmfs/cms.cern.ch/slc7_amd64_gcc700/cms/cmssw/CMSSW_10_6_0/src/CalibTracker/StandaloneTrackerTopology/interface/StandaloneTrackerTopology.h"

//gSystem->Load("libCalibTrackerStandaloneTrackerTopology.so") -> TO be used when loading root

bool sortbysec(const pair<int,int> &a, const pair<int,int> &b) 
{ 
    return (a.second > b.second); 
} 

void  displayBaselineMacro(TString file, int max_plots = -1, int min_occurences = 1, bool print = true){
    gStyle->SetOptStat(0);
    gROOT->SetBatch(true);
    /* Select particular detId and event numbers */
    std::vector<pair<int,int>> Selection;  // <event,detid>
    ////Selection.insert(std::make_pair(9,369121610)); // TEST, NOT A BAD BASELINE
    ////Selection.insert(std::make_pair(9,369120437)); // TEST, NOT A BAD BASELINE
    //Selection.insert(std::make_pair(23,470422441)); // TEST, NOT A BAD BASELINE
    //Selection.insert(std::make_pair(23,470422512)); // TEST, NOT A BAD BASELINE
    //Selection.insert(std::make_pair(23,470341157)); // TEST, NOT A BAD BASELINE
    //Selection.insert(std::make_pair(23,470442980)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470422441)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470422512)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470341157)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470442980)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470442732)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470425956)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470389157)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,470128558)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,369174796)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,369141274)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,369138262)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,369154572)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,369175140)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(68,369120358)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(23,369120637)); // TEST, NOT A BAD BASELINE
    //Selection.push_back(std::make_pair(24,436228793)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(67,436245750)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(71,470050086)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(112,369125510)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(107,369124773)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(650,470422954)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(156,369121390)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(201,369158484)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(206,402666289)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(251,369141845)); // BAD BASELINE run 321779
    //Selection.push_back(std::make_pair(301,369120421)); // BAD BASELINE run 321779

    //std::vector<bool> check(Selection.size(),false);
    std::vector<pair<int,int>> occurences; // pair (detId, number of occurences)

    /* Open file and generate canvas */
    TFile *f;//, *fo;
    TString BaseDir = "/afs/cern.ch/user/f/fbury/work/HybridStudy/";
    TString dir_ZS1[3];
    TString dir_ZS2[3];
    TString fullPath, title, subDet, genSubDet;
    TCanvas *C;
    C = new TCanvas();
    f = new TFile(BaseDir+file);
    dir_ZS1[0]="hybridBaselineAnalyzer/ProcessedRawDigis"; // OK
    dir_ZS1[1]="hybridBaselineAnalyzer/Baseline";
    dir_ZS1[2]="hybridBaselineAnalyzer/Clusters";
    dir_ZS2[0]="baselineAnalyzerZS2/ProcessedRawDigis";
    dir_ZS2[1]="baselineAnalyzerZS2/Baseline"; // no need
    dir_ZS2[2]="baselineAnalyzerZS2/Clusters";

    f->cd();
    //	fo->Write();
    //	fo->Close();
    f->cd(dir_ZS1[0]); // Go inside ProcessedRawDigis

    TIter nextkey(gDirectory->GetListOfKeys());
    TKey *key;
    int objcounter=1;
    
    TString output = file.Replace(file.Length()-5,5,".pdf");
    /* Start canvas printing */
    if (print) C->Print("Baseline_"+output+"[");

    /* Load tracker topo */
    const auto myTopo = StandaloneTrackerTopology::fromTrackerParametersXMLFile("/cvmfs/cms.cern.ch/slc7_amd64_gcc700/cms/cmssw/CMSSW_10_6_2/src/Geometry/TrackerCommonData/data/PhaseI/trackerParameters.xml");

    /* Loop over histograms */
    while ((key = (TKey*)nextkey())) {
            TObject *obj = key->ReadObj();

            /* Get the event number, detid and run info */
            TString name  = obj->GetName();

            int n_event = -1;
            int n_run = -1;
            int n_detid = -1;

            TPRegexp re_event("ev:\\d+");        
            TString event = name(re_event);
            event.Replace(0,3,"");
            n_event = event.Atoi();

            TPRegexp re_run("run:\\d+");        
            TString run = name(re_run);
            run.Replace(0,4,"");
            n_run = run.Atoi();

            TPRegexp re_detid("Id:\\d+");        
            TString detid = name(re_detid);
            detid.Replace(0,3,"");
            n_detid = detid.Atoi();

            TString layer = myTopo.print(n_detid);

            TPRegexp re = TPRegexp("[A-Za-z\\d \\-\\+]+?(?=\\s+Module)");
            layer = layer(re);

            /* Fill occurences vector */
            for(auto & it : occurences){ // Loop over already seen detIds
                if (it.first == n_detid){
                    it.second ++;
                    break;
                }
            }
            occurences.push_back(std::make_pair(n_detid,1)); // If not found, create new entry

            /* If max plotting encountered **/
            if (max_plots != -1 && objcounter>max_plots) continue;
            
            /* TH1 plots */
            if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
                /* Loop over selection */
                bool found = false;
                int idx = 0;
                //for (auto const& it : Selection)
                //{
                                //    //std::cout<<n_event<<" "<<n_detid<<" "<<n_run<<std::endl;
                //    if (it.first==n_event && it.second==n_detid) 
                //    {
                //        found = true;
                //        check[idx] = true;
                //        break;
                //    }
                //    idx++;
                //}
                //if (!found) continue;
                TString title;
                title.Form("Run %d, Event %d, DetId %d",n_run,n_event,n_detid);
                //title = "#splitline{"+title+"}{"+layer+"}";
                title += " ("+layer+")";
                std::cout<<title<<" obj number "<<objcounter<<std::endl;

                //std::cout << "Found object n: " << objcounter << " Name: " << obj->GetName() << " Title: " << obj->GetTitle()<< std::endl;
                //std::cout<<n_event<<" "<<n_detid<<" "<<n_run<<std::endl;

                ++objcounter;

                C->Clear();

                /* ZS1 histograms */
                TH1F* h = (TH1F*)key->ReadObj();

                TLegend leg(0.55,0.7,0.9,0.9);
                leg.SetHeader("Legend","C");
                //leg.AddEntry(h,"VR - Ped - apvCM_{mean}","l");
                leg.AddEntry(h,"Processed raw digis","l");

                h->SetLineWidth(1);
                h->SetLineStyle(1);
                //h->SetLineColorAlpha(603, 0.7);
                h->SetLineColor(1);
                h->SetTitle(title);
                h->SetXTitle("StripNumber");
                h->SetYTitle("Charge (ADC counts)");
                h->Draw("hist p l");
                f->cd();
                TH1F* hb = (TH1F*) f->Get(dir_ZS1[1]+"/"+obj->GetName());

                if(hb!=0){
                    hb->SetLineWidth(3);
                    hb->SetLineStyle(7);
                    hb->SetLineColorAlpha(600, 0.7);
                    //hb->SetLineColor(633);
                    leg.AddEntry(hb,"Baseline with hybrid ZS","l");
                    hb->Draw("hist p l same");
                }

                f->cd();
                TH1F* hc = (TH1F*) f->Get(dir_ZS1[2]+"/"+obj->GetName());

                if(hc!=0){
                    hc->SetLineWidth(3);
                    hc->SetLineStyle(7);
                    //hc->SetLineColor(418);
                    hc->SetLineColorAlpha(419, 0.7);
                    leg.AddEntry(hc,"Clusters with hybrid ZS","l");
                    hc->Draw("hist p l same");
                }
                else
                    std::cout << "not found " << obj->GetName()<< std::endl;


                /* ZS2 histograms */
                /*f->cd();
                TH1F* h2 = (TH1F*) f->Get(dir_ZS2[0]+"/"+obj->GetName()); // same by construction
                if (h2!=0){
                    h2->SetLineWidth(2);
                    h2->SetLineStyle(1);
                    h2->SetLineColor(433);
                    leg.AddEntry(h2,"Processed digis with classic ZS","l");
                    h2->Draw("hist p l same");
                }
                */
                f->cd();
                TH1F* hb2 = (TH1F*) f->Get(dir_ZS2[1]+"/"+obj->GetName());

                if(hb2!=0){
                    hb2->SetLineWidth(3);
                    hb2->SetLineStyle(7);
                    //hb2->SetLineColor(433);
                    hb2->SetLineColorAlpha(633, 0.7);
                    leg.AddEntry(hb2,"Baseline with classic ZS","l");
                    hb2->Draw("hist p l same");
                }

                f->cd();
                TH1F* h2c = (TH1F*) f->Get(dir_ZS2[2]+"/"+obj->GetName());
                if(h2c!=0){
                    h2c->SetLineWidth(3);
                    h2c->SetLineStyle(7);
                    //h2c->SetLineColor(412);
                    h2c->SetLineColorAlpha(400, 0.7);
                    leg.AddEntry(h2c,"Clusters with classic ZS","l");
                    h2c->Draw("hist p l same");
                }
                /* Legend and save */
                leg.Draw();
                C->Update();
                //	fo->cd();
                //	C->Write();
                if (print) C->Print("Baseline_"+output,"Title:"+title);
                //C->SaveAs(TString("img/")+obj->GetName()+TString(".png"));


            }
    }
    if (print) C->Print("Baseline_"+output);
    if (print) C->Print("Baseline_"+output+"]");
    //for (int i=0; i<Selection.size();i++)
    //{
    //    if (!check[i])
    //        std::cout<<"[WARNING] Event "<<Selection[i].first<<", detId "<<Selection[i].second<<" were not found"<<std::endl;
    //}

    /* Sort occurences */
    std::sort(occurences.begin(),occurences.end(),sortbysec); // sort according to number of occurences
    /* Print occurences */
    int total_occurences = 0;
    for(auto & it : occurences){ // Loop over already seen detIds
        if (it.second >= min_occurences)
            std::cout<<"DetId "<<it.first<<" showed up "<<it.second<<" times"<<std::endl;
        total_occurences += it.second;
    }
    std::cout<<"\nThe detId that occured less than "<<min_occurences<<" times were ignored"<<std::endl;
    std::cout<<"\nTotal occurences : "<<total_occurences<<std::endl;
}
