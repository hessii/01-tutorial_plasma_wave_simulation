//
//  Snapshot.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/27/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Snapshot.h"

#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <memory>

P1D::Snapshot::message_dispatch_t P1D::Snapshot::dispatch;
P1D::Snapshot::Snapshot(unsigned const rank, unsigned const size, ParamSet const &params, long const step_count)
: comm{dispatch.comm(rank)}, size{size}, step_count{step_count}
{
    // method dispatch
    //
    save = is_master() ? &Snapshot::save_master : &Snapshot::save_worker;
    load = is_master() ? &Snapshot::load_master : &Snapshot::load_worker;

    // calculate signature
    //
    signature = std::numeric_limits<long>::quiet_NaN();
}

std::string P1D::Snapshot::filepath(std::string_view const basename) const
{
    std::string const filename = std::string{basename} + ".snapshot";
    return std::string{Input::working_directory} + "/" + filename;
}

namespace {
    template <class T, long N>
    std::vector<T> vectorfy(P1D::GridQ<T, N> const &payload) {
        return {payload.begin(), payload.end()};
    }
    template <class T>
    std::vector<T> vectorfy(std::deque<T> const &payload) {
        return {begin(payload), end(payload)};
    }
    //
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ofstream &os, T const &payload) {
        return os.write(static_cast<char const*>(static_cast<void const*>(std::addressof(payload))), sizeof(T));
    }
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ofstream &os, std::vector<T> payload) {
        for (auto &&x : payload) { write(os, x); }
        return os;
    }
}
void P1D::Snapshot::save_master(Domain const &domain) const&
{
    auto save = [this](auto const &payload, std::string_view basename) {
        if (std::ofstream os{filepath(basename)}; os) {
            write(os, signature);
            write(os, step_count);
            //
            auto tk = comm.send(vectorfy(payload), master);
            for (unsigned rank = 0; rank < size; ++rank) {
                write(os, *comm.recv<decltype(vectorfy(payload))>(master));
            }
            std::move(tk).wait();
        } else {
            throw std::runtime_error{__PRETTY_FUNCTION__};
        }
    };

    // B & E
    save(domain.bfield, "bfield");
    save(domain.efield, "efield");

    // particles
    for (unsigned i = 0; i < domain.part_species.size(); ++i) {
        PartSpecies const &sp = domain.part_species.at(i);
        constexpr char prefix[] = "part_species_";
        save(sp.bucket, std::string{prefix} + std::to_string(i) + "-particles");
    }

    // cold fluid
    for (unsigned i = 0; i < domain.cold_species.size(); ++i) {
        ColdSpecies const &sp = domain.cold_species.at(i);
        constexpr char prefix[] = "cold_species_";
        save(sp.moment<0>(), std::string{prefix} + std::to_string(i) + "-moment_0");
        save(sp.moment<1>(), std::string{prefix} + std::to_string(i) + "-moment_1");
        save(sp.moment<2>(), std::string{prefix} + std::to_string(i) + "-moment_2");
    }
}
void P1D::Snapshot::save_worker(Domain const &domain) const&
{
    // B & E
    comm.send(vectorfy(domain.bfield), master).wait();
    comm.send(vectorfy(domain.efield), master).wait();

    // particles
    for (PartSpecies const &sp : domain.part_species) {
        comm.send(vectorfy(sp.bucket), master).wait();
    }

    // cold fluid
    for (ColdSpecies const &sp : domain.cold_species) {
        comm.send(vectorfy(sp.moment<0>()), master).wait();
        comm.send(vectorfy(sp.moment<1>()), master).wait();
        comm.send(vectorfy(sp.moment<2>()), master).wait();
    }
}

namespace {
    template <class T, long N>
    void unpack(std::vector<T> payload, P1D::GridQ<T, N> &to) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::move(begin(payload), end(payload), to.begin());
    }
    template <class T>
    void unpack(std::vector<T> payload, std::deque<T> &to) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::move(begin(payload), end(payload), std::back_inserter(to));
    }
}
long P1D::Snapshot::load_master(Domain &domain) const&
{
    return 0;
}
long P1D::Snapshot::load_worker(Domain &domain) const&
{
    // B & E
    unpack(*comm.recv<1>(master), domain.bfield);
    unpack(*comm.recv<1>(master), domain.efield);

    // particles
    for (PartSpecies &sp : domain.part_species) {
        sp.bucket.clear();
        unpack(*comm.recv<3>(master), sp.bucket);
    }

    // cold fluid
    for (ColdSpecies &sp : domain.cold_species) {
        unpack(*comm.recv<0>(master), sp.moment<0>());
        unpack(*comm.recv<1>(master), sp.moment<1>());
        unpack(*comm.recv<2>(master), sp.moment<2>());
    }

    // step count
    return comm.recv<long>(master);
}
