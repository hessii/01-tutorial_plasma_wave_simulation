# Overview

This simulation demos electromagnetic waves in free space.
The goal is to

1) Familiarize oneself with the compile-run process;
2) Read in and analyze the simulation data products; and
3) Confirm the linear theory predictions of the plasma oscillations.


# Build and Run

* In essence, issuing `CXX='clang++' make -j4 all` in the terminal will
compile the source codes and produce an executable called `pic_1d`.

    * The `-jx` flag instructs `make` to compile up to `x` number of source codes in parallel.
    The optimal value for `x` will be the number of cpu cores.

    * The `CXX` flag instructs `make` what c++ compiler it should use.
    The c++ compiler for the compilation must support the c++17 language standard or above.
    The default value for the `CXX` flag is `clang++` for macOS operating systems.
    GCC users, for example, will need to use `CXX=g++`.

    * Instead of passing it as the command line argument,
    one can specify the c++ compiler flag `CXX` in `Makefile`.
    In fact, `Makefile` contains other flags for further customization.

* As usual, issuing `make clean` will clean up the build products
(but not the data files produced by the simulation).

* To run the simulation, execute `./pic_1d`.

    Several default parameters of the simulation can be reconfigured through command line arguments.
    Supported are:

    * `-save` instructs to take a snapshot from which the simulation can resume at a later time.
    * `-load` instructs to restore the state from the latest snapshot and resume the simulation from the last point.
    * `--wd="working_directory"` instructs to use `"working_directory"` as the working directory.
    (The default is the value of the `working_directory` variable in the `Inputs.h`.)
    * `--outer_Nt="a non-negative integer"` instructs to run the simulation by `inner_Nt x outer_Nt` cycles.
    (The default is the value of the `outer_Nt` variable in the `Inputs.h`.)

    For this demo though, no extra arguments are necessary.


# Simulation Setup

The `Inputs.h` file contains all the physical parameters needed as well as 
those necessary to produce data products.
Some of them can be overwritten using the command line arguments (See the section [Compilation](#Compilation)).

The one-dimensional simulation of this demo is normalized such that
the speed of light is `c = 1` and the electron plasma frequency is `ω_pe = 1`.
Hence, the electron inertial length is `c/ω_pe = 1`.
What that means is that
all the quantities with the length dimension are normalized to the inertial length,
all the quantities with the time dimension are normalized to the inverse of the electron plasma frequency, and
all the quantities with the speed dimension are normalized to the speed of light.
Electrons are assumed to be cold fluid, and charge-neutralizing ions are immobile (infinite mass).

The grid size of the one-dimensional simulation domain (in the `x` direction) is `∆x = 0.200781` and
the integration time step is `∆t = 2π/100`.
The number of grid points are `Nx = 10` and the periodic boundary conditions are used at both ends.

To stir up the system at the beginning, which is otherwise quiet,
the electric field is initialized with small-amplitude random fluctuations.

The simulation results will be dumped into (by default) `./data` with data recording frequencies
specified by the `*_recording_frequency` options in the `Inputs.h` file.
An output directory other than the default one, say `my_sim` at the home directory,
can be reconfigured in the command line as follows:

> `./pic_1d --wd=~/my_sim`.


# Analysis of the Results

## Average Field and Plasma Energy

Let us read `energy.csv` with your favorite data analysis program.
(For `Mathematica` users, see the `analysis.nb` notebook.)

A couple of things are worth mentioning.
First, being electrostatic, there is no magnetic field fluctuations associated with the plasma oscillations.
In addition, the electric field vector oscillates in the `x` direction
(in this demo, the suffix-`1` component).
The same goes to the bulk flow velocity of cold electrons.

Second, because electrons are treated as a cold fluid, there is no thermal energy, and therefore
`mv1^2/2` == `mU1^2/2`.

What we are interested in is the total electric field energy (*E*^2/8π; equal to `dE1^2/2`), and
the total energy of cold electrons (*mv*^2/2; equal to `mv1^2/2`).
So grap them and plot them versus `time`.
(Remember the time is normalized by 1/ω_pe.)
The figure below shows them up to *t* = 2π/ω_pe, which is one cycle of the plasma oscillations.
(Think about why there are two cycles in the figure.)

![Field and Plasma Energy](./figures/field_electron_energy.png)


## Electric Field and Flow Velocity

Let us read the `field`s and `moment`s and retrieve `dE1` and `<v1>`,
which we will refer to as *E_x* and *U_x*, respectively.
Then take the values at the first grid point and plot them against `time`.
The figure below shows *E_x* and *U_x* up to *t* = 5π/ω_pe.

![Phase Relation between *E_x* and *U_x*](./figures/Ex_and_Ux_phase.png)

Notice the phase difference: *E_x* is behind *U_x* by a quarter cycle.
According to theory, *U_x* and *E_x* have the following relationship:

> *U_x* = –*i* (*e* *E_x*)/(ω_pe *m_e*).

Because –*i* = exp(–*i* π/2), the phase difference is π/2 and the '–' sign indicates that
*U_x* is indeed leading *E_x*.


