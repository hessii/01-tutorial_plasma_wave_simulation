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
    H1D::Real const theta = Input::theta*M_PI/180; // degree to radian
    this->fill(Vector{std::cos(theta), std::sin(theta), 0} *= Input::O0);
}
