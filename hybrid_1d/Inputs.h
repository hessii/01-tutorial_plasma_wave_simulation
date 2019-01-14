//
//  Inputs.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Inputs_h
#define Inputs_h

#include "Macros.h"
#include "Predefined.h"
#include <type_traits>
#include <array>

HYBRID1D_BEGIN_NAMESPACE
//
// input parameters;
// please consult "Predefined.h" header for symbol definitions and constants
//
namespace {
    //
    // housekeeping
    //
    constexpr _ExtrapolationMethod extrapMethod = PC;
    constexpr _ShapeOrder shape_order = CIC;
    constexpr long Nsubcycles = 4;
    constexpr long Nsmooths = 2;

    //
    // global parameters
    //
    namespace Global {
        constexpr Real c = 400;

        constexpr Real O0 = 1;

        /// cos(theta), where theta is the angle between the x-axis and the uniform magnetic field direction.
        ///
        constexpr Real x_dot_B = 0;

        constexpr Real Dx = 1;

        constexpr long Nx = 240;
    }

    //
    // electron fluid
    //
    namespace Fluid {
        constexpr Real Oc = -1836;
        constexpr Real op = 17139.4;
        constexpr Real beta = 0.01;
        constexpr _Closure closure = adiabatic;
    }

    //
    // particle species
    //
    namespace Kinetic {
        constexpr long Ns = 2;

        constexpr std::array<long, Ns> Ncs = {100, 100};

        constexpr std::array<Real, Ns> Ocs = {1, 1};

        constexpr std::array<Real, Ns> ops = {Global::c, Global::c};

        constexpr std::array<Real, Ns> betas = {1, .01};

        constexpr std::array<Real, Ns> T2OT1s = {3, 1};
    }
}

//
// input checks
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
    template <class T>
    constexpr bool is_all_positive(std::array<T, Kinetic::Ns> A) {
        return is_all_positive(A, std::integral_constant<unsigned long, 0>{});
    }
}

static_assert(Pad >= shape_order, "shape order should be less than or equal to the number of ghost cells");
static_assert(Nsubcycles >= 1, "number of subcyclings should be at least 1");

static_assert(Global::c > 0, "speed of light should be a positive number");
static_assert(Global::O0 > 0, "uniform background magnetic field should be a positive number");
static_assert(Global::x_dot_B >= -1, "out of range for a cosine of a real number");
static_assert(Global::x_dot_B <=  1, "out of range for a cosine of a real number");
static_assert(Global::Dx > 0, "grid size should be a positive number");
static_assert(Global::Nx >= 1, "there should be at least 1 grid point");

static_assert(Kinetic::Ns >= 0, "");
static_assert(is_all_positive(Kinetic::Ncs), "N-particles-per-cell array contain non-positive element(s)");
static_assert(is_all_positive(Kinetic::ops), "plasma frequency array contain non-positive element(s)");
static_assert(is_all_positive(Kinetic::betas), "plasma beta array contain non-positive element(s)");
static_assert(is_all_positive(Kinetic::T2OT1s), "T2/T1 array contain non-positive element(s)");

HYBRID1D_END_NAMESPACE

#endif /* Inputs_h */
