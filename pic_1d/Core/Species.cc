//
//  Species.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Species.h"

P1D::Species::Species(ParamSet const& params)
: params{params}, geomtr{params} {
}

auto P1D::Species::operator=(Species const &other) noexcept
-> Species &{
    {
        std::tie(this->moment<0>(), this->moment<1>()) =
        std::tie(other.moment<0>(), other.moment<1>());
    }
    return *this;
}
auto P1D::Species::operator=(Species &&other) noexcept
-> Species &{
    {
        std::tie(this->moment<0>(), this->moment<1>()) =
        std::forward_as_tuple(std::move(other.moment<0>()),
                              std::move(other.moment<1>()));
    }
    return *this;
}
