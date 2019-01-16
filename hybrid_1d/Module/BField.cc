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
    Vector B{Global::ex_dot_B, std::sqrt((1 - Global::ex_dot_B)*(1 + Global::ex_dot_B)), 0};
    this->fill(B *= Global::O0);
}
