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
    Real  dtOc_2O0{};
    Real cDtOc_2O0{};

public:
    constexpr BorisPush() noexcept = delete;
    constexpr BorisPush(Real const dt, Real const c, Real const O0, Real const Oc) noexcept {
        auto const dt_2O0 = 0.5*dt/O0;
        dtOc_2O0 = Oc*dt_2O0;
        cDtOc_2O0 = c*dtOc_2O0;
    }

//    void operator()(Vector &v, Real nu, Vector B, Vector E) const noexcept {
//        nu *= dtOc_2O0;
//        B  *= dtOc_2O0;
//        E *= cDtOc_2O0;
//    }
    void operator()(Vector &v, Vector B, Vector E) const noexcept {
        B  *= dtOc_2O0;
        E *= cDtOc_2O0;
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
