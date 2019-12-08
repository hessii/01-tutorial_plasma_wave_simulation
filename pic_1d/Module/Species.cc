//
//  Species.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Species.h"

#include <utility>

auto P1D::Species::operator=(Species const &o)
-> Species &{
    param = o.param;
    // no moment copy
    return *this;
}
auto P1D::Species::operator=(Species &&o)
-> Species &{
    param = std::move(o.param);
    // no moment move
    return *this;
}
