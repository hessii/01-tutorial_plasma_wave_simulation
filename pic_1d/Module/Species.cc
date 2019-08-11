//
//  Species.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Species.h"

#include <algorithm>
#include <utility>

P1D::Species &P1D::Species::operator=(Species const &o)
{
    std::tie(this->Oc, this->op) = std::forward_as_tuple(o.Oc, o.op);
    {
        std::copy(o.moment<0>().dead_begin(), o.moment<0>().dead_end(), moment<0>().dead_begin());
        std::copy(o.moment<1>().dead_begin(), o.moment<1>().dead_end(), moment<1>().dead_begin());
        std::copy(o.moment<2>().dead_begin(), o.moment<2>().dead_end(), moment<2>().dead_begin());
    }
    return *this;
}
P1D::Species &P1D::Species::operator=(Species &&o)
{
    std::tie(this->Oc, this->op) = std::forward_as_tuple(std::move(o.Oc), std::move(o.op));
    {
        std::move(o.moment<0>().dead_begin(), o.moment<0>().dead_end(), moment<0>().dead_begin());
        std::move(o.moment<1>().dead_begin(), o.moment<1>().dead_end(), moment<1>().dead_begin());
        std::move(o.moment<2>().dead_begin(), o.moment<2>().dead_end(), moment<2>().dead_begin());
    }
    return *this;
}
