//
//  Recorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Recorder_h
#define Recorder_h

#include "../Module/Domain.h"
#include "../Utility/Tensor.h"
#include "../Utility/Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

HYBRID1D_BEGIN_NAMESPACE
class Domain;

class Recorder {
public:
    virtual ~Recorder() = default;
    virtual void record(Domain const &domain, long const step_count) = 0;

protected:
    long const recording_frequency;
    explicit Recorder(unsigned const recording_frequency) noexcept;

    // field-aligned unit vectors
    //
    static Vector const e1;
    static Vector const e2;
    static Vector const e3;

    // field-aligned transformation of vector and compact tensor
    //
    static Vector fac(Vector const &v) noexcept {
        return {dot(e1, v), dot(e2, v), dot(e3, v)}; // {v_||, v_perp, v_z}
    }
    static Vector fac(Tensor const &vv) noexcept { // similarity transformation
        return {
            dot(vv.lo(), e1*e1) + 2*(vv.xy*e1.x*e1.y + vv.zx*e1.x*e1.z + vv.yz*e1.y*e1.z),
            dot(vv.lo(), e2*e2) + 2*(vv.xy*e2.x*e2.y + vv.zx*e2.x*e2.z + vv.yz*e2.y*e2.z),
            dot(vv.lo(), e3*e3) + 2*(vv.xy*e3.x*e3.y + vv.zx*e3.x*e3.z + vv.yz*e3.y*e3.z)
        }; // {v_||^2, v_perp^2, v_z^2}
    }
};
HYBRID1D_END_NAMESPACE

#endif /* Recorder_h */
