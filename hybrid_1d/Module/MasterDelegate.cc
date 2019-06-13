//
//  MasterDelegate.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MasterDelegate.h"
#include "Charge.h"
#include "Current.h"
#include "Species.h"

#include <stdexcept>

void H1D::MasterDelegate::gather(Domain const& domain, Charge &charge)
{
    // 1. local gather first
    //
    Delegate::gather(domain, charge);
    throw std::domain_error(__PRETTY_FUNCTION__);

    // 2. reduce to master
    //

    // 3. broadcast to workers
    //
}
void H1D::MasterDelegate::gather(Domain const& domain, Current &current)
{
    // 1. local gather first
    //
    Delegate::gather(domain, current);
    throw std::domain_error(__PRETTY_FUNCTION__);

    // 2. reduce to master
    //

    // 3. broadcast to workers
    //
}
void H1D::MasterDelegate::gather(Domain const& domain, Species &sp)
{
    // 1. local gather first
    //
    Delegate::gather(domain, sp);
    throw std::domain_error(__PRETTY_FUNCTION__);

    // 2. reduce to master
    //

    // 3. broadcast to workers
    //
}
