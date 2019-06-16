//
//  WorkerWrapper.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "WorkerWrapper.h"
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

#if defined(HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS) && HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS
void H1D::WorkerWrapper::pass(Domain const&, Species &sp)
{
    constexpr auto tag = pass_species_tag{};

    using Payload = std::remove_reference<decltype(sp.bucket)>::type;
    auto const &req = master_to_worker.recv(*this, tag);
    req.payload<Payload>()->swap(sp.bucket);
}
void H1D::WorkerWrapper::pass(Domain const&, BField &bfield)
{
    constexpr auto tag = pass_bfield_tag{};

    recv_from_master(tag, bfield);
}
void H1D::WorkerWrapper::pass(Domain const&, EField &efield)
{
    constexpr auto tag = pass_efield_tag{};

    recv_from_master(tag, efield);
}
void H1D::WorkerWrapper::pass(Domain const&, Charge &charge)
{
    constexpr auto tag = pass_charge_tag{};

    recv_from_master(tag, charge);
}
void H1D::WorkerWrapper::pass(Domain const&, Current &current)
{
    constexpr auto tag = pass_current_tag{};

    recv_from_master(tag, current);
}
#endif
void H1D::WorkerWrapper::gather(Domain const&, Charge &charge)
{
    constexpr auto tag = gather_charge_tag{};

    reduce_to_master(tag, charge);
    recv_from_master(tag, charge);
}
void H1D::WorkerWrapper::gather(Domain const&, Current &current)
{
    constexpr auto tag = gather_current_tag{};

    reduce_to_master(tag, current);
    recv_from_master(tag, current);
}
void H1D::WorkerWrapper::gather(Domain const&, Species &sp)
{
    constexpr auto tag = gather_species_tag{};

    {
        reduce_to_master(tag, sp.moment<0>());
        reduce_to_master(tag, sp.moment<1>());
        reduce_to_master(tag, sp.moment<2>());
    }
    {
        recv_from_master(tag, sp.moment<0>());
        recv_from_master(tag, sp.moment<1>());
        recv_from_master(tag, sp.moment<2>());
    }
}

template <long i, class Payload>
void H1D::WorkerWrapper::recv_from_master(std::integral_constant<long, i> tag, Payload &buffer, bool const is_boundary_only)
{
    auto const req = master_to_worker.recv(*this, tag);
    Payload const *payload = req.template payload<Payload const>();
    if (!is_boundary_only) {
        std::copy(payload->dead_begin(), payload->dead_end(), buffer.dead_begin());
    } else {
        throw std::runtime_error{__PRETTY_FUNCTION__};
    }
}
template <long i, class Payload>
void H1D::WorkerWrapper::reduce_to_master(std::integral_constant<long, i> tag, Payload const &payload)
{
    auto const req = master_to_worker.recv(*this, tag);
    Payload *buffer = req.template payload<Payload>();
    *buffer += payload;
}
