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
    constexpr auto tag = InterThreadComm::pass_species_tag{};

    using Payload = std::remove_reference<decltype(sp.bucket)>::type;
    for (WorkerWrapper &worker : workers) {
        delegate->pass(domain, sp); // previous worker's bucket
        auto req = worker.comm.process_job_request(tag);
        req.payload<Payload>()->swap(sp.bucket); // swap out
    }
    delegate->pass(domain, sp); // IMPORTANT!! last worker's bucket
}
void H1D::MasterWrapper::pass(Domain const& domain, BField &bfield)
{
    constexpr auto tag = InterThreadComm::pass_bfield_tag{};

    // pass global
    //
    delegate->pass(domain, bfield);

    // broadcast to local
    //
    using Payload = std::remove_reference<decltype(bfield)>::type;
    for (WorkerWrapper &worker : workers) {
        auto req = worker.comm.process_job_request(tag);
        std::copy(bfield.dead_begin(), bfield.dead_end(), req.payload<Payload>()->dead_begin());
    }
}
void H1D::MasterWrapper::pass(Domain const& domain, EField &efield)
{
    constexpr auto tag = InterThreadComm::pass_efield_tag{};

    // pass global
    //
    delegate->pass(domain, efield);

    // broadcast to local
    //
    using Payload = std::remove_reference<decltype(efield)>::type;
    for (WorkerWrapper &worker : workers) {
        auto req = worker.comm.process_job_request(tag);
        std::copy(efield.dead_begin(), efield.dead_end(), req.payload<Payload>()->dead_begin());
    }
}
void H1D::MasterWrapper::pass(Domain const& domain, Charge &charge)
{
    constexpr auto tag = InterThreadComm::pass_charge_tag{};

    // pass global
    //
    delegate->pass(domain, charge);

    // broadcast to local
    //
    using Payload = std::remove_reference<decltype(charge)>::type;
    for (WorkerWrapper &worker : workers) {
        auto req = worker.comm.process_job_request(tag);
        std::copy(charge.dead_begin(), charge.dead_end(), req.payload<Payload>()->dead_begin());
    }
}
void H1D::MasterWrapper::pass(Domain const& domain, Current &current)
{
    constexpr auto tag = InterThreadComm::pass_current_tag{};

    // pass global
    //
    delegate->pass(domain, current);

    // broadcast to local
    //
    using Payload = std::remove_reference<decltype(current)>::type;
    for (WorkerWrapper &worker : workers) {
        auto req = worker.comm.process_job_request(tag);
        std::copy(current.dead_begin(), current.dead_end(), req.payload<Payload>()->dead_begin());
    }
}
#endif
void H1D::MasterWrapper::gather(Domain const& domain, Charge &charge)
{
    constexpr auto tag = InterThreadComm::gather_charge_tag{};
    using Payload = std::remove_reference<decltype(charge)>::type;

    // 1. collect local
    //
    for (WorkerWrapper &worker : workers) {
        auto const &req = requests.emplace_back(worker.comm.process_job_request(tag));
        charge += *req.payload<Payload>();
    }

    // 2. gather global
    //
    delegate->gather(domain, charge);

    // 3. broadcast to local
    //
    for (auto const &req : requests) {
        std::copy(charge.dead_begin(), charge.dead_end(), req.payload<Payload>()->dead_begin());
        req.~Request();
    }
    requests.clear();
}
void H1D::MasterWrapper::gather(Domain const& domain, Current &current)
{
    constexpr auto tag = InterThreadComm::gather_current_tag{};
    using Payload = std::remove_reference<decltype(current)>::type;

    // 1. collect local
    //
    for (WorkerWrapper &worker : workers) {
        auto const &req = requests.emplace_back(worker.comm.process_job_request(tag));
        current += *req.payload<Payload>();
    }

    // 2. gather global
    //
    delegate->gather(domain, current);

    // 3. broadcast to local
    //
    for (auto const &req : requests) {
        std::copy(current.dead_begin(), current.dead_end(), req.payload<Payload>()->dead_begin());
        req.~Request();
    }
    requests.clear();
}
void H1D::MasterWrapper::gather(Domain const& domain, Species &sp)
{
    constexpr auto tag = InterThreadComm::gather_species_tag{};
    using Payload = std::remove_reference<decltype(sp.moments())>::type;

    // 1. collect local
    //
    for (WorkerWrapper &worker : workers) {
        auto const &req = requests.emplace_back(worker.comm.process_job_request(tag));
        Payload const &moms = *req.payload<Payload>();
        sp.moment<0>() += std::get<0>(moms);
        sp.moment<1>() += std::get<1>(moms);
        sp.moment<2>() += std::get<2>(moms);
    }

    // 2. gather global
    //
    delegate->gather(domain, sp);

    // 3. broadcast to local
    //
    for (auto const &req : requests) {
        Payload &moms = *req.payload<Payload>();
        std::copy(sp.moment<0>().dead_begin(), sp.moment<0>().dead_end(), std::get<0>(moms).dead_begin());
        std::copy(sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), std::get<1>(moms).dead_begin());
        std::copy(sp.moment<2>().dead_begin(), sp.moment<2>().dead_end(), std::get<2>(moms).dead_begin());
        req.~Request();
    }
    requests.clear();
}
