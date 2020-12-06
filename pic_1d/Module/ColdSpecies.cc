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

#include <algorithm>

// helpers
//
namespace {
    template <class T, long N>
    auto const &full_grid(P1D::GridQ<T, N> &F, P1D::GridQ<T, N> const &H) noexcept {
        for (long i = -P1D::Pad; i < F.size() + (P1D::Pad - 1); ++i) {
            (F[i] = H[i+1] + H[i+0]) *= 0.5;
        }
        return F;
    }
}

P1D::ColdSpecies::ColdSpecies(ParamSet const &params, ColdPlasmaDesc const &desc)
: Species{params}, desc{desc}
{
}
void P1D::ColdSpecies::populate()
{
    // initialize equilibrium moments
    //
    auto &n  = mom0_half;
    auto &nV = mom1_full;
    //
    constexpr Scalar n0{1};
    Vector const nV0 = Real{n0}*desc.Vd/params.O0*geomtr.B0;
    for (long i = 0; i < nV.size(); ++i) { // only the interior
        n[i] = n0;
        nV[i] = nV0;
    }
}

void P1D::ColdSpecies::update_den(Real const dt)
{
    constexpr Real Dy = 1, Dz = 1;
    Real const Dx = params.Dx, dV = Dx*Dy*Dz;
    _update_n(mom0_half, mom1_full, dt/dV);
}
void P1D::ColdSpecies::_update_n(ScalarGrid &n, VectorGrid const &nV, Real const dtOdV)
{
    constexpr Real Dy = 1, Dz = 1, dAx = Dy*Dz;
    for (long i = 0; i < n.size(); ++i) {
        Real const dnx = -dtOdV*(nV[i-0].x - nV[i-1].x)*dAx;
        Real const dny = -0;
        Real const dnz = -0;
        n[i] += dnx + dny + dnz;
        n[i] *= (Real{n[i]} >= 0);
    }
}

void P1D::ColdSpecies::update_vel(BField const &bfield, EField const &efield, Real const dt)
{
    _update_nV(mom1_full, BorisPush{dt, params.c, params.O0, desc.Oc},
               full_grid(moment<0>(), mom0_half),
               full_grid(moment<1>(), bfield), efield);
}
void P1D::ColdSpecies::_update_nV(VectorGrid &nV, BorisPush const pusher, ScalarGrid const &n, VectorGrid const &B, EField const &E)
{
    for (long i = 0; i < nV.size(); ++i) {
        pusher(nV[i], B[i], E[i]*Real{n[i]});
    }
}

void P1D::ColdSpecies::collect_part() {
    _collect_part(moment<0>(), moment<1>());
}
void P1D::ColdSpecies::collect_all()
{
    _collect_part(moment<0>(), moment<1>());
    _collect_nvv(moment<2>(), moment<0>(), moment<1>());
}
void P1D::ColdSpecies::_collect_part(ScalarGrid &n, VectorGrid &nV) const
{
    // must zero-out ghost cells
    //
    full_grid(n, mom0_half);
    std::fill(n.dead_begin(), n.begin()   , Scalar{});
    std::fill(n.end()       , n.dead_end(), Scalar{});
    //
    nV.fill(Vector{});
    std::copy(mom1_full.begin(), mom1_full.end(), nV.begin());
}
void P1D::ColdSpecies::_collect_nvv(TensorGrid &nvv, ScalarGrid const &n, VectorGrid const &nV)
{
    for (long i = 0; i < nV.size(); ++i) {
        Tensor &nvvi = nvv[i];
        Vector const &nVi = nV[i];
        //
        nvvi.hi() = nvvi.lo() = nVi/Real{n[i]}; // fill diag and off-diag terms with flow velocity
        nvvi.lo() *= nVi; // diagonal terms
        nvvi.hi() *= {nVi.y, nVi.z, nVi.x}; // off-diag terms
    }
}
