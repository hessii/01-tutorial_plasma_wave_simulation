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

public:
    [[nodiscard]] KineticPlasmaDesc const* operator->() const noexcept override {
        return &desc;
    }

    PartSpecies &operator=(PartSpecies&&) = delete;

    explicit PartSpecies() = default;
    explicit PartSpecies(KineticPlasmaDesc const &desc, std::unique_ptr<VDF> vdf);

    void update_vel(BField const &bfield, EField const &efield, Real const dt);
    void update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel);
    void collect_part(); // collect 1st moment
    void collect_all(); // collect all moments

private:
    [[nodiscard]] static bool _update_x(bucket_type &bucket, Real const dtODx, Real const travel_scale_factor);

    static void _update_v(bucket_type &bucket, GridQ<Vector> const &B, EField const &E, Real const nu, BorisPush const pusher);
    static void _update_v(bucket_type &bucket, GridQ<Vector> const &B, EField const &E, BorisPush const pusher);

    void _collect_full_f(GridQ<Vector> &nV) const; // weight is untouched
    void _collect_delta_f(GridQ<Vector> &nV, bucket_type &bucket) const; // weight is updated
    void _collect(GridQ<Scalar> &n, GridQ<Vector> &nV, GridQ<Tensor> &nvv) const;
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
