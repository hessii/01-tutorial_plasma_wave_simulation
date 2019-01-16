//
//  BField.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./BField.h"
#include "./EField.h"
#include <cmath>

H1D::BField::BField() noexcept
: GridQ() {
    using Input::ex_dot_B;
    H1D::Real const ey_dot_B = std::sqrt((1 - ex_dot_B)*(1 + ex_dot_B));
    this->fill(Vector{ex_dot_B, ey_dot_B, 0} *= Input::O0);
}
