//
//  Driver.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Driver.h"
#include "./Module/Domain.h"
#include "./Recorder/Snapshot.h"
#include "./Recorder/EnergyRecorder.h"
#include "./Recorder/FieldRecorder.h"
#include "./Recorder/MomentRecorder.h"
#include "./Recorder/ParticleRecorder.h"
#include "./Utility/println.h"
#include "./InputWrapper.h"

#include <set>
#include <iostream>
#include <string_view>

extern std::set<std::string_view> cmd_arg_set;
[[noreturn]] extern void lippincott() noexcept;

P1D::Driver::~Driver()
{
}
P1D::Driver::Worker::~Worker()
{
}

P1D::Driver::Driver(unsigned const rank, unsigned const size)
try : rank{rank}, size{size} {
    // init recorders
    //
    recorders["energy"] = std::make_unique<EnergyRecorder>(rank, size, cmd_arg_set.count("-load"));
    recorders["fields"] = std::make_unique<FieldRecorder>(rank, size);
    recorders["moment"] = std::make_unique<MomentRecorder>(rank, size);
    recorders["particles"] = std::make_unique<ParticleRecorder>(rank, size);

    // init master delegate
    //
    delegate = std::make_unique<SubdomainDelegate>(rank, size);
    master = std::make_unique<MasterDelegate>(delegate.get());

    // init master domain
    //
    if (0 == rank) {
        println(std::cout, __PRETTY_FUNCTION__, "> initializing domain(s)");
    }
    {
        Real const Mx = Real{Input::Nx}/size;
        ParamSet const params({rank*Mx, Mx});

        // master
        //
        domain = std::make_unique<Domain>(params, master.get());
    }

    // init particles or load snapshot
    //
    if (cmd_arg_set.count("-load")) {
        iteration_count = Snapshot{rank, size, domain->params, -1} >> *domain;
    } else {
        for (PartSpecies &sp : domain->part_species) {
            sp.populate();
        }
    }
} catch (...) {
    lippincott();
}

namespace {
    template <std::size_t N>
    void merge_append_assign(std::array<P1D::PartSpecies, N> &master, std::array<P1D::PartSpecies, N> &&worker) {
        for (unsigned i = 0; i < master.size(); ++i) {
            auto  &master_bucket = master[i].bucket;
            auto &&worker_bucket = std::move(worker[i]).bucket;
            std::move(begin(worker_bucket), end(worker_bucket), std::back_inserter(master_bucket));
            worker_bucket.clear();
        }
    }
}
void P1D::Driver::operator()()
{
    // worker setup
    //
    for (unsigned i = 0; i < workers.size(); ++i) {
        Worker &worker = workers[i];
        workers[i].domain = std::make_unique<Domain>(domain->params, &master->workers.at(i));
        worker.handle = std::async(std::launch::async, std::cref(worker));
    }

    // master loop
    //
    master_loop();

    // worker teardown
    //
    for (Worker &worker : workers) {
        worker.handle.get();
        merge_append_assign(domain->part_species, std::move(worker.domain->part_species));
        worker.domain.reset();
    }

    // take snapshot
    //
    if (cmd_arg_set.count("-save")) {
        Snapshot{rank, size, domain->params, iteration_count} << *domain;
    }
}
void P1D::Driver::master_loop()
try {
    for (long outer_step = 1; outer_step <= Input::outer_Nt; ++outer_step) {
        if (delegate->is_master()) {
            println(std::cout, __PRETTY_FUNCTION__, "> ",
                    "steps(x", Input::inner_Nt, ") = ", outer_step, "/", Input::outer_Nt,
                    "; time = ", iteration_count*Input::dt);
        }

        // inner loop
        //
        domain->advance_by(Input::inner_Nt);

        // increment step count
        //
        iteration_count += Input::inner_Nt;

        // record data
        //
        for (auto &pair : recorders) {
            if (pair.second) {
                pair.second->record(*domain, iteration_count);
            }
        }
    }
} catch (...) {
    lippincott();
}
void P1D::Driver::Worker::operator()() const
try {
    for (long outer_step = 1; outer_step <= Input::outer_Nt; ++outer_step) {
        domain->advance_by(Input::inner_Nt);
    }
} catch (...) {
    lippincott();
}
