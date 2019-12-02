//
//  BitReversedPattern.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 11/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef BitReversedPattern_h
#define BitReversedPattern_h

#include "../Macros.h"

#include <limits>

PIC1D_BEGIN_NAMESPACE
/**
 @brief Bit reversed pattern from Birdsall and Langdon (1985).
 @discussion The original implementation is found in Kaijun's PIC code.

 The numbers will repeat once the `sequence' variable wraps around.
 @note It satisfies the UniformRandomBitGenerator requirement.
 */
template <unsigned base>
class BitReversedPattern final {
    [[nodiscard]] static constexpr bool is_prime(unsigned const prime) {
        if (prime < 2) throw prime;
        unsigned i = prime;
        while (prime % --i);
        return 1 == i;
    }
    static_assert(base > 1 && is_prime(base), "base should be a prime number greater than 1");

public: // UniformRandomBitGenerator requirement
    using result_type = unsigned long;

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return _max; }

    [[nodiscard]] result_type operator()() noexcept { return next_pattern(sequence++); }

public:
    BitReversedPattern(BitReversedPattern const&) = delete;
    BitReversedPattern &operator=(BitReversedPattern const&) = delete;
    constexpr BitReversedPattern() noexcept = default;

private:
    result_type sequence{1};
    static constexpr result_type _max = [x = result_type{base}]() mutable noexcept {
        constexpr result_type max = std::numeric_limits<result_type>::max()/base;
        while (x < max) { x *= base; }
        return x; // base^n where n is an integer such that x < std::numeric_limits<result_type>::max()
    }();

    [[nodiscard]] static constexpr result_type next_pattern(result_type sequence) noexcept {
        result_type power = max(), bit_pattern = 0;
        while (sequence > 0) {
            bit_pattern += (sequence % base) * (power /= base);
            sequence /= base;
        }
        return bit_pattern;
    }
};

// not for public use
//
void test_BitReversedPattern();
PIC1D_END_NAMESPACE

#endif /* BitReversedPattern_h */
