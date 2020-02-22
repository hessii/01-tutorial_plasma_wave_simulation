# Overview

This tutorial illustrates longitudinal waves in hot isotropic plasmas.
The first demo describes the dispersion relation,
and the second demo illustrates the two-stream instability.


# Build and Run

This tutorial includes two demo simulations located in `01_longitudinal_wave_dispersion`
and `03_two_stream_instability`.

As usual, visit each directory, build the project using `make all`, and execute `./pic_1d`.

The two-stream instability demo takes a while to complete.


# 1. Longitudinal Wave Dispersion Relation

In this demo, we will check the dispersion relation of longitudinal waves.
Same as in the lecture note, *ω_pe* = 1 and *α* = 1.
For the simulation, the grid size is *Dx* = π/10 *α*/*ω_pe*, the number of grid points *Nx* = 480,
the time step *Dt* = 0.04/*ω_pe* and the duration of the simulation is *t_max* = 200/*ω_pe*.

The figure below displays a comparison between the electric field power spectrum and
the theoretical dispersion relation.

![Dispersion Relation](./figures/01_longitudinal_wave_dispersion-Ex_dispersion.png)


# 2. Two-Stream Instability

This demo illustrates growing waves driven by the two-stream instability.
The instability is assumed to be driven by two tenuous beams of electrons drifting in opposite
directions to each other.
Each beam is Maxwellian-distributed with the thermal speed *α_h* = 1 and the drift speed *v_d* = 2 *α_h*.
The combined density of two beams is 1% of the total electron density.
The other 99% electrons are assumed to be Maxwellian-distributed with zero temperature.

The figure below displays evolution of wave and electron kinetic energy densities.
The first panel shows electric field energy which exhibits exponential growth up until
*t* *ω_pe* < ~400, followed by a more gradual increase.
The rate at which the electric field experiences the exponential growth is estimated to be γ ≈ 0.012 *ω_pe*, which is somewhat larger than the theoretical value of about 0.008 *ω_pe*.
The second panel shows the field and kinetic energy curves.
The increase of electric field energy is at the expense of the electron energy.

![Energy Overview](./figures/03_two_stream_instability-energy_overview.png)

This figure shows the evolution of the beam electrons.
As time progresses, the Maxwellian peaks are flattened and the surplus electrons are gradually filled in between the two beams, where the speed is smaller.
Consequently, the total kinetic energy gradually decreases.

![Distribution Function](./figures/03_two_stream_instability-distribution_function.png)

This figure shows electric field power spectral densities at four different time ranges.
The dashed lines indicate *v_d*.
At the initial stage, power is strongest in 0.5 < *k* < 0.9, in agreement with the theory prediction.
Note that the power suddenly cuts off at about *k* ≈ 0.5, at which according to theory γ changes its sign.
At the later stages, the region of strong power extends to both the shorter and longer wave lengths.
The interpretation is that, electrons that have been evolved now make those wave modes unstable.

![Dispersion Relation](./figures/03_two_stream_instability-Ex_dispersion.png)

This last figure illustrates the broadening of the *k*-spectrum.

![k-Spectrum Evolution](./figures/03_two_stream_instability-Ex_kspec_evolution.png)

