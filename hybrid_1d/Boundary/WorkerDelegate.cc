//
//  WorkerDelegate.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "WorkerDelegate.h"
#include "MasterDelegate.h"
#include "../Module/BField.h"
#include "../Module/EField.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"

#include <algorithm>

#if defined(HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
void H1D::WorkerDelegate::pass(Domain const&, Species &sp)
{
    std::deque<Particle> L, R;
    master->delegate->partition(sp, L, R);
    {
        auto [ticket, payload] = constant_comm.recv<3>(*this);
        payload.first ->swap(L);
        payload.second->swap(R);
    }
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}
void H1D::WorkerDelegate::pass(Domain const&, BField &bfield)
{
    recv_from_master(bfield);
}
void H1D::WorkerDelegate::pass(Domain const&, EField &efield)
{
    recv_from_master(efield);
}
void H1D::WorkerDelegate::pass(Domain const&, Charge &charge)
{
    recv_from_master(charge);
}
void H1D::WorkerDelegate::pass(Domain const&, Current &current)
{
    recv_from_master(current);
}
#endif
void H1D::WorkerDelegate::gather(Domain const&, Charge &charge)
{
    reduce_to_master(charge);
    recv_from_master(charge);
}
void H1D::WorkerDelegate::gather(Domain const&, Current &current)
{
    reduce_to_master(current);
    recv_from_master(current);
}
void H1D::WorkerDelegate::gather(Domain const&, Species &sp)
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

template <class T>
void H1D::WorkerDelegate::recv_from_master(GridQ<T> &buffer)
{
    auto const [ticket, payload] = constant_comm.recv<GridQ<T> const*>(*this);
    std::copy(payload->dead_begin(), payload->dead_end(), buffer.dead_begin());
}
template <class T>
void H1D::WorkerDelegate::reduce_to_master(GridQ<T> &payload)
{
    reduce_divide_and_conquer(payload);
    accumulate_by_worker(payload);
}
template <class T>
void H1D::WorkerDelegate::reduce_divide_and_conquer(GridQ<T> &payload)
{
    // e.g., assume 9 worker threads (arrow indicating where data are accumulated)
    // stride = 1: [0 <- 1], [2 <- 3], [4 <- 5], [6 <- 7], 8
    // stride = 2: [0 <- 2], [4 <- 6], 8
    // stride = 4: [0 <- 4], 8
    // stride = 8: [0 <- 8]
    //
    long const id = this - master->workers.begin();
    for (long stride = 1; stride < Input::number_of_worker_threads; stride *= 2) {
        long const divisor = stride * 2;
        if (id % divisor == 0 && id + stride < Input::number_of_worker_threads) {
            (this + stride)->mutable_comm.send(*this, &payload)();
        }
    }
}
namespace {
    template <class T>
    auto &operator+=(H1D::GridQ<T> &lhs, H1D::GridQ<T> const &rhs) noexcept {
        auto lhs_first = lhs.dead_begin(), lhs_last = lhs.dead_end();
        auto rhs_first = rhs.dead_begin();
        while (lhs_first != lhs_last) {
            *lhs_first++ += *rhs_first++;
        }
        return lhs;
    }
}
template <class T>
void H1D::WorkerDelegate::accumulate_by_worker(GridQ<T> const &payload)
{
    auto [ticket, buffer] = mutable_comm.recv<GridQ<T>*>(*this);
    *buffer += payload;
}
