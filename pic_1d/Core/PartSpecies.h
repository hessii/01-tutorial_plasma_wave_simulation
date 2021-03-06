//
//  PartSpecies.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef PartSpecies_h
#define PartSpecies_h

#include "./Species.h"
#include "../VDF/VDF.h"

#include <deque>
#include <vector>
#include <memory>
#include <sstream>

PIC1D_BEGIN_NAMESPACE
class EField;
class BField;

/// discrete simulation particle species
///
class PartSpecies : public Species {
    KineticPlasmaDesc desc;
    std::unique_ptr<VDF> vdf;
public:
    using bucket_type = std::deque<Particle>;
    bucket_type bucket; //!< particle container
    Real Nc; //!< number of particles per cell to be used to normalization; don't modify this if you don't know what you are doing

public:
    [[nodiscard]] KineticPlasmaDesc const* operator->() const noexcept override {
        return &desc;
    }

    PartSpecies &operator=(PartSpecies&&) = delete;

    PartSpecies() = default; // needed for empty std::array
    PartSpecies(ParamSet const &params, KineticPlasmaDesc const &desc, std::unique_ptr<VDF> vdf); // leaves bucket empty

    void populate(); // load particles using VDF; should only be called by master thread
    void load_ptls(std::vector<Particle> const &payload); // load particles from a snapshot; particles' coordinates are expected to be relative to the whole domain
    [[nodiscard]] std::vector<Particle> dump_ptls() const; // dump particles whose coordinates are relative to the whole domain

    void update_vel(BField const &bfield, EField const &efield, Real const dt);
    void update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel);
    void collect_part(); // collect 1st moment
    void collect_all(); // collect all moments

private:
    void (*_update_velocity)(bucket_type&, VectorGrid const&, EField const&, BorisPush const);
    void (PartSpecies::*_collect_full_f)(VectorGrid&) const;
    void (PartSpecies::*_collect_delta_f)(VectorGrid&, bucket_type&) const;
private:
    [[nodiscard]] static bool _update_x(bucket_type &bucket, Real const dtODx, Real const travel_scale_factor);

    template <long Order>
    static void _update_velocity_(bucket_type &bucket, VectorGrid const &B, EField const &E, BorisPush const pusher);

    template <long Order>
    void _collect_full_f_(VectorGrid &nV) const; // weight is untouched
    template <long Order>
    void _collect_delta_f_(VectorGrid &nV, bucket_type &bucket) const; // weight is updated
    void _collect(ScalarGrid &n, VectorGrid &nV, TensorGrid &nvv) const;
};

// MARK:- pretty print for particle container
//
template <class CharT, class Traits>
decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, PartSpecies::bucket_type const &bucket) {
    std::basic_ostringstream<CharT, Traits> ss; {
        ss.flags(os.flags());
        ss.imbue(os.getloc());
        ss.precision(os.precision());
        //
        auto it = bucket.cbegin(), end = bucket.cend();
        ss << '{';
        if (it != end) { // check if bucket is empty
            ss << *it++;
        }
        while (it != end) {
            ss << ", " << *it++;
        }
        ss << '}';
    }
    return os << ss.str();
}
PIC1D_END_NAMESPACE

#endif /* PartSpecies_h */
