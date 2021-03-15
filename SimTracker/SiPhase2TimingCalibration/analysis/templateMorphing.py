import os
import sys
import yaml
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
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib import cm

ROOT.RooMsgService.instance().setGlobalKillBelow(ROOT.RooFit.WARNING)

# Slurm configuration
from CP3SlurmUtils.Configuration import Configuration
from CP3SlurmUtils.SubmitWorker import SubmitWorker
from CP3SlurmUtils.Exceptions import CP3SlurmUtilsException

class TemplateMorphing:
    def __init__(self,path,files,hists,parameters,**kwargs):
        self.path           = path
        self.files          = files
        self.hists          = hists
        self.parameters     = parameters

        self._getRootFiles()

    def _getRootFiles(self):
        for f in self.files.keys():
            if not os.path.exists(os.path.join(self.path,f)):
                print ('Could not find file : {}'.format(f))
        pbar = enlighten.Counter(total=len(self.files.keys()), desc='Progress', unit='files')
        for f,params in self.files.items():
            pbar.update()
            histDict = self._getHistograms(os.path.join(self.path,f))
            for mode, delayDict in histDict.items():
                paramDict = {'mode':mode}
                paramDict.update({self.parameters[i].replace(' ',''):params[i] for i in range(len(params))})
                self._makeMorphing(delayDict,paramDict)

    def _getHistograms(self,f):
        F = ROOT.TFile(f)
        histDict = {}
        for name,values in self.hists.items():
            h = deepcopy(F.Get(values['dir']+"/"+name))
            if values['mode'] not in histDict.keys():
                histDict[values['mode']] = {}
            histDict[values['mode']][values['delay']] = h
        return histDict

    def _makeMorphing(self,delayDict,paramDict):
        #name = "Morphings/Morphing"
        name = "Morphing"
        for k,v in paramDict.items():
            if isinstance(v,float):
                name += ("_%s_%0.1f"%(k,v)).replace('.','p')
            elif isinstance(v,int):
                name += "_%s_%d"%(k,v)
            elif isinstance(v,str):
                name += "_%s_%s"%(k,v)
            else:
                raise RuntimeError("Param value type not understood")
        name += ".root"
        print ("Producing "+name)

        bxidVar = ROOT.RooRealVar("bxidVar","bxidVar",-5.5,5.5)
        delayVar = ROOT.RooRealVar("delayVar","delayVar",min(list(delayDict.keys())),max(list(delayDict.keys())))
        bxidVar.setBins(11)
        delayVar.setBins(len(delayDict))
        paramVec = ROOT.TVectorD(len(delayDict))

        listOfMorphs = ROOT.RooArgList("listOfMorphs")
        listPdfs = []
        listHist = []

        for i,delay in enumerate(sorted(list(delayDict.keys()))):
            h = delayDict[delay]
            paramVec[i] = delay 
            # Delta for convergence #
            h.Scale(1./h.Integral())
            delta = 1e-3
            idx = np.array([h.GetBinContent(i) for i in range(1,h.GetNbinsX()+1)]).argmax()+1
            if idx > 0:
                h.SetBinContent(int(idx-1),h.GetBinContent(int(idx-1))+delta)
            if idx > 1:
                h.SetBinContent(int(idx-2),h.GetBinContent(int(idx-2))+delta)
            if idx < h.GetNbinsX():
                h.SetBinContent(int(idx+1),h.GetBinContent(int(idx+1))+delta)
            if idx < h.GetNbinsX()-1:
                h.SetBinContent(int(idx+2),h.GetBinContent(int(idx+2))+delta)

            hD = ROOT.RooDataHist(h.GetName()+"DataHist",h.GetName()+"DataHist",ROOT.RooArgList(bxidVar),h)
            listHist.append(h)
            hPdf = ROOT.RooHistPdf(h.GetName()+"Pdf",h.GetName()+"Pdf",ROOT.RooArgSet(bxidVar),hD)
            listPdfs.append(deepcopy(hPdf))

        for pdf in listPdfs:
            listOfMorphs.add(pdf)

        morph = ROOT.RooMomentMorph('morph','morph',
                                    delayVar,
                                    ROOT.RooArgList(bxidVar),
                                    listOfMorphs,
                                    paramVec,
                                    ROOT.RooMomentMorph.Linear)
                                    #ROOT.RooMomentMorph.SineLinear)

        h2D = morph.createHistogram("test", 
                                    bxidVar, 
                                    ROOT.RooFit.Binning(11),
                                    ROOT.RooFit.YVar(delayVar))

        w = ROOT.RooWorkspace("MorphWorkspace","MorphWorkspace")
        getattr(w,'import')(morph)

        w.writeToFile(name,True)

        #embed()

        f = ROOT.TFile(name,"UPDATE")
        for k,v in paramDict.items():
            if isinstance(v,str):
                p = ROOT.TNamed(k,v) 
            else:
                p = ROOT.TParameter(type(v))(k,v)
            p.Write()
        h2D.Write("Shape2D")
        f.Close()
            

def submit(yamlFile,name,N_per_job,debug):
    with open(yamlFile,'r') as f:
        config = yaml.load(f,Loader=yaml.FullLoader)

    path_split = os.path.join(os.path.abspath(os.path.dirname(__file__)),'split',name)
    if os.path.exists(path_split):
        for f in glob.glob(os.path.join(path_split,'*')):
            os.remove(f)
    else:
        os.makedirs(path_split)

    subDict = {k:v for k,v in config.items() if k != 'files'}
    subDict['files'] = {}
    N = 0
    counter = 0
    list_files = []
    for f,params in config['files'].items():
        counter += 1
        subDict['files'][f] = params
        if counter == N_per_job:
            filename = os.path.join(path_split,'dict_{}.yml'.format(N))
            list_files.append(filename)
            with open(filename,'w') as handle:
                yaml.dump(subDict,handle)
            print ("Saved {}".format(filename))
            subDict['files'] = {}
            counter = 0
            N += 1

    config = Configuration()
    config.sbatch_partition = 'Def'
    config.sbatch_qos = 'normal'
    config.sbatch_chdir = os.path.dirname(os.path.abspath(__file__))
    config.sbatch_time = '0-02:00:00'
    config.sbatch_memPerCPU= '2000'
    config.sbatch_additionalOptions = ["--export=ALL"]
    config.useJobArray = True
    config.inputParamsNames = []
    config.inputSandboxContent = ['templateMorphing.py']
    config.inputParams = []

    config.payload = "python templateMorphing.py --yaml ${yaml}"

    timestamp = datetime.datetime.now().strftime(name+'_%Y-%m-%d_%H-%M-%S')
    out_dir = os.path.abspath(os.path.dirname(__file__))

    slurm_config = deepcopy(config)
    slurm_working_dir = os.path.join(out_dir,'slurm',timestamp)

    slurm_config.batchScriptsDir = os.path.join(slurm_working_dir, 'scripts')
    slurm_config.inputSandboxDir = slurm_config.batchScriptsDir
    slurm_config.stageoutDir = os.path.join(slurm_working_dir, 'output')
    slurm_config.stageoutLogsDir = os.path.join(slurm_working_dir, 'logs')
    slurm_config.stageoutFiles = ["*.root"]

    slurm_config.inputParamsNames = ['yaml']
    slurm_config.inputParams = [[f] for f in list_files]

    # Submit job!
    print("Submitting job...")
    if not debug:
        submitWorker = SubmitWorker(slurm_config, submit=True, yes=True, debug=False, quiet=False)
        submitWorker()
        print("Done")
    else:
        print('Submitting {} jobs'.format(len(slurm_config.inputParams)))
        print('Param names : '+ ' --'.join([str(p) for p in slurm_config.inputParamsNames]))
        for param in slurm_config.inputParams:
            print ('...',param)
        print('... don\'t worry, jobs not sent')
    


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
        instance = TemplateMorphing(**f)


