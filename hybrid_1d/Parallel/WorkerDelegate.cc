//
//  WorkerDelegate.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./WorkerDelegate.h"
#include "./MasterDelegate.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"

#include <stdexcept>
#include <algorithm>
#include <memory>

H1D::WorkerDelegate::~WorkerDelegate()
{
}
H1D::WorkerDelegate::WorkerDelegate(MasterDelegate *master, unsigned const id) noexcept
: master{master}, id{id}, provider{}
{
    // set all flags
    //
    std::get<0>(provider).first.test_and_set();
    std::get<1>(provider).first.test_and_set();
    std::get<2>(provider).first.test_and_set();
}

void H1D::WorkerDelegate::gather(Domain const& domain, Charge &charge)
{
    constexpr Tag tag = gather_charge;

    // 1. local gather first
    //
    Delegate::gather(domain, charge);

    // 2. reduce to master
    //
    {
        auto &[flag, payload] = std::get<tag>(this->provider);
        std::copy(charge.dead_begin(), charge.dead_end(), payload.dead_begin());
        flag.clear(std::memory_order_release);
    }

    // 3. broadcast to workers
    //
    {
        auto &[flag, payload] = std::get<tag>(master->provider).at(id);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        std::copy(payload.dead_begin(), payload.dead_end(), charge.dead_begin());
    }
}
void H1D::WorkerDelegate::gather(Domain const& domain, Current &current)
{
    constexpr Tag tag = gather_current;

    // 1. local gather first
    //
    Delegate::gather(domain, current);

    // 2. reduce to master
    //
    {
        auto &[flag, payload] = std::get<tag>(this->provider);
        std::copy(current.dead_begin(), current.dead_end(), payload.dead_begin());
        flag.clear(std::memory_order_release);
    }

    // 3. broadcast to workers
    //
    {
        auto &[flag, payload] = std::get<tag>(master->provider).at(id);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        std::copy(payload.dead_begin(), payload.dead_end(), current.dead_begin());
    }
}
void H1D::WorkerDelegate::gather(Domain const& domain, Species &sp)
{
    constexpr Tag tag = gather_species;

    // 1. local gather first
    //
    Delegate::gather(domain, sp);

    // 2. reduce to master
    //
    {
        auto &[flag, payload] = std::get<tag>(this->provider);
        std::copy(sp.moment<0>().dead_begin(), sp.moment<0>().dead_end(), std::get<0>(payload).dead_begin());
        std::copy(sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), std::get<1>(payload).dead_begin());
        std::copy(sp.moment<2>().dead_begin(), sp.moment<2>().dead_end(), std::get<2>(payload).dead_begin());
        flag.clear(std::memory_order_release);
    }

    // 3. broadcast to workers
    //
    {
        auto &[flag, payload] = std::get<tag>(master->provider).at(id);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        std::copy(std::get<0>(payload).dead_begin(), std::get<0>(payload).dead_end(), sp.moment<0>().dead_begin());
        std::copy(std::get<1>(payload).dead_begin(), std::get<1>(payload).dead_end(), sp.moment<1>().dead_begin());
        std::copy(std::get<2>(payload).dead_begin(), std::get<2>(payload).dead_end(), sp.moment<2>().dead_begin());
    }
}
