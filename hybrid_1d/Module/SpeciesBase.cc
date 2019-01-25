//
//  SpeciesBase.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "SpeciesBase.h"
#include "../Inputs.h"

#include <stdexcept>
#include <algorithm>

H1D::_Species::_Species(Real const Oc, Real const op, long const Nc)
: Nc(Nc), Oc(Oc), op(op), bucket(), _mom() {
    if (Nc < 0) {
        throw std::invalid_argument(std::string(__FUNCTION__) + "negative Nc");
    }
}
H1D::_Species &H1D::_Species::operator=(_Species const &o)
{
    Nc = o.Nc;
    Oc = o.Oc;
    op = o.op;
    bucket = o.bucket;
    if ( (false) ) { // do not copy moments
        std::copy(o.moment<0>().dead_begin(), o.moment<0>().dead_end(), moment<0>().dead_begin());
        std::copy(o.moment<1>().dead_begin(), o.moment<1>().dead_end(), moment<1>().dead_begin());
        std::copy(o.moment<2>().dead_begin(), o.moment<2>().dead_end(), moment<2>().dead_begin());
    }
    return *this;
}
