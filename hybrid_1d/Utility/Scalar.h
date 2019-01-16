//
//  Scalar.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Scalar_h
#define Scalar_h

#include "../Predefined.h"
#include "../Macros.h"

#include <ostream>

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

    // compound operations
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

    // unary operations
    //
    friend Scalar const &operator+(Scalar const &s) noexcept {
        return s;
    }
    friend Scalar operator-(Scalar const &s) noexcept {
        return -Real{s};
    }

    // binary operations
    //
    friend Scalar operator+(Scalar a, Scalar const &b) noexcept {
        return a += b;
    }
    friend Scalar operator-(Scalar a, Scalar const &b) noexcept {
        return a -= b;
    }
    friend Scalar operator*(Scalar a, Scalar const &b) noexcept {
        return a *= b;
    }
    friend Scalar operator/(Scalar a, Scalar const &b) noexcept {
        return a /= b;
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend auto operator<<(std::basic_ostream<CharT, Traits> &os, Scalar const &s) -> std::basic_ostream<CharT, Traits> &{
        return os << Real{s};
    }
};
HYBRID1D_END_NAMESPACE

#endif /* Scalar_h */
