//
//  ColdSpecies.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 8/11/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef ColdSpecies_h
#define ColdSpecies_h

#include "./Species.h"

PIC1D_BEGIN_NAMESPACE
class EField;
class BField;

/// linearized cold fluid
///
class ColdSpecies : public Species {
    ColdPlasmaDesc desc;
public:
    ScalarGrid mom0_half{}; // 0th moment on half grid
    VectorGrid mom1_full{}; // 1st moment on full grid
public:
    [[nodiscard]] ColdPlasmaDesc const* operator->() const noexcept override {
        return &desc;
    }
    ColdSpecies &operator=(ColdSpecies&&) = delete;

    ColdSpecies() = default; // needed for empty std::array
    ColdSpecies(ParamSet const &params, ColdPlasmaDesc const &desc);
    void populate(); // load cold species; should only be called by master thread

    void update_den(Real const dt); // update fluid number density by dt; <1>^n -> <1>^n+1
    void update_vel(BField const &bfield, EField const &efield, Real const dt); // update flow velocity by dt; <v>^n-1/2 -> <v>^n+1/2
    void collect_part(); // collect 0th & 1st moments
    void collect_all(); // collect all moments

private:
    static void _update_n(ScalarGrid &n, VectorGrid const &nV, Real const dtOdV);
    static void _update_nV(VectorGrid &nV, BorisPush const pusher, ScalarGrid const &n, VectorGrid const &B, EField const &E);
    void        _collect_part(ScalarGrid &n, VectorGrid &nV) const;
    static void _collect_nvv(TensorGrid &nvv, ScalarGrid const &n, VectorGrid const &nV);
};
PIC1D_END_NAMESPACE

#endif /* ColdSpecies_h */
