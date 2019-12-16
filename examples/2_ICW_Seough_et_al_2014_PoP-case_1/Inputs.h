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
    /// part_desc.Nc*Nx must be divisible by n + 1
    ///
    constexpr unsigned number_of_worker_threads = 39;

    /// flag to suppress magnetic field
    ///
    constexpr bool is_electrostatic = false;

    //
    // MARK: Global parameters
    //

    /// light speed
    ///
    constexpr Real c = 10;

    /// magnitude of uniform background magnetic field
    ///
    constexpr Real O0 = 1;

    /// angle in degrees between the x-axis and the uniform magnetic field direction.
    ///
    constexpr Real theta = 0;

    /// simulation grid size
    ///
    constexpr Real Dx = 0.02;

    /// number of grid points
    ///
    constexpr unsigned Nx = 16384;

    /// time step size
    ///
    constexpr Real dt = 0.001;

    /// number of time steps for inner loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    constexpr unsigned inner_Nt = 100;

    /// number of time steps for outer loop
    /// total time step Nt = inner_Nt * outer_Nt
    /// simulation time t = dt*Nt
    ///
    constexpr unsigned outer_Nt = 3000;

    //
    // MARK: Plasma Species Descriptions
    //

    /// kinetic plasma descriptors
    ///
    constexpr auto part_descs =
    std::make_tuple(BiMaxPlasmaDesc({{  1, 10, 2}, 6000, TSC, full_f}, 0.1, 4),
                    BiMaxPlasmaDesc({{-25, 50, 2}, 6000, TSC, full_f}, 0.1, 1)
                    );

    /// cold fluid plasma descriptors
    ///
    constexpr auto cold_descs =
    std::make_tuple();

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
    constexpr unsigned field_recording_frequency = 10000;

    /// species moment recording frequency
    ///
    constexpr unsigned moment_recording_frequency = 10000;

    /// simulation particle recording frequency
    ///
    constexpr unsigned particle_recording_frequency = 10000;

    /// maximum number of particles to dump
    ///
    constexpr std::array<unsigned,
    std::tuple_size_v<decltype(part_descs)>> Ndumps = {1000, 900};
}

#endif /* Inputs_h */
