//
//  Shape.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/16/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Shape_h
#define Shape_h

#include "../Predefined.h"
#include "../Macros.h"

#include <cmath>

HYBRID1D_BEGIN_NAMESPACE
template <long Order> struct Shape;

// 1st-order CIC
//
template <>
struct Shape<1> {
    Real w[2];
    long i[2];
    
    explicit Shape() noexcept = default;
    explicit Shape(Real x) noexcept : Shape() { (*this)(x); }
    
    void operator()(Real x) noexcept {
        // where x = x/Dx
        i[0] = i[1] = static_cast<long>(std::floor(x));
        i[1] += 1;
        w[1] = x - i[0];
        w[0] = 1 - w[1];
    }
};

// 2nd-order TSC
//
template <>
struct Shape<2> {
    long i[3];
    Real w[3];
    
    explicit Shape() noexcept = default;
    explicit Shape(Real x) noexcept : Shape() { (*this)(x); }
    
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
};
HYBRID1D_END_NAMESPACE

#endif /* Shape_h */
