//
//  Driver.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "Driver.h"
#include "./Module/Domain_PC.h"
#include "./Module/Domain_CAMCL.h"
#include "./Recorder/EnergyRecorder.h"
#include "./Recorder/FieldRecorder.h"
#include "./Recorder/MomentRecorder.h"
#include "./Recorder/ParticleRecorder.h"
#include "./Utility/println.h"
#include <Inputs.h>

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
    long step_count{};
    for (long i_step = 1; i_step <= Input::outer_Nt; ++i_step) {
        println(std::cout, "%Hybrid1D> ",
                " - steps(x", Input::inner_Nt, ") = ", i_step, "/", Input::outer_Nt,
                "; time = ", step_count*Input::dt);

        // advance simulation
        //
        domain->advance_by(Input::inner_Nt);

        // update step count
        //
        long const steps = step_count += Input::inner_Nt;

        // record data
        //
        for (auto &pair : recorders) {
            if (pair.second) {
                pair.second->record(*domain, steps);
            }
        }
    }
}
