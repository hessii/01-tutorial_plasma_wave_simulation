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
namespace {
    inline void boris_push(Vector &v, Vector B, Vector const E) noexcept {
        //
        // first half acceleration
        //
        v += E;
        //
        // rotation
        //
        auto const &v0 = cross(v, B) + v;
        B *= 2 / (1 + dot(B, B));
        v += cross(v0, B);
        //
        // second half acceleration
        //
        v += E;
    }
}
PIC1D_END_NAMESPACE

#endif /* BorisPush_h */
