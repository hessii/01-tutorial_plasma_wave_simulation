//
//  GridQ.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef GridQ_h
#define GridQ_h

#include "./Shape.h"
#include "../Predefined.h"
#include "../Macros.h"

#include <array>
#include <memory>
#include <algorithm>
#include <sstream>

PIC1D_BEGIN_NAMESPACE
/// 1D array with paddings on both ends that act as ghost cells
///
template <class T, long N>
class GridQ {
public:
    constexpr static long size() noexcept { return N; }
    constexpr static long max_size() noexcept { return size() + 2*Pad; }

    GridQ(GridQ const&) = delete;
    GridQ& operator=(GridQ const&) = delete;
    GridQ(GridQ&&) = delete;
    GridQ& operator=(GridQ&&) = delete;

private:
    static_assert(size() > 0, "at least one element");
    using Backend = std::array<T, max_size()>;
    std::unique_ptr<Backend> ptr;

public:
    explicit GridQ() : ptr{std::make_unique<Backend>()} {}

    // iterators
    //
    using       iterator = T      *;
    using const_iterator = T const*;

    [[nodiscard]] T const *begin() const noexcept {
        return ptr->data() + Pad;
    }
    [[nodiscard]] T       *begin()       noexcept {
        return ptr->data() + Pad;
    }
    [[nodiscard]] T const *end() const noexcept {
        return begin() + size();
    }
    [[nodiscard]] T       *end()       noexcept {
        return begin() + size();
    }

    [[nodiscard]] T const *dead_begin() const noexcept {
        return begin() - Pad;
    }
    [[nodiscard]] T       *dead_begin()       noexcept {
        return begin() - Pad;
    }
    [[nodiscard]] T const *dead_end() const noexcept {
        return end() + Pad;
    }
    [[nodiscard]] T       *dead_end()       noexcept {
        return end() + Pad;
    }

    // subscripts; index relative to the first non-padding element (i.e., relative to *begin())
    //
    [[nodiscard]] T const &operator[](long const i) const noexcept {
        return *(begin() + i);
    }
    [[nodiscard]] T       &operator[](long const i)       noexcept {
        return *(begin() + i);
    }

    /// content filling (including paddings)
    ///
    void fill(T const &v) noexcept {
        std::fill(dead_begin(), dead_end(), v);
    }

    /// grid interpolator
    ///
    template <long Order> [[nodiscard]]
    T interp(Shape<Order> const &sx) const noexcept {
        T y{};
        for (long j = 0; j <= Order; ++j) {
            y += (*this)[sx.i[j]]*sx.w[j];
        }
        return y;
    }

    /// particle deposit; in-place operation
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
            filtered[i] = (source[i-1] + 2*source[i] + source[i+1]) * .25;
        }
    }

    // pretty print (buffered)
    //
    template <class CharT, class Traits>
    friend decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, GridQ const &g) {
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
PIC1D_END_NAMESPACE

#endif /* GridQ_h */
