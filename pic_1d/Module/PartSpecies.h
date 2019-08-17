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
public:
    using bucket_type = std::deque<Particle>;

    // member variables
    //
    Real Nc; //!< number particles per cell
    bucket_type bucket; //!< particle container
private:
    std::shared_ptr<VDF> vdf;
    _ParticleScheme scheme{};

    explicit PartSpecies(decltype(nullptr), Real const Oc, Real const op, long const Nc, std::unique_ptr<VDF> _vdf);
public:
    PartSpecies &operator=(PartSpecies const&);
    PartSpecies &operator=(PartSpecies&&);

    explicit PartSpecies() = default;
    template <class ConcreteVDF>
    explicit PartSpecies(Real const Oc, Real const op, long const Nc, _ParticleScheme const scheme, ConcreteVDF &&vdf)
    : PartSpecies(nullptr, Oc, op, Nc,
                  std::make_unique<std::decay_t<ConcreteVDF>>(std::forward<ConcreteVDF>(vdf))) {
        static_assert(std::is_base_of_v<VDF, std::decay_t<ConcreteVDF>>, "ConcreteVDF not base of VDF");
        static_assert(std::is_final_v<std::decay_t<ConcreteVDF>>, "ConcreteVDF not marked final");
        this->scheme = scheme;
    }

    void update_vel(BField const &bfield, EField const &efield, Real const dt);
    void update_pos(Real const dt, Real const fraction_of_grid_size_allowed_to_travel);
    void collect_part(); // collect 1st moment
    void collect_all(); // collect all moments

private:
    [[nodiscard]] static bool _update_position(bucket_type &bucket, Real const dtODx, Real const travel_scale_factor);

    static void _update_velocity(bucket_type &bucket, GridQ<Vector> const &B, Real const dtOc_2O0, EField const &E, Real const cDtOc_2O0);

    void _collect(GridQ<Vector> &nV) const;
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
