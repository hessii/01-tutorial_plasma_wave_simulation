//
//  WorkerDelegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "WorkerDelegate.h"
#include "./MasterDelegate.h"

#include <algorithm>

void P1D::WorkerDelegate::setup(Domain &domain)
{
    // distribute particles to workers
    //
    for (PartSpecies &sp : domain.part_species) {
        sp.Nc /= ParamSet::number_of_particle_parallelism;
        sp.bucket = comm.recv<PartBucket>(master->comm.rank());
    }

    // broadcast cold fluid moments to workers
    // broadcasting fields should be done during pass
    //
    for (ColdSpecies &sp : domain.cold_species) {
        recv_from_master(sp.moment<0>());
        recv_from_master(sp.moment<1>());
        recv_from_master(sp.moment<2>());
    }
}
void P1D::WorkerDelegate::teardown(Domain &domain)
{
    // collect particles to master
    //
    for (PartSpecies &sp : domain.part_species) {
        auto tk = comm.send(std::move(sp.bucket), master->comm.rank()); //.wait();
    }
}

void P1D::WorkerDelegate::prologue(Domain const& domain, long const i) const
{
    master->delegate->prologue(domain, i);
}
void P1D::WorkerDelegate::epilogue(Domain const& domain, long const i) const
{
    master->delegate->epilogue(domain, i);
}
void P1D::WorkerDelegate::once(Domain &domain) const
{
    master->delegate->once(domain);
}
void P1D::WorkerDelegate::pass(Domain const&, PartSpecies &sp) const
{
    PartBucket L, R;
    master->delegate->partition(sp, L, R);
    //
    comm.recv<0>(master->comm.rank()).unpack([&L, &R](auto payload) {
        payload.first ->swap(L);
        payload.second->swap(R);
    });
    //
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}
void P1D::WorkerDelegate::pass(Domain const&, BField &bfield) const
{
    recv_from_master(bfield);
}
void P1D::WorkerDelegate::pass(Domain const&, EField &efield) const
{
    recv_from_master(efield);
}
void P1D::WorkerDelegate::pass(Domain const&, Current &current) const
{
    recv_from_master(current);
}
void P1D::WorkerDelegate::gather(Domain const&, Current &current) const
{
    reduce_to_master(current);
    recv_from_master(current);
}
void P1D::WorkerDelegate::gather(Domain const&, PartSpecies &sp) const
{
    {
        reduce_to_master(sp.moment<0>());
        reduce_to_master(sp.moment<1>());
        reduce_to_master(sp.moment<2>());
    }
    {
        recv_from_master(sp.moment<0>());
        recv_from_master(sp.moment<1>());
        recv_from_master(sp.moment<2>());
    }
}

template <class T, long N>
void P1D::WorkerDelegate::recv_from_master(GridQ<T, N> &buffer) const
{
    comm.recv<GridQ<T, N> const*>(master->comm.rank()).unpack([&buffer](auto payload) {
        std::copy(payload->dead_begin(), payload->dead_end(), buffer.dead_begin());
    });
}
template <class T, long N>
void P1D::WorkerDelegate::reduce_to_master(GridQ<T, N> &payload) const
{
    reduce_divide_and_conquer(payload);
    accumulate_by_worker(payload);
}
template <class T, long N>
void P1D::WorkerDelegate::reduce_divide_and_conquer(GridQ<T, N> &payload) const
{
    // e.g., assume 9 worker threads (arrow indicating where data are accumulated)
    // stride = 1: [0 <- 1], [2 <- 3], [4 <- 5], [6 <- 7], 8
    // stride = 2: [0 <- 2], [4 <- 6], 8
    // stride = 4: [0 <- 4], 8
    // stride = 8: [0 <- 8]
    //
    int const rank = comm.rank();
    int const n_workers = master->comm.rank();
    for (int stride = 1; stride < n_workers; stride *= 2) {
        int const divisor = stride * 2;
        if (rank % divisor == 0 && rank + stride < n_workers) {
            master->dispatch.send(&payload, {-1, rank + stride}).wait();
        }
    }
}
namespace {
    template <class T, long N>
    auto &operator+=(P1D::GridQ<T, N> &lhs, P1D::GridQ<T, N> const &rhs) noexcept {
        auto lhs_first = lhs.dead_begin(), lhs_last = lhs.dead_end();
        auto rhs_first = rhs.dead_begin();
        while (lhs_first != lhs_last) {
            *lhs_first++ += *rhs_first++;
        }
        return lhs;
    }
}
template <class T, long N>
void P1D::WorkerDelegate::accumulate_by_worker(GridQ<T, N> const &payload) const
{
    master->dispatch.recv<GridQ<T, N>*>({-1, comm.rank()}).unpack([&payload](auto buffer) {
        *buffer += payload;
    });
}
