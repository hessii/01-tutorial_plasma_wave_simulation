//
//  MasterDelegate.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MasterDelegate.h"
#include "../Module/BField.h"
#include "../Module/EField.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"

#include <memory>
#include <iterator>

H1D::MasterDelegate::~MasterDelegate()
{
}
H1D::MasterDelegate::MasterDelegate(std::unique_ptr<Delegate> delegate) noexcept
: delegate{std::move(delegate)}
{
    for (WorkerDelegate &worker : workers) {
        worker.master = this;
    }
}

#if defined(HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
void H1D::MasterDelegate::pass(Domain const& domain, Species &sp)
{
    std::deque<Particle> L, R;

    // 1. gather
    //
    for (WorkerDelegate &worker : workers) {
        worker.constant_comm.send(*this, std::make_pair(&L, &R))();
    }
    std::pair<unsigned long, unsigned long> const n_worker_ptls{L.size(), R.size()};
    delegate->partition(sp, L, R); // appended to L and R

    // 2. boundary pass
    //
    delegate->pass(domain, L, R); // L and R switched upon return
    if (L.size() < n_worker_ptls.first || R.size() < n_worker_ptls.second) {
        throw std::runtime_error{__PRETTY_FUNCTION__};
    }

    // 3. distribute
    //
    for (WorkerDelegate &worker : workers) {
        tickets.push_back(worker.constant_comm.send(*this, std::make_pair(&L, &R)));
    }
    std::copy_n(L.crbegin(), L.size() - n_worker_ptls.first , std::back_inserter(sp.bucket));
    std::copy_n(R.crbegin(), R.size() - n_worker_ptls.second, std::back_inserter(sp.bucket));
    tickets.clear();
}
void H1D::MasterDelegate::pass(Domain const& domain, BField &bfield)
{
    delegate->pass(domain, bfield);
    broadcast_to_workers(bfield);
}
void H1D::MasterDelegate::pass(Domain const& domain, EField &efield)
{
    delegate->pass(domain, efield);
    broadcast_to_workers(efield);
}
void H1D::MasterDelegate::pass(Domain const& domain, Charge &charge)
{
    delegate->pass(domain, charge);
    broadcast_to_workers(charge);
}
void H1D::MasterDelegate::pass(Domain const& domain, Current &current)
{
    delegate->pass(domain, current);
    broadcast_to_workers(current);
}
#endif
void H1D::MasterDelegate::gather(Domain const& domain, Charge &charge)
{
    collect_from_workers(charge);
    delegate->gather(domain, charge);
    broadcast_to_workers(charge);
}
void H1D::MasterDelegate::gather(Domain const& domain, Current &current)
{
    collect_from_workers(current);
    delegate->gather(domain, current);
    broadcast_to_workers(current);
}
void H1D::MasterDelegate::gather(Domain const& domain, Species &sp)
{
    {
        collect_from_workers(sp.moment<0>());
        collect_from_workers(sp.moment<1>());
        collect_from_workers(sp.moment<2>());
    }
    delegate->gather(domain, sp);
    {
        broadcast_to_workers(sp.moment<0>());
        broadcast_to_workers(sp.moment<1>());
        broadcast_to_workers(sp.moment<2>());
    }
}

template <class T>
void H1D::MasterDelegate::broadcast_to_workers(GridQ<T> const &payload)
{
    for (WorkerDelegate &worker : workers) {
        tickets.push_back(worker.constant_comm.send(*this, &payload));
    }
    tickets.clear();
}
template <class T>
void H1D::MasterDelegate::collect_from_workers(GridQ<T> &buffer)
{
    // the first worker will collect all workers'
    //
    if (auto first = workers.begin(); first != workers.end()) {
        first->mutable_comm.send(*this, &buffer)();
    }
}
