import os
import sys
import copy
import yaml
import math
import glob
import datetime
import itertools
import argparse
import subprocess
import numpy as np
import ROOT
from pprint import pprint 

default_params = {'N'                  : 100,
                  'threshold'          : 5800, 
                  'thresholdsmearing'  : 0., 
                  'tofsmearing'        : 0., 
                  'mode'               : 'scan',
                  'offset'             : -1.,
               }

class SubmitScans:
    def __init__(self):
        pass

    def setParameterDict(self,params):
        self.params = params
        print ('Parameters for scan :')
        pprint (self.params)
        self.parameterCombinations()

    def parameterCombinations(self):
        self.inputParamsNames = list(self.params.keys())
        self.inputParams = []
        for prod in itertools.product(*self.params.values()):
            inputParam = []
            for p in prod:
                if isinstance(p,float):
                    p = round(p,10) # Truncation of 0.000..00x problems
                inputParam.append(str(p))
            self.inputParams.append(inputParam)
        print ('Generated %d combinations'%len(self.inputParams))

    @staticmethod
    def run_command(command,return_output=False):
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
        outputs = []
        while True:
            out = process.stdout.readline()
            if out == '' and process.poll() is not None:
                break
            if out:
                print (out.strip())
            if return_output:
                outputs.append(out)
        rc = process.poll()
        if return_output:
            return rc,outputs
        else:
            return rc

    @classmethod
    def run(self,args):
        args_dict = {arg.split('=')[0]:arg.split('=')[1] for arg in args}
        for name,defp in default_params.items():
            if name not in args_dict.keys():
                args_dict[name] = str(defp)
                args += ["{}={}".format(name,defp)]
        print ('Starting the DQM file production')
        print ('Arguments : '+' '.join(args))
        dqm_cmd = ['cmsRun','PythiaGunCalibration_cfg.py'] + args
        rc,output = self.run_command(dqm_cmd,True)
        dqm_file = None
        for line in output:
            print (line)
            if 'BXHist' in line and '.root' in line:
                for l in line.split():
                    if '.root' in l:
                        dqm_file = l
                if dqm_file is not None:
                    break
        print ('... exit code : %d'%rc)
        if rc != 0:
            raise RuntimeError("Failed to produce the DQM root file")
        if dqm_file is None or not '.root' in dqm_file:
            raise RuntimeError("Wrong output root file : %s"%dqm_file)
        print ('DQM root file created as %s'%dqm_file)
        if not os.path.exists(dqm_file):
            raise RuntimeError("DQM root file not present")
            
        print ('Starting harvesting')
        harvest_cmd = ['cmsRun','SimTest_Harvest_cfg.py','input=%s'%dqm_file]
        rc = self.run_command(harvest_cmd)
        print ('... exit code : %d'%rc)
        if rc != 0:
            raise RuntimeError("Harvesting failed")

        print ('Starting renaming')
        hist_file = dqm_file.replace('raw','harvested')
        rename_cmd = ['mv','DQM_V0001_R000000001__Global__CMSSW_X_Y_Z__RECO.root',hist_file]
        rc = self.run_command(rename_cmd)
        if rc != 0:
            raise RuntimeError("Could not rename the harvested file")
        print ('... exit code : %d'%rc)

        print ('Starting cleaning')
        clean_cmd = ['rm', dqm_file]
        rc = self.run_command(clean_cmd)
        if rc != 0:
            raise RuntimeError("Could note clean intermediate root file")
        print ('... exit code : %d'%rc)

        F = ROOT.TFile(hist_file,"UPDATE")
        for name,arg in args_dict.items():
            p = ROOT.TNamed(name,arg)
            p.Write()
        F.Close()

    def findMissingJobs(self,path):
        with open(os.path.join(path,'infiles.yml'),'rb') as handle:
            infiles = yaml.safe_load(handle,)
        N = len(infiles)
        print ("Looking over directory",path)

        import enlighten
        rootfiles = list(glob.glob(os.path.join(path,'output','*.root')))
        pbar = enlighten.Counter(total=len(rootfiles), desc='Progress', unit='Files')

        for rootfile in rootfiles:
            pbar.update()
            F = ROOT.TFile(rootfile,"READ")
            ROOT.SetOwnership(F,False)
            params = {}
            for key in F.GetListOfKeys():
                if key.GetClassName() == "TNamed":
                    params[key.GetName()] = F.Get(key.GetName()).GetTitle()
            if params not in infiles:
                print ('Warning : parameters not in infiles',params)
            else:
                infiles.remove(params) 
            F.Close()
            #del F

        print ("All jobs     = {}".format(N))
        print ("Missing jobs = {}".format(len(infiles)))
        if len(infiles) > 0:
            print ("Do not forget to send the jobs with --submit")
            for infile in infiles:
                print (infile)

            self.inputParamsNames = list(infiles[0].keys())
            self.inputParams = [[infile[name] for name in self.inputParamsNames] for infile in infiles]
                    
    def submit_on_slurm(self,name,debug=False):
        # Slurm configuration
        from CP3SlurmUtils.Configuration import Configuration
        from CP3SlurmUtils.SubmitWorker import SubmitWorker
        from CP3SlurmUtils.Exceptions import CP3SlurmUtilsException

        config = Configuration()
        config.sbatch_partition = 'cp3'
        config.sbatch_qos = 'cp3'
        config.sbatch_chdir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),'python')
        config.sbatch_time = '0-02:00:00'
        config.sbatch_memPerCPU= '3000'
        config.environmentType = "cms"
        config.cmsswDir = config.sbatch_chdir.split('src')[0]+'src'
        config.inputSandboxContent = ['*py']
        config.sbatch_additionalOptions = ["--export=None"]
        config.useJobArray = True
        config.inputParamsNames = []
        config.inputParams = []

        base_payload = "python2 submit.py --run " + " ".join([n+"={"+n+"}" for n in self.inputParamsNames])
    
        config.payload = "${taskcmd}"
    
        timestamp = datetime.datetime.now().strftime(name+'_%Y-%m-%d_%H-%M-%S')
        out_dir = os.path.abspath(os.path.dirname(__file__))
    
        slurm_config = copy.deepcopy(config)
        slurm_working_dir = os.path.join(out_dir,'slurm',timestamp)
    
        slurm_config.batchScriptsDir = os.path.join(slurm_working_dir, 'scripts')
        slurm_config.inputSandboxDir = slurm_config.batchScriptsDir
        slurm_config.stageoutDir = os.path.join(slurm_working_dir, 'output')
        slurm_config.stageoutLogsDir = os.path.join(slurm_working_dir, 'logs')
        slurm_config.stageoutFiles = ["*.root"]

        all_params = []
        for params in self.inputParams:
            paramSet = {name:str(params[i]) for i,name in enumerate(self.inputParamsNames)}
            paramSet.update({k:v for k,v in default_params.items() if k not in paramSet.keys()})
            all_params.append(paramSet)

        slurm_config.inputParamsNames = ['taskcmd']
        maxArr = 3000
        Njobs = int(math.ceil(float(len(self.inputParams))/maxArr))
        for job in range(Njobs):
            print ('Submitting batch of jobs %d/%d'%(job,Njobs))
            slurm_config.inputParams = []
            for inputParam in self.inputParams[job*maxArr:(job+1)*maxArr]:
                dParam = {n:p for n,p in zip(self.inputParamsNames,inputParam)}
                taskcmd = base_payload.format(**dParam)
                slurm_config.inputParams.append([taskcmd])
            # Submit job!
            print("Submitting job...")
            if not debug:
                submitWorker = SubmitWorker(slurm_config, submit=True, yes=True, debug=False, quiet=False)
                submitWorker()
                print("Done")
            else:
                print('Submitting {} jobs'.format(len(slurm_config.inputParams)))
                print('Payload : '+slurm_config.payload)
                print('Param names : '+' '.join([str(p) for p in slurm_config.inputParamsNames]))
                print ('Parameters :')
                for inputParam in slurm_config.inputParams:
                    print ("\t",inputParam)
                print('... don\'t worry, jobs not sent')

        # Save params #
        if not debug:
            all_params = []
            for params in self.inputParams:
                paramSet = {name:str(params[i]) for i,name in enumerate(self.inputParamsNames)}
                paramSet.update({k:str(v) for k,v in default_params.items() if k not in paramSet.keys()})
                all_params.append(paramSet)

            with open(os.path.join(slurm_working_dir,'infiles.yml'),'w') as handle:
                yaml.dump(all_params,handle)

        
parser = argparse.ArgumentParser(description='Timing calibration setup job sumission')
parser.add_argument('--submit',action='store',required=False,type=str,default=None,
                    help='Name for job submission')
parser.add_argument('--resubmit',action='store',required=False,type=str,default=None,
                    help='Slurm output dir to check for previous results and only resubmit the missing ones')
parser.add_argument('--run',nargs='*',required=False,type=str,default=None,
                    help='Run parameters')
parser.add_argument('--debug',action='store_true',required=False,default=False,
                    help='Debug for job submission')

args = parser.parse_args()

params = {
           'threshold' : np.linspace(2000,8000,31),
           'thresholdsmearing' : np.linspace(0,1000,11),
           'tofsmearing' : np.linspace(0,5,26),
           'N' : [100],
         }
#params = {
#           'threshold' : [3000,5000,8000],
#           'thresholdsmearing' : [0,100,200,300,400,500],
#           'tofsmearing' : [0.,0.5,1.],
#           'N' : np.arange(100)+1,
#         }
#params = {
#           'threshold' : [5000],
#           'thresholdsmearing' : np.linspace(0,1000,21),
#           'tofsmearing' : np.linspace(0,3,61),
#           'N' : [100],
#           'mode': ['scan'],
#         }
#params = {
#           'threshold' : [5000],
#           'thresholdsmearing' : [0.],
#           'tofsmearing' : [0.],
#           'N' : [100],
#           'mode': ['scan'],
#         }

if args.run is not None:
    SubmitScans.run(args.run)
    sys.exit()
instance = SubmitScans()
if args.resubmit is not None:
    instance.findMissingJobs(args.resubmit)
    if args.submit is not None:
        instance.submit_on_slurm(args.submit,args.debug)
    sys.exit()
instance.setParameterDict(params)
if args.submit is not None:
    instance.submit_on_slurm(args.submit,args.debug)


