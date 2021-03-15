import os
import sys
import re
import yaml
import math
import glob
import ROOT
import argparse
import enlighten
import itertools
import datetime
from pprint import pprint
from copy import copy,deepcopy
from IPython import embed
import numpy as np

ROOT.RooMsgService.instance().setGlobalKillBelow(ROOT.RooFit.WARNING)
ROOT.gROOT.ProcessLine("gErrorIgnoreLevel = 1001;")
ROOT.TH1.AddDirectory(False)

# Slurm configuration
from CP3SlurmUtils.Configuration import Configuration
from CP3SlurmUtils.SubmitWorker import SubmitWorker
from CP3SlurmUtils.Exceptions import CP3SlurmUtilsException

DELTA_COARSE = 5.
DELTA_FINE = 0.1

class MorphingEvaluator:
    def __init__(self,path):
        # Load morphing #
        self._loadMorphing(path)
        
    def _loadMorphing(self,path):
        if not os.path.exists(path):
            raise RuntimeError("Could not find morphing file {}".format(path))
        F = ROOT.TFile(path)
        self.workspace = deepcopy(F.Get("MorphWorkspace"))
        self.delayVar = self.workspace.var("delayVar")
        self.bxidVar = self.workspace.var("bxidVar")
        self.morphing = self.workspace.pdf("morph")

        F.Close()

    def __call__(self,hist,return_graphs=False):
        hist.Scale(1/hist.Integral())

        # Coarse scan #
        print ("Starting coarse scan")
        bxCoarse = np.arange(0.,50.,DELTA_COARSE).round(0)
        chi2Coarse = self._scanBX(hist,bxCoarse)

        # Get two minimums #
        idxTwoMins = np.sort(np.argpartition(chi2Coarse,2)[:3])
        x1 = bxCoarse[idxTwoMins[0]]
        x2 = bxCoarse[idxTwoMins[1]]
        x3 = bxCoarse[idxTwoMins[2]]
        y1 = chi2Coarse[idxTwoMins[0]]
        y2 = chi2Coarse[idxTwoMins[1]]
        y3 = chi2Coarse[idxTwoMins[2]]
        denom = (x1 - x2)*(x1 - x3)*(x2 - x3)
        a = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom
        b = (x3**2 * (y1 - y2) + x2**2 * (y3 - y1) + x1**2 * (y2 - y3)) / denom
        c = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom
        xmin = min(50.,max(0.,-b/(2*a)))
        xmin = 10.

        #print ("Found three points around minimum : [%0.3f,%0.3f,%0.3f] with values [%0.3f,%0.3f,%0.3f]"%(x1,x2,x3,y1,y2,y3))
        #print ("Estimated minimum : %0.3f"%xmin)

        # Fine scan #
        print ("Starting fine scan")
        bxFine = np.arange(max(0.,xmin-DELTA_COARSE/2),min(50.,xmin+DELTA_COARSE/2),DELTA_FINE).round(2)
        chi2Fine = self._scanBX(hist,bxFine)
        
        gCoarse = ROOT.TGraph(bxCoarse.shape[0],bxCoarse,chi2Coarse)
        gFine = ROOT.TGraph(bxFine.shape[0],bxFine,chi2Fine)
            
        gCoarse.SetTitle(";Test delay [ns];#chi^{2} value")
        gFine.SetTitle(";Test delay [ns];#chi^{2} value")

        numerical_min = bxFine[np.argmin(chi2Fine)]
        self._fitChi2Graph(gFine)
        fit_min,sigma = self._fitChi2Graph(gFine)
        print ("Numerical = {}, fit = {} +/- {}".format(numerical_min,fit_min,sigma))

        return 0,0,0,0


        if return_graphs:
            return numerical_min,sigma,gCoarse,gFine
        else:
            return numerical_min,sigma

    def _scanBX(self,recoHist,bxRange):

        import time 

        chi2 = []
        pbar = enlighten.Counter(total=bxRange.shape[0], desc='Progress', unit='BX scan')
        for i in range(bxRange.shape[0]):
            pbar.update()
            time.sleep(0.1)
            self.delayVar.setVal(bxRange[i])
            trueHist = self.morphing.createHistogram("trueHist{:.2f}".format(bxRange[i]),self.bxidVar)
            chi2.append(recoHist.Chi2Test(trueHist,"WW CHI2"))
            trueHist.SetDirectory(0)
            ROOT.SetOwnership(trueHist, True)
            trueHist.Delete()
    
        return np.array(chi2)

    @staticmethod
    def _fitChi2Graph(graph):
        graph.Fit("pol2","SQ")
        fit = graph.GetListOfFunctions().FindObject("pol2")
        a = fit.GetParameter(0)
        b = fit.GetParameter(1)
        c = fit.GetParameter(2)
        x1 = (-b+math.sqrt(2*a))/(2*a)
        x2 = (-b-math.sqrt(2*a))/(2*a)
        print ("Solutions : {} and {}".format(x1,x2))
        sigma = math.sqrt(2/a)/2
        print ("Width = {}".format(sigma*2))
        xmin = -b/(2*a)
        f = lambda x : a*x**2+b*x+c
        print ("f(x2)-f(xmin) = {}".format(f(x2)-f(xmin)))
        print ("f(x1)-f(xmin) = {}".format(f(x1)-f(xmin)))

        return xmin,sigma


def RunEvaluation(morphing_path,hist_path,mode):
    morphingEval = MorphingEvaluator(morphing_path)
    if not os.path.exists(hist_path):
        raise RuntimeError("Could not find histogram root file {}".format(hist_path))
    F_in = ROOT.TFile(hist_path)
    directory = "DQMData/Run 1/Ph2TkBXHist/Run summary/Hist1D"
    F_in.cd(directory)
#    results = []
#    gCoarses = []
#    gFines = []
    for key in ROOT.gDirectory.GetListOfKeys():
        if mode in key.GetName():
            print ('-------------------------------------------')
            true_delay = float(re.findall(r'\d+.\d+|\d+',key.GetName())[0])
            print (key.GetName())
            print ("True delay = {}".format(true_delay))
            h = deepcopy(F_in.Get(directory+'/'+key.GetName()))
            estimated_delay,error,gCoarse,gFine = morphingEval(h,True)
            h.Delete()
            print ("True delay = {}, estimated delay = {} +/- {}".format(true_delay,estimated_delay,error))
#            results.append((true_delay,estimated_delay,error))
#            gCoarse.SetName("CoarseScan_{:.2f}".format(true_delay).replace('.','p'))
#            gFine.SetName("FineScan_{:.2f}".format(true_delay).replace('.','p'))
#            gCoarses.append(gCoarse)
#            gFines.append(gFine)
#    F_in.Close()
#    
#    n     = len(results)
#    x     = np.array([r[0] for r in results])
#    y     = np.array([r[1] for r in results])
#    ex    = np.zeros(n)
#    ey    = np.array([r[2] for r in results])
#    
#    gResult = ROOT.TGraphErrors(n,x,y,ex,ey)
#    gResult.GetHistogram().GetXaxis().SetRangeUser(-5.,55.)
#    gResult.GetHistogram().GetYaxis().SetRangeUser(-5.,55.)
#    gResult.GetXaxis().SetTitle("True delay [ns]")
#    gResult.GetYaxis().SetTitle("Estimated delay [ns]")
#    
#    outName = '{}__{}.root'.format(os.path.basename(morphing_path).replace('.root',''),os.path.basename(hist_path).replace('.root',''))
#    F_out = ROOT.TFile(outName,"RECREATE")
#    gResult.Write()
#    for gCoarse in gCoarses:
#        gCoarse.Write()
#    for gFine in gFines:
#        gFine.Write()
#    F_out.Write()
#    F_out.Close()
#
#    print ("Produced file {}".format(outName))
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Produce datacards')
    parser.add_argument('--yaml', action='store', required=True, type=str, 
                        help='Yaml containing parameters')
    parser.add_argument('--submit', action='store', required=False, type=str, default=None,
                        help='Name for submission')
    parser.add_argument('--split', action='store', required=False, type=int, default=None,
                        help='Number of files per job')
    parser.add_argument('--debug', action='store_true', required=False, default=False,
                        help='Debug for submit jobs')
    args = parser.parse_args()

    
    if args.submit is not None and args.split is not None:
        submit(args.yaml,args.submit,args.split,args.debug)
    else:
        with open(args.yaml,'r') as handle:
            f = yaml.load(handle,Loader=yaml.FullLoader)
        morphing_path_base = f['morphing_path']
        hist_path_base = f['hist_path']
        for evaluation in f['evalList']: 
            for morphing in evaluation['morphings']:
                for hist in evaluation['hists']:
                    RunEvaluation(morphing_path = os.path.join(morphing_path_base,morphing),
                                  hist_path     = os.path.join(hist_path_base,hist),
                                  mode          = evaluation['mode'])


