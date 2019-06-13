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

H1D::MasterDelegate::~MasterDelegate()
{
}
H1D::MasterDelegate::MasterDelegate()
: workers{}, provider{}
{
    // construct worker delegates
    //
    for (unsigned id = 0; id < workers.size(); ++id) {
        workers[id] = std::make_unique<WorkerDelegate>(this, id);
    }

    // set all flags
    //
    for (auto &pair : std::get<0>(provider)) {
        pair.first.test_and_set();
    }
    for (auto &pair : std::get<1>(provider)) {
        pair.first.test_and_set();
    }
    for (auto &pair : std::get<2>(provider)) {
        pair.first.test_and_set();
    }
}

void H1D::MasterDelegate::gather(Domain const& domain, Charge &charge)
{
    constexpr Tag tag = gather_charge;

    // 1. collect from workers
    //
    for (auto &worker : workers) {
        auto &[flag, payload] = std::get<tag>(worker->provider);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        charge += payload;
    }

    // 2. boundary gather
    //
    Delegate::gather(domain, charge);

    // 3. broadcast to workers
    //
    for (auto &pair : std::get<tag>(this->provider)) {
        auto &[flag, payload] = pair;
        std::copy(charge.dead_begin(), charge.dead_end(), payload.dead_begin());
        flag.clear(std::memory_order_release);
    }
}
void H1D::MasterDelegate::gather(Domain const& domain, Current &current)
{
    constexpr Tag tag = gather_current;

    // 1. collect from workers
    //
    for (auto &worker : workers) {
        auto &[flag, payload] = std::get<tag>(worker->provider);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        current += payload;
    }

    // 2. boundary gather
    //
    Delegate::gather(domain, current);

    // 3. broadcast to workers
    //
    for (auto &pair : std::get<tag>(this->provider)) {
        auto &[flag, payload] = pair;
        std::copy(current.dead_begin(), current.dead_end(), payload.dead_begin());
        flag.clear(std::memory_order_release);
    }
}
void H1D::MasterDelegate::gather(Domain const& domain, Species &sp)
{
    constexpr Tag tag = gather_species;

    // 1. collect from workers
    //
    for (auto &worker : workers) {
        auto &[flag, payload] = std::get<tag>(worker->provider);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        sp.moment<0>() += std::get<0>(payload);
        sp.moment<1>() += std::get<1>(payload);
        sp.moment<2>() += std::get<2>(payload);
    }

    // 2. boundary gather
    //
    Delegate::gather(domain, sp);

    // 3. broadcast to workers
    //
    for (auto &pair : std::get<tag>(this->provider)) {
        auto &[flag, payload] = pair;
        std::copy(sp.moment<0>().dead_begin(), sp.moment<0>().dead_end(), std::get<0>(payload).dead_begin());
        std::copy(sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), std::get<1>(payload).dead_begin());
        std::copy(sp.moment<2>().dead_begin(), sp.moment<2>().dead_end(), std::get<2>(payload).dead_begin());
        flag.clear(std::memory_order_release);
    }
}
