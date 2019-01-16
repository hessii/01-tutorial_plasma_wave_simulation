//
//  Tensor.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Tensor_h
#define Tensor_h

#include "../Predefined.h"
#include "../Macros.h"

#include <ostream>
#include <cmath>

HYBRID1D_BEGIN_NAMESPACE
/// compact symmetric tensor
///
struct Tensor {
    // tensor elements
    //
    Real x{};
    Real y{};
    Real z{};

    // constructors
    //
    constexpr explicit Tensor() noexcept {}
    constexpr explicit Tensor(Real v) noexcept : x(v), y(v), z(v) {}
    constexpr Tensor(Real x, Real y, Real z) noexcept : x(x), y(y), z(z) {}

    // Tensor calculus
    //
    friend Real dot(Tensor const &A, Tensor const &B) noexcept {
        return A.x*B.x + A.y*B.y + A.z*B.z;
    }
    friend Real norm(Tensor const &A) noexcept {
        return std::sqrt(dot(A, A));
    }
    friend Tensor cross(Tensor const &A, Tensor const &B) noexcept {
        return {
            A.y*B.z - A.z*B.y,
            A.z*B.x - A.x*B.z,
            A.x*B.y - A.y*B.x
        };
    }

    // compound operations: tensor@tensor (element-wise)
    //
    Tensor &operator+=(Tensor const &v) noexcept {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    Tensor &operator-=(Tensor const &v) noexcept {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    Tensor &operator*=(Tensor const &v) noexcept {
        x *= v.x; y *= v.y; z *= v.z;
        return *this;
    }
    Tensor &operator/=(Tensor const &v) noexcept {
        x /= v.x; y /= v.y; z /= v.z;
        return *this;
    }

    // compound operations: tensor@real (element-wise)
    //
    Tensor &operator+=(Real const &s) noexcept {
        x += s; y += s; z += s;
        return *this;
    }
    Tensor &operator-=(Real const &s) noexcept {
        x -= s; y -= s; z -= s;
        return *this;
    }
    Tensor &operator*=(Real const &s) noexcept {
        x *= s; y *= s; z *= s;
        return *this;
    }
    Tensor &operator/=(Real const &s) noexcept {
        x /= s; y /= s; z /= s;
        return *this;
    }

    // unary operations
    //
    friend Tensor const &operator+(Tensor const &v) noexcept {
        return v;
    }
    friend Tensor operator-(Tensor v) noexcept {
        return v *= Real{-1};
    }

    // binary operations: tensor@(tensor|real)
    //
    template <class B>
    friend Tensor operator+(Tensor a, B const &b) noexcept {
        return a += b;
    }
    template <class B>
    friend Tensor operator-(Tensor a, B const &b) noexcept {
        return a -= b;
    }
    template <class B>
    friend Tensor operator*(Tensor a, B const &b) noexcept {
        return a *= b;
    }
    template <class B>
    friend Tensor operator/(Tensor a, B const &b) noexcept {
        return a /= b;
    }

    // binary operations: real@tensor
    //
    friend Tensor operator+(Real const &b, Tensor a) noexcept {
        return a += b;
    }
    friend Tensor operator-(Real const &a, Tensor const &b) noexcept {
        return Tensor{a} -= b;
    }
    friend Tensor operator*(Real const &b, Tensor a) noexcept {
        return a *= b;
    }
    friend Tensor operator/(Real const &a, Tensor const &b) noexcept {
        return Tensor{a} /= b;
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend auto operator<<(std::basic_ostream<CharT, Traits> &os, Tensor const &v) -> std::basic_ostream<CharT, Traits> &{
        return os << "{"
        << v.x << ", "
        << v.y << ", "
        << v.z << "}";
    }
};
HYBRID1D_END_NAMESPACE

#endif /* Tensor_h */
