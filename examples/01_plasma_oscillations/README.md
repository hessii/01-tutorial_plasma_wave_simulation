# Overview

This is a simulation demoing plasma oscillations (or Langmuir oscillations) for a plasma 
with cold electrons and immobile ions.
The goal of this exercise is to

1) Familiarize oneself with the compile-run process;
2) Read in and analyze the simulation data products; and
3) Confirm the linear theory predictions regarding plasma oscillations.


# Compilation and Run

In essence, executing `CXX='clang++' make -j4 all` in the working directory will
compile the source codes and produce an executable called `pic_1d`.
The `-jx` flag instructs `make` to compile up to `x` number of source codes in parallel.
The optimal value for `x` will be the number of cpu cores.
The `CXX` flag instructs `make` what c++ compiler it should use.
The c++ compiler for the compilation must support the c++17 language standard or above.
The default value for the `CXX` flag is `clang++` for the macOS operating system.
GCC users, for example, will need to use `CXX=g++`.
Instead of passing it as the command line argument,
one can specify the c++ compiler flag `CXX` in `Makefile`.
In fact, `Makefile` contains other flags for further customization.

Executing `make clean` will clean up the build products
(but not the data files produced by the simulation).

To run the simulation, execute `./pic_1d`.
Several default parameters of the simulation can be reconfigured through command line arguments.
Supported are:

* `-save` instructs to take a snapshot from which the simulation can resume at a later time.
* `-load` instructs to restore the state from the latest snapshot and resume the simulation from the last point.
* `--wd="working_directory"` instructs to use `"working directory"` as the working directory.
(The default is `working_directory` in the `Inputs.h`.)
* `--outer_Nt="a non-negative integer"` instructs to run the simulation by `inner_Nt x outer_Nt` cycles.
(The default is `outer_Nt` in the `Inputs.h`.)

For this demo though, no argument passing is required.


# Simulation Setup

The `Inputs.h` file describes all the physical parameters needed as well as 
those necessary to produce data products.
(Some of them can be overwritten using the command line arguments.)

The one-dimensional simulation is normalized such that
the speed of light is `c = 1` and the electron plasma frequency is `ω_pe = 1`.
Hence, the electron inertial length is `c/ω_pe = 1`.
What that means is that
all the quantities with the length dimension are normalized to the inertial length,
all the quantities with the time dimension are normalized to the inverse of the electron plasma frequency, and
all the quantities with the speed dimension are normalized to the speed of light.
Electrons are assumed to be cold fluid, and charge-neutralizing ions are immobile (infinite mass).

The grid size of the one-dimensional simulation domain (in the `x` direction) is `∆x = 0.200781` and
the integration time step is `∆t = 0.04`.
The number of grid points are `Nx = 480` and the periodic boundary conditions are used at both ends.

To stir up the system at the beginning, which is otherwise quiet,
the electric field is initialized with small-amplitude random fluctuations.

The simulation results will be dumped into (by default) `./data` with data recording frequencies
specified by the `*_recording_frequency` options in the `Inputs.h` file.
An output directory other than the default one, say `my_sim` at the home directory,
can be reconfigured in the command line as follows:

> `./pic_1d --wd=~/my_sim`.


# Data Products

All data products (except for the snapshots) are formatted as comma-separated-values, or `csv`.
There are four types of data products that the code is configured to produce:

* `energy.csv` contains grid-point averaged field and plasma species energy density.

    The first field, `step`, is the simulation step count.
    The second field, `time`, is the simulation time, equivalent to `step` x `∆t`.
    The `dB[123]^2/2` fields are the energy densities of the fluctuating magnetic field components
    parallel (`1`) and perpendicular (`2` and `3`) to the background magnetic field.
    Likewise, The `dE[123]^2/2` fields are the energy densities for the electric field.
    The subsequent fields, appearing in a group of six, describe the energy densities of, if present,
    each plasma species.
    Since there is only one plasma species for this demo (i.e., cold electrons), there is only one group.
    In each group, the `mv[123]^2/2` fields are the kinetic energy densities, and
    the `mU[123]^2/2` fields are the bulk flow energy densities (that is, kinetic energy - thermal energy).
    The suffixes `123` denote the directions relative to the background magnetic field.

* `field-*.csv` contains the magnetic and electric fields at every grid points.

    Each file contains the field components at an instance of time which is encoded in the first line
    of its contents.
    The first line is self-explanatory:
    `step` and `time` are the same as those appeared in `energy.csv`, and
    `Dx` and `Nx` are the grid size, `∆x`, and the number of grid points, `Nx`, respectively.

    The rest of the data are the magnetic (`dB[123]`) and electric (`dB[123]`) data.

* `moment-*.csv` contains velocity moments of the plasma species.

    Similar to the field data, the first line encodes information about the current simulation time,
    the simulation grid, and additionally the number of plasma species (for this demo, `1`).

    The rest of the data are the plasma moments.
    For each plasma species, there are seven columns:

    - `<1>` is the zeroth-order velocity moment;
    - `<v[123]>` are the three components of the first-order velocity moment
    (again, the suffixes denoting directions relative to the background magnetic field); and
    - `<v[123]v[123]>` are the diagonal components of the second-order velocity moment,
    
    where `<...>` denotes ∫ (...) *f*(**v**) d^3*v*.


* Finally, `particle-*.csv` contains a subset of simulation particles for the particle species.
This is not used for this demo.


# Analysis of Data
