//
//  Vector.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Vector_h
#define Vector_h

#include "../Predefined.h"
#include "../Macros.h"

#include <ostream>

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
    constexpr explicit Vector(Real const v) noexcept : x{v}, y{v}, z{v} {}
    constexpr Vector(Real const x, Real const y, Real const z) noexcept : x{x}, y{y}, z{z} {}

    // vector calculus
    //
    friend Real dot(Vector const &A, Vector const &B) noexcept {
        return A.x*B.x + A.y*B.y + A.z*B.z;
    }
    friend Vector cross(Vector const &A, Vector const &B) noexcept {
        return {
            A.y*B.z - A.z*B.y,
            A.z*B.x - A.x*B.z,
            A.x*B.y - A.y*B.x
        };
    }

    // compound operations: vector @= vector, where @ is one of +, -, *, and / (element-wise)
    //
    Vector &operator+=(Vector const &v) noexcept {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    Vector &operator-=(Vector const &v) noexcept {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    Vector &operator*=(Vector const &v) noexcept {
        x *= v.x; y *= v.y; z *= v.z;
        return *this;
    }
    Vector &operator/=(Vector const &v) noexcept {
        x /= v.x; y /= v.y; z /= v.z;
        return *this;
    }

    // compound operations: vector @= real, where @ is one of +, -, *, and / (applied to all elements)
    //
    Vector &operator+=(Real const &s) noexcept {
        x += s; y += s; z += s;
        return *this;
    }
    Vector &operator-=(Real const &s) noexcept {
        x -= s; y -= s; z -= s;
        return *this;
    }
    Vector &operator*=(Real const &s) noexcept {
        x *= s; y *= s; z *= s;
        return *this;
    }
    Vector &operator/=(Real const &s) noexcept {
        x /= s; y /= s; z /= s;
        return *this;
    }

    // unary operations
    //
    friend Vector const &operator+(Vector const &v) noexcept {
        return v;
    }
    friend Vector operator-(Vector v) noexcept {
        return v *= Real{-1};
    }

    // binary operations: vector @ {vector|real}, where @ is one of +, -, *, and /
    //
    template <class B>
    friend Vector operator+(Vector a, B const &b) noexcept {
        return a += b;
    }
    template <class B>
    friend Vector operator-(Vector a, B const &b) noexcept {
        return a -= b;
    }
    template <class B>
    friend Vector operator*(Vector a, B const &b) noexcept {
        return a *= b;
    }
    template <class B>
    friend Vector operator/(Vector a, B const &b) noexcept {
        return a /= b;
    }

    // binary operations: real @ vector, where @ is one of +, -, *, and /
    //
    friend Vector operator+(Real const &b, Vector a) noexcept {
        return a += b;
    }
    friend Vector operator-(Real const &a, Vector const &b) noexcept {
        return Vector{a} -= b;
    }
    friend Vector operator*(Real const &b, Vector a) noexcept {
        return a *= b;
    }
    friend Vector operator/(Real const &a, Vector const &b) noexcept {
        return Vector{a} /= b;
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, Vector const &v) {
        return os << '{'
        << v.x << ", "
        << v.y << ", "
        << v.z << '}';
    }
};
HYBRID1D_END_NAMESPACE

#endif /* Vector_h */
