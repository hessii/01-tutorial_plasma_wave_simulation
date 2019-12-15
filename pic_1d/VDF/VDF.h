//
//  VDF.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef VDF_h
#define VDF_h

#include "BitReversedPattern.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../InputWrapper.h"
#include "../PlasmaDesc.h"

#include <random>
#include <memory>

PIC1D_BEGIN_NAMESPACE
/// base class for velocity distribution function
///
class VDF {
public:
    static std::unique_ptr<VDF> make(BiMaxPlasmaDesc const&);

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
    explicit VDF() noexcept = default;
    VDF(VDF const &) noexcept = default;
    VDF &operator=(VDF const &) noexcept = default;

private:
    template <class URBG>
    [[nodiscard]] static Real uniform_real(URBG &g) noexcept { // (0, 1)
        constexpr Real eps = 1e-15;
        static std::uniform_real_distribution<> uniform{eps, 1 - eps};
        return uniform(g);
    }
protected:
    // uniform distribution
    //
    template <unsigned seed>
    [[nodiscard]] static Real uniform_real(/*seed must be passed as a template parameter*/) noexcept {
        static_assert(seed > 0, "seed has to be a positive number");
        /*constinit*/ static std::mt19937 g{seed};
        return uniform_real(g);
    }
    template <unsigned base>
    [[nodiscard]] static Real bit_reversed() noexcept {
        static_assert(base > 0, "base has to be a positive number");
        /*constinit*/ static BitReversedPattern<base> g{};
        return uniform_real(g);
    }

protected:
    // field-aligned unit vectors
    //
    static Vector const e1;
    static Vector const e2;
    static Vector const e3;

    // transformation from field-aligned to cartesian
    //
    [[nodiscard]] static Vector fac2cart(Vector const &v) noexcept { // v = {v1, v2, v3}
        return e1*v.x + e2*v.y + e3*v.z;
    }
    [[nodiscard]] static Tensor fac2cart(Tensor const &vv) noexcept { // vv = {v1v1, v2v2, v3v3, 0, 0, 0};
        Tensor ret;
        ret.lo() = vv.xx*e1*e1 + vv.yy*e2*e2 + vv.zz*e3*e3;
        ret.xy = e1.x*e1.y*vv.xx + e2.x*e2.y*vv.yy + e3.x*e3.y*vv.zz;
        ret.yz = e1.y*e1.z*vv.xx + e2.y*e2.z*vv.yy + e3.y*e3.z*vv.zz;
        ret.zx = e1.x*e1.z*vv.xx + e2.x*e2.z*vv.yy + e3.x*e3.z*vv.zz;
        return ret;
    }

    // transformation from cartesian to field-aligned
    //
    [[nodiscard]] static Vector cart2fac(Vector const &v) noexcept {
        return {dot(e1, v), dot(e2, v), dot(e3, v)}; // {v_||, v_perp, v_z}
    }
    [[nodiscard]] static Vector cart2fac(Tensor const &vv) noexcept { // similarity transformation
        return {
            dot(vv.lo(), e1*e1) + 2*(vv.xy*e1.x*e1.y + vv.zx*e1.x*e1.z + vv.yz*e1.y*e1.z),
            dot(vv.lo(), e2*e2) + 2*(vv.xy*e2.x*e2.y + vv.zx*e2.x*e2.z + vv.yz*e2.y*e2.z),
            dot(vv.lo(), e3*e3) + 2*(vv.xy*e3.x*e3.y + vv.zx*e3.x*e3.z + vv.yz*e3.y*e3.z)
        }; // {v_||^2, v_perp^2, v_z^2}
    }
};
PIC1D_END_NAMESPACE

#endif /* VDF_h */
