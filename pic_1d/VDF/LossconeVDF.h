//
//  LossconeVDF.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/23/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef LossconeVDF_h
#define LossconeVDF_h

#include "./VDF.h"

PIC1D_BEGIN_NAMESPACE
/// losscone velocity distribution function
///
/// f(v1, v2) = exp(-(x1 - xd)^2)/(π^3/2 vth1 vth2^2) * ((1 - Δ*β)*exp(-x2^2) - (1 - Δ)*exp(-x2^2/β))/(1 - β),
/// where x1 = v1/vth1, xd = vd/vth1, x2 = v2/vth2.
///
/// the effective temperature in the perpendicular direction is 2*T2/vth2^2 = 1 + (1 - Δ)*β
///
class LossconeVDF final : public VDF {
#if defined(DEBUG)
public:
#endif
    struct RejectionSampler { // rejection sampler
        RejectionSampler() noexcept = default;
        RejectionSampler(Real const Delta, Real const beta/*must not be 1*/);
        [[nodiscard]] inline Real sample() const noexcept;
        [[nodiscard]] inline Real fOg(Real const x) const noexcept; // ratio of the target to the proposed distributions
        //
        Real Delta; //!< Δ parameter.
        Real beta; //!< β parameter.
    public:
        Real alpha; //!< thermal spread of of the proposed distribution
        Real M; //!< the ratio f(x_pk)/g(x_pk)
        static constexpr Real aoffset = 0.3; //!< optimal value for thermal spread of the proposed distribution
    } rs;
    Real vth1; //!< Parallel thermal speed.
    Real T2OT1; //!< Temperature anisotropy, T2/T1.
    Real xd; //!< Parallel drift speed normalized to vth1.
    //
    Real xth2_squared; //!< The ratio of vth2^2 to vth1^2.
    Real vth1_cubed; //!< vth1^3.

public:
    explicit LossconeVDF(LossconePlasmaDesc const &desc);

public:
    [[nodiscard]] Scalar n0(Real) const override {
        constexpr Real n0 = 1;
        return n0;
    }
    [[nodiscard]] Vector nV0(Real const pos_x) const override {
        return geomtr.fac2cart({xd*vth1, 0, 0}) * Real{n0(pos_x)};
    }
    [[nodiscard]] Tensor nvv0(Real const pos_x) const override {
        Tensor vv{1 + 2*xd*xd, T2OT1, T2OT1, 0, 0, 0}; // field-aligned 2nd moment
        return geomtr.fac2cart(vv *= .5*vth1*vth1) * Real{n0(pos_x)};
    }

    [[nodiscard]] Real delta_f(Particle const &ptl) const override {
        return 1 - f0(ptl)/ptl.f;
    }

    [[nodiscard]] Particle variate() const override;
private:
    [[nodiscard]] Particle load() const;

    // equilibrium physical distribution function
    //
    [[nodiscard]] Real f0(Vector const &vel) const noexcept;
    [[nodiscard]] Real f0(Particle const &ptl) const noexcept {
        return f0(geomtr.cart2fac(ptl.vel)/vth1) / vth1_cubed;
    }

    // marker particle distribution function
    //
    [[nodiscard]] Real g0(Vector const &vel) const noexcept { return f0(vel); }
    [[nodiscard]] Real g0(Particle const &ptl) const noexcept {
        return g0(geomtr.cart2fac(ptl.vel)/vth1) / vth1_cubed;
    }
};

void test_LossconeVDF();
PIC1D_END_NAMESPACE

#endif /* LossconeVDF_h */
