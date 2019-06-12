//
//  Species.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "./SpeciesBase.h"
#include "../VDF/VDF.h"

HYBRID1D_BEGIN_NAMESPACE
class EField;
class BField;

class Species : public _Species {
public:
    explicit Species() = default;
    Species &operator=(Species const&) = default;
    explicit Species(Real const Oc, Real const op, long const Nc, VDF const &vdf);

    void update_vel(BField const &bfield, EField const &efield, Real const dt);
    void update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel);
    void collect_part(); // collect 0th and 1st moments
    void collect_all(); // collect all moments

private:
    [[nodiscard]] static bool dispatch_update_pos(decltype(_Species::bucket) &bucket, Real const dtODx, Real const travel_scale_factor);
    template <class It>
    [[nodiscard]] static bool _update_position(It first, It last, Real const dtODx, Real const travel_scale_factor);

    static void dispatch_update_vel(decltype(_Species::bucket) &bucket, BField const &B, Real const dtOc_2O0, GridQ<Vector> const &E, Real const cDtOc_2O0);
    template <class It>
    static void _update_velocity(It first, It last, BField const &B, Real const dtOc_2O0, GridQ<Vector> const &E, Real const cDtOc_2O0);

    template <class It>
    void dispatch_collect(void (Species::*collector)(MomTuple &, It, It) const);
    template <class It>
    void dispatch_collect(void (Species::*collector)(MomTuple &, It, It) const,
                       MomTuple *mom_first, MomTuple const *mom_last, It ptl_first, It ptl_last) const;

    template <class It>
    void _collect_part(MomTuple &mom, It first, It last) const;
    template <class It>
    void _collect_all(MomTuple &mom, It first, It last) const;
};
HYBRID1D_END_NAMESPACE

#endif /* Species_h */
