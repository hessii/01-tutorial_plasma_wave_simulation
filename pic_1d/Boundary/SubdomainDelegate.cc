//
//  SubdomainDelegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 12/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "SubdomainDelegate.h"
#include "../InputWrapper.h"

#include <functional>
#include <algorithm>
#include <iterator>
#include <utility>
#include <random>

P1D::SubdomainDelegate::message_dispatch_t P1D::SubdomainDelegate::dispatch;
P1D::SubdomainDelegate::SubdomainDelegate(unsigned const rank, unsigned const size) noexcept
: comm{dispatch.comm(rank)}, size{size}
, left_{(size + rank - 1)%size}
, right{(size + rank + 1)%size} {
}

// MARK: Interface
//
void P1D::SubdomainDelegate::once(Domain &domain) const
{
    std::mt19937 g{123 + static_cast<unsigned>(comm.rank())};
    std::uniform_real_distribution<> d{-1, 1};
    for (Vector &v : domain.efield) {
        v.x += d(g) * Debug::initial_efield_noise_amplitude;
        v.y += d(g) * Debug::initial_efield_noise_amplitude;
        v.z += d(g) * Debug::initial_efield_noise_amplitude;
    }
}

void P1D::SubdomainDelegate::pass(Domain const &domain, PartBucket &L_bucket, PartBucket &R_bucket) const
{
    // send boundaries
    //
    std::map<unsigned, PartBucket> payloads;
    payloads.try_emplace(left_, std::move(L_bucket));
    payloads.try_emplace(right, std::move(R_bucket));
    auto tks = comm.scatter(std::move(payloads));

    // recv boundaries
    // should not use gather because return package order can change
    //
    L_bucket = comm.recv<PartBucket>(right);
    R_bucket = comm.recv<PartBucket>(left_);

    // wait for delievery
    //
    //std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)), std::mem_fn(&ticket_t::wait));

    // adjust coordinates
    //
    Delegate::pass(domain, L_bucket, R_bucket);
}
void P1D::SubdomainDelegate::pass(Domain const&, BField &bfield) const
{
    if constexpr (Debug::zero_out_electromagnetic_field) {
        bfield.fill(bfield.geomtr.B0);
    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
        for (Vector &v : bfield) {
            v.y = bfield.geomtr.B0.y;
            v.z = bfield.geomtr.B0.z;
        }
    }
    pass(bfield);
}
void P1D::SubdomainDelegate::pass(Domain const&, EField &efield) const
{
    if constexpr (Debug::zero_out_electromagnetic_field) {
        efield.fill(Vector{});
    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
        for (Vector &v : efield) {
            v.y = v.z = 0;
        }
    }
    pass(efield);
}
void P1D::SubdomainDelegate::pass(Domain const&, Current &current) const
{
    pass(current);
}
void P1D::SubdomainDelegate::gather(Domain const&, Current &current) const
{
    gather(current);
}
void P1D::SubdomainDelegate::gather(Domain const&, PartSpecies &sp) const
{
    gather(sp.moment<0>());
    gather(sp.moment<1>());
    gather(sp.moment<2>());
}

template <class T, long N>
void P1D::SubdomainDelegate::pass(GridQ<T, N> &grid) const
{
    // from inside out
    //
    std::map<unsigned, T const*> payloads;
    payloads.try_emplace(left_, grid.begin());
    payloads.try_emplace(right, grid.end());
    auto tks = comm.scatter(std::move(payloads));
    //
    comm.recv<T const*>(right).unpack([](T const* right, T *last) {
        for (long i = 0; i < Pad; ++i) {
            last[i] = right[i];
        }
    }, grid.end());
    //
    comm.recv<T const*>(left_).unpack([](T const* left, T *first) {
        for (long i = -1; i >= -Pad; --i) {
            first[i] = left[i];
        }
    }, grid.begin());
    //
    std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)),
                  std::mem_fn(&ticket_t::wait));
}
template <class T, long N>
void P1D::SubdomainDelegate::gather(GridQ<T, N> &grid) const
{
    // from outside in
    //
    std::map<unsigned, T const*> payloads;
    payloads.try_emplace(left_, grid.begin());
    payloads.try_emplace(right, grid.end());
    auto tks = comm.scatter(std::move(payloads));
    //
    comm.recv<T const*>(right).unpack([](T const* right, T *last) {
        for (long i = -Pad; i < 0; ++i) {
            last[i] += right[i];
        }
    }, grid.end());
    //
    comm.recv<T const*>(left_).unpack([](T const* left, T *first) {
        for (long i = Pad - 1; i >= 0; --i) {
            first[i] += left[i];
        }
    }, grid.begin());
    //
    std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)),
                  std::mem_fn(&ticket_t::wait));
}
