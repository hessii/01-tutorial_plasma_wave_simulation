# Description

This is a setup for simulations of plasma oscillations (or Langmuir oscillations) for a plasma with cold electrons and immobile ions.
The `Inputs.h` file describes all the parameters needed for the simulation as well as auxiliary options to run the simulation and produce data products.

The system is normalized such that the speed of light is `c = 1` and the electron plasma frequency is `omega_pe = 1`, hence the electron inertial length is `c/omega_pe = 1`.
The grid size is `∆x = 0.200781` and the integration time step is `∆t = 0.04`.
The number of one-dimensional simulation grid points are `Nx = 480` and periodic boundary conditions are used at both ends.
To stir up the system that is otherwise quiet, the electric field is initialized with small-amplitude random fluctuations.

The intermediate results will be dumped into the `./data` directory with frequencies depending on the `*_recording_frequency` options.


## Compilation

* In essence, executing `CXX='clang++' make -j4 all` in the working directory will compile the source codes and produce an executable called `pic_1d`.
The `-j4` flag instructs `make` to compile up to four source codes in parallel.
Of course, GCC users, for example, will need to change the `CXX` environment variable to `CXX=g++`.
The compiler in use should support the c++17 language standard.

* Executing `make clean` will clean up the object files and the build product, but not the data files produced during simulations.


## Analysis
