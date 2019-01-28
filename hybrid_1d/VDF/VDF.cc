//
//  VDF.c
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "VDF.h"

#include <random>

H1D::Real H1D::VDF::uniform_real() noexcept
{
    constexpr Real eps = 1e-15;
    static std::mt19937 gen{100};
    static std::uniform_real_distribution<> uniform{eps, 1 - eps};
    return uniform(gen);
}
