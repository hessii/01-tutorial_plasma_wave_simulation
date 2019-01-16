//
//  GridQ.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef GridQ_h
#define GridQ_h

#include "./Shape.h"
#include "../Inputs.h"
#include "../Predefined.h"
#include "../Macros.h"

#include <memory>

HYBRID1D_BEGIN_NAMESPACE
template <class T>
class GridQ {
    std::unique_ptr<T[]> _Q;

public:
    constexpr static long size() noexcept { return Global::Nx; }
    constexpr static long max_size() noexcept { return size() + 2*Pad; }

    GridQ(GridQ &&) = default;
    GridQ &operator=(GridQ &&) = default;
    explicit GridQ() : _Q(new T[max_size()]) {}

    // iterators
    //
    using iterator = T*;
    using const_iterator = T const*;

    T const *begin() const noexcept {
        return _Q.get() + Pad;
    }
    T       *begin()       noexcept {
        return _Q.get() + Pad;
    }
    T const *end() const noexcept {
        return begin() + size();
    }
    T       *end()       noexcept {
        return begin() + size();
    }

    T const *dead_begin() const noexcept {
        return begin() - Pad;
    }
    T       *dead_begin()       noexcept {
        return begin() - Pad;
    }
    T const *dead_end() const noexcept {
        return end() + Pad;
    }
    T       *dead_end()       noexcept {
        return end() + Pad;
    }

    // subscripts
    //
    T const &operator[](long i) const noexcept {
        return *(begin() + i);
    }
    T       &operator[](long i)       noexcept {
        return *(begin() + i);
    }

    // helpers
    //
    /// grid interpolator
    ///
    template <long Order>
    T interp(Shape<Order> const &sx) const noexcept {
        T y{};
        for (long j = 0; j <= Order; ++j) {
            y += (*this)[sx.i[j]]*sx.w[j];
        }
        return y;
    }

    /// particle deposit
    ///
    template <long Order>
    void deposit(Shape<Order> const &sx, Real weight) const noexcept {
        for (long j = 0; j <= Order; ++j) {
            (*this)[sx.i[j]] += weight*sx.w[j];
        }
    }

    /// 3-point smoothing
    ///
    friend void smooth(GridQ &lhs, GridQ const &rhs) noexcept {
        for (long i = 0; i < size(); ++i) {
            lhs[i] = (rhs[i-1] + 2*rhs[i] + rhs[i+1]) *= .25;
        }
    }
};
HYBRID1D_END_NAMESPACE

#endif /* GridQ_h */
