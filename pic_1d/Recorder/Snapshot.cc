//
//  Snapshot.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/27/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Snapshot.h"

#include <algorithm>
#include <iterator>

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

void P1D::Snapshot::save_master(Domain const &domain) &
{
}
void P1D::Snapshot::save_worker(Domain const &domain) &
{
    auto vectorfy = [](auto const &payload) {
        using T = typename std::iterator_traits<decltype(payload.begin())>::value_type;
        return std::vector<T>(payload.begin(), payload.end());
    };

    // B & E
    comm.send(vectorfy(domain.bfield), master).wait();
    comm.send(vectorfy(domain.efield), master).wait();

    // particles
    for (PartSpecies const &sp : domain.part_species) {
        comm.send(sp.bucket, master).wait();
    }

    // cold fluid
    for (ColdSpecies const &sp : domain.cold_species) {
        comm.send(vectorfy(sp.moment<0>()), master).wait();
        comm.send(vectorfy(sp.moment<1>()), master).wait();
        comm.send(vectorfy(sp.moment<2>()), master).wait();
    }
}

long P1D::Snapshot::load_master(Domain &domain) const &
{
    return 0;
}
long P1D::Snapshot::load_worker(Domain &domain) const &
{
    auto unpack = [](auto const payload, auto &dest) noexcept {
        std::copy(begin(payload), end(payload), dest.begin());
    };

    // B & E
    comm.recv<1>(master).unpack(unpack, domain.bfield);
    comm.recv<1>(master).unpack(unpack, domain.efield);

    // particles
    for (PartSpecies &sp : domain.part_species) {
        sp.bucket = comm.recv<PartBucket>(master);
    }

    // cold fluid
    for (ColdSpecies &sp : domain.cold_species) {
        comm.recv<0>(master).unpack(unpack, sp.moment<0>());
        comm.recv<1>(master).unpack(unpack, sp.moment<1>());
        comm.recv<2>(master).unpack(unpack, sp.moment<2>());
    }

    // step count
    return comm.recv<long>(master);
}
