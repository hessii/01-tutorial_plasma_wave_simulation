//
//  ParticlePush.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/16/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef ParticlePush_h
#define ParticlePush_h

#include "./Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

HYBRID1D_BEGIN_NAMESPACE
namespace {
    inline void boris_push(Vector &v, Vector const &E, Vector B) noexcept {
        //
        // step 1
        //
        v += E;
        //
        // step 2
        //
        Vector v0{v};
        v0 += cross(v, B);
        //
        // step 3
        //
        B *= 2 / (1 + dot(B, B));
        v += cross(v0, B);
        //
        // step 4
        //
        v += E;
    }
}
HYBRID1D_END_NAMESPACE

#endif /* ParticlePush_h */
