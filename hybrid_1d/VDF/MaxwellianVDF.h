//
//  MaxwellianVDF.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef MaxwellianVDF_h
#define MaxwellianVDF_h

#include "./VDF.h"

HYBRID1D_BEGIN_NAMESPACE
class MaxwellianVDF : public VDF {
    Real vth1; //!< Parallel thermal speed.
    Real T2OT1; //!< T2/T1.

public:
    explicit MaxwellianVDF() noexcept;
    explicit MaxwellianVDF(Real const vth1, Real const T2OT1);

    Particle operator()() const override;
private:
    Particle load() const;
};
HYBRID1D_END_NAMESPACE

#endif /* MaxwellianVDF_h */
