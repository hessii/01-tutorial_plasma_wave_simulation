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
        std::invalid_argument(std::string(__FUNCTION__) + " - non-positive parallel thermal speed");
    }
    if (T2OT1 <= 0) {
        std::invalid_argument(std::string(__FUNCTION__) + " - non-positive temperature ratio");
    }
    this->vth1 = vth1;
    this->T2OT1 = T2OT1;
}

auto H1D::MaxwellianVDF::variate() const
-> Particle {
    Particle ptl = load();
    // rescale
    ptl.vel *= vth1;
    ptl.pos_x *= Input::Nx; // [0, Nx)
    return ptl;
}
auto H1D::MaxwellianVDF::load() const
-> Particle {
    // position
    //
    Real const pos_x = uniform_real(); // [0, 1]

    // velocity in field-aligned frame (Hu et al., 2010, doi:10.1029/2009JA015158)
    //
    Real const phi1 = uniform_real()*2*M_PI; // [0, 2pi]
    Real const v1 = std::sqrt(-std::log(uniform_real()))*std::sin(phi1); // v_para
    //
    Real const phi2 = uniform_real()*2*M_PI; // [0, 2pi]
    Real const _v2 = std::sqrt(-std::log(uniform_real())*T2OT1);
    Real const v2 = std::cos(phi2)*_v2; // in-plane v_perp
    Real const v3 = std::sin(phi2)*_v2; // out-of-plane v_perp

    // velocity in Cartesian
    //
    Vector const vel = v1*e1 + v2*e2 + v3*e3;

    return Particle{vel, pos_x};
}
