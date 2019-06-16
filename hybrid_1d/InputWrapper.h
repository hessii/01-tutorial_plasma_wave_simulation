//
//  InputWrapper.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 2/6/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef InputWrapper_h
#define InputWrapper_h

#include "./Predefined.h"
#include "./Macros.h"

#include <type_traits>
#include <utility>
#include <array>

HYBRID1D_BEGIN_NAMESPACE
// input parameter header
//
#include <Inputs.h>

/// debugging options
///
namespace Debug {
    constexpr bool zero_out_electromagnetic_field = false;
}

//
// MARK:- Input Checks
//
namespace {
    template <class T, unsigned long... Is>
    [[nodiscard]] constexpr bool is_all_positive(std::array<T, sizeof...(Is)> A, std::index_sequence<Is...>) {
        return (true && ... && (std::get<Is>(A) > 0));
    }
    template <class T, unsigned long N>
    [[nodiscard]] constexpr bool is_all_positive(std::array<T, N> A) {
        return is_all_positive(A, std::make_index_sequence<N>{});
    }

    template <class T, unsigned long... Is>
    [[nodiscard]] constexpr bool is_all_divisible_by(std::array<T, sizeof...(Is)> A, T denom, std::index_sequence<Is...>) {
        return (true && ... && (std::get<Is>(A) % denom == 0));
    }
    template <class T, unsigned long N>
    [[nodiscard]] constexpr bool is_all_divisible_by(std::array<T, N> A, T denom) {
        return is_all_divisible_by(A, denom, std::make_index_sequence<N>{});
    }

    static_assert(Pad >= Input::shape_order, "shape order should be less than or equal to the number of ghost cells");
    static_assert(Input::Nsubcycles >= 2, "number of subcyclings should be at least 2");
    static_assert(Input::number_of_worker_threads < 128, "too large number of worker threads");

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
    static_assert(is_all_divisible_by(Input::iKinetic::Ncs, Input::number_of_worker_threads + 1), "N-particles-per-cell array contain element(s) not divisible by Input::number_of_worker_threads");
    static_assert(is_all_positive(Input::iKinetic::ops), "plasma frequency array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::betas), "plasma beta array contain non-positive element(s)");
    static_assert(is_all_positive(Input::iKinetic::T2OT1s), "T2/T1 array contain non-positive element(s)");
}
HYBRID1D_END_NAMESPACE

#endif /* InputWrapper_h */
