//
//  EnergyRecorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef EnergyRecorder_h
#define EnergyRecorder_h

#include "./Recorder.h"
#include "../Utility/Tensor.h"

#include <fstream>

HYBRID1D_BEGIN_NAMESPACE
class EnergyRecorder : public Recorder {
    std::ofstream os;

public:
    explicit EnergyRecorder();

private:
    void record(Domain const &domain, long const step_count) override;
    static void record(std::ostream &os, BField const &bfield);
    static void record(std::ostream &os, EField const &efield);
    static void record(std::ostream &os, Species const &species);

    static Vector fac(Vector const &v) noexcept {
        return {dot(e1, v), dot(e2, v), dot(e3, v)}; // {v_||, v_perp, v_z}
    }
    static Vector fac(Tensor const &vv) noexcept { // similarity transformation
        return {
            dot(vv.lo(), e1*e1) + 2*(vv.xy*e1.x*e1.y + vv.zx*e1.x*e1.z + vv.yz*e1.y*e1.z),
            dot(vv.lo(), e2*e2) + 2*(vv.xy*e2.x*e2.y + vv.zx*e2.x*e2.z + vv.yz*e2.y*e2.z),
            dot(vv.lo(), e3*e3) + 2*(vv.xy*e3.x*e3.y + vv.zx*e3.x*e3.z + vv.yz*e3.y*e3.z)
        };
    }
};
HYBRID1D_END_NAMESPACE

#endif /* EnergyRecorder_h */
