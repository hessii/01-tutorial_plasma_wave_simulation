//
//  PartSpecies.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./PartSpecies.h"
#include "./EField.h"
#include "./BField.h"
#include "../Utility/ParticlePush.h"
#include "../InputWrapper.h"

#include <stdexcept>
#include <utility>

// helpers
//
namespace {
    using Shape = P1D::Shape<P1D::Input::shape_order>;
    //
    template <class T>
    auto &operator/=(P1D::GridQ<T> &G, T const w) noexcept {
        for (auto it = G.dead_begin(), end = G.dead_end(); it != end; ++it) {
            *it /= w;
        }
        return G;
    }
    //
    template <class T>
    auto const &full_grid(P1D::GridQ<T> &F, P1D::BField const &H) noexcept {
        for (long i = -P1D::Pad; i < F.size() + (P1D::Pad - 1); ++i) {
            (F[i] = H[i+1] + H[i+0]) *= 0.5;
        }
        return F;
    }
    //
    template <class T>
    auto &operator+=(P1D::GridQ<T> &lhs, P1D::GridQ<T> const &rhs) noexcept {
        auto lhs_first = lhs.dead_begin(), lhs_last = lhs.dead_end();
        auto rhs_first = rhs.dead_begin();
        while (lhs_first != lhs_last) {
            *lhs_first++ += *rhs_first++;
        }
        return lhs;
    }
}

// assignment
//
auto P1D::PartSpecies::operator=(PartSpecies const& o)
-> PartSpecies &{
    Species::operator=(o);
    std::tie(this->Nc, this->bucket) = std::forward_as_tuple(o.Nc, o.bucket);
    return *this;
}
auto P1D::PartSpecies::operator=(PartSpecies&& o)
-> PartSpecies &{
    Species::operator=(std::move(o));
    std::tie(this->Nc, this->bucket) = std::forward_as_tuple(std::move(o.Nc), std::move(o.bucket));
    return *this;
}

// constructor
//
P1D::PartSpecies::PartSpecies(Real const Oc, Real const op, long const Nc, VDF const &vdf)
: Species{Oc, op}, Nc(Nc), bucket{}
{
    // argument check
    //
    if (Nc < 0) {
        throw std::invalid_argument{std::string{__FUNCTION__} + "negative Nc"};
    }

    // populate particles
    //
    long const Np = Nc*Input::Nx / (Input::number_of_worker_threads + 1);
    //bucket.reserve(static_cast<unsigned long>(Np));
    for (long i = 0; i < Np; ++i) {
        bucket.push_back(vdf.variate());
    }
}

// update & collect interface
//
void P1D::PartSpecies::update_vel(BField const &bfield, EField const &efield, Real const dt)
{
    Real const dtOc_2O0 = Oc/Input::O0*(dt/2.0), cDtOc_2O0 = Input::c*dtOc_2O0;
    auto const &full_B = full_grid(moment<1>(), bfield); // use 1st moment as a temporary holder for B field interpolated at full grid
    _update_velocity(bucket, full_B, dtOc_2O0, efield, cDtOc_2O0);
}
void P1D::PartSpecies::update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel)
{
    Real const dtODx = dt/Input::Dx; // normalize position by grid size
    if (!_update_position(bucket, dtODx, 1.0/fraction_of_grid_size_allowed_to_travel))
    {
        throw std::domain_error{std::string{__FUNCTION__} + " - particle(s) moved too far"};
    }
}
void P1D::PartSpecies::collect_part()
{
    _collect(moment<1>());
}
void P1D::PartSpecies::collect_all()
{
    _collect(moment<0>(), moment<1>(), moment<2>());
}

// heavy lifting
//
bool P1D::PartSpecies::_update_position(bucket_type &bucket, Real const dtODx, Real const travel_distance_scale_factor)
{
    bool did_not_move_too_far = true;
    for (Particle &ptl : bucket)
    {
        Real moved_x = ptl.vel.x*dtODx;
        ptl.pos_x += moved_x; // position is normalized by grid size

        // travel distance check
        //
        moved_x *= travel_distance_scale_factor;
        did_not_move_too_far &= 0 == long(moved_x);
    }
    return did_not_move_too_far;
}

void P1D::PartSpecies::_update_velocity(bucket_type &bucket, GridQ<Vector> const &B, Real const dtOc_2O0, EField const &E, Real const cDtOc_2O0)
{
    ::Shape sx;
    for (Particle &ptl : bucket)
    {
        sx(ptl.pos_x); // position is normalized by grid size
        boris_push(ptl.vel, B.interp(sx) *= dtOc_2O0, E.interp(sx) *= cDtOc_2O0);
    }
}

void P1D::PartSpecies::_collect(GridQ<Vector> &nV) const
{
    nV.fill(Vector{0});
    //
    ::Shape sx;
    for (Particle const &ptl : bucket)
    {
        sx(ptl.pos_x); // position is normalized by grid size
        nV.deposit(sx, ptl.vel);
    }
    //
    Real const Nc = this->Nc == 0.0 ? 1.0 : this->Nc; // avoid division by zero
    nV /= Vector{Nc};
}
void P1D::PartSpecies::_collect(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv) const
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    nvv.fill(Tensor{0});
    //
    Tensor tmp{0};
    ::Shape sx;
    for (Particle const &ptl : bucket)
    {
        sx(ptl.pos_x); // position is normalized by grid size
        n.deposit(sx, 1);
        nV.deposit(sx, ptl.vel);
        tmp.hi() = tmp.lo() = ptl.vel;
        tmp.lo() *= ptl.vel;                           // diagonal part; {vx*vx, vy*vy, vz*vz}
        tmp.hi() *= {ptl.vel.y, ptl.vel.z, ptl.vel.x}; // off-diag part; {vx*vy, vy*vz, vz*vx}
        nvv.deposit(sx, tmp);
    }
    //
    Real const Nc = this->Nc == 0.0 ? 1.0 : this->Nc; // avoid division by zero
    n /= Scalar{Nc};
    nV /= Vector{Nc};
    nvv /= Tensor{Nc};
}
