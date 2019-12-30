//
//  MasterDelegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "MasterDelegate.h"
#include "../Module/Domain.h"

#include <utility>

P1D::MasterDelegate::~MasterDelegate()
{
}
P1D::MasterDelegate::MasterDelegate(std::unique_ptr<Delegate> delegate) noexcept
: delegate{std::move(delegate)}
{
    for (unsigned i = 0; i < workers.size(); ++i) {
        workers[i].master = this;
        workers[i].id = i;
    }
}

void P1D::MasterDelegate::once(Domain &domain)
{
    delegate->once(domain);
}
#if defined(PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
void P1D::MasterDelegate::pass(Domain const& domain, PartSpecies &sp)
{
    PartBucket L, R;
    delegate->partition(sp, L, R);
    {
        delegate->pass(domain, L, R);
        for (WorkerDelegate &worker : workers) {
            worker.constant_comm.send(*this, std::make_pair(&L, &R))();
            delegate->pass(domain, L, R);
        }
    }
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}
void P1D::MasterDelegate::pass(Domain const& domain, BField &bfield)
{
    delegate->pass(domain, bfield);
    broadcast_to_workers(bfield);
}
void P1D::MasterDelegate::pass(Domain const& domain, EField &efield)
{
    delegate->pass(domain, efield);
    broadcast_to_workers(efield);
}
void P1D::MasterDelegate::pass(Domain const& domain, Current &current)
{
    delegate->pass(domain, current);
    broadcast_to_workers(current);
}
#endif
void P1D::MasterDelegate::gather(Domain const& domain, Current &current)
{
    collect_from_workers(current);
    delegate->gather(domain, current);
    broadcast_to_workers(current);
}
void P1D::MasterDelegate::gather(Domain const& domain, PartSpecies &sp)
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

template <class T, long N>
void P1D::MasterDelegate::broadcast_to_workers(GridQ<T, N> const &payload)
{
    for (WorkerDelegate &worker : workers) {
        tickets.push_back(worker.constant_comm.send(*this, &payload));
    }
    tickets.clear();
}
template <class T, long N>
void P1D::MasterDelegate::collect_from_workers(GridQ<T, N> &buffer)
{
    // the first worker will collect all workers'
    //
    if (auto first = workers.begin(); first != workers.end()) {
        first->mutable_comm.send(*this, &buffer)();
    }
}
