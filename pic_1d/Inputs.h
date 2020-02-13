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
    static constexpr unsigned number_of_subdomains = 2;

    /// flag to suppress transverse electromagnetic fields
    ///
    static constexpr bool is_electrostatic = true;

    //
    // MARK: Global parameters
    //

    /// light speed
    ///
    static constexpr Real c = 10;

    /// magnitude of uniform background magnetic field
    ///
    static constexpr Real O0 = 1;

    /// angle in degrees between the x-axis and the uniform magnetic field direction.
    ///
    static constexpr Real theta = 0;

    /// simulation grid size
    ///
    static constexpr Real Dx = 1.023e-2;

    /// number of grid points
    ///
    static constexpr unsigned Nx = 65536;

    /// time step size
    ///
    static constexpr Real dt = 0.000511;

    /// number of time steps for inner loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned inner_Nt = 100;

    /// number of time steps for outer loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned outer_Nt = 588;

    //
    // MARK: Plasma Species Descriptions
    //

    /// kinetic plasma descriptors
    ///
    static constexpr auto part_descs =
    std::make_tuple(BiMaxPlasmaDesc({{-1, 2.23607, 2}, 20, CIC, full_f}, 0.018, 1, 3.8),
                    BiMaxPlasmaDesc({{-1, 9.74679, 2}, 20, CIC, full_f}, 0.038, 1, -0.2)
                    );

    /// cold fluid plasma descriptors
    ///
    static constexpr auto cold_descs =
    std::make_tuple(ColdPlasmaDesc({1./25, 2})
                    );

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
    static constexpr unsigned moment_recording_frequency = 10;

    /// simulation particle recording frequency
    ///
    static constexpr unsigned particle_recording_frequency = 100;

    /// maximum number of particles to dump
    ///
    static constexpr std::array<unsigned,
    std::tuple_size_v<decltype(part_descs)>> Ndumps = {1000, 900};
};

/// debugging options
///
namespace Debug {
    constexpr bool zero_out_electromagnetic_field = false;
    constexpr Real initial_efield_noise_amplitude = 0e0;
}

#endif /* Inputs_h */
