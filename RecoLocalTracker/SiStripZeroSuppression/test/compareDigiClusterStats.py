#!/usr/bin/env python2
"""
A few quick (ZS) digi and cluster statistics comparison plots
"""
__author__ = "Pieter David <pieter.david@uclouvain.be>"
__date__ = "22 September 2018"

import os.path
import ROOT
from matplotlib import pyplot as plt
import tdrstyle

ROOT.gROOT.SetBatch(True)
tdrstyle.setTDRStyle()

def addOverflows(histo):
    histo.SetBinContent(1, histo.GetBinContent(0)+histo.GetBinContent(1))
    nB = histo.GetNbinsX()
    histo.SetBinContent(nB, histo.GetBinContent(nB)+histo.GetBinContent(nB+1))

if __name__ == "__main__":
    #name = "combinedRuns.root"
    name = "diffhistos321779.root"
    #name = "diffhistos321054.root"
    hFile = ROOT.TFile.Open(name, "READ")
    refSuffix = "B" # Classic ZS
    newSuffix = "A" # Hybrid ZS
    outpath = "comparisonPlots"
    histo1DList = ["digiStatDiff/nDigis", "clusterStatDiff/nClus",
            "clusterStatDiff/clusCharge", "clusterStatDiff/clusWidth", "clusterStatDiff/clusBary", "clusterStatDiff/clusVar"]
    title1DList = ["Number of digis", "Number of clusters",
                "Cluster charge", "Cluster Width", "Cluster barycenter", "Cluster variance"]

    histo2DList = ["clusterStatDiff/clusWidthVsCharge"]
    title2DList = ["Cluster width VS cluster charge"]

    canvas = ROOT.TCanvas('c1','c1',800,700)
    canvas.Print(name.replace('.root','.pdf['))

    # Loop over 1D hist #
    for hName,hTitle in zip(histo1DList,title1DList):
        print (hName)
        # Get histos #
        hName_n = hName.replace("/", "_")
        hRef = hFile.Get(hName+refSuffix)
        hNew = hFile.Get(hName+newSuffix)
        addOverflows(hRef)
        addOverflows(hNew)

        # Make Ratio #
        hRatio = hNew.Clone(hName+"Ratio")
        hRatio.Sumw2()
        hRatio.Divide(hRef)

        # Plot the Ratio #
        pad1 = ROOT.TPad("pad1", "pad1", 0, 0.0, 1, 1.0)
        pad1.SetBottomMargin(0.32)
        pad1.SetGridx()
        pad1.Draw()
        pad1.cd()
        pad1.SetLogy()
        hRef.SetStats(0)
        hRef.SetTitle(hTitle)
        hRef.SetTitleSize(14)
        hRef.Draw()
        hNew.Draw("same")
        legend = ROOT.TLegend(0.65,0.7,0.85,0.83)
        legend.SetHeader("Legend","C")
        legend.AddEntry(hRef,"Classic ZS","l")
        legend.AddEntry(hNew,"Hybrid ZS","l")
        legend.Draw()

        pad2 = ROOT.TPad("pad2", "pad2", 0, 0.0, 1, 0.26)
        pad2.SetTopMargin(0)
        pad2.SetBottomMargin(0.4)
        pad2.SetGridx()
        pad2.Draw()
        pad2.cd() 
        hRatio.Draw()
        hRatio.SetLineColor(ROOT.kBlack)
        hRatio.SetMinimum(0.5)
        hRatio.SetMaximum(1.5)
        hRatio.SetStats(0)                                                                                                                                                                              
        hRatio.SetMarkerStyle(21)
        hRatio.Draw("ep")

        hRef.SetLineColor(ROOT.kBlue+1)
        hRef.SetLineWidth(2)
        max_y = max(hRef.GetMaximum(),hNew.GetMaximum())
        hRef.SetMaximum(max_y*1.1)
        hRef.GetYaxis().SetNdivisions(505)                                                                                                                                                           
        hRef.GetYaxis().SetTitleSize(14)
        hRef.GetYaxis().SetTitleFont(43)
        hRef.GetYaxis().SetTitleOffset(1.8)

        hNew.SetLineColor(ROOT.kRed)
        hNew.SetLineWidth(2)

        hRatio.SetTitle("")

        hRatio.GetYaxis().SetTitle("Ratio")
        hRatio.GetYaxis().SetNdivisions(505)
        hRatio.GetYaxis().SetTitleSize(14)
        hRatio.GetYaxis().SetTitleFont(43)
        hRatio.GetYaxis().SetTitleOffset(1.8)
        hRatio.GetYaxis().SetLabelFont(43)
        hRatio.GetYaxis().SetLabelSize(15)

        hRatio.GetXaxis().SetNdivisions(510)
        hRatio.GetXaxis().SetTitleSize(14)
        hRatio.GetXaxis().SetTitleFont(43)
        hRatio.GetXaxis().SetTitleOffset(4.)
        hRatio.GetXaxis().SetLabelFont(43)
        hRatio.GetXaxis().SetLabelSize(15)
    
        #canvas.Print(name.replace('.root',hName_n+'.pdf'))
        canvas.Print(name.replace('.root','.pdf'),'Title:'+hName_n)
        ##
        hDiff = hFile.Get(hName+"Diff")
        hRelDiff = hFile.Get(hName+"RelDiff")
        h2D = hFile.Get(hName+"2D")
        ##
        canvas.Clear()

        if hDiff:
            canvas.SetLogy()
            canvas.SetRightMargin(0.1)
            canvas.SetLeftMargin(0.1)
            addOverflows(hDiff)
            hDiff.SetTitle(hTitle+' difference')
            hDiff.SetLineColor(ROOT.kBlue)
            hDiff.SetLineWidth(2)
            hDiff.Draw()
            canvas.Print(name.replace('.root','.pdf'),'Title:'+hName_n+'_diff')
            canvas.Clear()
        if hRelDiff: 
            canvas.SetLogy()
            canvas.SetRightMargin(0.1)
            canvas.SetLeftMargin(0.1)
            addOverflows(hRelDiff)
            hRelDiff.SetTitle(hTitle+' relative difference')
            hRelDiff.SetLineColor(ROOT.kBlue)
            hRelDiff.SetLineWidth(2)
            hRelDiff.Draw()
            canvas.Print(name.replace('.root','.pdf'),'Title:'+hName_n+'_reldiff')
            canvas.Clear()
        if h2D: 
            #addOverflows(hRelDiff)
            canvas.SetLogz()
            canvas.SetLogy(False)
            canvas.SetRightMargin(0.2)
            h2D.SetTitle(hTitle+' 2D comparison')
            h2D.GetZaxis().SetTitleOffset(1.6)
            h2D.Draw("colz")
            canvas.Print(name.replace('.root','.pdf'),'Title:'+hName_n+'_2D')
            canvas.Clear()

    # Loop over 2D hist #
    for hName,hTitle in zip(histo2DList,title2DList):
        print (hName)
        # Get histos #
        hName_n = hName.replace("/", "_")
        hRef = hFile.Get(hName+refSuffix)
        hNew = hFile.Get(hName+newSuffix)

        # Draw Ref #
        canvas.SetRightMargin(0.2)
        canvas.SetLogz()
        canvas.SetLogy(False)
        hRef.Draw("colz")
        hRef.GetZaxis().SetTitleOffset(1.6)
        canvas.Print(name.replace('.root','.pdf'),'Title:'+hName_n+'_classic')
        canvas.Clear()
        
        # Draw New #
        canvas.SetRightMargin(0.2)
        canvas.SetLogz()
        canvas.SetLogy(False)
        hNew.Draw("colz")
        hNew.GetZaxis().SetTitleOffset(1.6)
        canvas.Print(name.replace('.root','.pdf'),'Title:'+hName_n+'_hybrid')
        canvas.Clear()

        # Ratio #
        canvas.SetRightMargin(0.2)
        canvas.SetLogz(False)
        canvas.SetLogy(False)
        hRatio = hNew.Clone(hName+"Ratio")
        hRatio.Sumw2()
        hRatio.Divide(hRef)
        hRatio.Draw("colz")
        canvas.Print(name.replace('.root','.pdf'),'Title:'+hName_n+'_ratio')
        canvas.Clear()



    canvas.Print(name.replace('.root','.pdf'))
    canvas.Print(name.replace('.root','.pdf]'))
       
