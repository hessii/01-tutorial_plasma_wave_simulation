//
//  Particle.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Particle_h
#define Particle_h

#include "./Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

#include <limits>
#include <ostream>

PIC1D_BEGIN_NAMESPACE
/// single particle description
///
struct Particle {
    static constexpr Real quiet_nan = std::numeric_limits<Real>::quiet_NaN();

    Vector vel{quiet_nan}; //!< 3-component velocity vector
    Real pos_x{quiet_nan}; //!< x-component of position

    explicit Particle() noexcept {}
    explicit Particle(Vector const &vel, Real const pos_x) noexcept : vel{vel}, pos_x{pos_x} {
    }

    // for delta-f
    //
    Real g{quiet_nan}; // g(0, x(0), v(0))
    Real fOg{quiet_nan}; // f(0, x(0), v(0))/g(0, x(0), v(0))

    // pretty print
    //
    template <class CharT, class Traits>
    friend decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, Particle const &ptl) {
        return os << '{'
        << ptl.vel << ", "
        << '{' << ptl.pos_x << '}'
        << '}';
    }
};
PIC1D_END_NAMESPACE

#endif /* Particle_h */
