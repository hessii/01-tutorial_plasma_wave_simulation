# Description

## Overview

This is a simulation demoing plasma oscillations (or Langmuir oscillations) for a plasma with cold electrons and immobile ions.
The `Inputs.h` file describes all the physical parameters needed as well as those necessary to produce data products.

## Compilation and Run

In essence, executing `CXX='clang++' make -j4 all` in the working directory will
compile the source codes and produce an executable called `pic_1d`.
Of course, GCC users, for example, will need to change the `CXX` environment variable to `CXX=g++`.
The `-jx` flag instructs `make` to compile up to `x` number of source codes in parallel.
The c++ compiler in use should support the c++17 language standard.

Executing `make clean` will clean up the object files and the build product,
but not the data files produced during simulations.

To run the simulation, execute `./pic_1d`.


## Simulation Setup

The one-dimensional system is normalized such that
the speed of light is `c = 1` and the electron plasma frequency is `ω_pe = 1`, hence the electron inertial length is `c/ω_pe = 1`.
Electrons are assumed to be cold and charge-neutralizing ions are immobile.
The grid size of the one-dimensional domain is `∆x = 0.200781` and the integration time step is `∆t = 0.04`.
The number of grid points are `Nx = 480` and the periodic boundary conditions are used at both ends.
To stir up the system at the beginning, which is otherwise quiet,
the electric field is initialized with small-amplitude random fluctuations.

The intermediate results will be dumped into the `./data` directory with recording frequencies
depending on the `*_recording_frequency` options.


## Analysis of Results
