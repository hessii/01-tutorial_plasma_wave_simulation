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

/// linearized cold fluid
///
class ColdSpecies : public Species {
    ColdPlasmaDesc desc;
public:
    [[nodiscard]] ColdPlasmaDesc const* operator->() const noexcept override {
        return &desc;
    }
    void zero_out_plasma_frequency() noexcept { desc.op = 0; }

    ColdSpecies &operator=(ColdSpecies const&) = default;
    ColdSpecies &operator=(ColdSpecies&&) = default;

    explicit ColdSpecies() = default;
    [[deprecated]] explicit ColdSpecies(PlasmaDesc const &desc, Real const Vd);

    void update(EField const &efield, Real const dt); // update flow velocity by dt; nV^n-1/2 -> nV^n+1/2
    void collect_all(); // calculate moment<2>

private:
    static void _update_nV(GridQ<Vector> &nV, GridQ<Scalar> const &n0, Vector const B0, EField const &E, Real const nu, BorisPush const pusher);
    static void _collect_nvv(GridQ<Tensor> &nvv, GridQ<Scalar> const &n, GridQ<Vector> const &nV);
};
PIC1D_END_NAMESPACE

#endif /* ColdSpecies_h */
