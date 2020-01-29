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

    // init domain
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
        if (cmd_arg_set.count("-load")) { // snapshot loading only master thread
            iteration_count = Snapshot{rank, size, params, -1} >> *domain;
        } else { // init particles; only master thread
            for (PartSpecies &sp : domain->part_species) {
                sp.populate();
            }
        }

        // workers; should be initialized by master thread (not worker thread)
        //
        for (unsigned i = 0; i < workers.size(); ++i) {
            workers[i].domain = std::make_unique<Domain>(params, &master->workers.at(i));
        }
    }
} catch (...) {
    lippincott();
}

void P1D::Driver::operator()()
{
    // worker setup
    //
    for (Worker &worker : workers) {
        worker.handle = std::async(std::launch::async, [&worker]()->void { worker(); });
    }

    // master loop
    //
    master_loop();

    // worker teardown
    //
    for (Worker &worker : workers) {
        worker.handle.get();
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
    if (cmd_arg_set.count("-save")) { // snapshot save only master
        Snapshot{rank, size, domain->params, iteration_count} << *domain;
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
