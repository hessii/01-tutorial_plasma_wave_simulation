# Overview

This simulation demos electromagnetic waves in free space.
The goal is to understand

* Polarization;
* Field energy density and Poynting flux; and
* Wave packets and group velocity.

There are three demos related to Problems 14.2, 14.4, 14.6, and 14.12.


# Build and Run

First, build the project using `make all` (with the options appropriate for your build system).

To run the demos,

> `./pic_1d -load --wd ./$DEMO`

replacing `$DEMO` with one of `01_prob_14_2`, `02_prob_14_4`, and `03_prob_14_12`.

The demos produce `field-*.csv` files in the `$DEMO` directory.


# Common Parameters

For simplicity, the light speed is *c* = 1.
The number of grid points are *Nx* = 500 and the grid size is *Dx* = 2π/50.
Since the CFL condition requires *c* *dt* < *Dx*, the time step is set to *dt* = *Dx*/2.
But, because the **E** and **B** are dumped at every other step,
the time interval between two sequential dump files is *Dt* = 2*dt*,
as far as the data analysis is concerned.

It is important to note that the **E** and **B** do not correspond to the **E** and **B**
at the same spatial and temporal locations.
Instead, they are off by *Dx*/2 in space and by *dt*/2 in time
in such a way that **B** is trailing **E** in both space and time.
In other words, if the *x* coordinates for **E** are {0, 1, 2, ...} *Dx*, then
the *x* coordinates for **B** are {–1/2, 1/2, 3/2, ...} *Dx*.
Likewise, if the timestamps for **E** are {0, 1, 2, ...} *dt*, then
the timestamps for **B** are {-1/2, 1/2, 3/2, ...} *dt*.
(Not *Dt*!)


# Demos

## 1. A Linearly Polarized Plane Wave

This demo is related to Problem 14.2(c).

For simplicity, let us assume *k* = 1 (thus *ω* = *kc* = 1), and *E0* = 1.
The wave is traveling in the positive *x* direction, and
**E** associated with it is linearly polarized in the *y* direction.
Then, by Faraday's law, **B** is also linearly polarized in the *z* direction.
(In cgs units, Faraday's law reads **B** = **k***c*/*ω* x **E** = **k**/*k* x **E**.)

The two-dimensional intensity plots of *Ey* and *Bz* are shown below.
(Be sure to shift *Bz* in time and space appropriately.)
Observe that the wave indeed travels towards right.
Pick a point on a crest.
As time increases, that crest is traveling in the increasing *x* direction.
The slope of the constant phases gives you the phase speed, which is of course *c*.

![E & B Phase](./figures/01_prob_14_2-em_wave_fronts.png)

Now let us calculate the Poynting vector, **S**.
For the normalization used in this code, it is given by **S** = *c* **E** x **B**.
Therefore, the only non-zero component is *Sx* = *c* *Ey* *Ez*.
Using the wave period *P* = 2π/ω, the average Poynting vector is given by

![Average Poynting Vector](./figures/average_Poynting_vector.png)

The figure below shows *Sx* and <*Sx*> versus time at *x* = 0.
While *Sx* experiences sinusoidal oscillations (because *Ey* and *Bz* are in phase),
<*Sx*> maintains a constant value.

![Average Poynting Vector](./figures/01_prob_14_2-poynting_vector.png)

As a final check, Problem 14.4(c) shows <**S**> and the wave energy density *W* are related
through <**S**> = *c* <*W*>, where *W* is, for the normalization used in this code, given by
*W* = (*E*^2 + *B*^2)/2.
The figure below shows *W* and <*W*> versus time at *x* = 0.

![Average EM Energy](./figures/01_prob_14_2-em_energy.png)


## 2. A Circularly Polarized Plane Wave

This demo is related to Problems 14.4 and 14.6.

Let us use *E*^0 = 1/√2; α_1 = 0; α_2 = π/2; *k* = 1; and *c* = 1.
This is the description of a righthand circularly polarized wave traveling in the increasing *x*
(or **e**_3 in the problem) direction.

The animation below demonstrates the sense of polarization of **E** and **B**.
The point of view is looking behind in front of the wave.
Confirm this yourself.

![Average EM Energy](./figures/02_prob_14_4-polarization.gif)

The figures below show <**S**> and <*W*>.

![Average Poynting Vector](./figures/02_prob_14_4-poynting_vector.png)
![Average EM Energy](./figures/02_prob_14_4-em_energy.png)


















