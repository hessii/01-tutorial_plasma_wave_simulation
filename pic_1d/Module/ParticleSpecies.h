//
//  ParticleSpecies.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef ParticleSpecies_h
#define ParticleSpecies_h

#include "./SpeciesBase.h"
#include "../VDF/VDF.h"

PIC1D_BEGIN_NAMESPACE
class EField;
class BField;

class ParticleSpecies : public _Species {
public:
    explicit ParticleSpecies() = default;
    ParticleSpecies &operator=(ParticleSpecies const&) = default;
    explicit ParticleSpecies(Real const Oc, Real const op, long const Nc, VDF const &vdf);

    void update_vel(BField const &bfield, EField const &efield, Real const dt);
    void update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel);
    void collect_part(); // collect 1st moment
    void collect_all(); // collect all moments

private:
    [[nodiscard]] static bool _update_position(decltype(_Species::bucket) &bucket, Real const dtODx, Real const travel_scale_factor);

    static void _update_velocity(decltype(_Species::bucket) &bucket, GridQ<Vector> const &B, Real const dtOc_2O0, EField const &E, Real const cDtOc_2O0);

    void _collect(GridQ<Vector> &nV) const;
    void _collect(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv) const;
};
PIC1D_END_NAMESPACE

#endif /* ParticleSpecies_h */
