//
//  MasterDelegate.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./MasterDelegate.h"
#include "./WorkerDelegate.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"

#include <stdexcept>

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

H1D::MasterDelegate::~MasterDelegate()
{
}
H1D::MasterDelegate::MasterDelegate() noexcept
: workers{}, provider{}
{
    // construct worker delegates
    //
    for (unsigned id = 0; id < workers.size(); ++id) {
        workers[id] = std::make_unique<WorkerDelegate>(this, id);
    }

    // set all flags
    //
    for (std::atomic_flag &flag : std::get<0>(provider).first) {
        flag.test_and_set();
    }
    for (std::atomic_flag &flag : std::get<1>(provider).first) {
        flag.test_and_set();
    }
    for (std::atomic_flag &flag : std::get<2>(provider).first) {
        flag.test_and_set();
    }
}

void H1D::MasterDelegate::gather(Domain const& domain, Charge &charge)
{
    constexpr Tag tag = gather_charge;

    // 1. local gather first
    //
    Delegate::gather(domain, charge);

    // 2. reduce to master
    //
    for (auto &worker : workers) {
        auto &[flag, payload] = std::get<tag>(worker->provider);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        charge += *payload;
    }

    // 3. broadcast to workers
    //
    {
        auto &[flags, payload] = std::get<tag>(this->provider);
        payload = std::addressof(charge);
        for (std::atomic_flag &flag : flags) {
            flag.clear(std::memory_order_release);
        }
    }
}
void H1D::MasterDelegate::gather(Domain const& domain, Current &current)
{
    constexpr Tag tag = gather_current;

    // 1. local gather first
    //
    Delegate::gather(domain, current);

    // 2. reduce to master
    //
    for (auto &worker : workers) {
        auto &[flag, payload] = std::get<tag>(worker->provider);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        current += *payload;
    }

    // 3. broadcast to workers
    //
    {
        auto &[flags, payload] = std::get<tag>(this->provider);
        payload = std::addressof(current);
        for (std::atomic_flag &flag : flags) {
            flag.clear(std::memory_order_release);
        }
    }
}
void H1D::MasterDelegate::gather(Domain const& domain, Species &sp)
{
    constexpr Tag tag = gather_species;

    // 1. local gather first
    //
    Delegate::gather(domain, sp);

    // 2. reduce to master
    //
    for (auto &worker : workers) {
        auto &[flag, payload] = std::get<tag>(worker->provider);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        sp.moment<0>() += payload->moment<0>();
        sp.moment<1>() += payload->moment<1>();
        sp.moment<2>() += payload->moment<2>();
    }

    // 3. broadcast to workers
    //
    {
        auto &[flags, payload] = std::get<tag>(this->provider);
        payload = std::addressof(sp);
        for (std::atomic_flag &flag : flags) {
            flag.clear(std::memory_order_release);
        }
    }
}
