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

P1D::MasterDelegate::~MasterDelegate()
{
}
P1D::MasterDelegate::MasterDelegate(Delegate *const delegate) noexcept
: delegate{delegate}
{
    comm = dispatch.comm(static_cast<unsigned>(workers.size()));
    for (unsigned i = 0; i < workers.size(); ++i) {
        workers[i].master = this;
        workers[i].comm = dispatch.comm(i);
    }
}

void P1D::MasterDelegate::setup(Domain &domain)
{
    // distribute particles to workers
    //
    for (PartSpecies &sp : domain.part_species) {
        long const chunk = static_cast<long>(sp.bucket.size()/(workers.size() + 1));
        std::array<ticket_t, std::tuple_size_v<decltype(workers)>> tks;
        for (unsigned i = 0; i < workers.size(); ++i) {
            auto const &worker = workers[i];
            auto const last = end(sp.bucket), first = std::prev(last, chunk);
            tks.at(i) = comm.send(PartBucket{first, last}, worker.comm.rank());
            sp.bucket.erase(first, last);
        }
        // use the fact that wait is called on destruction of ticket object
    }
}
void P1D::MasterDelegate::teardown(Domain &domain)
{
    // collect particles to master
    //
    for (PartSpecies &sp : domain.part_species) {
        for (auto const &worker : workers) {
            comm.recv<PartBucket>(worker.comm.rank()).unpack([](PartBucket payload, PartBucket &bucket) {
                std::move(begin(payload), end(payload), std::back_inserter(bucket));
            }, sp.bucket);
        }
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

template <class T, long N>
void P1D::MasterDelegate::broadcast_to_workers(GridQ<T, N> const &payload) const
{
    std::array<ticket_t, std::tuple_size_v<decltype(workers)>> tks;
    for (unsigned i = 0; i < workers.size(); ++i) {
        auto const &worker = workers[i];
        tks[i] = comm.send(&payload, worker.comm.rank());
    }
    // use the fact that wait is called on destruction of ticket object
}
template <class T, long N>
void P1D::MasterDelegate::collect_from_workers(GridQ<T, N> &buffer) const
{
    // the first worker will collect all workers'
    //
    if (!workers.empty()) {
        dispatch.send(&buffer, {-1, 0}).wait();
    }
}
