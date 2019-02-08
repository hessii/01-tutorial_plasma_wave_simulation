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
H1D::Driver::Driver()
{
    // init domain
    //
    switch (Input::algorithm) {
        case PC:
            domain.reset(new Domain_PC{this});
            break;
        case CAMCL:
            domain.reset(new Domain_CAMCL{this});
            break;
    }

    // init recorders
    //
    recorders["energy"].reset(new EnergyRecorder);
    recorders["fields"].reset(new FieldRecorder);
    recorders["moment"].reset(new MomentRecorder);
    recorders["particles"].reset(new ParticleRecorder);
}

void H1D::Driver::operator()() const
{
    long iteration_count{};
    //
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
}
