//
//  VDF.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "VDF.h"
#include "../InputWrapper.h"

#include <cmath>
#include <random>

P1D::Real P1D::VDF::uniform_real() noexcept
{
    constexpr Real eps = 1e-15;
    static std::mt19937 gen{100};
    static std::uniform_real_distribution uniform{eps, 1 - eps};
    return uniform(gen);
}

P1D::Vector const P1D::VDF::e3 = {0, 0, 1};
P1D::Vector const P1D::VDF::e1 = []{
    constexpr Real theta = Input::theta*M_PI/180;
    return Vector{std::cos(theta), std::sin(theta), 0};
}();
P1D::Vector const P1D::VDF::e2 = []{
    constexpr Real theta = Input::theta*M_PI/180;
    return Vector{-std::sin(theta), std::cos(theta), 0};
}();
