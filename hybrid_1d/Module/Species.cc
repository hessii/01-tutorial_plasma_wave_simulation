//
//  Species.c
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

using Shape = H1D::Shape<H1D::Input::shape_order>;
namespace {
    template <class T>
    H1D::GridQ<T> &operator/=(H1D::GridQ<T> &G, T const w) noexcept {
        for (auto it = G.dead_begin(), end = G.dead_end(); it != end; ++it) {
            *it /= w;
        }
        return G;
    }
}

bool H1D::Species::_update_position(decltype(_Species::bucket) &bucket, Real const dtODx, Real const travel_scale_factor)
{
    bool did_not_move_too_far = true;
    for (Particle &ptl : bucket) {
        Real moved_x = ptl.vel.x*dtODx;
        ptl.pos_x += moved_x; // position is normalized by D

        // travel distance check
        //
        moved_x *= travel_scale_factor;
        did_not_move_too_far &= 0 == long(moved_x);
    }
    return did_not_move_too_far;
}

void H1D::Species::_update_velocity(decltype(_Species::bucket) &bucket, BField const &B, Real const dtOc_2O0, EField const &E, Real const cDtOc_2O0)
{
    ::Shape sx;
    for (Particle &ptl : bucket) {
        sx(ptl.pos_x); // ptl.pos is already divided by D
        Vector Bi = B.interp(sx);
        Vector Ei = E.interp(sx);
        boris_push(ptl.vel, Bi *= dtOc_2O0, Ei *= cDtOc_2O0);
    }
}

void H1D::Species::_collect_part(GridQ<Scalar> &n, GridQ<Vector> &nV, decltype(_Species::bucket) const &bucket) const
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    //
    ::Shape sx;
    for (Particle const &ptl : bucket) {
        sx(ptl.pos_x); // ptl.pos is already divided by D
        n.deposit(sx, 1);
        nV.deposit(sx, ptl.vel);
    }
    //
    n /= Scalar{Nc};
    nV /= Vector{Nc};
}
void H1D::Species::_collect_all(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv, decltype(_Species::bucket) const &bucket) const
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    nvv.fill(Tensor{0});
    //
    Tensor tmp{0};
    Vector *tmp_hi = reinterpret_cast<Vector *>(&tmp), *tmp_lo = tmp_hi++; // dirty trick
    //
    ::Shape sx;
    for (Particle const &ptl : bucket) {
        sx(ptl.pos_x); // ptl.pos is already divided by D
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
