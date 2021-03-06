# Demo 1D PIC Code

* Compiler: C++ with support for C++17 or above.

* Dependences: None (except for the standard library).

* Compilation: All source codes are under 'pic_1d/'. Open 'Makefile' and change "CXX" appropriate for your compiler (e.g., g++ or clang++). Then issue the 'make' command.

* HPC multi-thread job script example (for SLURM):
```
#!/bin/bash

#SBATCH --job-name="pic_1d"        # Job name
#SBATCH --output=job.%j.out        # Name of stdout output file (%j expands to jobId)
#SBATCH --time=2-00:00:00          # time limit, 2 days, for this job.  
#SBATCH --partition=ibm            # select partition or queue 
#SBATCH --ntasks=1                 # requested number of cpus (precedes --ntasks-per-node)
#SBATCH --cpus-per-task=16         # number of threads per task; so if ntasks=1, then total number of cpus allocated will be 1*16

cd $SLURM_SUBMIT_DIR               # change your working directory where you launch your job.
srun ./pic_1d
```

* Terms of Use: See the LICENSE file.

* Note

Check out

Seiji Zenitani, Tsunehiko N. Kato,
Multiple Boris integrators for particle-in-cell simulation,
Computer Physics Communications,
Volume 247,
2020,
106954,
ISSN 0010-4655,
https://doi.org/10.1016/j.cpc.2019.106954.
