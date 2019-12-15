//
//  ColdSpecies.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 8/11/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "ColdSpecies.h"
#include "./EField.h"
#include "./BField.h"

P1D::ColdSpecies::ColdSpecies(PlasmaDesc const &desc, Real const Vd)
: Species{}, desc{desc, Vd}
{
    // initialize equilibrium moments
    //
    auto &n = moment<0>();
    auto &nV = moment<1>();
    n.fill(Scalar{});
    nV.fill(Vector{});
    //
    constexpr Scalar n0{1};
    Vector const nV0 = Real{n0}*Vd/Input::O0*BField::B0;
    for (long i = 0; i < Input::Nx; ++i) { // only the interior
        n[i] = n0;
        nV[i] = nV0;
    }
}

void P1D::ColdSpecies::update(EField const &efield, Real const dt)
{
    _update_nV(moment<1>(), moment<0>(), BField::B0, efield,
               desc.nu, BorisPush{dt, Input::c, Input::O0, desc.Oc});
}
void P1D::ColdSpecies::_update_nV(GridQ<Vector> &nV, GridQ<Scalar> const &n0, Vector const B0, EField const &E, Real const nu, BorisPush const pusher)
{
    for (long i = 0; i < Input::Nx; ++i) {
        pusher(nV[i], B0, E[i]*Real{n0[i]}, nu);
    }
}

void P1D::ColdSpecies::collect_all()
{
    _collect_nvv(moment<2>(), moment<0>(), moment<1>());
}
void P1D::ColdSpecies::_collect_nvv(GridQ<Tensor> &nvv, GridQ<Scalar> const &n, GridQ<Vector> const &nV)
{
    for (long i = 0; i < Input::Nx; ++i) {
        Tensor &nvvi = nvv[i];
        Vector const &nVi = nV[i];
        //
        nvvi.hi() = nvvi.lo() = nVi/Real{n[i]}; // fill diag and off-diag terms with flow velocity
        nvvi.lo() *= nVi; // diagonal terms
        nvvi.hi() *= {nVi.y, nVi.z, nVi.x}; // off-diag terms
    }
}
