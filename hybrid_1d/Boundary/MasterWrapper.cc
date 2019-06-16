//
//  MasterWrapper.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MasterWrapper.h"
#include "../Module/BField.h"
#include "../Module/EField.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"

#include <utility>

H1D::MasterWrapper::~MasterWrapper()
{
}
H1D::MasterWrapper::MasterWrapper(std::unique_ptr<Delegate> delegate) noexcept
: delegate{std::move(delegate)}
{
    for (WorkerWrapper &worker : workers) {
        worker.master = this;
    }
}

#if defined(HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS) && HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS
void H1D::MasterWrapper::pass(Domain const& domain, Species &sp)
{
    constexpr auto tag = WorkerWrapper::pass_species_tag{};

    delegate->pass(domain, sp);
    for (WorkerWrapper &worker : workers) {
        worker.master_to_worker.send(*this, tag, &sp.bucket).~Ticket();
        delegate->pass(domain, sp);
    }
}
void H1D::MasterWrapper::pass(Domain const& domain, BField &bfield)
{
    constexpr auto tag = WorkerWrapper::pass_bfield_tag{};

    delegate->pass(domain, bfield);
    broadcast_to_workers(tag, bfield);
}
void H1D::MasterWrapper::pass(Domain const& domain, EField &efield)
{
    constexpr auto tag = WorkerWrapper::pass_efield_tag{};

    delegate->pass(domain, efield);
    broadcast_to_workers(tag, efield);
}
void H1D::MasterWrapper::pass(Domain const& domain, Charge &charge)
{
    constexpr auto tag = WorkerWrapper::pass_charge_tag{};

    delegate->pass(domain, charge);
    broadcast_to_workers(tag, charge);
}
void H1D::MasterWrapper::pass(Domain const& domain, Current &current)
{
    constexpr auto tag = WorkerWrapper::pass_current_tag{};

    delegate->pass(domain, current);
    broadcast_to_workers(tag, current);
}
#endif
void H1D::MasterWrapper::gather(Domain const& domain, Charge &charge)
{
    constexpr auto tag = WorkerWrapper::gather_charge_tag{};

    collect_from_workers(tag, charge);
    delegate->gather(domain, charge);
    broadcast_to_workers(tag, charge);
}
void H1D::MasterWrapper::gather(Domain const& domain, Current &current)
{
    constexpr auto tag = WorkerWrapper::gather_current_tag{};

    collect_from_workers(tag, current);
    delegate->gather(domain, current);
    broadcast_to_workers(tag, current);
}
void H1D::MasterWrapper::gather(Domain const& domain, Species &sp)
{
    constexpr auto tag = WorkerWrapper::gather_species_tag{};

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

template <long i, class Payload>
void H1D::MasterWrapper::broadcast_to_workers(std::integral_constant<long, i> tag, Payload const &payload)
{
    for (WorkerWrapper &worker : workers) {
        tickets.push_back(worker.master_to_worker.send(*this, tag, &payload));
    }
    tickets.clear();
}
template <long i, class Payload>
void H1D::MasterWrapper::collect_from_workers(std::integral_constant<long, i> tag, Payload &buffer)
{
    for (WorkerWrapper &worker : workers) {
        worker.master_to_worker.send(*this, tag, &buffer).~Ticket();
    }
}
