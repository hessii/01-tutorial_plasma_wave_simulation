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

auto P1D::Species::operator=(Species const &o)
-> Species &{
    std::tie(this->Oc, this->op, this->nu) = std::forward_as_tuple(o.Oc, o.op, o.nu);
    // no moment copy
    return *this;
}
auto P1D::Species::operator=(Species &&o)
-> Species &{
    std::tie(this->Oc, this->op, this->nu) = std::forward_as_tuple(std::move(o.Oc), std::move(o.op), std::move(o.nu));
    // no moment move
    return *this;
}
