//
//  BorisPush.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/16/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef BorisPush_h
#define BorisPush_h

#include "./Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

PIC1D_BEGIN_NAMESPACE
class BorisPush {
    Real nu{}; //!< collisional frequency
public:
    constexpr BorisPush() noexcept = default;
    constexpr BorisPush(Real const nu) noexcept : nu{nu} {}

    void operator()(Vector &v, Vector const B, Vector const E) const noexcept {
        //
        // first half acceleration
        //
        v += translate(v, E);
        //
        // rotation
        //
        v += rotate(v, B);
        //
        // second half acceleration
        //
        v += translate(v, E);
    }

private:
    [[nodiscard]] constexpr Vector translate([[maybe_unused]] Vector const &v, Vector const &E) const noexcept {
        return E;
    }
    [[nodiscard]] constexpr Vector rotate(Vector const &v, Vector const &B) const noexcept {
        return cross(v + cross(v, B), (2 / (1 + dot(B, B))) * B);
    }
};
PIC1D_END_NAMESPACE

#endif /* BorisPush_h */
