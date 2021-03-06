//
//  BField.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "BField.h"
#include "./EField.h"

P1D::BField::BField(ParamSet const &params)
: GridQ{}, params{params}, geomtr{params} {
    this->fill(geomtr.B0); // fill with background B
}

void P1D::BField::update(EField const &efield, Real const dt) noexcept
{
    Real const cdtODx = dt*params.c/params.Dx;
    _update(*this, efield, cdtODx);
}
void P1D::BField::_update(BField &B, EField const &E, Real const cdtODx) noexcept
{
    for (long i = 0; i < E.size(); ++i) {
        B[i].x += 0;
        B[i].y += (+E[i-0].z -E[i-1].z) * cdtODx;
        B[i].z += (-E[i-0].y +E[i-1].y) * cdtODx;
    }
}
