//
//  Inputs.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Inputs_h
#define Inputs_h

#include "./Predefined.h"
#include "./Macros.h"

#include <type_traits>
#include <array>

HYBRID1D_BEGIN_NAMESPACE
//
// simulation input parameters;
// modify the variables under the `Input' namespace
// consult "Predefined.h" header for symbol definitions and constants
//
namespace Input {
    //
    // MARK:- Housekeeping
    //

    /// parallelize particle update
    ///
    constexpr bool enable_concurrency = false;

    /// electric field extrapolation method
    ///
    constexpr _Algorithm algorithm = PC;

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
    constexpr char working_directory[] = ".";

    /// frequency of field and particle energy density recordings; in units of inner_Nt
    /// `0' means `not interested'
    ///
    constexpr unsigned energy_recording_frequency = 1;

    /// frequency of electric and magnetic field recordings
    ///
    constexpr unsigned field_recording_frequency = 3;

    /// frequency of kinetic ion moment recordings
    ///
    constexpr unsigned moment_recording_frequency = 5;

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
        constexpr unsigned Ns = 2;

        /// number of simulation particles per cell for individual populations
        ///
        constexpr std::array<unsigned, Ns> Ncs = {100, 100};

        /// ion cyclotron frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> Ocs = {1, 1};

        /// ion plasma frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> ops = {282.843, 282.843};

        /// ion betas for individual populations
        ///
        constexpr std::array<Real, Ns> betas = {.5, .5};

        /// ion temperature anisotropies (T_perp/T_para) for individual populations
        ///
        constexpr std::array<Real, Ns> T2OT1s = {3, 3};
    }
}

namespace Debug {
    constexpr bool zero_out_electromagnetic_field = false;
}

//
// MARK:- Input Checks
//
namespace {
    template <class T, unsigned long N>
    constexpr bool is_all_positive(std::array<T, N> A, std::integral_constant<unsigned long, N>) {
        return true;
    }
    template <class T, unsigned long N, unsigned long i>
    constexpr bool is_all_positive(std::array<T, N> A, std::integral_constant<unsigned long, i>) {
        return std::get<i>(A) > 0 && is_all_positive(A, std::integral_constant<unsigned long, i + 1>{});
    }
    template <class T, unsigned long N>
    constexpr bool is_all_positive(std::array<T, N> A) {
        return is_all_positive(A, std::integral_constant<unsigned long, 0>{});
    }

    static_assert(Pad >= Input::shape_order, "shape order should be less than or equal to the number of ghost cells");
    static_assert(Input::Nsubcycles >= 2, "number of subcyclings should be at least 2");

    static_assert(Input::c > 0, "speed of light should be a positive number");
    static_assert(Input::O0 > 0, "uniform background magnetic field should be a positive number");
    static_assert(Input::Dx > 0, "grid size should be a positive number");
    static_assert(Input::Nx > 0, "there should be at least 1 grid point");
    static_assert(Input::dt > 0, "time step should be a positive number");
    static_assert(Input::inner_Nt > 0, "inner loop count should be a positive number");

    static_assert(Input::eFluid::Oc < 0, "electron cyclotron frequency should be a negative number");
    static_assert(Input::eFluid::op > 0, "electron plamsa frequency should be a positive number");
    static_assert(Input::eFluid::beta >= 0, "electron beta should be a non-negative number");

    static_assert(is_all_positive(Input::iKinetic::Ncs), "N-particles-per-cell array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::ops), "plasma frequency array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::betas), "plasma beta array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::T2OT1s), "T2/T1 array contain non-positive element(s)");
}
HYBRID1D_END_NAMESPACE

#endif /* Inputs_h */
