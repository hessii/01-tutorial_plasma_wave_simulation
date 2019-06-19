# Demo 1D Hybrid Code

* Compiler: C++ with support for C++17 or above.

* Dependences: None (except for the standard library).

* Compilation: All source codes are under 'hybrid_1d/'. Open 'Makefile' and change "CXX" appropriate for your compiler (e.g., g++ or clang++). Then issue the 'make' command.

* HPC multi-thread job script example (for SLURM):
```
#!/bin/bash

#SBATCH --job-name="hybrid_1d"     # Job name
#SBATCH --output=job.%j.out        # Name of stdout output file (%j expands to jobId)
#SBATCH --time=2-00:00:00          # time limit, 1 hour, for this job.  
#SBATCH --partition=ibm            # select partition or queue 
#SBATCH --ntasks=1                 # requested number of cpus (precedes --ntasks-per-node)
#SBATCH --cpus-per-task=16         # number of threads per task; so if ntasks=1, then total number of cpus allocated will be 1*16

cd $SLURM_SUBMIT_DIR               # change your working directory where you launch your job.
srun ./hybrid_1d
```

* Terms of Use: See the LICENSE file.
