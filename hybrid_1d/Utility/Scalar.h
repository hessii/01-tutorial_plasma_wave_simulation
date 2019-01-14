//
//  Scalar.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Scalar_h
#define Scalar_h

#include "../Macros.h"
#include "../Predefined.h"

#include <type_traits>

HYBRID1D_BEGIN_NAMESPACE
class Scalar {
    Real _v{};

public:
    // value access
    //
    constexpr explicit operator Real() const noexcept { return _v; }

    // constructors
    //
    constexpr explicit Scalar() noexcept {}
    constexpr Scalar(Real v) noexcept : _v(v) {}

    // compound scalar-scalar arithmetic operations
    //
    Scalar &operator+=(Scalar const &o) noexcept {
        _v += o._v;
        return *this;
    }
    Scalar &operator-=(Scalar const &o) noexcept {
        _v -= o._v;
        return *this;
    }
    Scalar &operator*=(Scalar const &o) noexcept {
        _v *= o._v;
        return *this;
    }
    Scalar &operator/=(Scalar const &o) noexcept {
        _v /= o._v;
        return *this;
    }
};
HYBRID1D_END_NAMESPACE

#endif /* Scalar_h */
