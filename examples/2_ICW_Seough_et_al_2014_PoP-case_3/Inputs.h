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
    static constexpr unsigned number_of_worker_threads = 39;

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
    static constexpr Real c = 100;

    /// magnitude of uniform background magnetic field
    ///
    static constexpr Real O0 = 1;

    /// angle in degrees between the x-axis and the uniform magnetic field direction.
    ///
    static constexpr Real theta = 0;

    /// simulation grid size
    ///
    static constexpr Real Dx = 0.03;

    /// number of grid points
    ///
    static constexpr unsigned Nx = 8192;

    /// time step size
    ///
    static constexpr Real dt = 0.0002;

    /// number of time steps for inner loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned inner_Nt = 500;

    /// number of time steps for outer loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    static constexpr unsigned outer_Nt = 800;

    //
    // MARK: Plasma Species Descriptions
    //

    /// kinetic plasma descriptors
    ///
    static constexpr auto part_descs =
    std::make_tuple(BiMaxPlasmaDesc({{  1, 100, 2}, 12000, TSC, full_f}, 10, 4),
                    BiMaxPlasmaDesc({{-25, 500, 2}, 12000, TSC, full_f}, 4.5, 1)
                    );

    /// cold fluid plasma descriptors
    ///
    static constexpr auto cold_descs =
    std::make_tuple();

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
    static constexpr unsigned field_recording_frequency = 10000;

    /// species moment recording frequency
    ///
    static constexpr unsigned moment_recording_frequency = 10000;

    /// simulation particle recording frequency
    ///
    static constexpr unsigned particle_recording_frequency = 10000;

    /// maximum number of particles to dump
    ///
    static constexpr std::array<unsigned,
    std::tuple_size_v<decltype(part_descs)>> Ndumps = {1000, 900};

    /// velocity histogram recording frequency
    ///
    static constexpr unsigned vhistogram_recording_frequency = 0;

    /// per-species gyro-averaged velocity space specification used for sampling velocity histogram
    ///
    /// the parallel (v1) and perpendicular (v2) velocity specs are described by
    /// the range of the velocity space extent and the number of velocity bins
    ///
    /// note that the Range type is initialized with the OFFSET (or location) and the LENGTH
    ///
    /// recording histograms corresponding to specifications with the bin count being 0 will be skipped over
    ///
    static constexpr std::array<std::pair<Range, unsigned>,
    std::tuple_size_v<decltype(part_descs)>> v1hist_specs = {
        std::make_pair(Range{-2, 5}, 17)
    };
    static constexpr std::array<std::pair<Range, unsigned>,
    std::tuple_size_v<decltype(part_descs)>> v2hist_specs = {
        std::make_pair(Range{0, 1}, 10)
    };
};

/// debugging options
///
namespace Debug {
    constexpr bool zero_out_electromagnetic_field = false;
    constexpr Real initial_efield_noise_amplitude = 0e0;
}

#endif /* Inputs_h */
