//
//  MasterDelegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "MasterDelegate.h"

#include <utility>
#include <iterator>
#include <algorithm>
#include <functional>

P1D::MasterDelegate::~MasterDelegate()
{
}
P1D::MasterDelegate::MasterDelegate(Delegate *const delegate)
: delegate{delegate}, all_but_master{}
{
    comm = dispatch.comm(static_cast<unsigned>(workers.size()));
    for (unsigned i = 0; i < workers.size(); ++i) {
        workers[i].master = this;
        workers[i].comm = dispatch.comm(i);
        all_but_master.emplace_back(i);
    }
}

void P1D::MasterDelegate::setup(Domain &domain)
{
    // distribute particles to workers
    //
    for (PartSpecies &sp : domain.part_species) {
        sp.Nc /= ParamSet::number_of_particle_parallelism;
        long const chunk = static_cast<long>(sp.bucket.size()/(workers.size() + 1));
        std::vector<PartBucket> payloads;
        payloads.reserve(all_but_master.size());
        for ([[maybe_unused]] unsigned const &rank : all_but_master) { // master excluded
            auto const last = end(sp.bucket), first = std::prev(last, chunk);
            payloads.emplace_back(std::make_move_iterator(first), std::make_move_iterator(last));
            sp.bucket.erase(first, last);
        }
        auto tks = comm.scatter(std::move(payloads), all_but_master);
        std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)),
                      std::mem_fn(&ticket_t::wait));
    }
}
void P1D::MasterDelegate::teardown(Domain &domain)
{
    // gather particles from workers
    // with master excluded
    //
    for (PartSpecies &sp : domain.part_species) {
        comm.for_each<PartBucket>(all_but_master, [](PartBucket payload, PartBucket &bucket) {
            std::move(begin(payload), end(payload), std::back_inserter(bucket));
        }, sp.bucket);
    }
}

void P1D::MasterDelegate::prologue(Domain const& domain, long const i) const
{
    delegate->prologue(domain, i);
}
void P1D::MasterDelegate::epilogue(Domain const& domain, long const i) const
{
    delegate->epilogue(domain, i);
}
void P1D::MasterDelegate::once(Domain &domain) const
{
    delegate->once(domain);
}
void P1D::MasterDelegate::pass(Domain const& domain, PartSpecies &sp) const
{
    PartBucket L, R;
    delegate->partition(sp, L, R);
    //
    delegate->pass(domain, L, R);
    for (auto const &worker : workers) {
        comm.send(std::make_pair(&L, &R), worker.comm.rank()).wait();
        delegate->pass(domain, L, R);
    }
    //
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}
void P1D::MasterDelegate::pass(Domain const& domain, ColdSpecies &sp) const
{
    delegate->pass(domain, sp);
    broadcast_to_workers(sp.mom0_half);
    broadcast_to_workers(sp.mom1_full);
}
void P1D::MasterDelegate::pass(Domain const& domain, BField &bfield) const
{
    delegate->pass(domain, bfield);
    broadcast_to_workers(bfield);
}
void P1D::MasterDelegate::pass(Domain const& domain, EField &efield) const
{
    delegate->pass(domain, efield);
    broadcast_to_workers(efield);
}
void P1D::MasterDelegate::pass(Domain const& domain, Current &current) const
{
    delegate->pass(domain, current);
    broadcast_to_workers(current);
}
void P1D::MasterDelegate::gather(Domain const& domain, Current &current) const
{
    collect_from_workers(current);
    delegate->gather(domain, current);
    broadcast_to_workers(current);
}
void P1D::MasterDelegate::gather(Domain const& domain, PartSpecies &sp) const
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

namespace {
    template <class T, long N, class U>
    decltype(auto) operator/=(P1D::GridQ<T, N> &lhs, U const w) noexcept { // include padding
        for (auto it = lhs.dead_begin(), end = lhs.dead_end(); it != end; ++it) {
            *it /= w;
        }
        return lhs;
    }
    template <class T, long N>
    decltype(auto) operator+=(P1D::GridQ<T, N> &lhs, P1D::GridQ<T, N> const &rhs) noexcept {
        auto lhs_first = lhs.dead_begin(), lhs_last = lhs.dead_end();
        auto rhs_first = rhs.dead_begin();
        while (lhs_first != lhs_last) {
            *lhs_first++ += *rhs_first++;
        }
        return lhs;
    }
}
template <class T, long N>
void P1D::MasterDelegate::broadcast_to_workers(GridQ<T, N> const &payload) const
{
    auto tks = comm.bcast(&payload, all_but_master);
    std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)),
                  std::mem_fn(&ticket_t::wait));
}
template <class T, long N>
void P1D::MasterDelegate::collect_from_workers(GridQ<T, N> &buffer) const
{
    // the first worker will collect all workers'
    //
    comm.for_each<GridQ<T, N> const*>(all_but_master, [](auto payload, GridQ<T, N> &buffer) {
        buffer += *payload;
    }, buffer);

    // normalize by the particle parallelism
    //
    buffer /= ParamSet::number_of_particle_parallelism;
}
