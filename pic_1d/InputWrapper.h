//
//  InputWrapper.h
//  pic_1d
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

PIC1D_BEGIN_NAMESPACE
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
    template <class Pred, class T, unsigned long... Is>
    [[nodiscard]] constexpr auto is_all(Pred pred, std::array<T, sizeof...(Is)> A, std::index_sequence<Is...>) noexcept(noexcept(pred(std::declval<T>())))
    -> std::enable_if_t<std::is_invocable_r_v<bool, Pred, T const&>, bool> {
        return (true && ... && pred(std::get<Is>(A)));
    }
    template <class Pred, class T, unsigned long N>
    [[nodiscard]] constexpr auto is_all(Pred pred, std::array<T, N> A) noexcept(noexcept(pred(std::declval<T>())))
    -> std::enable_if_t<std::is_invocable_r_v<bool, Pred, T const&>, bool> {
        return is_all(pred, A, std::make_index_sequence<N>{});
    }

    template <class T, unsigned long N>
    [[nodiscard]] constexpr bool is_all_positive(std::array<T, N> A) {
        return is_all([](T const &x) noexcept { return x > 0; }, A);
    }
    template <class T, unsigned long N>
    [[nodiscard]] constexpr bool is_all_nonnegative(std::array<T, N> A) {
        return is_all([](T const &x) noexcept { return x >= 0; }, A);
    }
    template <class T, unsigned long N>
    [[nodiscard]] constexpr bool is_all_non_zero(std::array<T, N> A) {
        return is_all([](T const &x) noexcept { return x != 0; }, A);
    }
    template <class T, unsigned long N>
    [[nodiscard]] constexpr bool is_all_divisible_by(std::array<T, N> A, T Nx, T denom) {
        return is_all([Nx, denom](T const &x) noexcept { return x*Nx % denom == 0; }, A);
    }

    static_assert(Pad >= Input::shape_order, "shape order should be less than or equal to the number of ghost cells");
    static_assert(Input::number_of_worker_threads < 128, "too large number of worker threads");

    static_assert(Input::c > 0, "speed of light should be a positive number");
    static_assert(Input::O0 > 0, "uniform background magnetic field should be a positive number");
    static_assert(Input::Dx > 0, "grid size should be a positive number");
    static_assert(Input::Nx > 0, "there should be at least 1 grid point");
    static_assert(Input::dt > 0, "time step should be a positive number");
    static_assert(Input::inner_Nt > 0, "inner loop count should be a positive number");

    static_assert(is_all_positive(Input::PartDesc::Ncs), "N-particles-per-cell array contain non-positive element(s)");
    static_assert(is_all_divisible_by(Input::PartDesc::Ncs, Input::Nx, Input::number_of_worker_threads + 1), "N-particles-per-cell array contain element(s) not divisible by Input::number_of_worker_threads");
    static_assert(is_all_non_zero(Input::PartDesc::Ocs), "cyclotron frequency array contain zero element(s)");
    static_assert(is_all_positive(Input::PartDesc::ops), "particle plasma frequency array contain non-positive element(s)");
    static_assert(is_all_positive(Input::PartDesc::betas), "particle plasma beta array contain non-positive element(s)");
    static_assert(is_all_positive(Input::PartDesc::T2OT1s), "particle T2/T1 array contain non-positive element(s)");
    static_assert(is_all_nonnegative(Input::PartDesc::nus), "particle collisional frequency array contain negative element(s)");

    static_assert(is_all_non_zero(Input::ColdDesc::Ocs), "cyclotron frequency array contain zero element(s)");
    static_assert(is_all_positive(Input::ColdDesc::ops), "cold plasma frequency array contain non-positive element(s)");
    static_assert(is_all_nonnegative(Input::ColdDesc::nus), "cold plasma collisional frequency array contain negative element(s)");
}
PIC1D_END_NAMESPACE

#endif /* InputWrapper_h */
