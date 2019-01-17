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
    using _Species::_Species;

    void update_vel(BField const &bfield, EField const &efield, Real const dt);
    void update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel = 1.0);
    void collect_partial_moments(); // collect 0th and 1st moments
    void collect_all_moments(); // collect all moments

private:
    static inline bool _update_position(decltype(_Species::bucket) &bucket, Real const dtODx, Real const travel_scale_factor);

    static inline void _update_velocity(decltype(_Species::bucket) &bucket, BField const &B, Real const dtOc_2O0, EField const &E, Real const cDtOc_2O0);

    inline void _collect_part(GridQ<Scalar> &n, GridQ<Vector> &nV, decltype(_Species::bucket) const &bucket) const;
    inline void _collect_all(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv, decltype(_Species::bucket) const &bucket) const;
};
HYBRID1D_END_NAMESPACE

#endif /* Species_h */
