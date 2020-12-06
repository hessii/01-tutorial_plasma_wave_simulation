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
#include <stdexcept>

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
    constexpr bool enable = false;
    constexpr Real Dy = 1, Dz = 1;
    Real const Dx = params.Dx, dV = Dx*Dy*Dz;
    _update_n(mom0_half, mom1_full, dt/dV * enable);
}
void P1D::ColdSpecies::_update_n(ScalarGrid &n, VectorGrid const &nV, Real const dtOdV)
{
    constexpr Real Dy = 1, Dz = 1, dAx = Dy*Dz;
    for (long i = 0; i < n.size(); ++i) {
        n[i] -= dtOdV*(nV[i-0].x - nV[i-1].x)*dAx;
        if (Real{n[i]} < 0) {
            throw std::runtime_error{std::string{__FUNCTION__} + " - negative density"};
        }
    }
}

void P1D::ColdSpecies::update_vel(BField const &bfield, EField const &efield, Real const dt)
{
    moment<1>().fill(bfield.geomtr.B0);
    _update_nV(mom1_full, vect_buff = mom1_full,
               BorisPush{dt, params.c, params.O0, desc.Oc},
               full_grid(moment<0>(), mom0_half),
               moment<1>(), efield); // full_grid(moment<1>(), bfield), efield);
}
void P1D::ColdSpecies::_update_nV(VectorGrid &new_nV, VectorGrid &old_nV, BorisPush const pusher, ScalarGrid const &n, VectorGrid const &B, EField const &E) const
{
    // Lorentz force
    //
    for (long i = 0 - 1; i < new_nV.size() + 1; ++i) {
        pusher(new_nV[i], B[i], E[i]*Real{n[i]});
        (old_nV[i] += new_nV[i]) *= 0.5;
    }
    //
    // div nVV
    //
    for (long i = 0; i < new_nV.size(); ++i) {
        auto const &nV = old_nV;
        Vector const div_nVV = Vector{
            nV[i+1].x*nV[i+1].x/Real{n[i+1]} - nV[i-1].x*nV[i-1].x/Real{n[i-1]},
            nV[i+1].x*nV[i+1].y/Real{n[i+1]} - nV[i-1].x*nV[i-1].y/Real{n[i-1]},
            nV[i+1].x*nV[i+1].z/Real{n[i+1]} - nV[i-1].x*nV[i-1].z/Real{n[i-1]}
        }/(2*params.Dx);
        new_nV[i] -= 2*pusher.dt_2*div_nVV;
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
