//
//  GridQ.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef GridQ_h
#define GridQ_h

#include "./Shape.h"
#include "../Predefined.h"
#include "../Macros.h"
#include "../InputWrapper.h"

#include <array>
#include <memory>
#include <algorithm>
#include <sstream>

HYBRID1D_BEGIN_NAMESPACE
/// 1D array with paddings on both ends that act as ghost cells
///
template <class T>
class GridQ {
public:
    constexpr static long size() noexcept { return Input::Nx; }
    constexpr static long max_size() noexcept { return size() + 2*Pad; }

private:
    static_assert(max_size() > 0, "at least one element");
    using Backend = std::array<T, max_size()>;
    std::unique_ptr<Backend> ptr;

public:
    explicit GridQ() : ptr{std::make_unique<Backend>()} {}

    // iterators
    //
    using       iterator = T      *;
    using const_iterator = T const*;

    T const *begin() const noexcept {
        return ptr->data() + Pad;
    }
    T       *begin()       noexcept {
        return ptr->data() + Pad;
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

    // subscripts; index relative to the first non-padding element (i.e., relative to *begin())
    //
    T const &operator[](long const i) const noexcept {
        return *(begin() + i);
    }
    T       &operator[](long const i)       noexcept {
        return *(begin() + i);
    }

    /// content filling (including paddings)
    ///
    void fill(T const &v) noexcept {
        std::fill(dead_begin(), dead_end(), v);
    }

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
    template <long Order, class U>
    void deposit(Shape<Order> const &sx, U const &weight) noexcept {
        for (long j = 0; j <= Order; ++j) {
            (*this)[sx.i[j]] += weight*sx.w[j];
        }
    }

protected:
    /// content swap
    ///
    void swap(GridQ &o) noexcept {
        ptr.swap(o.ptr);
    }

    /// 3-point smoothing
    ///
    friend void _smooth(GridQ &filtered, GridQ const &source) noexcept {
        for (long i = 0; i < size(); ++i) {
            filtered[i] = (source[i-1] + 2*source[i] + source[i+1]) *= .25;
        }
    }

    // pretty print
    //
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, GridQ const &g) {
        std::basic_ostringstream<CharT, Traits> ss; {
            ss.flags(os.flags());
            ss.imbue(os.getloc());
            ss.precision(os.precision());
            //
            const_iterator it = g.begin(), end = g.end();
            ss << '{' << *it++; // guarrenteed to be at least one element
            while (it != end) {
                ss << ", " << *it++;
            }
            ss << '}';
        }
        return os << ss.str();
    }
};
HYBRID1D_END_NAMESPACE

#endif /* GridQ_h */
