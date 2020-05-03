//
//  Range.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/7/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Range_h
#define Range_h

#include "../Predefined.h"
#include "../Macros.h"

#include <ostream>

PIC1D_BEGIN_NAMESPACE
/// represents a range between two points, a and b.
///
struct [[nodiscard]] Range {
    Real loc; //!< beginning of the range.
    Real len; //!< length of the interval; must be non-negative.
    //
    [[nodiscard]] constexpr Real min() const noexcept { return loc; }
    [[nodiscard]] constexpr Real max() const noexcept { return loc + len; }

    /// return true if a point, x, is contained in [a, b)
    ///
    [[nodiscard]] constexpr bool is_member(Real const x) const noexcept {
        return x >= min() && x < max();
    }

    // compound operations
    //
    constexpr Range &operator+=(Real const &o) noexcept {
        loc += o;
        return *this;
    }
    constexpr Range &operator-=(Real const &o) noexcept {
        return *this += -o;
    }
    constexpr Range &operator*=(Real const &o) noexcept {
        if (o >= 0) {
            loc *= o; len *= o;
        } else {
            loc = o*max(); len *= -o;
        }
        return *this;
    }
    constexpr Range &operator/=(Real const &o) noexcept {
        return *this *= 1/o;
    }

    // unary operations
    //
    [[nodiscard]] friend constexpr Range const &operator+(Range const &s) noexcept {
        return s;
    }
    [[nodiscard]] friend constexpr Range operator-(Range s) noexcept {
        s *= -1;
        return s;
    }

    // binary operations
    //
    [[nodiscard]] friend constexpr Range operator+(Range a, Real const &b) noexcept {
        a += b;
        return a;
    }
    [[nodiscard]] friend constexpr Range operator-(Range a, Real const &b) noexcept {
        a -= b;
        return a;
    }
    [[nodiscard]] friend constexpr Range operator*(Range a, Real const &b) noexcept {
        a *= b;
        return a;
    }
    [[nodiscard]] friend constexpr Range operator/(Range a, Real const &b) noexcept {
        a /= b;
        return a;
    }

    [[nodiscard]] friend constexpr Range operator+(Real const &b, Range a) noexcept {
        a += b;
        return a;
    }
    [[nodiscard]] friend constexpr Range operator-(Real const &b, Range const &a) noexcept {
        return (-a) + b;
    }
    [[nodiscard]] friend constexpr Range operator*(Real const &b, Range a) noexcept {
        a *= b;
        return a;
    }

private:
    // pretty print
    //
    template <class CharT, class Traits>
    friend decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, Range const &r) {
        return os << '{' << r.min() << ", " << r.max() << '}';
    }
};
PIC1D_END_NAMESPACE

#endif /* Range_h */
