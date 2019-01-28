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

H1D::Driver::~Driver()
{
}
H1D::Driver::Driver()
: step_count(0) {
    // init domain
    //
    switch (Input::algorithm) {
        case PC:
            domain.reset(new Domain_PC(this));
            break;
        case CAMCL:
            domain.reset(new Domain_CAMCL(this));
            break;
    }

    // init recorders
    //
}

void H1D::Driver::run()
{
    for (unsigned i = 1; i <= Input::outer_Nt; ++i) {
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
