//
//  VDF.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef VDF_h
#define VDF_h

#include "../Utility/Particle.h"
#include "../Utility/Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

HYBRID1D_BEGIN_NAMESPACE
class VDF {
protected:
    explicit VDF() noexcept = default;
    VDF(VDF const &) noexcept = default;
    VDF &operator=(VDF const &) noexcept = default;

    static Real uniform_real() noexcept; // (0, 1)

public:
    virtual ~VDF() = default;

    virtual Particle operator()() const = 0;
};
HYBRID1D_END_NAMESPACE

#endif /* VDF_h */
