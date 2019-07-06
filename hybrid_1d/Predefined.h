//
//  Predefined.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Predefined_h
#define Predefined_h

#include "./Macros.h"

HYBRID1D_BEGIN_NAMESPACE
namespace {
    using Real = double;

    /// Number of ghost cells.
    ///
    constexpr long Pad = 2;

    /// Algorithm for electric field extrapolation.
    ///
    enum _Algorithm : long {
        PC,   //!< Using predictor-corrector by Kunz et al. (2014).
        CAMCL //!< Using CAM-CL by Matthew (1994).
    };

    /// Order of the shape function.
    ///
    enum _ShapeOrder : long {
        CIC = 1, //!<  First order; cloud-in-cell scheme.
        TSC = 2, //!< Second order; Triangular-shaped density cloud sheme.
        _3rd = 3 //!< 3rd order.
    };

    enum _Closure : long {
        isothermal = 11, //!< gamma = 1/1
         adiabatic = 53  //!< gamma = 5/3
    };
}
HYBRID1D_END_NAMESPACE

#endif /* Predefined_h */
