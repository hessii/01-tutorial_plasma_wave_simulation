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
#include <tuple>

PIC1D_BEGIN_NAMESPACE
/// Common parameters for all plasmas.
///
struct [[nodiscard]] PlasmaDesc {
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
private:
    [[nodiscard]] friend constexpr auto serialize(PlasmaDesc const &desc) noexcept {
        return std::make_tuple(desc.Oc, desc.op);
    }
};

/// Cold plasma descriptor.
///
struct [[nodiscard]] ColdPlasmaDesc : public PlasmaDesc {
    Real Vd; //!< Equilibrium parallel drift speed.
    //
    explicit ColdPlasmaDesc() noexcept = default;
    constexpr ColdPlasmaDesc(PlasmaDesc const &desc, Real Vd)
    : PlasmaDesc(desc), Vd{Vd} {}
    constexpr ColdPlasmaDesc(PlasmaDesc const &desc)
    : ColdPlasmaDesc(desc, {}) {}
private:
    [[nodiscard]] friend constexpr auto serialize(ColdPlasmaDesc const &desc) noexcept {
        PlasmaDesc const &base = desc;
        return std::tuple_cat(serialize(base), std::make_tuple(desc.Vd));
    }
};

//
// MARK:- Kinetic Plasmas
//

/// Common parameters for all kinetic plasmas.
///
struct [[nodiscard]] KineticPlasmaDesc : public PlasmaDesc {
    long Nc; //!< The number of simulation particles per cell.
    ShapeOrder shape_order; //!< The order of the shape function.
    ParticleScheme scheme; //!< Full-f or delta-f scheme.
    //
    explicit KineticPlasmaDesc() noexcept = default;
    constexpr KineticPlasmaDesc(PlasmaDesc const &desc, unsigned Nc, ShapeOrder shape_order, ParticleScheme scheme = full_f)
    : PlasmaDesc(desc), Nc{Nc}, shape_order{shape_order}, scheme{scheme} {
        if (this->Nc <= 0) throw std::invalid_argument{"Nc should be positive"};
    }
private:
    [[nodiscard]] friend constexpr auto serialize(KineticPlasmaDesc const &desc) noexcept {
        PlasmaDesc const &base = desc;
        return std::tuple_cat(serialize(base), std::make_tuple(desc.Nc, desc.scheme));
    }
};

/// Bi-Maxwellian plasma descriptor.
///
struct [[nodiscard]] BiMaxPlasmaDesc : public KineticPlasmaDesc {
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
private:
    [[nodiscard]] friend constexpr auto serialize(BiMaxPlasmaDesc const &desc) noexcept {
        KineticPlasmaDesc const &base = desc;
        return std::tuple_cat(serialize(base), std::make_tuple(desc.beta1, desc.T2_T1, desc.Vd));
    }
};

/// Losscone distribution plasma descriptor.
///
/// The effective perpendicular temperature is 2*T2 = 1 + (1 - Δ)*β.
///
struct [[nodiscard]] LossconePlasmaDesc : public BiMaxPlasmaDesc {
    Real Delta; // Losscone VDF Δ parameter.
    Real beta; // Losscone VDF β parameter.
    //
    constexpr LossconePlasmaDesc(BiMaxPlasmaDesc const &desc, Real Delta = 1, Real beta = 1)
    : BiMaxPlasmaDesc(desc), Delta{Delta}, beta{beta} {
        if (this->Delta < 0 || this->Delta > 1) throw std::invalid_argument{"Losscone.Delta should be in the range of [0, 1]"};
        if (this->beta <= 0) throw std::invalid_argument{"Losscone.beta should be positive"};
    }
private:
    [[nodiscard]] friend constexpr auto serialize(LossconePlasmaDesc const &desc) noexcept {
        BiMaxPlasmaDesc const &base = desc;
        return std::tuple_cat(serialize(base), std::make_tuple(desc.Delta, desc.beta));
    }
};
PIC1D_END_NAMESPACE

#endif /* PlasmaDesc_h */
