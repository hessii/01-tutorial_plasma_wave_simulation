//
//  EField.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./EField.h"
#include "./BField.h"
#include "./Current.h"

P1D::EField::EField(ParamSet const &params)
: GridQ{}, params{params}, geomtr{params} {
}

void P1D::EField::update(BField const &bfield, Current const &current, Real const dt) noexcept
{
    Real const cdtODx = dt*Input::c/Input::Dx;
    _update(*this, bfield, cdtODx, current, dt);
}

void P1D::EField::_update(EField &E, BField const &B, Real const cdtODx, Current const &J, Real const dt) noexcept
{
    for (long i = 0; i < B.size(); ++i) {
        E[i].x += 0;
        E[i].y += (-B[i+1].z +B[i+0].z) * cdtODx;
        E[i].z += (+B[i+1].y -B[i+0].y) * cdtODx;
        //
        E[i] -= J[i] * dt;
    }
}
