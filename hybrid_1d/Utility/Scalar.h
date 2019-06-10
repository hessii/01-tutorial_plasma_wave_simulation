//
//  Scalar.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Scalar_h
#define Scalar_h

#include "../Predefined.h"
#include "../Macros.h"

#include <ostream>

HYBRID1D_BEGIN_NAMESPACE
class Scalar {
    Real v{};

public:
    // value access
    //
    constexpr explicit operator Real() const noexcept { return v; }

    // constructors
    //
    constexpr explicit Scalar() noexcept {}
    constexpr Scalar(Real const v) noexcept : v{v} {}

    // compound operations
    //
    constexpr Scalar &operator+=(Scalar const &o) noexcept {
        v += Real{o};
        return *this;
    }
    constexpr Scalar &operator-=(Scalar const &o) noexcept {
        v -= Real{o};
        return *this;
    }
    constexpr Scalar &operator*=(Scalar const &o) noexcept {
        v *= Real{o};
        return *this;
    }
    constexpr Scalar &operator/=(Scalar const &o) noexcept {
        v /= Real{o};
        return *this;
    }

    // unary operations
    //
    [[nodiscard]] friend constexpr Scalar operator+(Scalar const &s) noexcept {
        return +Real{s};
    }
    [[nodiscard]] friend constexpr Scalar operator-(Scalar const &s) noexcept {
        return -Real{s};
    }

    // binary operations
    //
    [[nodiscard]] friend constexpr Scalar operator+(Scalar a, Scalar const &b) noexcept {
        return a += b;
    }
    [[nodiscard]] friend constexpr Scalar operator-(Scalar a, Scalar const &b) noexcept {
        return a -= b;
    }
    [[nodiscard]] friend constexpr Scalar operator*(Scalar a, Scalar const &b) noexcept {
        return a *= b;
    }
    [[nodiscard]] friend constexpr Scalar operator/(Scalar a, Scalar const &b) noexcept {
        return a /= b;
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, Scalar const &s) {
        return os << Real{s};
    }
};
HYBRID1D_END_NAMESPACE

#endif /* Scalar_h */
