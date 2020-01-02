//
//  WorkerDelegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "WorkerDelegate.h"
#include "MasterDelegate.h"
#include "../Module/Domain.h"

#include <algorithm>

void P1D::WorkerDelegate::once(Domain &domain)
{
    master->delegate->once(domain);

    // zero-out cold fluid plasma frequency to suppress workers' cold fluid contribution
    //
    for (ColdSpecies &sp : domain.cold_species) {
        sp.zero_out_plasma_frequency();
    }
}
#if defined(PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
void P1D::WorkerDelegate::pass(Domain const&, PartSpecies &sp)
{
    PartBucket L, R;
    master->delegate->partition(sp, L, R);
    {
        auto [ticket, payload] = constant_comm.recv<3>(*this);
        payload.first ->swap(L);
        payload.second->swap(R);
    }
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}
void P1D::WorkerDelegate::pass(Domain const&, BField &bfield)
{
    recv_from_master(bfield);
}
void P1D::WorkerDelegate::pass(Domain const&, EField &efield)
{
    recv_from_master(efield);
}
void P1D::WorkerDelegate::pass(Domain const&, Current &current)
{
    recv_from_master(current);
}
#endif
void P1D::WorkerDelegate::gather(Domain const&, Current &current)
{
    reduce_to_master(current);
    recv_from_master(current);
}
void P1D::WorkerDelegate::gather(Domain const&, PartSpecies &sp)
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
void P1D::WorkerDelegate::recv_from_master(GridQ<T, N> &buffer)
{
    auto const [ticket, payload] = constant_comm.recv<GridQ<T, N> const*>(*this);
    std::copy(payload->dead_begin(), payload->dead_end(), buffer.dead_begin());
}
template <class T, long N>
void P1D::WorkerDelegate::reduce_to_master(GridQ<T, N> &payload)
{
    reduce_divide_and_conquer(payload);
    accumulate_by_worker(payload);
}
template <class T, long N>
void P1D::WorkerDelegate::reduce_divide_and_conquer(GridQ<T, N> &payload)
{
    // e.g., assume 9 worker threads (arrow indicating where data are accumulated)
    // stride = 1: [0 <- 1], [2 <- 3], [4 <- 5], [6 <- 7], 8
    // stride = 2: [0 <- 2], [4 <- 6], 8
    // stride = 4: [0 <- 4], 8
    // stride = 8: [0 <- 8]
    //
    long const id = this - master->workers.begin();
    long const n_workers = static_cast<long>(master->workers.size());
    for (long stride = 1; stride < n_workers; stride *= 2) {
        long const divisor = stride * 2;
        if (id % divisor == 0 && id + stride < n_workers) {
            (this + stride)->mutable_comm.send(*this, &payload)();
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
void P1D::WorkerDelegate::accumulate_by_worker(GridQ<T, N> const &payload)
{
    auto [ticket, buffer] = mutable_comm.recv<GridQ<T, N>*>(*this);
    *buffer += payload;
}
