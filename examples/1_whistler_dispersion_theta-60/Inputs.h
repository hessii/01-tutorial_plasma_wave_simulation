//
//  Inputs.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
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

    /// number of worker threads for parallelization
    ///
    /// value `0' means serial update; value `n' means parallelization using n + 1 threads
    /// PartDesc::Ncs*Nx must be divisible by n + 1
    ///
    constexpr unsigned number_of_worker_threads = 19;

    /// particle and interpolation order
    ///
    constexpr _ShapeOrder shape_order = TSC;

    /// flag to suppress magnetic field
    ///
    constexpr bool is_electrostatic = false;

    //
    // MARK: Global parameters
    //

    /// light speed
    ///
    constexpr Real c = 4;

    /// magnitude of uniform background magnetic field
    ///
    constexpr Real O0 = 1;

    /// angle in degrees between the x-axis and the uniform magnetic field direction.
    ///
    constexpr Real theta = 60;

    /// simulation grid size
    ///
    constexpr Real Dx = 0.3;

    /// number of grid points
    ///
    constexpr unsigned Nx = 1000;

    /// time step size
    ///
    constexpr Real dt = 0.02;

    /// number of time steps for inner loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    constexpr unsigned inner_Nt = 15;

    /// number of time steps for outer loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    constexpr unsigned outer_Nt = 1000;

    //
    // MARK: Particle Species Description
    //
    namespace PartDesc {
        /// number of particle species (or populations)
        ///
        constexpr unsigned Ns = 2;

        /// full-f versus delta-f
        ///
        constexpr std::array<_ParticleScheme, Ns> schemes = {full_f, full_f};

        /// number of simulation particles per cell for individual populations
        ///
        constexpr std::array<unsigned, Ns> Ncs = {1000, 1000};

        /// cyclotron frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> Ocs = {-1, 0.000544662};

        /// plasma frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> ops = {4, 0.093352};

        /// parallel (w.r.t the background magnetic field direction)
        /// betas for individual populations
        ///
        constexpr std::array<Real, Ns> betas = {0.01, 0.01};

        /// temperature anisotropies (T_perp/T_para) for individual populations
        ///
        constexpr std::array<Real, Ns> T2OT1s = {1, 1};

        /// parallel drift speed for individual populations
        ///
        constexpr std::array<Real, Ns> vds = {};

        /// number of source smoothings
        ///
        constexpr std::array<unsigned, Ns> Nsmooths = {2, 2};

        /// collisional frequency for numerical damping factor of current
        ///
        constexpr std::array<Real, Ns> nus = {};
    }

    //
    // MARK: Cold Species Description
    //
    namespace ColdDesc {
        /// number of cold species (or populations)
        ///
        constexpr unsigned Ns = 0;

        /// cyclotron frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> Ocs = {};

        /// plasma frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> ops = {};

        /// parallel flow drift speed for individual populations
        ///
        constexpr std::array<Real, Ns> vds = {};

        /// number of source smoothings
        ///
        constexpr std::array<unsigned, Ns> Nsmooths = {};

        /// collisional frequency for numerical damping factor of current
        ///
        constexpr std::array<Real, Ns> nus = {};
    }

    //
    // MARK: Data Recording
    //

    /// a top-level directory to which outputs will be saved
    ///
    constexpr char working_directory[] = "./data";

    /// field and particle energy density recording frequency; in units of inner_Nt
    /// `0' means `not interested'
    ///
    constexpr unsigned energy_recording_frequency = 1;

    /// electric and magnetic field recording frequency
    ///
    constexpr unsigned field_recording_frequency = 1;

    /// species moment recording frequency
    ///
    constexpr unsigned moment_recording_frequency = 10000;

    /// simulation particle recording frequency
    ///
    constexpr unsigned particle_recording_frequency = 10000;

    /// maximum number of particles to dump
    ///
    constexpr std::array<unsigned, PartDesc::Ns> Ndumps = {1000, 900};
}

#endif /* Inputs_h */
