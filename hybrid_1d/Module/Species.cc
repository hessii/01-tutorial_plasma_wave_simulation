//
//  Species.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./Species.h"
#include "./EField.h"
#include "./BField.h"
#include "../Utility/ParticlePush.h"
#include "../Inputs.h"

#include <stdexcept>

// helpers
//
namespace {
    using Shape = H1D::Shape<H1D::Input::shape_order>;
    //
    template <class T>
    auto &operator/=(H1D::GridQ<T> &G, T const w) noexcept {
        for (auto it = G.dead_begin(), end = G.dead_end(); it != end; ++it) {
            *it /= w;
        }
        return G;
    }
    //
    template <class T>
    auto const &full_grid(H1D::GridQ<T> &F, H1D::GridQ<T> const &H) noexcept {
        for (long i = -H1D::Pad + 1; i < F.size() + H1D::Pad; ++i) {
            (F[i] = H[i-0] + H[i-1]) /= 2;
        }
        return F;
    }
}

H1D::Species::Species(Real const Oc, Real const op, long const Nc)
: _Species(Oc, op, Nc) {
    // TODO: VDF initialization.
}

// update & collect interface
//
void H1D::Species::update_vel(BField const &bfield, EField const &efield, Real const dt)
{
    Real const dtOc_2O0 = Oc/Input::O0*(dt/2.0), cDtOc_2O0 = Input::c*dtOc_2O0;
    auto const &full_E = full_grid(moment<1>(), efield); // use 1st moment as a temporary holder for E field at full grid
    _update_velocity(bucket, bfield, dtOc_2O0, full_E, cDtOc_2O0);
}
void H1D::Species::update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel)
{
    Real const dtODx = dt/Input::Dx; // normalize position by grid size
    if (!_update_position(bucket, dtODx, 1.0/fraction_of_grid_size_allowed_to_travel)) {
        throw std::domain_error(std::string(__PRETTY_FUNCTION__) + " - particle(s) moved too far");
    }
}
void H1D::Species::collect_part()
{
    _collect_part(moment<0>(), moment<1>());
}
void H1D::Species::collect_all()
{
    _collect_all(moment<0>(), moment<1>(), moment<2>());
}

// heavy lifting
//
bool H1D::Species::_update_position(decltype(_Species::bucket) &bucket, Real const dtODx, Real const travel_scale_factor)
{
    bool did_not_move_too_far = true;
    for (Particle &ptl : bucket) {
        Real moved_x = ptl.vel.x*dtODx;
        ptl.pos_x += moved_x; // position is normalized by grid size

        // travel distance check
        //
        moved_x *= travel_scale_factor;
        did_not_move_too_far &= 0 == long(moved_x);
    }
    return did_not_move_too_far;
}

void H1D::Species::_update_velocity(decltype(_Species::bucket) &bucket, BField const &B, Real const dtOc_2O0, GridQ<Vector> const &E, Real const cDtOc_2O0)
{
    ::Shape sx;
    for (Particle &ptl : bucket) {
        sx(ptl.pos_x); // position is normalized by grid size
        H1D::boris_push(ptl.vel, B.interp(sx) *= dtOc_2O0, E.interp(sx) *= cDtOc_2O0);
    }
}

void H1D::Species::_collect_part(GridQ<Scalar> &n, GridQ<Vector> &nV) const
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    //
    ::Shape sx;
    for (Particle const &ptl : bucket) {
        sx(ptl.pos_x); // position is normalized by grid size
        n.deposit(sx, 1);
        nV.deposit(sx, ptl.vel);
    }
    //
    n /= Scalar{Nc};
    nV /= Vector{Nc};
}
void H1D::Species::_collect_all(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv) const
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    nvv.fill(Tensor{0});
    //
    Tensor tmp{0};
    Vector *tmp_hi = reinterpret_cast<Vector *>(&tmp), *tmp_lo = tmp_hi++; // dirty shortcut
    static_assert(alignof(Tensor) == alignof(Vector) && sizeof(Tensor) == 2*sizeof(Vector), "incompatible layout");
    //
    ::Shape sx;
    for (Particle const &ptl : bucket) {
        sx(ptl.pos_x); // position is normalized by grid size
        n.deposit(sx, 1);
        nV.deposit(sx, ptl.vel);
        *tmp_hi = *tmp_lo = ptl.vel;
        *tmp_lo *= ptl.vel; // diagonal part; {vx*vx, vy*vy, vz*vz}
        *tmp_hi *= {ptl.vel.y, ptl.vel.z, ptl.vel.x}; // off-diag part; {vx*vy, vy*vz, vz*vx}
        nvv.deposit(sx, tmp);
    }
    //
    n /= Scalar{Nc};
    nV /= Vector{Nc};
    nvv /= Tensor{Nc};
}
