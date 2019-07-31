//
//  VDF.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef VDF_h
#define VDF_h

#include "../Utility/Particle.h"
#include "../Utility/Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

PIC1D_BEGIN_NAMESPACE
/// base class for velocity distribution function
///
class VDF {
public:
    virtual ~VDF() = default;

    [[nodiscard]] virtual Particle variate() const = 0;

protected:
    explicit VDF() noexcept = default;
    VDF(VDF const &) noexcept = default;
    VDF &operator=(VDF const &) noexcept = default;

    [[nodiscard]] static Real uniform_real() noexcept; // (0, 1)

    // field-aligned unit vectors
    //
    static Vector const e1;
    static Vector const e2;
    static Vector const e3;
};
PIC1D_END_NAMESPACE

#endif /* VDF_h */