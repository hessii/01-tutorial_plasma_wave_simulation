//
//  Inputs.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Inputs_h
#define Inputs_h

/// simulation input parameters;
/// modify the variables under the `Input' namespace
/// consult "Predefined.h" header for symbol definitions and constants
///
namespace Input {
    //
    // MARK:- Housekeeping
    //

    /// parallelize particle update
    ///
    constexpr bool enable_concurrency = true;

    /// electric field extrapolation method
    ///
    constexpr _Algorithm algorithm = CAMCL;

    /// particle and interpolation order
    ///
    constexpr _ShapeOrder shape_order = CIC;

    /// number of subscyles for magnetic field update; applied only for CAM-CL algorithm
    ///
    constexpr unsigned Nsubcycles = 4;

    /// number of smoothings
    ///
    constexpr unsigned Nsmooths = 2;

    //
    // MARK: Data Recording
    //

    /// a top-level directory to which outputs will be saved
    ///
    constexpr char working_directory[] = "./data";

    /// frequency of field and particle energy density recordings; in units of inner_Nt
    /// `0' means `not interested'
    ///
    constexpr unsigned energy_recording_frequency = 1;

    /// frequency of electric and magnetic field recordings
    ///
    constexpr unsigned field_recording_frequency = 3;

    /// frequency of kinetic ion moment recordings
    ///
    constexpr unsigned moment_recording_frequency = 3;

    /// frequency of simulation particle recordings
    ///
    constexpr unsigned particle_recording_frequency = 10000;

    //
    // MARK: Global parameters
    //

    /// light speed
    ///
    constexpr Real c = 400;

    /// magnitude of uniform background magnetic field
    ///
    constexpr Real O0 = 1;

    /// angle in degrees between the x-axis and the uniform magnetic field direction.
    ///
    constexpr Real theta = 0;

    /// simulation grid size
    ///
    constexpr Real Dx = 0.4;

    /// number of grid points
    ///
    constexpr unsigned Nx = 960;

    /// time step size
    ///
    constexpr Real dt = 0.04;

    /// number of time steps for inner loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    constexpr unsigned inner_Nt = 5;

    /// number of time steps for outer loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    constexpr unsigned outer_Nt = 1000;

    //
    // MARK: Fluid Electrons
    //
    namespace eFluid {
        /// electron cyclotron frequency
        ///
        constexpr Real Oc = -1836;

        /// electron plasma frequency
        ///
        constexpr Real op = 17139.4;

        /// electron beta
        ///
        constexpr Real beta = 0.0;

        /// specific heat ratio, gamma
        ///
        constexpr _Closure closure = isothermal;
    }

    //
    // MARK: Kinetic Ions
    //
    namespace iKinetic {
        /// number of ion species
        ///
        constexpr unsigned Ns = 1;

        /// number of simulation particles per cell for individual populations
        ///
        constexpr std::array<unsigned, Ns> Ncs = {100000};

        /// ion cyclotron frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> Ocs = {1};

        /// ion plasma frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> ops = {c};

        /// ion betas for individual populations
        ///
        constexpr std::array<Real, Ns> betas = {3};

        /// ion temperature anisotropies (T_perp/T_para) for individual populations
        ///
        constexpr std::array<Real, Ns> T2OT1s = {1./3};
    }
}

#endif /* Inputs_h */
