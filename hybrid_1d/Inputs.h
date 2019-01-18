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
// input parameters;
// modify accordingly the variables under the `Input' namespace
// consult "Predefined.h" header for symbol definitions and constants
//
namespace Input {
    //
    // MARK:- Housekeeping
    //

    /// electric field extrapolation method
    ///
    constexpr _ExtrapolationMethod extrap_method = PC;

    /// particle and interpolation shape order
    ///
    constexpr _ShapeOrder shape_order = CIC;

    /// number of subscylings; applied only for CAMCL
    ///
    constexpr long Nsubcycles = 4;

    /// number of smoothings
    ///
    constexpr long Nsmooths = 2;

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
    constexpr Real Dx = 1;

    /// number of grid points
    ///
    constexpr long Nx = 240;

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
        constexpr Real beta = 0.01;

        /// specific heat ratio, gamma
        ///
        constexpr _Closure closure = adiabatic;
    }

    //
    // MARK: Kinetic Ions
    //
    namespace iKinetic {
        /// number of ion species
        ///
        constexpr long Ns = 2;

        /// number of simulation particles per cell for individual populations
        ///
        constexpr std::array<long, Ns> Ncs = {100, 100};

        /// ion cyclotron frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> Ocs = {1, 1};

        /// ion plasma frequencies for individual populations
        ///
        constexpr std::array<Real, Ns> ops = {c, c};

        /// ion betas for individual populations
        ///
        constexpr std::array<Real, Ns> betas = {1, .01};

        /// ion temperature anisotropies (T_perp/T_para) for individual populations
        ///
        constexpr std::array<Real, Ns> T2OT1s = {3, 1};
    }
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
    static_assert(Input::Nsubcycles >= 1, "number of subcyclings should be at least 1");

    static_assert(Input::c > 0, "speed of light should be a positive number");
    static_assert(Input::O0 > 0, "uniform background magnetic field should be a positive number");
    static_assert(Input::Dx > 0, "grid size should be a positive number");
    static_assert(Input::Nx >= 1, "there should be at least 1 grid point");

    static_assert(Input::iKinetic::Ns >= 0, "");
    static_assert(is_all_positive(Input::iKinetic::Ncs), "N-particles-per-cell array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::ops), "plasma frequency array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::betas), "plasma beta array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::T2OT1s), "T2/T1 array contain non-positive element(s)");
}
HYBRID1D_END_NAMESPACE

#endif /* Inputs_h */