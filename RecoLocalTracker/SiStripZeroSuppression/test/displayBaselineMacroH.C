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
//#include "iterator"
#include "iterator"



void  displayBaselineMacroH(TString file){
    gStyle->SetOptStat(0);
    gROOT->SetBatch(true);
    /* Select particular detId and event numbers */
    std::vector<pair<int,int>> Selection;  // <event,detid>
    //Selection.insert(std::make_pair(9,369121610)); // TEST, NOT A BAD BASELINE
    //Selection.insert(std::make_pair(9,369120437)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470422441)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470422512)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470341157)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470442980)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470422441)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470422512)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470341157)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470442980)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470442732)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470425956)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470389157)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,470128558)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369174796)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369141274)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369138262)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369154572)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369175140)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369120358)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369121562)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(23,369120637)); // TEST, NOT A BAD BASELINE
    Selection.push_back(std::make_pair(24,436228793)); // BAD BASELINE run 321779
    Selection.push_back(std::make_pair(67,436245750)); // BAD BASELINE run 321779
    Selection.push_back(std::make_pair(71,470050086)); // BAD BASELINE run 321779

    std::vector<bool> check(Selection.size(),false);

    /* Open file and generate canvas */
    TFile *f;//, *fo;
    TString BaseDir;
    TString dir_ZS1[4];
    TString dir_ZS2[4];
    TString fullPath, title, subDet, genSubDet;
    TCanvas *C;
    C = new TCanvas();
    f = new TFile(file);
    //    fo = new TFile(ofile, "RECREATE");
    //BaseDir="DQMData/Results/SpyChannel/";
    dir_ZS1[0]="hybridBaselineAnalyzer/ProcessedRawDigis"; // OK
    dir_ZS1[1]="hybridBaselineAnalyzer/Baseline";
    dir_ZS1[2]="hybridBaselineAnalyzer/Clusters";
    dir_ZS1[3]="hybridBaselineAnalyzer/RawDigis"; // ???
    dir_ZS2[0]="baselineAnalyzerZS2/ProcessedRawDigis";
    dir_ZS2[1]="baselineAnalyzerZS2/Baseline"; // no need
    dir_ZS2[2]="baselineAnalyzerZS2/Clusters";
    dir_ZS2[3]="baselineAnalyzerZS2/RawDigis"; // ???
    f->cd();
    //	fo->Write();
    //	fo->Close();
    f->cd(dir_ZS1[0]); // Go inside ProcessedRawDigis

    TIter nextkey(gDirectory->GetListOfKeys());
    TKey *key;
    int objcounter=1;
    
    /* Start canvas printing */
    C->Print("Baseline.pdf[");

    /* Loop over histograms */
    while ((key = (TKey*)nextkey())) {
            TObject *obj = key->ReadObj();

            if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
                /* Loop over selection */
                bool found = false;
                int n_event = -1;
                int n_run = -1;
                int n_detid = -1;
                int idx = 0;
                for (auto const& it : Selection)
                {
                    /* Get the event number, detid and run info */
                    TString name  = obj->GetName();

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
                    //std::cout<<n_event<<" "<<n_detid<<" "<<n_run<<std::endl;
                    if (it.first==n_event && it.second==n_detid) 
                    {
                        found = true;
                        check[idx] = true;
                        break;
                    }
                    idx++;
                }
                if (!found) continue;
                TString title;
                title.Form("Run %d, Event %d, DetId %d",n_run,n_event,n_detid);
                std::cout<<title<<std::endl;

                //std::cout << "Found object n: " << objcounter << " Name: " << obj->GetName() << " Title: " << obj->GetTitle()<< std::endl;
                //std::cout<<n_event<<" "<<n_detid<<" "<<n_run<<std::endl;

                ++objcounter;

                C->Clear();

                /* ZS1 histograms */
                TH1F* h = (TH1F*)key->ReadObj();

                TLegend leg(0.5,0.7,0.9,0.9);
                leg.SetHeader("Legend","C");
                //leg.AddEntry(h,"VR - Ped - apvCM_{mean}","l");
                leg.AddEntry(h,"Processed raw digis","l");

                h->SetLineWidth(2);
                h->SetLineStyle(2);
                h->SetLineColor(602);
                h->SetTitle(title);
                h->SetXTitle("StripNumber");
                h->SetYTitle("Charge (ADC counts)");
                h->Draw("hist p l");
                f->cd();
                TH1F* hb = (TH1F*) f->Get(dir_ZS1[1]+"/"+obj->GetName());

                if(hb!=0){
                    hb->SetLineWidth(2);
                    hb->SetLineStyle(2);
                    hb->SetLineColor(633);
                    leg.AddEntry(hb,"Baseline with hybrid ZS","l");
                    hb->Draw("hist p l same");
                }

                f->cd();
                TH1F* hc = (TH1F*) f->Get(dir_ZS1[2]+"/"+obj->GetName());

                if(hc!=0){
                    hc->SetLineWidth(2);
                    hc->SetLineStyle(7);
                    hc->SetLineColor(418);
                    leg.AddEntry(hc,"Clusters with hybrid ZS","l");
                    hc->Draw("hist p l same");
                }
                TH1F* hd = (TH1F*) f->Get(dir_ZS1[3]+"/"+obj->GetName());

                if(hd!=0){
                    hd->SetLineWidth(2);
                    hd->SetLineStyle(2);
                    hd->SetLineColor(6);
                    leg.AddEntry(hd,"Raw digis","l");
                    hd->Draw("hist p l same");
                }

                else
                    std::cout << "not found " << obj->GetName()<< std::endl;


                /* ZS2 histograms */
                /* should be the same by construction
                f->cd();
                TH1F* h2 = (TH1F*) f->Get(dir_ZS2[0]+"/"+obj->GetName());
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
                    hb2->SetLineWidth(2);
                    hb2->SetLineStyle(2);
                    hb2->SetLineColor(433);
                    leg.AddEntry(hb2,"Baseline with classic ZS","l");
                    hb2->Draw("hist p l same");
                }

                f->cd();
                TH1F* h2c = (TH1F*) f->Get(dir_ZS2[2]+"/"+obj->GetName());
                if(h2c!=0){
                    h2c->SetLineWidth(2);
                    h2c->SetLineStyle(7);
                    h2c->SetLineColor(412);
                    leg.AddEntry(h2c,"Clusters with classic ZS","l");
                    h2c->Draw("hist p l same");
                }
                /* Legend and save */
                leg.Draw();
                C->Update();
                //	fo->cd();
                //	C->Write();
                C->Print("Baseline.pdf","Title:"+title);
                //C->SaveAs(TString("img/")+obj->GetName()+TString(".png"));


            }
    }
    C->Print("Baseline.pdf");
    C->Print("Baseline.pdf]");
    for (int i=0; i<Selection.size();i++)
    {
        if (!check[i])
            std::cout<<"[WARNING] Event "<<Selection[i].first<<", detId "<<Selection[i].second<<" were not found"<<std::endl;
    }
}
