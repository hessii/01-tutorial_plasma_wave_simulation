//
//  EField.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./EField.h"
#include "./BField.h"
#include "./Charge.h"
#include "./Current.h"
#include "../Inputs.h"

#include <cmath>

void H1D::EField::update(BField const &bfield, Charge const &charge, Current const &current) noexcept
{
    _update_Pe(Pe, charge);
    _update_Je(Je, current, bfield);
    _update_E(*this, bfield, charge);
}

void H1D::EField::_update_Pe(decltype(Pe) &Pe, Charge const &rho) noexcept
{
    namespace eFluid = Input::eFluid;
    //
    using Input::O0;
    Real const O02beO2 = (O0*O0)*eFluid::beta*0.5;
    Real const mOeOO0oe2 = -eFluid::Oc/(O0*(eFluid::op*eFluid::op));
    Real const gamma = Real{eFluid::closure/10}/(eFluid::closure%10);
    for (long i = -Pad; i < Pe.size() + Pad; ++i) {
        Pe[i] = std::pow(mOeOO0oe2*Real{rho[i]}, gamma) * O02beO2;
    }
}
void H1D::EField::_update_Je(decltype(Je) &Je, Current const &Ji, BField const &B) noexcept
{
    Real const cODx = Input::c/Input::Dx;
    for (long i = 0; i < B.size(); ++i) {
        // J total
        //
        Je[i].x = 0;
        Je[i].y = (-B[i+1].z +B[i+0].z)*cODx;
        Je[i].z = (+B[i+1].y -B[i+0].y)*cODx;

        // Je = J - Ji
        //
        Je[i] -= (Ji[i+1] + Ji[i+0])*0.5;
    }
}
void H1D::EField::_update_E(EField &E, BField const &B, Charge const &rho) const noexcept
{
    Real const cODx = Input::c/Input::Dx;
    for (long i = 0; i < E.size(); ++i) {
        Vector &Ei = E[i];

        // 1. Je x B term
        //
        Ei = cross(Je[i], (B[i+1] + B[i+0])*0.5);

        // 2. pressure gradient term
        //
        Ei.x -= Real{Pe[i+1] -Pe[i+0]}*cODx;
        Ei.y -= 0;
        Ei.z -= 0;

        // 3. divide by charge density
        //
        Ei /= Real{rho[i+1] + rho[i+0]}*0.5;
    }
}
