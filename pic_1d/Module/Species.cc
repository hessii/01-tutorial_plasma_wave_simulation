//
//  Species.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Species.h"

#include <stdexcept>
#include <algorithm>
#include <utility>

P1D::Species::Species(Real const Oc, Real const op, long const Nc)
: Nc(Nc), Oc{Oc}, op{op}, bucket{}, _mom{} {
    if (Nc < 0) {
        throw std::invalid_argument{std::string{__FUNCTION__} + "negative Nc"};
    }
}
P1D::Species &P1D::Species::operator=(Species const &o)
{
    Nc = o.Nc;
    Oc = o.Oc;
    op = o.op;
    bucket = o.bucket;
    if constexpr ( (false) ) { // do not copy moments
        std::copy(o.moment<0>().dead_begin(), o.moment<0>().dead_end(), moment<0>().dead_begin());
        std::copy(o.moment<1>().dead_begin(), o.moment<1>().dead_end(), moment<1>().dead_begin());
        std::copy(o.moment<2>().dead_begin(), o.moment<2>().dead_end(), moment<2>().dead_begin());
    }
    return *this;
}
P1D::Species &P1D::Species::operator=(Species &&o)
{
    Nc = std::move(o.Nc);
    Oc = std::move(o.Oc);
    op = std::move(o.op);
    bucket = std::move(o.bucket);
    if constexpr ( (false) ) { // do not move moments
        std::move(o.moment<0>().dead_begin(), o.moment<0>().dead_end(), moment<0>().dead_begin());
        std::move(o.moment<1>().dead_begin(), o.moment<1>().dead_end(), moment<1>().dead_begin());
        std::move(o.moment<2>().dead_begin(), o.moment<2>().dead_end(), moment<2>().dead_begin());
    }
    return *this;
}
