//
//  PlasmaDesc.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 12/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef PlasmaDesc_h
#define PlasmaDesc_h

#include "./Predefined.h"
#include "./Macros.h"

#include <stdexcept>

PIC1D_BEGIN_NAMESPACE
/// Common parameters for all plasmas.
///
struct PlasmaDesc {
    Real Oc; //!< Cyclotron frequency.
    Real op; //!< Plasma frequency.
    long number_of_source_smoothings; //!< The number of source smoothings.
    //
    constexpr PlasmaDesc(Real Oc, Real op, unsigned n_smooths = {})
    : Oc{Oc}, op{op}, number_of_source_smoothings{n_smooths} {
        if (this->Oc == 0) throw std::invalid_argument{"Oc should not be zero"};
        if (this->op <= 0) throw std::invalid_argument{"op should be positive"};
    }
protected:
    explicit PlasmaDesc() noexcept = default;
};

/// Cold plasma descriptor.
///
struct ColdPlasmaDesc : public PlasmaDesc {
    Real Vd; //!< Equilibrium parallel drift speed.
    //
    explicit ColdPlasmaDesc() noexcept = default;
    constexpr ColdPlasmaDesc(PlasmaDesc const &desc, Real Vd)
    : PlasmaDesc(desc), Vd{Vd} {}
    constexpr ColdPlasmaDesc(PlasmaDesc const &desc)
    : ColdPlasmaDesc(desc, {}) {}
};

//
// MARK:- Kinetic Plasmas
//

/// Common parameters for all kinetic plasmas.
///
struct KineticPlasmaDesc : public PlasmaDesc {
    long Nc; //!< The number of simulation particles per cell.
    ParticleScheme scheme; //!< Full-f or delta-f scheme.
    ShapeOrder shape_order; //!< The order of the shape function.
    //
    explicit KineticPlasmaDesc() noexcept = default;
    constexpr KineticPlasmaDesc(PlasmaDesc const &desc, unsigned Nc, ParticleScheme scheme, ShapeOrder shape_order = CIC)
    : PlasmaDesc(desc), Nc{Nc}, scheme{scheme}, shape_order{shape_order} {
        if (this->Nc <= 0) throw std::invalid_argument{"Nc should be positive"};
    }
};

/// Bi-Maxwellian plasma descriptor.
///
struct BiMaxPlasmaDesc : public KineticPlasmaDesc {
    Real beta1; //!< The parallel component of plasma beta.
    Real T2_T1; //!< The ratio of the perpendicular to parallel temperatures.
    Real Vd; //!< Equilibrium parallel drift speed.
    //
    constexpr BiMaxPlasmaDesc(KineticPlasmaDesc const &desc, Real beta1, Real T2_T1, Real Vd)
    : KineticPlasmaDesc(desc), beta1{beta1}, T2_T1{T2_T1}, Vd{Vd} {
        if (this->beta1 <= 0) throw std::invalid_argument{"beta1 should be positive"};
        if (this->T2_T1 <= 0) throw std::invalid_argument{"T2_T1 should be positive"};
    }
    constexpr BiMaxPlasmaDesc(KineticPlasmaDesc const &desc, Real beta1, Real T2_T1 = 1)
    : BiMaxPlasmaDesc(desc, beta1, T2_T1, {}) {}
};
PIC1D_END_NAMESPACE

#endif /* PlasmaDesc_h */
