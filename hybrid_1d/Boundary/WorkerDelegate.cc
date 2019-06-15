//
//  WorkerDelegate.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "WorkerDelegate.h"
#include "MasterDelegate.h"
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

void H1D::WorkerDelegate::gather(Domain const&, Charge &charge)
{
    constexpr Tag tag = gather_charge;

    // 1. send a local copy to master
    //
    {
        auto &[flag, payload] = std::get<tag>(this->provider);
        std::copy(charge.dead_begin(), charge.dead_end(), payload.dead_begin());
        flag.clear(std::memory_order_release);
    }

    // 2. receive a master copy from master
    //
    {
        auto &[flag, payload] = std::get<tag>(master->provider).at(id);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        std::copy(payload.dead_begin(), payload.dead_end(), charge.dead_begin());
    }
}
void H1D::WorkerDelegate::gather(Domain const&, Current &current)
{
    constexpr Tag tag = gather_current;

    // 1. send a local copy to master
    //
    {
        auto &[flag, payload] = std::get<tag>(this->provider);
        std::copy(current.dead_begin(), current.dead_end(), payload.dead_begin());
        flag.clear(std::memory_order_release);
    }

    // 2. receive a master copy from master
    //
    {
        auto &[flag, payload] = std::get<tag>(master->provider).at(id);
        while (flag.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        std::copy(payload.dead_begin(), payload.dead_end(), current.dead_begin());
    }
}
void H1D::WorkerDelegate::gather(Domain const&, Species &sp)
{
    constexpr Tag tag = gather_species;

    // 1. send a local copy to master
    //
    {
        auto &[flag, payload] = std::get<tag>(this->provider);
        std::copy(sp.moment<0>().dead_begin(), sp.moment<0>().dead_end(), std::get<0>(payload).dead_begin());
        std::copy(sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), std::get<1>(payload).dead_begin());
        std::copy(sp.moment<2>().dead_begin(), sp.moment<2>().dead_end(), std::get<2>(payload).dead_begin());
        flag.clear(std::memory_order_release);
    }

    // 2. receive a master copy from master
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
