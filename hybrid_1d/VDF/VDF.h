//
//  VDF.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef VDF_h
#define VDF_h

#include "../Utility/Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

HYBRID1D_BEGIN_NAMESPACE
class VDF {
public:
    // single particle description
    //
    struct Particle {
        Vector vel;
        Real pos_x;
    };

    virtual ~VDF() = default;

    virtual Particle operator()() const = 0;
};
HYBRID1D_END_NAMESPACE

#endif /* VDF_h */
