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
    ColdSpecies &operator=(ColdSpecies&&) = delete;

    ColdSpecies() = default; // needed for empty std::array
    ColdSpecies(ParamSet const &params, ColdPlasmaDesc const &desc);

    void update(EField const &efield, Real const dt); // update flow velocity by dt; nV^n-1/2 -> nV^n+1/2
    void collect_all(); // calculate moment<2>

private:
    static void _update_nV(VectorGrid &nV, ScalarGrid const &n0, Vector const B0, EField const &E, BorisPush const pusher);
    static void _collect_nvv(TensorGrid &nvv, ScalarGrid const &n, VectorGrid const &nV);
};
PIC1D_END_NAMESPACE

#endif /* ColdSpecies_h */
