//
//  Particle.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Particle_h
#define Particle_h

#include "./Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

#include <ostream>

HYBRID1D_BEGIN_NAMESPACE
/// single particle description
///
struct Particle {
    Vector vel; //!< 3-component velocity vector
    Real pos_x; //!< x-component of position

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
HYBRID1D_END_NAMESPACE

#endif /* Particle_h */
