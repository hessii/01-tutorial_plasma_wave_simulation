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
    static constexpr unsigned number_of_worker_threads = 19;

    /// number of subdomains for domain decomposition (positive integer)
    ///
    /// Nx must be divisible by this number
    ///
    static constexpr unsigned number_of_subdomains = number_of_worker_threads + 1;

    /// flag to suppress transverse electromagnetic fields
    ///
    static constexpr bool is_electrostatic = true;

    //
    // MARK: Global parameters
    //

    /// light speed
    ///
    static constexpr Real c = 1;

    /// magnitude of uniform background magnetic field
    ///
    static constexpr Real O0 = 1;

    /// angle in degrees between the x-axis and the uniform magnetic field direction.
    ///
    static constexpr Real theta = 0;

    /// simulation grid size
    ///
    static constexpr Real Dx = M_PI/5;

    /// number of grid points
    ///
    static constexpr unsigned Nx = 480;

    /// time step size
    ///
    static constexpr Real dt = 0.02;

    /// number of time steps for inner loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned inner_Nt = 10;

    /// number of time steps for outer loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned outer_Nt = 10000;

    //
    // MARK: Plasma Species Descriptions
    //

    /// kinetic plasma descriptors
    ///
    static constexpr auto part_descs =
    std::make_tuple(BiMaxPlasmaDesc({{-O0, 0.1}, 2500, _2nd, delta_f}, .01, 1, 2),
                    BiMaxPlasmaDesc({{-O0, 0.1}, 2500, _2nd, delta_f}, .01, 1,-2)
                    );

    /// cold fluid plasma descriptors
    ///
    static constexpr auto cold_descs =
    std::make_tuple(ColdPlasmaDesc({-O0, 0.9949874371}));

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
    static constexpr unsigned field_recording_frequency = 5;

    /// species moment recording frequency
    ///
    static constexpr unsigned moment_recording_frequency = 0;

    /// simulation particle recording frequency
    ///
    static constexpr unsigned particle_recording_frequency = 1000;

    /// maximum number of particles to dump
    ///
    static constexpr std::array<unsigned,
    std::tuple_size_v<decltype(part_descs)>> Ndumps = {50000, 50000};
};

/// debugging options
///
namespace Debug {
    constexpr bool zero_out_electromagnetic_field = false;
    constexpr Real initial_efield_noise_amplitude = 1e-3;
}

#endif /* Inputs_h */
