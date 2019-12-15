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
/// Common parameters for all plasma species/populations
///
struct PlasmaDesc {
    Real Oc; //!< Cyclotron frequency.
    Real op; //!< Plasma frequency.
    Real nu; //!< Collisional damping factor.
    unsigned number_of_source_smoothings; //!< The number of source smoothings.
    //
    explicit PlasmaDesc() noexcept = default;
    constexpr PlasmaDesc(Real Oc, Real op,
                               unsigned n_smooths = 2,
                               Real nu = {})
    : Oc{Oc}, op{op}, nu{nu}, number_of_source_smoothings{n_smooths} {
        if (this->Oc == 0) throw std::invalid_argument{"Oc should not be zero"};
        if (this->op <= 0) throw std::invalid_argument{"op should be positive"};
        if (this->nu <  0) throw std::invalid_argument{"nu should be non-negative"};
    }
};

/// Cold plasma species/population descriptor.
///
struct ColdPlasmaDesc : public PlasmaDesc {
    Real Vd; //!< Equilibrium parallel drift speed.
    //
    constexpr ColdPlasmaDesc(PlasmaDesc const &desc, Real Vd)
    : PlasmaDesc(desc), Vd{Vd} {}
    constexpr ColdPlasmaDesc(PlasmaDesc const &desc)
    : ColdPlasmaDesc(desc, {}) {}
};

/// Bi-Maxwellian plasma species/population descriptor.
///
struct BiMaxPlasmaDesc : public PlasmaDesc {
    unsigned Nc; //!< The number of simulation particles per cell.
    ParticleScheme scheme; //!< Full-f or delta-f scheme.
    Real beta1; //!< The parallel component of plasma beta.
    Real T2_T1; //!< The ratio of the perpendicular to parallel temperatures.
    Real Vd; //!< Equilibrium parallel drift speed.
    //
    struct Pair { // helper
        Real beta1;
        Real T2_T1{1};
    };
    constexpr BiMaxPlasmaDesc(PlasmaDesc const &desc, unsigned Nc, ParticleScheme scheme, Pair b1_ani, Real Vd)
    : PlasmaDesc(desc), Nc{Nc}, scheme{scheme}, beta1{b1_ani.beta1}, T2_T1{b1_ani.T2_T1}, Vd{Vd} {
        if (this->Nc <= 0) throw std::invalid_argument{"Nc should be positive"};
        if (this->beta1 <= 0) throw std::invalid_argument{"beta1 should be positive"};
        if (this->T2_T1 <= 0) throw std::invalid_argument{"T2_T1 should be positive"};
    }
    constexpr BiMaxPlasmaDesc(PlasmaDesc const &desc, unsigned Nc, ParticleScheme scheme, Pair b1_ani)
    : BiMaxPlasmaDesc(desc, Nc, scheme, b1_ani, {}) {}
};
PIC1D_END_NAMESPACE

#endif /* PlasmaDesc_h */
