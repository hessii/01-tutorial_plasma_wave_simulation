//
//  Species.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "./SpeciesBase.h"

HYBRID1D_BEGIN_NAMESPACE
class EField;
class BField;

class Species : public _Species {
public:
    Species &operator=(Species &&o) noexcept = default;
    explicit Species() noexcept = default;
    explicit Species(Real const Oc, Real const op, long const Nc);

    void update_vel(BField const &bfield, EField const &efield, Real const dt);
    void update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel);
    void collect_part(); // collect 0th and 1st moments
    void collect_all(); // collect all moments

private:
    static inline bool _update_position(decltype(_Species::bucket) &bucket, Real const dtODx, Real const travel_scale_factor);

    static inline void _update_velocity(decltype(_Species::bucket) &bucket, BField const &B, Real const dtOc_2O0, GridQ<Vector> const &E, Real const cDtOc_2O0);

    inline void _collect_part(GridQ<Scalar> &n, GridQ<Vector> &nV) const;
    inline void _collect_all(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv) const;
};
HYBRID1D_END_NAMESPACE

#endif /* Species_h */
