//
//  Shape.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/16/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Shape_h
#define Shape_h

#include "../Predefined.h"
#include "../Macros.h"

#include <algorithm>
#include <iterator>
#include <cmath>
#include <ostream>

PIC1D_BEGIN_NAMESPACE
template <long Order> struct Shape;

/// 1st-order CIC
///
template <>
struct Shape<1> {
    Real w[2]; //!< weights
    long i[2]; //!< indices

    explicit Shape() noexcept = default;
    explicit Shape(Real const x) noexcept { (*this)(x); }

    void operator()(Real const x) noexcept {
        // where x = x/Dx
        i[0] = i[1] = static_cast<long>(std::floor(x));
        i[1] += 1;
        w[1] = x - i[0];
        w[0] = 1 - w[1];
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, Shape const &s) {
        return os << "Shape["
        << "indices = {" << s.i[0] << ", " << s.i[1] << "}, "
        << "weights = {" << s.w[0] << ", " << s.w[1] << '}'
        << ']';
    }
};

/// 2nd-order TSC
///
template <>
struct Shape<2> {
    long i[3]; //!< indices
    Real w[3]; //!< weights

    explicit Shape() noexcept = default;
    explicit Shape(Real const x) noexcept { (*this)(x); }

    void operator()(Real x) noexcept {
        // where x = x/Dx
        constexpr Real half = 0.5, _3_4 = 0.75;

        i[0] = i[1] = i[2] = static_cast<long>(std::round(x));
        i[0] -= 1;
        i[2] += 1;
        //
        // i = i1
        //
        x = i[1] - x; // (i - x)
        w[1] = _3_4 - (x*x); // i = i1, w1 = 3/4 - (x-i)^2
        //
        // i = i0
        //
        x += half; // (i - x) + 1/2
        w[0] = half * (x*x); // i = i0, w0 = 1/2 * (1/2 - (x-i))^2
        //
        // i = i2
        //
        w[2] = 1 - (w[0]+w[1]);
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, Shape const &s) {
        return os << "Shape["
        << "indices = {" << s.i[0] << ", " << s.i[1] << ", " << s.i[2] << "}, "
        << "weights = {" << s.w[0] << ", " << s.w[1] << ", " << s.w[2] << '}'
        << ']';
    }
};

/// 3rd-order
///
///  W(x) =
///         (2 + x)^3/6           for -2 <= x < -1
///         (4 - 6x^2 - 3x^3)/6   for -1 <= x < 0
///         (4 - 6x^2 + 3x^3)/6   for 0 <= x < 1
///         (2 - x)^3/6           for 1 <= x < 2
///         0                     otherwise
///
template <>
struct Shape<3> {
    long i[4]; //!< indices
    Real w[4]; //!< weights

    explicit Shape() noexcept = default;
    explicit Shape(Real const x) noexcept { (*this)(x); }

    void operator()(Real const x) noexcept {
        Real tmp;
        std::fill(std::begin(w), std::end(w), 1./6);
        i[0] = static_cast<long>(std::ceil(x)) - 2;
        i[1] = i[0] + 1;
        i[2] = i[1] + 1;
        i[3] = i[2] + 1;

        // for -2 <= x < -1
        //
        tmp = 2 + (i[0] - x); // -1 + i0 - x
        w[0] *= tmp*tmp*tmp;

        // for 1 <= x < 2
        //
        tmp = 2 - (i[3] - x); // 2 + i0 - x
        w[3] *= tmp*tmp*tmp;

        // for -1 <= x < 0
        //
        tmp = x - i[1]; // x - i0
        w[1] *= 4 + 3*tmp*tmp*(tmp - 2);

        // for 0 <= x < 1
        //
        w[2] = 1 - (w[0] + w[1] + w[3]);
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, Shape const &s) {
        return os << "Shape["
        << "indices = {" << s.i[0] << ", " << s.i[1] << ", " << s.i[2] << ", " << s.i[3] << "}, "
        << "weights = {" << s.w[0] << ", " << s.w[1] << ", " << s.w[2] << ", " << s.w[3] << '}'
        << ']';
    }
};
PIC1D_END_NAMESPACE

#endif /* Shape_h */
