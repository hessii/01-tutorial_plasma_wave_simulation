//
//  VDF.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef VDF_h
#define VDF_h

#include "./BitReversedPattern.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../InputWrapper.h"
#include "../PlasmaDesc.h"
#include "../Geometry.h"

#include <random>
#include <memory>

PIC1D_BEGIN_NAMESPACE
/// base class for velocity distribution function
///
class VDF {
public:
    static std::unique_ptr<VDF> make(BiMaxPlasmaDesc const&);
    static std::unique_ptr<VDF> make(LossconePlasmaDesc const&);

public:
    virtual ~VDF() = default;

    [[nodiscard]] virtual Particle variate() const = 0; // load a single particle with g0

    [[nodiscard]] virtual Scalar n0(Real const) const = 0; // <1>_0(x)
    [[nodiscard]] virtual Vector nV0(Real const) const = 0; // <v>_0(x)
    [[nodiscard]] virtual Tensor nvv0(Real const) const = 0; // <vv>_0(x)

    [[nodiscard]] virtual Real delta_f(Particle const &) const = 0; // 1 - f_0(x(t), v(t))/f(0, x(0), v(0))
    [[nodiscard]] Real weight(Particle const &ptl) const {
        // f(0, x(0), v(0))/g(0, x(0), v(0)) - f_0(x(t), v(t))/g(0, x(0), v(0))
        // where g is the marker particle distribution
        //
        return ptl.fOg*delta_f(ptl);
    }

protected:
    Geometry const geomtr;
    //
    explicit VDF() noexcept;

private:
    template <class URBG>
    [[nodiscard]] static Real uniform_real(URBG &g) noexcept { // (0, 1)
        constexpr Real eps = 1e-15;
        thread_local static std::uniform_real_distribution<> uniform{eps, 1 - eps};
        return uniform(g);
    }
public:
    // uniform distribution
    //
    template <unsigned seed>
    [[nodiscard]] static Real uniform_real(/*seed must be passed as a template parameter*/) noexcept {
        static_assert(seed > 0, "seed has to be a positive number");
        thread_local static std::mt19937 g{seed};
        return uniform_real(g);
    }
    template <unsigned base>
    [[nodiscard]] static Real bit_reversed() noexcept {
        static_assert(base > 0, "base has to be a positive number");
        thread_local static BitReversedPattern<base> g{};
        return uniform_real(g);
    }
};
PIC1D_END_NAMESPACE

#endif /* VDF_h */
