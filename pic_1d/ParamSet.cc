//
//  ParamSet.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/1/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "ParamSet.h"

#include <string_view>
#include <variant>
#include <limits>
#include <map>

namespace {
    constexpr auto quiet_nan = std::numeric_limits<double>::quiet_NaN();
}
P1D::ParamSet::ParamSet() noexcept
: domain_extent{quiet_nan, quiet_nan} {
}
P1D::ParamSet::ParamSet(unsigned const rank, Options const &opts)
: ParamSet{} {
    // domain extent
    //
    static_assert(Input::Nx % Input::number_of_subdomains == 0, "Nx should be divisible by number_of_subdomains");
    Real const Mx = Input::Nx/Input::number_of_subdomains;
    domain_extent = {rank*Mx, Mx};

    // optional parameters
    //
    std::map<std::string_view, std::variant<long*, bool*>> const map{
        {"outer_Nt", &outer_Nt},
        {    "save", &save},
        {    "load", &load}
    };
    for (auto const &[key, val] : *opts) {
        std::visit(val, map.at(key));
    }
}
