//
//  ParamSet.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/1/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef ParamSet_h
#define ParamSet_h

#include "./InputWrapper.h"
#include "./Utility/Options.h"

PIC1D_BEGIN_NAMESPACE
struct [[nodiscard]] ParamSet : public Input {

    /// number of threads for particle async update
    ///
    static constexpr unsigned number_of_particle_parallelism = (number_of_worker_threads + 1)/number_of_subdomains;

    /// index sequence of kinetic plasma descriptors
    ///
    using part_indices = std::make_index_sequence<std::tuple_size_v<decltype(part_descs)>>;

    /// index sequence of cold plasma descriptors
    ///
    using cold_indices = std::make_index_sequence<std::tuple_size_v<decltype(cold_descs)>>;

public:
    Range domain_extent;
    long outer_Nt{Input::outer_Nt};
    std::string working_directory{Input::working_directory};
    bool snapshot_save{false};
    bool snapshot_load{false};
    //
    ParamSet() noexcept;
    ParamSet(unsigned const rank, Options const &opts);

private:
    template <class... Ts, class Int, Int... Is>
    [[nodiscard]] static constexpr auto _serialize(std::tuple<Ts...> const &t, std::integer_sequence<Int, Is...>) noexcept {
        return std::tuple_cat(serialize(std::get<Is>(t))...);
    }
    [[nodiscard]] friend constexpr auto serialize(ParamSet const &params) noexcept {
        auto const global = std::make_tuple(params.is_electrostatic, params.c, params.O0, params.theta, params.Dx, params.Nx, params.dt, params.inner_Nt);
        auto const parts = _serialize(params.part_descs, part_indices{});
        auto const colds = _serialize(params.cold_descs, cold_indices{});
        return std::tuple_cat(global, parts, colds);
    }
};
PIC1D_END_NAMESPACE

#endif /* ParamSet_h */
