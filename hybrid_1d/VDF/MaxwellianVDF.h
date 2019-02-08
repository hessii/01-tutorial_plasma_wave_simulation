//
//  MaxwellianVDF.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MaxwellianVDF_h
#define MaxwellianVDF_h

#include "./VDF.h"

HYBRID1D_BEGIN_NAMESPACE
/// bi-Maxwellian velocity distribution function
/// f(v1, v2) = exp(-x1^2 -x2^2)/(π^3/2 vth1^3 T2/T1),
/// where x1 = v1/vth1, x2 = v2/(vth1*√(T2/T1))), and
/// T2 and T1 are temperatures in directions perpendicular and parallel to the background magnetic field direction, respectively
///
class MaxwellianVDF : public VDF {
    Real vth1; //!< Parallel thermal speed.
    Real T2OT1; //!< T2/T1.

public:
    explicit MaxwellianVDF() noexcept;
    explicit MaxwellianVDF(Real const vth1, Real const T2OT1);

    Particle variate() const override;
private:
    Particle load() const;
};
HYBRID1D_END_NAMESPACE

#endif /* MaxwellianVDF_h */
