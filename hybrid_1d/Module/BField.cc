//
//  BField.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./BField.h"
#include "./EField.h"
#include "../Inputs.h"

#include <cmath>

H1D::BField::BField() noexcept
: GridQ() {
    H1D::Real const theta = Input::theta*M_PI/180; // degree to radian
    this->fill(Vector{std::cos(theta), std::sin(theta), 0} *= Input::O0); // fill with background B
}

void H1D::BField::update(EField const &efield, Real const dt) noexcept
{
    Real const cdtODx = dt*Input::c/Input::Dx;
    _update(*this, efield, cdtODx);
}
void H1D::BField::_update(BField &B, EField const &E, Real cdtODx) noexcept
{
    for (long i = 0; i < E.size(); ++i) {
        B[i].x += 0;
        B[i].y += (+E[i-0].z -E[i-1].z) * cdtODx;
        B[i].z += (-E[i-0].y +E[i-1].y) * cdtODx;
    }
}
