//
//  Vector.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Vector_h
#define Vector_h

#include "../Macros.h"
#include "../Predefined.h"

#include <type_traits>
#include <cmath>

HYBRID1D_BEGIN_NAMESPACE
struct Vector {
    // vector elements
    //
    Real x{};
    Real y{};
    Real z{};

    // constructors
    //
    constexpr explicit Vector() noexcept {}
    constexpr explicit Vector(Real v) noexcept : x(v), y(v), z(v) {}
    constexpr Vector(Real x, Real y, Real z) noexcept : x(x), y(y), z(z) {}

    // compound vector-vector (element-wise) arithmetic operations
    //
    Vector &operator+=(Vector const &o) noexcept {
        x += o.x; y += o.y; z += o.z;
        return *this;
    }
    Vector &operator-=(Vector const &o) noexcept {
        x -= o.x; y -= o.y; z -= o.z;
        return *this;
    }
    Vector &operator*=(Vector const &o) noexcept {
        x *= o.x; y *= o.y; z *= o.z;
        return *this;
    }
    Vector &operator/=(Vector const &o) noexcept {
        x /= o.x; y /= o.y; z /= o.z;
        return *this;
    }

    // vector calculus
    //
    friend Real dot(Vector const &A, Vector const &B) noexcept {
        return A.x*B.x + A.y*B.y + A.z*B.z;
    }
    friend Real abs(Vector const &A, Vector const &B) noexcept {
        return std::sqrt(dot(A, B));
    }
    friend Vector cross(Vector const &A, Vector const &B) noexcept {
        return {
            A.y*B.z - A.z*B.y,
            A.z*B.x - A.x*B.z,
            A.x*B.y - A.y*B.x
        };
    }
};
HYBRID1D_END_NAMESPACE

#endif /* Vector_h */
