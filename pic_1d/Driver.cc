//
//  Driver.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Driver.h"
#include "./Module/Domain.h"
#include "./Boundary/Delegate.h"
#include "./Recorder/EnergyRecorder.h"
#include "./Recorder/FieldRecorder.h"
#include "./Recorder/MomentRecorder.h"
#include "./Recorder/ParticleRecorder.h"
#include "./Utility/println.h"
#include "./InputWrapper.h"

#include <iostream>

P1D::Driver::~Driver()
{
}
P1D::Driver::Driver()
{
    // init recorders
    //
    recorders["energy"] = std::make_unique<EnergyRecorder>();
    recorders["fields"] = std::make_unique<FieldRecorder>();
    recorders["moment"] = std::make_unique<MomentRecorder>();
    recorders["particles"] = std::make_unique<ParticleRecorder>();

    // init delegate
    //
    delegate = std::make_unique<Delegate>();

    // init domain
    //
    domain = std::make_unique<Domain>(ParamSet({0, Input::Nx}), delegate.get());
}

void P1D::Driver::operator()()
{
    for (long outer_step = 1; outer_step <= Input::outer_Nt; ++outer_step) {
        println(std::cout, "%PIC1D> ",
                "steps(x", Input::inner_Nt, ") = ", outer_step, "/", Input::outer_Nt,
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
}
