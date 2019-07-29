//
//  Driver.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Driver.h"
#include "./Module/Domain_PC.h"
#include "./Module/Domain_CAMCL.h"
#include "./Boundary/WorkerDelegate.h"
#include "./Recorder/EnergyRecorder.h"
#include "./Recorder/FieldRecorder.h"
#include "./Recorder/MomentRecorder.h"
#include "./Recorder/ParticleRecorder.h"
#include "./Utility/println.h"
#include "./InputWrapper.h"

#include <iostream>

H1D::Driver::~Driver()
{
}
H1D::Driver::Worker::~Worker()
{
}

H1D::Driver::Driver()
{
    // init recorders
    //
    recorders["energy"] = std::make_unique<EnergyRecorder>();
    recorders["fields"] = std::make_unique<FieldRecorder>();
    recorders["moment"] = std::make_unique<MomentRecorder>();
    recorders["particles"] = std::make_unique<ParticleRecorder>();

    // init master delegate
    //
    master = std::make_unique<MasterDelegate>(std::make_unique<Delegate>());

    // init domain
    //
    switch (Input::algorithm) {
        case PC: {
            using Domian = Domain_PC;

            // master
            //
            domain = std::make_unique<Domian>(master.get());

            // workers
            //
            for (unsigned i = 0; i < workers.size(); ++i) {
                workers[i].domain = std::make_unique<Domian>(&master->workers.at(i));
            }

            break;
        }
        case CAMCL: {
            using Domain = Domain_CAMCL;
            // master
            //
            domain = std::make_unique<Domain>(master.get());

            // workers
            //
            for (unsigned i = 0; i < workers.size(); ++i) {
                workers[i].domain = std::make_unique<Domain>(&master->workers.at(i));
            }

            break;
        }
    }
}

void H1D::Driver::operator()()
{
    // worker setup
    //
    for (Worker &worker : workers) {
        worker.handle = std::async(std::launch::async, [&worker]()->void { worker(); });
    }

    // master loop
    //
    long iteration_count{};
    for (long outer_step = 1; outer_step <= Input::outer_Nt; ++outer_step) {
        println(std::cout, "%Hybrid1D>",
                " - steps(x", Input::inner_Nt, ") = ", outer_step, "/", Input::outer_Nt,
                "; time = ", iteration_count*Input::dt);

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

    // worker teardown
    //
    for (Worker &worker : workers) {
        worker.handle.wait();
    }
}
void H1D::Driver::Worker::operator()() const
{
    for (long outer_step = 1; outer_step <= Input::outer_Nt; ++outer_step) {
        domain->advance_by(Input::inner_Nt);
    }
}
