//
//  CAMCLDomain.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "CAMCLDomain.h"
#include "Delegate.h"

#include <stdexcept>

H1D::CAMCLDomain::CAMCLDomain()
{
}

void H1D::CAMCLDomain::advance_by(unsigned const n_steps)
{
    Domain const &domain = *this;

    // pre-process
    //
    if (!is_recurring_pass) { // execute only once
        is_recurring_pass = true;
        //
        if (nullptr == delegate) {
            throw std::runtime_error(std::string(__FUNCTION__) + " - delegate is not set");
        }
    }

    // cycle
    //
    for (long i = 1; i <= n_steps; ++i) {
        cycle(domain);
    }

    // post-process; collect all moments
    //
    for (Species &sp : species) {
        sp.collect_all();
        delegate->gather(domain, sp);
    }
}
void H1D::CAMCLDomain::cycle(Domain const &domain)
{
    throw std::runtime_error(std::string(__FUNCTION__) + " - not implemented");
}
