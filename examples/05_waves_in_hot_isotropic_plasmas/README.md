# Overview

This tutorial illustrates longitudinal waves in hot isotropic plasmas.
The first demo describes the dispersion relation,
and the second demo illustrates the two-stream instability.


# Build and Run

This tutorial includes two demo simulations located in `01_longitudinal_wave_dispersion`
and `03_two_stream_instability`.

As usual, visit each directory, build the project using `make all`, and execute `./pic_1d`.

The two-stream instability demo takes a while to complete.

# Demo Parameters

The demos assume the same parameters as the homework problems.
That is, ions are immobile and electrons are cold (magnetoionic plasma).
The uniform magnetostatic field is in the *z* direction, and the wave vector is assumed to lie in
the *x*-*z* plane.
The ratio of the electron plasma frequency to the electron cyclotron frequency is *ωpe*/*Ωce* = 2.

Time is normalized to 1/*Ωce* and the distance to the electron skin depth, *de*.


# Dispersion Relations

We are going to use the three components of the electric field and display them in frequency
and wave number space by taking Fast Fourier Transform.
The simulated dispersion relations are compared with those from magnetoionic theory (homeworks).


- At θ = 0 

The figure below displays power spectral densities of the fluctuating electric field propagating
parallel to the background magnetic field.
The white dashed curves represent the theoretical dispersion relations for (from top to bottom)
the RCP, LCP, and RCP modes, respectively.

![Dispersion Relation at Parallel Propagation](./figures/01_parallel-efield_dispersion.png)


- At θ = 90°

The figure below displays power spectral densities of the fluctuating electric field propagating
perpendicular to the background magnetic field.
The white dashed curves represent the theoretical dispersion relations for
two extraordinary modes (abscent in *Ez*) and the ordinary mode (appearling only in *Ez*).

![Dispersion Relation at Perpendicular Propagation](./figures/02_perpendicular-efield_dispersion.png)


- At θ = 60°

The figure below displays power spectral densities of the fluctuating electric field propagating
at an intermediate angle from the background magnetic field.
The white dashed curves represent the theoretical dispersion relations for
the extraordinary modes and the ordinary mode.
The black dashed curves represent the theoretical dispersion relations for
the RCP and LCP modes.

![Dispersion Relation at Intermediate Propagation](./figures/03_intermediate-efield_dispersion.png)
