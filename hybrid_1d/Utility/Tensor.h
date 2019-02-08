//
//  Tensor.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Tensor_h
#define Tensor_h

#include "./Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

#include <ostream>

HYBRID1D_BEGIN_NAMESPACE
/// compact symmetric tensor
///
struct Tensor {
    // tensor elements
    //
    Real xx{}, yy{}, zz{}; // diagonal
    Real xy{}, yz{}, zx{}; // off-diag

    // constructors
    //
    constexpr explicit Tensor() noexcept {}
    constexpr explicit Tensor(Real const v) noexcept : xx{v}, yy{v}, zz{v}, xy{v}, yz{v}, zx{v} {}
    constexpr Tensor(Real xx, Real yy, Real zz, Real xy, Real yz, Real zx) noexcept : xx{xx}, yy{yy}, zz{zz}, xy{xy}, yz{yz}, zx{zx} {}

    // access to lower and upper halves
    //
    Vector const &lo() const noexcept { return *reinterpret_cast<Vector const*>(&xx); }
    Vector       &lo()       noexcept { return *reinterpret_cast<Vector      *>(&xx); }
    Vector const &hi() const noexcept { return *reinterpret_cast<Vector const*>(&xy); }
    Vector       &hi()       noexcept { return *reinterpret_cast<Vector      *>(&xy); }

    // compound operations: tensor @= tensor, where @ is one of +, -, *, and / (element-wise)
    //
    Tensor &operator+=(Tensor const &v) noexcept {
        xx += v.xx; yy += v.yy; zz += v.zz;
        xy += v.xy; yz += v.yz; zx += v.zx;
        return *this;
    }
    Tensor &operator-=(Tensor const &v) noexcept {
        xx -= v.xx; yy -= v.yy; zz -= v.zz;
        xy -= v.xy; yz -= v.yz; zx -= v.zx;
        return *this;
    }
    Tensor &operator*=(Tensor const &v) noexcept {
        xx *= v.xx; yy *= v.yy; zz *= v.zz;
        xy *= v.xy; yz *= v.yz; zx *= v.zx;
        return *this;
    }
    Tensor &operator/=(Tensor const &v) noexcept {
        xx /= v.xx; yy /= v.yy; zz /= v.zz;
        xy /= v.xy; yz /= v.yz; zx /= v.zx;
        return *this;
    }

    // compound operations: tensor @= real, where @ is one of +, -, *, and / (applied to all elements)
    //
    Tensor &operator+=(Real const &s) noexcept {
        xx += s; yy += s; zz += s;
        xy += s; yz += s; zx += s;
        return *this;
    }
    Tensor &operator-=(Real const &s) noexcept {
        xx -= s; yy -= s; zz -= s;
        xy -= s; yz -= s; zx -= s;
        return *this;
    }
    Tensor &operator*=(Real const &s) noexcept {
        xx *= s; yy *= s; zz *= s;
        xy *= s; yz *= s; zx *= s;
        return *this;
    }
    Tensor &operator/=(Real const &s) noexcept {
        xx /= s; yy /= s; zz /= s;
        xy /= s; yz /= s; zx /= s;
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

    // binary operations: tensor @ {vector|real}, where @ is one of +, -, *, and /
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

    // binary operations: real @ tensor, where @ is one of +, -, *, and /
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
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, Tensor const &v) {
        return os << '{'
        << v.xx << ", "
        << v.yy << ", "
        << v.zz << ", "
        << v.xy << ", "
        << v.yz << ", "
        << v.zx << '}';
    }
};

// make sure that memory layout of Tensor and Vector are compatible
//
static_assert(alignof(Tensor) == alignof(Vector) && sizeof(Tensor) == 2*sizeof(Vector), "incompatible memory layout");
HYBRID1D_END_NAMESPACE

#endif /* Tensor_h */
