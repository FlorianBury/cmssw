import os
import sys
import yaml
import glob
import ROOT
import argparse
import enlighten
import itertools
from pprint import pprint
from copy import copy,deepcopy
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib import cm

class Data:
    #cache_base = os.path.join(os.path.abspath(os.path.dirname(__file__)),'cache_{}_{}.npy')

    def __init__(self,array,labels):
        self.array = array
        self.labels = labels

    def getSlice(self,**kwargs):
        for key in kwargs.keys():
            if key not in self.labels.keys():
                raise ValueError("Could not find label {} in data labels : [".format(key)+','.join(self.labels.keys())+']')
        arr_select = self.array
        labels_select = {}
        for idx,key in reversed(list(enumerate(self.labels.keys()))):
            if key in kwargs.keys():
                try:
                    element = np.where(self.labels[key]==kwargs[key])[0][0]
                except IndexError:
                    raise IndexError("Cannot find element with value {} of label {}".format(kwargs[key],key))
                arr_select = arr_select.take(element,idx)            
            else:
                labels_select[key] = self.labels[key]
        labels_select = {key:labels_select[key] for key in reversed(list(labels_select.keys()))}
        return Data(arr_select,labels_select)

#    def save(self,name):
#        name = name.replace(' ','_')
#        with open(Data.cache_base.format(name,'array'),'wb') as handle:
#            np.save(handle,self.array)
#        print ('Saved : %s'%Data.cache_base.format(name,'array'))
#        with open(Data.cache_base.format(name,'labels'),'wb') as handle:
#            np.save(handle,self.labels)
#        print ('Saved : %s'%Data.cache_base.format(name,'labels'))
#
#    @classmethod
#    def load(cls,name): 
#        name = name.replace(' ','_')
#        with open(cls.cache_base.format(name,'array'),'rb') as handle:
#            array = np.load(handle)
#        print ('Loaded : %s'%cls.cache_base.format(name,'array'))
#        with open(cls.cache_base.format(name,'labels'),'rb') as handle:
#            labels = np.load(handle,allow_pickle=True)
#        print ('Loaded : %s'%cls.cache_base.format(name,'labels'))
#        return cls(array,labels)
#
#    @classmethod
#    def checkCache(cls,name):
#        name = name.replace(' ','_')
#        return os.path.exists(cls.cache_base.format(name,'array')) and os.path.exists(cls.cache_base.format(name,'labels'))
        

class PlotScan:
    def __init__(self,path,suffix,files,hists,parameters,efficiency,**kwargs):
        self.path           = path
        self.files          = files
        self.suffix         = suffix
        self.hists          = hists
        self.parameters     = parameters
        self.efficiency     = efficiency
        self.content        = []
        #self.cache          = os.path.join(os.path.abspath(os.path.dirname(__file__)),'cache','cache_{}.npy')
        self.cache          = os.path.join(os.path.abspath(os.path.dirname(__file__)),'cache','cache_{}.pkl'.format(self.suffix))

        self._getRootFiles()

    def _getRootFiles(self):
        self.variables = ['Efficiency','Out-of-time fraction','Mean','Next BX contamination','Previous BX contamination']
        self.limits = {'Efficiency'             : (0.,1.),
                       'Out-of-time fraction'   : (0.,1.),
                       'Mean'                   : (-1.,1.),
                       'Next BX contamination'  : (0.,1.),
                       'Previous BX contamination'  : (0.,1.)}

        if not os.path.exists(self.cache):
        #if not all([Data.checkCache(self.suffix+'_'+var) for var in self.variables]):
            print('Not all cache files are present, will process root files')
            for f in self.files.keys():
                if not os.path.exists(os.path.join(self.path,f)):
                    print ('Could not find file : {}'.format(os.path.join(self.path,f)))
            pbar = enlighten.Counter(total=len(self.files.keys()), desc='Progress', unit='files')
            i = 0
            for f,params in self.files.items():
                pbar.update()
                hDict = self._getHistograms(os.path.join(self.path,f))
                effDict,contNextDict,contPrevDict = self._getEfficiency(os.path.join(self.path,f))

                for h,values in hDict.items():
                    if values['type'] == 'TH1':
                        entry = {k:v for k,v in zip(self.parameters,params)}
                        entry.update(values)
                        entry['Efficiency'] = effDict[values['mode']][values['delay']]
                        entry['Next BX contamination'] = contNextDict[values['mode']][values['delay']]
                        entry['Previous BX contamination'] = contPrevDict[values['mode']][values['delay']]
                        fo,fo_err = self._fracOutHisto(h)
                        entry['Out-of-time fraction'] = fo
                        entry['Out-of-time fraction error'] = fo_err
                        m,m_err = self._meanHisto(h)
                        entry['Mean'] = m
                        entry['Mean error'] = m_err
                        
                        self.content.append(entry)
            self.df = pd.DataFrame(self.content)
            self.df.to_pickle(self.cache)
            print ('Saved cache in {}'.format(self.cache))

            #self.varDict = {var:self._makeArray(var) for var in self.variables}
            #for var in self.variables:
            #    self.varDict[var].save(self.suffix+'_'+var)
        else:
            self.df = pd.read_pickle(self.cache)
            print ('Loaded cache from {}'.format(self.cache))
#            print ('Cache files are present, will load them')
#            self.varDict = {}
#            for var in self.variables:
#                self.varDict[var] = Data.load(self.suffix+'_'+var) 

        self.varDict = {var:self._makeArray(var) for var in self.variables}

    def _getHistograms(self,f):
        F = ROOT.TFile(f)
        #ROOT.SetOwnership(F,False)
        histDict = {}
        for name,values in self.hists.items():
            h = deepcopy(F.Get(values['dir']+"/"+name))
            histDict[h] = {k:v for k,v in values.items() if k!='dir'}
        F.Close()
        return histDict

    def _getEfficiency(self,f):
        F = ROOT.TFile(f)
        effDict = {mode:{} for mode in self.efficiency.keys()}
        contNextDict = {mode:{} for mode in self.efficiency.keys()}
        contPrevDict = {mode:{} for mode in self.efficiency.keys()}
        for mode,d in self.efficiency.items():
            trueName = d['dir'] + '/' + d['truth']
            recoName = d['dir'] + '/' + d['reco']
            h_true = F.Get(trueName)
            h_reco = F.Get(recoName)
            h_eff = h_reco.Clone("Eff")
            h_eff.Divide(h_true)
            bin_center  = h_eff.GetXaxis().FindBin(0)
            bin_next    = h_eff.GetXaxis().FindBin(1)
            bin_prev    = h_eff.GetXaxis().FindBin(-1)
            for y in range(1,h_eff.GetNbinsY()+1):
                delay = round(h_eff.GetYaxis().GetBinCenter(y),1)
                effDict[mode][delay] = h_eff.GetBinContent(bin_center,y)
                contNextDict[mode][delay] = h_eff.GetBinContent(bin_next,y)
                contPrevDict[mode][delay] = h_eff.GetBinContent(bin_prev,y)

        F.Close()
        return effDict,contNextDict,contPrevDict
        

    @staticmethod
    def _getHistContent(h):
        e = [h.GetXaxis().GetBinUpEdge(0)]
        w = []
        s = []
        for i in range(1,h.GetNbinsX()+1):
            e.append(h.GetXaxis().GetBinUpEdge(i))
            w.append(h.GetBinContent(i))
            s.append(h.GetBinError(i))
        return np.array(e),np.array(w),np.array(s)
 
    def _fracOutHisto(self,h): 
        e,w,s = self._getHistContent(h)
        c = (e[:-1]+e[1:])/2
        idx = np.delete(np.arange(w.shape[0]),np.abs(c).argmin())
        return w[idx].sum()/w.sum(),np.sqrt((s[idx]**2).sum())/w.sum()

    def _meanHisto(self,h): 
        e,w,s = self._getHistContent(h)
        c = (e[:-1]+e[1:])/2
        return (w*c).sum()/w.sum(),np.sqrt(((s*c)**2).sum())/w.sum()
    
    def _makeArray(self,var):
        axNames = self.parameters + ['delay']
        df = self.df[axNames+[var]]
        grouped = df.groupby(axNames)[var].mean()
        shape = tuple(map(len, grouped.index.levels))
        arr = np.full(shape, np.nan)
        arr[tuple(grouped.index.codes)] = grouped.values.flat
        labels = {level.name:level.values for level in grouped.index.levels}
        print ('Produced array for variable {}'.format(var))
        return Data(arr,labels)

    def Plots(self):
        for var in self.variables:
            data = self.varDict[var]
            for param in self.parameters:
                print ("Plotting {} curves of {} parameter".format(var,param))
                with PdfPages('pdf/curves_{}_{}_{}.pdf'.format(self.suffix,var,param).replace(' ','')) as pdf:
                    labels_to_vary = {key:data.labels[key] for key in data.labels.keys() if key != param and key != 'delay'}

                    for comb in itertools.product(*list(labels_to_vary.values())):
                        varied_labels = {k:c for k,c in zip(labels_to_vary.keys(),comb)}
                        curves = data.getSlice(**varied_labels)

                        # Plot 1D #
                        fig = plt.figure(figsize=(8,7))
                        plt.subplots_adjust(left=0.15, right=0.90, top=0.85, bottom=0.12)
                        colors = cm.jet(np.linspace(0,1,curves.labels[param].shape[0]))
                        for idx in range(curves.labels[param].shape[0]):
                            plt.plot(curves.labels['delay'],curves.array[idx,:],color=colors[idx])
                        sm = cm.ScalarMappable(cmap=cm.rainbow, norm=plt.Normalize(vmin=curves.labels[param].min(), vmax=curves.labels[param].max()))
                        cbar = plt.colorbar(sm)
                        cbar.set_label(param,fontsize=18,labelpad=20)
                        cbar.ax.tick_params(labelsize=14)
                        plt.xlabel('Delay [ns]',fontsize=18,labelpad=20)
                        plt.ylabel(var,fontsize=18,labelpad=10)
                        plt.ylim(self.limits[var])
                        title = '{} curves\n('.format(param)+','.join(['{} = {}'.format(p,v) for p,v in varied_labels.items()])+')'
                        plt.title(title,fontsize=20,pad=25)
                        plt.tick_params(axis='both', which='major', labelsize=14)
                        print ('\t'+title)
                        pdf.savefig()
                        plt.close()

                        # Plot 2D #
                        fig,ax = plt.subplots(figsize=(8,7))
                        plt.subplots_adjust(left=0.15, right=0.95, top=0.85, bottom=0.1)
                        lambda_edges = lambda x : np.concatenate(((x[:-1] - np.diff(x)/2)[:2],x[1:] + np.diff(x)/2))
                        X,Y = np.meshgrid(lambda_edges(curves.labels['delay']),lambda_edges(curves.labels[param]))
                        #levels=20
                        #colors = cm.viridis(np.linspace(0.,1.,levels))
                        #cont = plt.contourf(labels[-1],labels[ip],curves,levels=levels,colors=colors,vmin=self.limits[var][0],vmax=self.limits[var][1])
                        #mesh = ax.pcolormesh(X,Y,curves.array,vmin=self.limits[var][0],vmax=self.limits[var][1],shading='auto',linewidth=0,rasterized=True)
                        mesh = ax.pcolormesh(X,Y,curves.array,vmin=self.limits[var][0],vmax=self.limits[var][1],shading="auto",linewidth=0)
                        cbar = fig.colorbar(mesh)
                        cbar.set_label(var,fontsize=18,labelpad=20)
                        cbar.ax.tick_params(labelsize=14)
                        ax.set_xlabel('Delay [ns]',fontsize=18,labelpad=10)
                        ax.set_ylabel(param,fontsize=18,labelpad=20)
                        ax.set_title(title,fontsize=20,pad=25)
                        ax.tick_params(axis='both', which='major', labelsize=14)
                        pdf.savefig()
                        plt.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Produce datacards')
    parser.add_argument('--yaml', action='store', required=True, type=str,
                        help='Yaml containing parameters')
    args = parser.parse_args()


    if args.yaml is None:
        raise RuntimeError("Must provide the YAML file")
    with open(args.yaml,'r') as handle:
        f = yaml.load(handle,Loader=yaml.FullLoader)

    instance = PlotScan(**f)
    instance.Plots()


