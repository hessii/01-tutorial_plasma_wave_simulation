//
//  VDF.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "VDF.h"
#include "../InputWrapper.h"

#include <cmath>
#include <random>

H1D::Real H1D::VDF::uniform_real() noexcept
{
    constexpr Real eps = 1e-15;
    static std::mt19937 gen{100};
    static std::uniform_real_distribution<> uniform{eps, 1 - eps};
    return uniform(gen);
}

H1D::Vector const H1D::VDF::e3 = {0, 0, 1};
H1D::Vector const H1D::VDF::e1 = []{
    constexpr Real theta = Input::theta*M_PI/180;
    return Vector{std::cos(theta), std::sin(theta), 0};
}();
H1D::Vector const H1D::VDF::e2 = []{
    constexpr Real theta = Input::theta*M_PI/180;
    return Vector{-std::sin(theta), std::cos(theta), 0};
}();
