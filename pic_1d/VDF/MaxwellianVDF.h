//
//  MaxwellianVDF.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MaxwellianVDF_h
#define MaxwellianVDF_h

#include "./VDF.h"

PIC1D_BEGIN_NAMESPACE
/// bi-Maxwellian velocity distribution function
///
/// f(v1, v2) = exp(-(x1 - xd)^2 -x2^2)/(π^3/2 vth1^3 T2/T1),
/// where x1 = v1/vth1, xd = vd/vth1, x2 = v2/(vth1*√(T2/T1))), and
/// T2 and T1 are temperatures in directions perpendicular and
/// parallel to the background magnetic field direction, respectively
///
class MaxwellianVDF final : public VDF {
    Real vth1; //!< Parallel thermal speed.
    Real T2OT1; //!< Temperature anisotropy, T2/T1.
    Real xd; //!< Parallel drift speed normalized to vth1.
    //
    Real vth1_cubed;

public:
    explicit MaxwellianVDF() noexcept;
    explicit MaxwellianVDF(Real const vth1, Real const T2OT1, Real const vd);

    [[nodiscard]] Scalar n0(Real) const override {
        constexpr Real n0 = 1;
        return n0/(Input::number_of_worker_threads + 1);
    }
    [[nodiscard]] Vector nV0(Real const pos_x) const override {
        return fac2cart({xd*vth1, 0, 0}) * Real{n0(pos_x)};
    }
    [[nodiscard]] Tensor nvv0(Real const pos_x) const override {
        Tensor vv{1 + 2*xd*xd, T2OT1, T2OT1, 0, 0, 0}; // field-aligned 2nd moment
        return fac2cart(vv *= .5*vth1*vth1) * Real{n0(pos_x)};
    }

    [[nodiscard]] Real delta_f(Particle const &ptl) const override {
        return 1 - f0(ptl)/ptl.f;
    }

    [[nodiscard]] Particle variate() const override;
private:
    [[nodiscard]] Particle load() const;

    // equilibrium physical distribution function
    // f0(x1, x2, x3) = exp(-(x1 - xd)^2)/√π * exp(-(x2^2 + x3^2)/(T2/T1))/(π T2/T1)
    //
    [[nodiscard]] Real f0(Vector const &vel) const noexcept;
    [[nodiscard]] Real f0(Particle const &ptl) const noexcept {
        return f0(cart2fac(ptl.vel)/vth1) / vth1_cubed;
    }

    // marker particle distribution function
    //
    [[nodiscard]] Real g0(Vector const &vel) const noexcept { return f0(vel); }
    [[nodiscard]] Real g0(Particle const &ptl) const noexcept {
        return g0(cart2fac(ptl.vel)/vth1) / vth1_cubed;
    }
};
PIC1D_END_NAMESPACE

#endif /* MaxwellianVDF_h */
