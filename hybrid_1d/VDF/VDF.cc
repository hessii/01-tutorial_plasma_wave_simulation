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
    static std::mt19937 gen{100};
    static std::uniform_real_distribution<> uniform{0, 1};
    return uniform(gen);
}
