import os 

#JOB OPTION: 
Memory = 3500
NbCPU = 1 
##################

#FILES OPTION:
EVTMAX=1000
seed=0
FILEPATH = "/sps/juno/llabit/JUNO_J22rc2/workdir/input/"
FILENAME = "Muon_207000.0MeV_5_1seed"#"IBD_NH_0MeV_1000Evts_0to17200R_"
##################
for seed in range(1): 
	JOBPREFIX=FILENAME#+str(int(seed))+"seed"

	os.system("touch "+JOBPREFIX+".sh")
	os.system("echo '#!/bin/sh' >"+JOBPREFIX+".sh")
	os.system("echo '# SLURM options:' >>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --job-name='"+JOBPREFIX+">>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --cpus-per-task' "+str(NbCPU)+" >>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --mem' "+str(Memory)+" >>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --time' "+"10:00:00"+" >>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --partition htc' >>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --output='"+JOBPREFIX+"_CALIB.log >>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --mail-user=<labit@cenbg.in2p3.fr>' >>"+JOBPREFIX+".sh")
	os.system("echo '#SBATCH --mail-type=NONE' >>"+JOBPREFIX+".sh")
	os.system("echo '# Commands to be submitted:' >>"+JOBPREFIX+".sh")

	SIM_FILENAME=FILEPATH+JOBPREFIX+".root"
	ELEC_FILENAME=FILEPATH+JOBPREFIX+"_det2elec.root"
	CALIB_FILENAME=FILEPATH+JOBPREFIX+"_det2elec2calib.root"
#	CALIB_FILENAME=FILEPATH+JOBPREFIX+".root"#+"_det2elec2calib.root"

	PLAIN_FILENAME=FILEPATH+JOBPREFIX+"_plainCalib.root"

	os.system("echo 'python run.py --input "+SIM_FILENAME+" "+ELEC_FILENAME+" "+CALIB_FILENAME+" --output "+PLAIN_FILENAME+"'>>"+JOBPREFIX+".sh")	
	os.system("sbatch "+JOBPREFIX+".sh")
	os.system("sleep .5")
