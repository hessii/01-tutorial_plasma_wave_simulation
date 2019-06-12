//
//  Species.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./Species.h"
#include "./EField.h"
#include "./BField.h"
#include "../Utility/ParticlePush.h"
#include "../InputWrapper.h"

#include <functional>
#include <stdexcept>
#include <thread>
#include <future>

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
    //
    template <class T>
    H1D::GridQ<T> &operator+=(H1D::GridQ<T> &lhs, H1D::GridQ<T> const &rhs) noexcept {
        auto lhs_first = lhs.dead_begin(), lhs_last = lhs.dead_end();
        auto rhs_first = rhs.dead_begin();
        while (lhs_first != lhs_last) {
            *lhs_first++ += *rhs_first++;
        }
        return lhs;
    }
}

// constructor
//
H1D::Species::Species(Real const Oc, Real const op, long const Nc, VDF const &vdf)
: _Species{Oc, op, Nc} {
    long const Np = Nc*Input::Nx;
    bucket.reserve(static_cast<unsigned long>(Np));
    for (long i = 0; i < Np; ++i) {
        bucket.push_back(vdf.variate());
    }
}

// update & collect interface
//
void H1D::Species::update_vel(BField const &bfield, EField const &efield, Real const dt)
{
    Real const dtOc_2O0 = Oc/Input::O0*(dt/2.0), cDtOc_2O0 = Input::c*dtOc_2O0;
    auto const &full_E = full_grid(moment<1>(), efield); // use 1st moment as a temporary holder for E field interpolated at full grid
    wrapper_update_vel(bucket, bfield, dtOc_2O0, full_E, cDtOc_2O0);
}
void H1D::Species::update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel)
{
    Real const dtODx = dt/Input::Dx; // normalize position by grid size
    if (!wrapper_update_pos(bucket, dtODx, 1.0/fraction_of_grid_size_allowed_to_travel)) {
        throw std::domain_error(std::string{__FUNCTION__} + " - particle(s) moved too far");
    }
}
void H1D::Species::collect_part()
{
    auto const n_moms = _moms.size();
    if (n_moms == 0) {
        throw std::runtime_error(__PRETTY_FUNCTION__);
    } else if (n_moms == 1 || bucket.size() < 5*n_moms) {
        //
        // serial
        //
        _collect_part(moment<0>(), moment<1>(), bucket.cbegin(), bucket.cend(), Nc);
    } else {
        //
        // parallel
        //
        async_collect_part(_moms.data(), _moms.data() + n_moms, bucket.cbegin(), bucket.cend());
    }
}
void H1D::Species::collect_all()
{
    auto const n_moms = _moms.size();
    if (n_moms == 0) {
        throw std::runtime_error(__PRETTY_FUNCTION__);
    } else if (n_moms == 1 || bucket.size() < 5*n_moms) {
        //
        // serial
        //
        _collect_all(moment<0>(), moment<1>(), moment<2>(), bucket.cbegin(), bucket.cend(), Nc);
    } else {
        //
        // parallel
        //
        async_collect_all(_moms.data(), _moms.data() + n_moms, bucket.cbegin(), bucket.cend());
    }
}

// heavy lifting
//
bool H1D::Species::wrapper_update_pos(decltype(_Species::bucket) &bucket, Real const dtODx, Real const travel_scale_factor)
{
    static unsigned const n_threads = std::thread::hardware_concurrency();
    if (!Input::enable_asynchronous_particle_push || n_threads <= 1 || bucket.size() < 5*n_threads) {
        //
        // serial
        //
        return _update_position(bucket.begin(), bucket.end(), dtODx, travel_scale_factor);
    } else {
        //
        // parallel
        //
        // spawn worker threads and assign tasks
        //
        std::vector<std::future<bool>> workers;
        long const chunk_size = static_cast<long>(bucket.size()/n_threads);
        auto iter = bucket.begin();
        for (unsigned i = 1; i < n_threads; ++i, iter += chunk_size) {
            workers.emplace_back(std::async(std::launch::async, &_update_position<decltype(iter)>, iter, iter + chunk_size, dtODx, travel_scale_factor));
        }

        // main thread picks up the rest
        //
        bool result = _update_position(iter, bucket.end(), dtODx, travel_scale_factor);

        // collect results from workers
        //
        for (auto &handle : workers) {
            result &= handle.get();
        }

        return result;
    }
}
template <class It>
bool H1D::Species::_update_position(It first, It last, Real const dtODx, Real const travel_scale_factor)
{
    bool did_not_move_too_far = true;
    while (first != last) {
        Particle &ptl = *first++;

        Real moved_x = ptl.vel.x*dtODx;
        ptl.pos_x += moved_x; // position is normalized by grid size

        // travel distance check
        //
        moved_x *= travel_scale_factor;
        did_not_move_too_far &= 0 == long(moved_x);
    }
    return did_not_move_too_far;
}

void H1D::Species::wrapper_update_vel(decltype(_Species::bucket) &bucket, BField const &B, Real const dtOc_2O0, GridQ<Vector> const &E, Real const cDtOc_2O0)
{
    static unsigned const n_threads = std::thread::hardware_concurrency();
    if (!Input::enable_asynchronous_particle_push || n_threads <= 1 || bucket.size() < 5*n_threads) {
        //
        // serial
        //
        _update_velocity(bucket.begin(), bucket.end(), B, dtOc_2O0, E, cDtOc_2O0);
    } else {
        //
        // parallel
        //
        // spawn worker threads and assign tasks
        //
        std::vector<std::future<void>> workers;
        long const chunk_size = static_cast<long>(bucket.size()/n_threads);
        auto iter = bucket.begin();
        for (unsigned i = 1; i < n_threads; ++i, iter += chunk_size) {
            workers.emplace_back(std::async(std::launch::async, &_update_velocity<decltype(iter)>, iter, iter + chunk_size, std::cref(B), dtOc_2O0, std::cref(E), cDtOc_2O0));
        }

        // main thread picks up the rest
        //
        _update_velocity(iter, bucket.end(), B, dtOc_2O0, E, cDtOc_2O0);

        // wait for workers
        //
        for (auto &handle : workers) {
            handle.wait();
        }
    }
}
template <class It>
void H1D::Species::_update_velocity(It first, It last, BField const &B, Real const dtOc_2O0, GridQ<Vector> const &E, Real const cDtOc_2O0)
{
    ::Shape sx;
    while (first != last) {
        Particle &ptl = *first++;

        sx(ptl.pos_x); // position is normalized by grid size
        boris_push(ptl.vel, B.interp(sx) *= dtOc_2O0, E.interp(sx) *= cDtOc_2O0);
    }
}

template <class It>
void H1D::Species::async_collect_part(MomTuple *mom_first, MomTuple const *mom_last, It ptl_first, It ptl_last) const
{
    long const mom_len = mom_last - mom_first;
    long const ptl_len = ptl_last - ptl_first;
    if (mom_len == 0) { // this (should) never occurs
        throw std::runtime_error(__PRETTY_FUNCTION__);
    } else if (mom_len == 1) { // actual work
        return _collect_part(std::get<0>(*mom_first), std::get<1>(*mom_first), ptl_first, ptl_last, Nc);
    } else { // divide & conquer
        auto mom_mid = mom_first + mom_len/2;
        auto ptl_mid = ptl_first + ptl_len/2;

        // worker thread processes the second half
        //
        auto handle = std::async(std::launch::async, &Species::async_collect_part<It>, this,
                                 mom_mid, mom_last, ptl_mid, ptl_last);

        // this thread processes the first half
        //
        async_collect_part(mom_first, mom_mid, ptl_first, ptl_mid);

        // collect worker's moments
        //
        handle.wait();
        std::get<0>(*mom_first) += std::get<0>(*mom_mid);
        std::get<1>(*mom_first) += std::get<1>(*mom_mid);
    }
}
template <class It>
void H1D::Species::_collect_part(GridQ<Scalar> &n, GridQ<Vector> &nV, It first, It last, Real const Nc)
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    //
    ::Shape sx;
    for (It it = first; it != last; ++it) {
        Particle const &ptl = *it;
        sx(ptl.pos_x); // position is normalized by grid size
        n.deposit(sx, 1);
        nV.deposit(sx, ptl.vel);
    }
    //
    n /= Scalar{Nc};
    nV /= Vector{Nc};
}

template <class It>
void H1D::Species::async_collect_all(MomTuple *mom_first, MomTuple const *mom_last, It ptl_first, It ptl_last) const
{
    long const mom_len = mom_last - mom_first;
    long const ptl_len = ptl_last - ptl_first;
    if (mom_len == 0) { // this (should) never occurs
        throw std::runtime_error(__PRETTY_FUNCTION__);
    } else if (mom_len == 1) { // actual work
        return _collect_all(std::get<0>(*mom_first), std::get<1>(*mom_first), std::get<2>(*mom_first), ptl_first, ptl_last, Nc);
    } else { // divide & conquer
        auto mom_mid = mom_first + mom_len/2;
        auto ptl_mid = ptl_first + ptl_len/2;

        // worker thread processes the second half
        //
        auto handle = std::async(std::launch::async, &Species::async_collect_all<It>, this,
                                 mom_mid, mom_last, ptl_mid, ptl_last);

        // this thread processes the first half
        //
        async_collect_all(mom_first, mom_mid, ptl_first, ptl_mid);

        // collect worker's moments
        //
        handle.wait();
        std::get<0>(*mom_first) += std::get<0>(*mom_mid);
        std::get<1>(*mom_first) += std::get<1>(*mom_mid);
    }
}
template <class It>
void H1D::Species::_collect_all(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv, It first, It last, Real const Nc)
{
    n.fill(Scalar{0});
    nV.fill(Vector{0});
    nvv.fill(Tensor{0});
    Tensor tmp{0};
    ::Shape sx;
    for (It it = first; it != last; ++it) {
        Particle const &ptl = *it;
        sx(ptl.pos_x); // position is normalized by grid size
        n.deposit(sx, 1);
        nV.deposit(sx, ptl.vel);
        tmp.hi() = tmp.lo() = ptl.vel;
        tmp.lo() *= ptl.vel;                           // diagonal part; {vx*vx, vy*vy, vz*vz}
        tmp.hi() *= {ptl.vel.y, ptl.vel.z, ptl.vel.x}; // off-diag part; {vx*vy, vy*vz, vz*vx}
        nvv.deposit(sx, tmp);
    }
    //
    n /= Scalar{Nc};
    nV /= Vector{Nc};
    nvv /= Tensor{Nc};
}
