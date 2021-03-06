//
//  Driver.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Driver.h"
#include "./Core/Domain.h"
#include "./Recorder/Snapshot.h"
#include "./Recorder/EnergyRecorder.h"
#include "./Recorder/FieldRecorder.h"
#include "./Recorder/MomentRecorder.h"
#include "./Recorder/ParticleRecorder.h"
#include "./Recorder/VHistogramRecorder.h"
#include "./Utility/lippincott.h"
#include "./Utility/println.h"

#include <iostream>
#include <chrono>

P1D::Driver::~Driver()
{
}
P1D::Driver::Worker::~Worker()
{
}

P1D::Driver::Driver(unsigned const rank, unsigned const size, ParamSet const &params)
try : rank{rank}, size{size}, params{params} {
    // init recorders
    //
    recorders["energy"] = std::make_unique<EnergyRecorder>(rank, size, params);
    recorders["fields"] = std::make_unique<FieldRecorder>(rank, size);
    recorders["moment"] = std::make_unique<MomentRecorder>(rank, size);
    recorders["vhists"] = std::make_unique<VHistogramRecorder>(rank, size);
    recorders["particles"] = std::make_unique<ParticleRecorder>(rank, size);

    // init master delegate
    //
    delegate = std::make_unique<SubdomainDelegate>(rank, size);
    master = std::make_unique<MasterDelegate>(delegate.get());

    // init master domain
    //
    if (0 == rank) {
        println(std::cout, __FUNCTION__, "> initializing domain(s)");
    }
    domain = make_domain(params, master.get());

    // init particles or load snapshot
    //
    if (params.snapshot_load) {
        if (0 == rank) {
            print(std::cout, "\tloading snapshots") << std::endl;
        }
        iteration_count = Snapshot{rank, size, params, -1} >> *domain;
    } else {
        if (0 == rank) {
            print(std::cout, "\tinitializing particles") << std::endl;
        }
        for (PartSpecies &sp : domain->part_species) {
            sp.populate();
        }
        for (ColdSpecies &sp : domain->cold_species) {
            sp.populate();
        }
    }
} catch (...) {
    lippincott();
}
auto P1D::Driver::make_domain(ParamSet const &params, Delegate *delegate)
-> std::unique_ptr<Domain> {
    return std::make_unique<Domain>(params, delegate);
}

namespace {
template <class F, class... Args>
auto measure(F&& f, Args&&... args) {
    static_assert(std::is_invocable_v<F&&, Args&&...>);
    auto const start = std::chrono::steady_clock::now();
    {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }
    auto const end = std::chrono::steady_clock::now();
    std::chrono::duration<double> const diff = end - start;
    return diff;
}
}
void P1D::Driver::operator()()
try {
    // worker setup
    //
    for (unsigned i = 0; i < workers.size(); ++i) {
        Worker &worker = workers[i];
        worker.driver = this;
        worker.iteration_count = iteration_count;
        worker.delegate = &master->workers.at(i);
        worker.domain = make_domain(params, worker.delegate);
        worker.handle = std::async(std::launch::async, worker.delegate->wrap_loop(std::ref(worker)), worker.domain.get());
    }

    // master loop
    //
    if (auto const elapsed =
        measure(master->wrap_loop(&Driver::master_loop, this), this->domain.get());
        delegate->is_master()) {
        println(std::cout, "%% time elapsed: ", elapsed.count(), 's');
    }

    // worker teardown
    //
    for (Worker &worker : workers) {
        worker.handle.get();
        worker.domain.reset();
    }

    // take snapshot
    //
    if (params.snapshot_save) {
        if (0 == rank) print(std::cout, "\tsaving snapshots") << std::endl;
        Snapshot{rank, size, params, iteration_count} << *domain;
    }
} catch (...) {
    lippincott();
}
void P1D::Driver::master_loop()
try {
    for (long outer_step = 1; outer_step <= domain->params.outer_Nt; ++outer_step) {
        if (delegate->is_master()) {
            print(std::cout, __FUNCTION__, "> ",
                  "steps(x", domain->params.inner_Nt, ") = ", outer_step, "/", domain->params.outer_Nt,
                  "; time = ", iteration_count*domain->params.dt) << std::endl;
        }

        // inner loop
        //
        domain->advance_by(domain->params.inner_Nt);

        // increment step count
        //
        iteration_count += domain->params.inner_Nt;

        // record data
        //
        if (iteration_count % this->recorders.at("vhists")->recording_frequency &&
            iteration_count % this->recorders.at("particles")->recording_frequency) {
            // no particle collection needed
            //
            for (auto &pair : recorders) {
                if (pair.second) {
                    pair.second->record(*domain, iteration_count);
                }
            }
        } else {
            // collect particles before recording
            //
            auto const *delegate = master.get();
            delegate->teardown(*domain);

            // record data
            //
            for (auto &pair : recorders) {
                if (pair.second) {
                    pair.second->record(*domain, iteration_count);
                }
            }

            // re-distribute particles
            //
            delegate->setup(*domain);
        }
    }
} catch (...) {
    lippincott();
}
void P1D::Driver::Worker::operator()()
try {
    for (long outer_step = 1; outer_step <= domain->params.outer_Nt; ++outer_step) {
        // inner loop
        //
        domain->advance_by(domain->params.inner_Nt);

        // increment step count
        //
        iteration_count += domain->params.inner_Nt;

        // record data
        //
        if (iteration_count % driver->recorders.at("vhists")->recording_frequency &&
            iteration_count % driver->recorders.at("particles")->recording_frequency) {
            // no particle collection needed
            //
        } else {
            // collect particles before recording
            //
            delegate->teardown(*domain);
            delegate->setup(*domain);
        }
    }
} catch (...) {
    lippincott();
}
