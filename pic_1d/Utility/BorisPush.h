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
    Real  dt_2{};
    Real  dtOc_2O0{};
    Real cDtOc_2O0{};

public:
    constexpr BorisPush() noexcept = delete;
    constexpr BorisPush(Real const dt, Real const c, Real const O0, Real const Oc) noexcept {
        dt_2 = 0.5*dt;
        dtOc_2O0 = Oc*dt_2/O0;
        cDtOc_2O0 = c*dtOc_2O0;
    }

    void operator()(Vector &V, Vector B, Vector cE, Real nu) const noexcept {
        nu *=  dt_2;
        B  *=  dtOc_2O0;
        cE *= cDtOc_2O0;
        //
        // first half acceleration
        //
        V += (cE - nu*V)/(1 + nu/2);
        //
        // rotation
        //
        V += rotate(V, B);
        //
        // second half acceleration
        //
        V += (cE - nu*V)/(1 + nu/2);
    }
    void operator()(Vector &v, Vector B, Vector cE) const noexcept {
        B  *=  dtOc_2O0;
        cE *= cDtOc_2O0;
        //
        // first half acceleration
        //
        v += cE;
        //
        // rotation
        //
        v += rotate(v, B);
        //
        // second half acceleration
        //
        v += cE;
    }

private:
    [[nodiscard]] constexpr static Vector rotate(Vector const &v, Vector const &B) noexcept {
        return cross(v + cross(v, B), (2 / (1 + dot(B, B))) * B);
    }
};
PIC1D_END_NAMESPACE

#endif /* BorisPush_h */
