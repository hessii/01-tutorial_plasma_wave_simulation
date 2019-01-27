//
//  MaxwellianVDF.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MaxwellianVDF.h"
#include "../Inputs.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
    constexpr H1D::Real quiet_nan = std::numeric_limits<H1D::Real>::quiet_NaN();
}

H1D::MaxwellianVDF::MaxwellianVDF() noexcept
: vth1(quiet_nan), T2OT1(quiet_nan) {
}
H1D::MaxwellianVDF::MaxwellianVDF(Real const vth1, Real const T2OT1)
: MaxwellianVDF() {
    if (vth1 <= 0) {
        std::invalid_argument(std::string(__PRETTY_FUNCTION__) + " - non-positive parallel thermal speed");
    }
    if (T2OT1 <= 0) {
        std::invalid_argument(std::string(__PRETTY_FUNCTION__) + " - non-positive temperature ratio");
    }
    this->vth1 = vth1;
    this->T2OT1 = T2OT1;
}

auto H1D::MaxwellianVDF::operator()() const
-> Particle {
    Particle ptl = load();
    // rescale
    ptl.vel *= vth1;
    ptl.pos_x *= Input::Nx;
    return ptl;
}
auto H1D::MaxwellianVDF::load() const
-> Particle {
    // position
    //
    Real const pos_x = uniform_real();

    // velocity
    //
    Real const phi = uniform_real()*2*M_PI; // [0, 2pi]
    Real const cos_alpha = uniform_real()*2 + 1; // [-1, 1]
    Real const v =
}
