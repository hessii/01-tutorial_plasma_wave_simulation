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

#include <stdexcept>
#include <utility>

// helpers
//
namespace {
    using Shape = P1D::Shape<P1D::Input::shape_order>;
    //
    template <class T>
    auto &operator/=(P1D::GridQ<T> &G, T const w) noexcept { // include padding
        for (auto it = G.dead_begin(), end = G.dead_end(); it != end; ++it) {
            *it /= w;
        }
        return G;
    }
    template <class T>
    auto &operator+=(P1D::GridQ<T> &G, T const w) noexcept { // exclude padding
        for (auto it = G.begin(), end = G.end(); it != end; ++it) {
            *it += w;
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
}

// assignment
//
auto P1D::PartSpecies::operator=(PartSpecies const& o)
-> PartSpecies &{
    Species::operator=(o);
    std::tie(this->Nc, this->bucket, this->vdf, this->scheme) = std::forward_as_tuple(o.Nc, o.bucket, o.vdf, o.scheme);
    return *this;
}
auto P1D::PartSpecies::operator=(PartSpecies&& o)
-> PartSpecies &{
    Species::operator=(std::move(o));
    std::tie(this->Nc, this->bucket, this->vdf, this->scheme) = std::forward_as_tuple(std::move(o.Nc), std::move(o.bucket), std::move(o.vdf), std::move(o.scheme));
    return *this;
}

// constructor
//
void P1D::PartSpecies::populate_bucket(bucket_type &bucket, long const Nc) const
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
        bucket.emplace_back(vdf->variate()).w = scheme == full_f;
    }
}

// update & collect interface
//
void P1D::PartSpecies::update_vel(BField const &bfield, EField const &efield, Real const dt)
{
    if (param.nu > 0) {
        _update_v(bucket, full_grid(moment<1>(), bfield), efield,
                  param.nu, BorisPush{dt, Input::c, Input::O0, param.Oc});
    } else {
        _update_v(bucket, full_grid(moment<1>(), bfield), efield,
                  BorisPush{dt, Input::c, Input::O0, param.Oc});
    }
}
void P1D::PartSpecies::update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel)
{
    Real const dtODx = dt/Input::Dx; // normalize position by grid size
    if (!_update_x(bucket, dtODx, 1.0/fraction_of_grid_size_allowed_to_travel))
    {
        throw std::domain_error{std::string{__FUNCTION__} + " - particle(s) moved too far"};
    }
}
void P1D::PartSpecies::collect_part()
{
    switch (scheme) {
        case full_f:
            _collect_full_f(moment<1>());
            break;
        case delta_f:
            _collect_delta_f(moment<1>(), bucket);
            break;
    }
}
void P1D::PartSpecies::collect_all()
{
    _collect(moment<0>(), moment<1>(), moment<2>());
}

// heavy lifting
//
bool P1D::PartSpecies::_update_x(bucket_type &bucket, Real const dtODx, Real const travel_distance_scale_factor)
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

void P1D::PartSpecies::_update_v(bucket_type &bucket, GridQ<Vector> const &B, EField const &E, BorisPush const pusher)
{
    ::Shape sx;
    for (Particle &ptl : bucket)
    {
        sx(ptl.pos_x); // position is normalized by grid size
        pusher(ptl.vel, B.interp(sx), E.interp(sx));
    }
}
void P1D::PartSpecies::_update_v(bucket_type &bucket, GridQ<Vector> const &B, EField const &E, Real const nu, BorisPush const pusher)
{
    ::Shape sx;
    for (Particle &ptl : bucket)
    {
        sx(ptl.pos_x); // position is normalized by grid size
        pusher(ptl.vel, B.interp(sx), E.interp(sx), nu);
    }
}

void P1D::PartSpecies::_collect_full_f(GridQ<Vector> &nV) const
{
    nV.fill(Vector{0});
    //
    ::Shape sx;
    for (Particle const &ptl : bucket) {
        sx(ptl.pos_x); // position is normalized by grid size
        nV.deposit(sx, ptl.vel);
    }
    //
    Real const Nc = this->Nc == 0.0 ? 1.0 : this->Nc; // avoid division by zero
    nV /= Vector{Nc};
}
void P1D::PartSpecies::_collect_delta_f(GridQ<Vector> &nV, bucket_type &bucket) const
{
    VDF const &vdf = *this->vdf;
    //
    nV.fill(Vector{0});
    //
    ::Shape sx;
    for (Particle &ptl : bucket) {
        sx(ptl.pos_x); // position is normalized by grid size
        ptl.w = vdf.weight(ptl);
        nV.deposit(sx, ptl.vel*ptl.w);
    }
    //
    Real const Nc = this->Nc == 0.0 ? 1.0 : this->Nc; // avoid division by zero
    (nV /= Vector{Nc}) += vdf.nV0(Particle::quiet_nan)*scheme;
}
void P1D::PartSpecies::_collect(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv) const
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    nvv.fill(Tensor{0});
    //
    Tensor tmp{0};
    ::Shape sx;
    for (Particle const &ptl : bucket) {
        sx(ptl.pos_x); // position is normalized by grid size
        n.deposit(sx, ptl.w);
        nV.deposit(sx, ptl.vel*ptl.w);
        tmp.hi() = tmp.lo() = ptl.vel;
        tmp.lo() *= ptl.vel;                           // diagonal part; {vx*vx, vy*vy, vz*vz}
        tmp.hi() *= {ptl.vel.y, ptl.vel.z, ptl.vel.x}; // off-diag part; {vx*vy, vy*vz, vz*vx}
        nvv.deposit(sx, tmp *= ptl.w);
    }
    //
    Real const Nc = this->Nc == 0.0 ? 1.0 : this->Nc; // avoid division by zero
    VDF const &vdf = *this->vdf;
    (n /= Scalar{Nc}) += vdf.n0(Particle::quiet_nan)*scheme;
    (nV /= Vector{Nc}) += vdf.nV0(Particle::quiet_nan)*scheme;
    (nvv /= Tensor{Nc}) += vdf.nvv0(Particle::quiet_nan)*scheme;
}
