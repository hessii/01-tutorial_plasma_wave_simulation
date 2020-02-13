# Overview

This simulation demos electromagnetic waves in free space.
The goal is to understand

* Polarization;
* Field energy density and Poynting flux; and
* Wave packets and group velocity.

There are three demos based on problems 14.2, 14.4, 14.6, and 14.12.


# Build and Run

First, build the project using `make all` (with appropriate options for your system).

Then, to run the demos, issue the following command
> `./pic_1d -load --wd ./$DEMO`

with `$DEMO` replaced by one of `01_prob_14_2`, `02_prob_14_4`, and `03_prob_14_12`.

The demos produce `field-*.csv` files.


# Common Parameters

For simplicity, the light speed *c* = 1.
The number of grid points are *Nx* = 500 and the grid size is *Dx* = 2π/50.
Since the CFL condition requires *c* *dt* < *Dx*, the time step is set to *dt* = *Dx*/2.

The **E** and **B** are dumped at every other steps.
Hence, as far as the data analysis is concerned,
the time interval between two sequential dump files is *Dt* = 2*dt*.

It is important to mention that the **E** and **B** in a single file do not correspond to
**E** and **B** at the same spatial and temporal locations
(for the reasons of stability of the code and simplicity in implementation).
Instead, they are off by *Dx*/2 in space and by *dt*/2 in time
such that **B** is trailing **E** in both space and time.
In other words, if the *x* coordinates for **E** are {0, 1, 2, ...}*Dx*, then
the *x* coordinates for **B** are {–1/2, 1/2, 3/2, ...}*Dx*.
Likewise, if the timestamps for **E** are {0, 1, 2, ...}*dt*, then
the timestamps for **B** are {-1/2, 1/2, 3/2, ...}*dt*.


# Demo 1. A Linearly Polarized Plane Wave

This demo tests Problem 14.2(c).
For simplicity, *k* = 1 (thus *ω* = *kc* = 1), and *E0* = 1.
The wave is traveling in the positive *x* direction, and **E** is linearly polarized in the *y* direction.
Then, by Faraday's law, **B** is also linearly polarized in the *z* direction.
In cgs units, Faraday's law reads **B** = **k***c*/*ω* x **E** = **k**/*k* x **E**.

The two-dimensional intensity plots of *Ey* and *Bz* are shown below.
(Be sure to shift *Bz* in time and space appropriately.)

![E & B Phase](./figures/01_prob_14_2-em_wave_fronts.png)
