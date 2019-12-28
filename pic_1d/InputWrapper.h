//
//  InputWrapper.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/6/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef InputWrapper_h
#define InputWrapper_h

#include "./PlasmaDesc.h"
#include "./Predefined.h"
#include "./Macros.h"

#include <type_traits>
#include <utility>
#include <array>
#include <tuple>

PIC1D_BEGIN_NAMESPACE
//
// MARK: Input Parameters
//
#include <Inputs.h>

//
// MARK: Simulation Parameter Set
//
struct ParamSet : public Input {

    /// domain decomposition
    ///
    static constexpr unsigned number_of_subdomains = 1 + number_of_worker_threads;

    /// index sequence of kinetic plasma descriptors
    ///
    using part_indices = std::make_index_sequence<std::tuple_size_v<decltype(Input::part_descs)>>;

    /// index sequence of cold plasma descriptors
    ///
    using cold_indices = std::make_index_sequence<std::tuple_size_v<decltype(Input::cold_descs)>>;
};

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

    template <class... Ts, class Int, Int... Is>
    [[nodiscard]] constexpr bool check_shape(std::tuple<Ts...> const &descs, std::integer_sequence<Int, Is...>) noexcept {
        return is_all([pad = Pad](ShapeOrder const &order) noexcept {
            return pad >= order;
        }, std::array<ShapeOrder, sizeof...(Ts)>{std::get<Is>(descs).shape_order...});
    }

    static_assert(ParamSet::c > 0, "speed of light should be a positive number");
    static_assert(ParamSet::O0 > 0, "uniform background magnetic field should be a positive number");
    static_assert(ParamSet::Dx > 0, "grid size should be a positive number");
    static_assert(ParamSet::Nx > 0, "there should be at least 1 grid point");
    static_assert(ParamSet::Nx % ParamSet::number_of_subdomains == 0, "simulation domain is not evenly divisible");
    static_assert(ParamSet::dt > 0, "time step should be a positive number");
    static_assert(ParamSet::inner_Nt > 0, "inner loop count should be a positive number");

    static_assert(check_shape(ParamSet::part_descs, ParamSet::part_indices{}), "shape order should be less than or equal to the number of ghost cells");
}
PIC1D_END_NAMESPACE

#endif /* InputWrapper_h */
