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

#include <utility>

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
    constexpr auto tag = WorkerDelegate::pass_species_tag{};

    delegate->pass(domain, sp);
    for (WorkerDelegate &worker : workers) {
        worker.master_to_worker.send(*this, tag, &sp.bucket).wait();
        delegate->pass(domain, sp);
    }
}
void H1D::MasterDelegate::pass(Domain const& domain, BField &bfield)
{
    constexpr auto tag = WorkerDelegate::pass_bfield_tag{};

    delegate->pass(domain, bfield);
    broadcast_to_workers(tag, bfield);
}
void H1D::MasterDelegate::pass(Domain const& domain, EField &efield)
{
    constexpr auto tag = WorkerDelegate::pass_efield_tag{};

    delegate->pass(domain, efield);
    broadcast_to_workers(tag, efield);
}
void H1D::MasterDelegate::pass(Domain const& domain, Charge &charge)
{
    constexpr auto tag = WorkerDelegate::pass_charge_tag{};

    delegate->pass(domain, charge);
    broadcast_to_workers(tag, charge);
}
void H1D::MasterDelegate::pass(Domain const& domain, Current &current)
{
    constexpr auto tag = WorkerDelegate::pass_current_tag{};

    delegate->pass(domain, current);
    broadcast_to_workers(tag, current);
}
#endif
void H1D::MasterDelegate::gather(Domain const& domain, Charge &charge)
{
    constexpr auto tag = WorkerDelegate::gather_charge_tag{};

    collect_from_workers(tag, charge);
    delegate->gather(domain, charge);
    broadcast_to_workers(tag, charge);
}
void H1D::MasterDelegate::gather(Domain const& domain, Current &current)
{
    constexpr auto tag = WorkerDelegate::gather_current_tag{};

    collect_from_workers(tag, current);
    delegate->gather(domain, current);
    broadcast_to_workers(tag, current);
}
void H1D::MasterDelegate::gather(Domain const& domain, Species &sp)
{
    constexpr auto tag = WorkerDelegate::gather_species_tag{};

    {
        collect_from_workers(tag, sp.moment<0>());
        collect_from_workers(tag, sp.moment<1>());
        collect_from_workers(tag, sp.moment<2>());
    }
    delegate->gather(domain, sp);
    {
        broadcast_to_workers(tag, sp.moment<0>());
        broadcast_to_workers(tag, sp.moment<1>());
        broadcast_to_workers(tag, sp.moment<2>());
    }
}

template <long i, class T>
void H1D::MasterDelegate::broadcast_to_workers(std::integral_constant<long, i> tag, GridQ<T> const &payload)
{
    for (WorkerDelegate &worker : workers) {
        tickets.push_back(worker.master_to_worker.send(*this, tag, &payload));
    }
    tickets.clear();
}
template <long i, class T>
void H1D::MasterDelegate::collect_from_workers(std::integral_constant<long, i> tag, GridQ<T> &buffer)
{
    // the first worker will collect all workers'
    //
    if (auto first = workers.begin(); first != workers.end()) {
        first->master_to_worker.send(*this, tag, &buffer).wait();
    }
}
