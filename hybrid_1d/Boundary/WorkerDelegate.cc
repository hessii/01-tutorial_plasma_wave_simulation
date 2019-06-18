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

// helpers
//
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

#if defined(HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
void H1D::WorkerDelegate::pass(Domain const&, Species &sp)
{
    auto [ticket, payload] = master_to_worker.recv(*this, particle_tag{});
    payload->swap(sp.bucket);
}
void H1D::WorkerDelegate::pass(Domain const&, BField &bfield)
{
    recv_from_master(vector_grid_tag{}, bfield);
}
void H1D::WorkerDelegate::pass(Domain const&, EField &efield)
{
    recv_from_master(vector_grid_tag{}, efield);
}
void H1D::WorkerDelegate::pass(Domain const&, Charge &charge)
{
    recv_from_master(scalar_grid_tag{}, charge);
}
void H1D::WorkerDelegate::pass(Domain const&, Current &current)
{
    recv_from_master(vector_grid_tag{}, current);
}
#endif
void H1D::WorkerDelegate::gather(Domain const&, Charge &charge)
{
    reduce_to_master(scalar_grid_tag{}, charge);
    recv_from_master(scalar_grid_tag{}, charge);
}
void H1D::WorkerDelegate::gather(Domain const&, Current &current)
{
    reduce_to_master(vector_grid_tag{}, current);
    recv_from_master(vector_grid_tag{}, current);
}
void H1D::WorkerDelegate::gather(Domain const&, Species &sp)
{
    {
        reduce_to_master(scalar_grid_tag{}, sp.moment<0>());
        reduce_to_master(vector_grid_tag{}, sp.moment<1>());
        reduce_to_master(tensor_grid_tag{}, sp.moment<2>());
    }
    {
        recv_from_master(scalar_grid_tag{}, sp.moment<0>());
        recv_from_master(vector_grid_tag{}, sp.moment<1>());
        recv_from_master(tensor_grid_tag{}, sp.moment<2>());
    }
}

template <long i, class T>
void H1D::WorkerDelegate::recv_from_master(std::integral_constant<long, i> tag, GridQ<T> &buffer)
{
    auto const [ticket, payload] = master_to_worker.recv(*this, tag);
    std::copy(payload->dead_begin(), payload->dead_end(), buffer.dead_begin());
}
template <long i, class T>
void H1D::WorkerDelegate::reduce_to_master(std::integral_constant<long, i> tag, GridQ<T> &payload)
{
    reduce_divide_and_conquer(tag, payload);
    accumulate_by_worker(tag, payload);
}
template <long i, class T>
void H1D::WorkerDelegate::reduce_divide_and_conquer(std::integral_constant<long, i> tag, GridQ<T> &payload)
{
    // divide and conquer
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
            (this + stride)->worker_to_worker.send(*this, tag, &payload)();
        }
    }
}
template <long i, class T>
void H1D::WorkerDelegate::accumulate_by_worker(std::integral_constant<long, i> tag, GridQ<T> const &payload)
{
    // accumulation by workers
    //
    using Payload = GridQ<T>;
    if (this == master->workers.begin()) {
        auto [ticket, buffer] = master_to_worker.recv(*this, tag);
        *buffer += payload;
    } else {
        auto [ticket, buffer] = worker_to_worker.recv(*this, tag);
        *buffer += payload;
    }
}
