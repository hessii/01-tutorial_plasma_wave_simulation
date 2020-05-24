# Overview

This tutorial illustrates electrostatic longitudinal waves in hot isotropic plasmas
without a static uniform magnetic field.
The first demo (`01_longitudinal_wave_dispersion`) describes the dispersion relation of electrostatic longitudinal waves,
and the second demo (`03_two_stream_instability`) describes the two-stream instability.


# Build and Run

This tutorial includes two demo simulations located in `01_longitudinal_wave_dispersion`
and `03_two_stream_instability`.

As usual, visit each directory, build the project using `make all`, and execute `./pic_1d`.

The two-stream instability demo takes a while to complete.


# 1. Longitudinal Wave Dispersion Relation

In this demo, we will check the dispersion relation of electrostatic longitudinal waves,
which is given by *κ*^2 = *Z*'(*ζ*), where *κ* = *kα*/*ω_pe* is the normalized wave number,
and *ζ* = *ω*/(*kα*) is the normalized wave phase speed.
Same as in the lecture note, the plasma frequency *ω_pe* = 1
and the thermal velocity of the Maxwellian distribution *α* = 1.

For the simulation, the grid size is *Dx* = π/10 *α*/*ω_pe*, the number of grid points *Nx* = 480,
the time step *Dt* = 0.04/*ω_pe* and the duration of the simulation is *t_max* = 200/*ω_pe*.

Extract the *x* component of the electric field *E_x* = `dE1` for the spectral analysis.
The figure below displays a comparison between the electric field power spectrum and
the theoretical dispersion relation.
The dispersion relation approaches *ω_pe* at the high phase speed limit.

![Dispersion Relation](./figures/01_longitudinal_wave_dispersion-Ex_dispersion.png)


# 2. Two-Stream Instability

This demo illustrates growing waves driven by the two-stream instability.
Here, this instability is driven by two tenuous beams of electrons drifting in opposite
directions to each other.
Each beam is Maxwellian-distributed with the thermal speed *α_h* = 1 and the drift speed *v_d* = 2 *α_h*.
The combined density of two beams is 1% of the total electron density.
The other 99% electrons are assumed to be cold with no drift.

## Average Energy in the System

You should be familiar with the data layout in `energy.csv` file.
The (electric field) wave energy density is `dE1^2/2` (6th column),
and the total kinetic energy density is the sum of `part_species(1) mv1^2/2`,
`part_species(2) mv1^2/2`, and `cold_species(1) mv1^2/2`.

The figure below displays the evolution of wave and electron kinetic energy densities.
The first panel shows the electric field energy curve, in a logarithmic scale.
Notice that the curve exhibits exponential growth until
*t* *ω_pe* < ~400, followed by a more gradual increase.
The estimated growth rate is γ ≈ 0.012 *ω_pe* [*don't need to reproduce this*].
(This is somewhat larger than the theoretical value of about 0.008 *ω_pe*.)

The second panel shows a comparison of the field energy
and the kinetic energy (the departure from the initial).
Since the total of these two remains steady (dashed curve),
the wave energy must come from the electrons.

![Energy Overview](./figures/03_two_stream_instability-energy_overview.png)


## Wave Dispersion Relation

To identify the wave mode and understand the propagation characteristics,
read in the `dE1` column (the *x* component of the electric field) from the `field-*.csv` files.

This figure shows the power spectral densities at four different time ranges labeled in each panel.
(That is, bracket the data with the given time ranges and do the fourier transform.)
The dashed line indicates *v_d*.
At an initial stage, the wave power is strongest between 0.5 < *k* < 0.9, in agreement with a theory prediction.
Note that the wave power suddenly drops at about *k* ≈ 0.5, where according to the theory, *γ* abruptly changes its sign.
At later stages, the region of strong power extends to both the shorter and longer wave length regions.
It can be understood that the electrons that have been evolved generate new modes that were stable initially.

![Dispersion Relation](./figures/03_two_stream_instability-Ex_dispersion.png)

This is another way of looking at how waves evolve in *k* space.

![k-Spectrum Evolution](./figures/03_two_stream_instability-Ex_kspec_evolution.png)


## Electron Dynamics

To understand how electrons respond to the growing waves, we need to analyze
the electrons' velocity distribution function.
The simulation produces the files named `particle-sp_1-*.csv` and `particle-sp_2-*.csv`,
which we have not dealt with previously.
They contain the state of sample beam electrons at given times.
Each file contains one line of metadata and a 5-column table of sample electron information.
Among the five columns, we only need the `v1` column, which stands for *v_x* of the sample electrons.
The infixes, `sp_1` and `sp_2`, in the file name correspond to the two beam populations.
For the purpose of this demo, we can combine the two types of the samples.

Use the electron samples to generate histograms at each time with an appropriate *v_x* bin specification
guided by the plot below.
(Consult with *[this manual](https://www.harrisgeospatial.com/docs/HISTOGRAM.html)* for *IDL* users, and
*[this manual](https://numpy.org/doc/stable/reference/generated/numpy.histogram.html)* for *python* users.)

This figure shows the evolution of the beam electrons.
(*Let's not worry about normalization as long as you get the shape right.*)
As time progresses, the initial beams are flattened and the surplus electrons gradually fill the void between the two beams.
Since there are more electrons losing the speed than there are accelerating,
the total kinetic energy we saw earlier gradually decreases.
Theoretically, this change of the velocity distribution function amounts to a diffusion.

![Distribution Function](./figures/03_two_stream_instability-distribution_function.png)

*[This link](https://www.youtube.com/watch?v=y9cY5Y_X2qI)* demonstrates the dynamic evolution of the beam particles.
