//
//  SpeciesBase.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "SpeciesBase.h"

#include <stdexcept>
#include <algorithm>
#include <utility>
#include <thread>

H1D::_Species::_Species(Real const Oc, Real const op, long const Nc)
: Nc(Nc), Oc{Oc}, op{op}, bucket{}, _moms(1) {
    if (Nc < 0) {
        throw std::invalid_argument(std::string{__FUNCTION__} + "negative Nc");
    }
    static unsigned const n_threads = std::thread::hardware_concurrency();
    if (Input::enable_asynchronous_particle_push) {
        _moms.resize(n_threads);
    }
}
H1D::_Species &H1D::_Species::operator=(_Species const &o)
{
    Nc = o.Nc;
    Oc = o.Oc;
    op = o.op;
    bucket = o.bucket;
    if constexpr ( (false) ) { // do not copy moments
        std::copy(o.moment<0>().dead_begin(), o.moment<0>().dead_end(), moment<0>().dead_begin());
        std::copy(o.moment<1>().dead_begin(), o.moment<1>().dead_end(), moment<1>().dead_begin());
        std::copy(o.moment<2>().dead_begin(), o.moment<2>().dead_end(), moment<2>().dead_begin());
    }
    return *this;
}
H1D::_Species &H1D::_Species::operator=(_Species &&o)
{
    Nc = std::move(o.Nc);
    Oc = std::move(o.Oc);
    op = std::move(o.op);
    bucket = std::move(o.bucket);
    if constexpr ( (false) ) { // do not move moments
        std::move(o.moment<0>().dead_begin(), o.moment<0>().dead_end(), moment<0>().dead_begin());
        std::move(o.moment<1>().dead_begin(), o.moment<1>().dead_end(), moment<1>().dead_begin());
        std::move(o.moment<2>().dead_begin(), o.moment<2>().dead_end(), moment<2>().dead_begin());
    }
    return *this;
}
