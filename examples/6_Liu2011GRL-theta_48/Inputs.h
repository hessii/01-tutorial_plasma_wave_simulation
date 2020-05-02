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
struct Input {
    //
    // MARK:- Housekeeping
    //

    /// number of worker threads to spawn for parallelization
    ///
    /// value `0' means serial update; value `n' means parallelization using n + 1 threads
    /// part_desc.Nc*Nx must be divisible by n + 1, and
    /// n + 1 must be divisible by number_of_subdomains
    ///
    static constexpr unsigned number_of_worker_threads = 9;

    /// number of subdomains for domain decomposition (positive integer)
    ///
    /// Nx must be divisible by this number
    ///
    static constexpr unsigned number_of_subdomains = 1;

    /// flag to suppress transverse electromagnetic fields
    ///
    static constexpr bool is_electrostatic = false;

    //
    // MARK: Global parameters
    //

    /// light speed
    ///
    static constexpr Real c = 4;

    /// magnitude of uniform background magnetic field
    ///
    static constexpr Real O0 = 1;

    /// angle in degrees between the x-axis and the uniform magnetic field direction.
    ///
    static constexpr Real theta = 48;

    /// simulation grid size
    ///
    static constexpr Real Dx = 0.200781;

    /// number of grid points
    ///
    static constexpr unsigned Nx = 400;

    /// time step size
    ///
    static constexpr Real dt = 0.02;

    /// number of time steps for inner loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned inner_Nt = 20;

    /// number of time steps for outer loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned outer_Nt = 1000;

    //
    // MARK: Plasma Species Descriptions
    //

    /// kinetic plasma descriptors
    ///
    static constexpr auto part_descs =
    std::make_tuple(BiMaxPlasmaDesc({{-1,  c*0.316227766, 2}, 1000, _1st, full_f},  1.0*0.1, 2),
                    BiMaxPlasmaDesc({{-1, c*0.9486832981, 2}, 1000, _1st, full_f}, 0.01*0.9, 5)
                    );

    /// cold fluid plasma descriptors
    ///
    static constexpr auto cold_descs =
    std::make_tuple(ColdPlasmaDesc({1./1836, c/42.8485705713, 0}));

    //
    // MARK: Data Recording
    //

    /// a top-level directory to which outputs will be saved
    ///
    static constexpr char working_directory[] = "./data";

    /// field and particle energy density recording frequency; in units of inner_Nt
    /// `0' means `not interested'
    ///
    static constexpr unsigned energy_recording_frequency = 1;

    /// electric and magnetic field recording frequency
    ///
    static constexpr unsigned field_recording_frequency = 1;

    /// species moment recording frequency
    ///
    static constexpr unsigned moment_recording_frequency = 1;

    /// simulation particle recording frequency
    ///
    static constexpr unsigned particle_recording_frequency = 100000;

    /// maximum number of particles to dump
    ///
    static constexpr std::array<unsigned,
    std::tuple_size_v<decltype(part_descs)>> Ndumps = {};
};

/// debugging options
///
namespace Debug {
    constexpr bool zero_out_electromagnetic_field = false;
    constexpr Real initial_efield_noise_amplitude = 0e0;
}

#endif /* Inputs_h */