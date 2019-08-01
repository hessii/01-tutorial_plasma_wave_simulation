//
//  SpeciesBase.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef SpeciesBase_h
#define SpeciesBase_h

#include "../Utility/GridQ.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../InputWrapper.h"

#include <tuple>
#include <deque>
#include <sstream>

PIC1D_BEGIN_NAMESPACE
class _Species {
public:
    // member variables
    //
    Real Nc; //!< number particles per cell
    Real Oc; //!< cyclotron frequency
    Real op; //!< plasma frequency
    std::deque<Particle> bucket; //!< particle container
protected:
    using MomTuple = std::tuple<GridQ<Scalar>, GridQ<Vector>, GridQ<Tensor>>;
private:
    MomTuple _mom; //!< velocity moments at grid points

public:
    // accessors
    //
    [[nodiscard]] Real charge_density_conversion_factor() const noexcept {
        return (op*op)*Input::O0/Oc;
    }
    [[nodiscard]] Real current_density_conversion_factor() const noexcept {
        return charge_density_conversion_factor()/Input::c;
    }
    [[nodiscard]] Real energy_density_conversion_factor() const noexcept {
        Real const tmp = Input::O0/Oc*op/Input::c;
        return tmp*tmp;
    }

    // access to i'th velocity moment
    //
    template <long i> [[nodiscard]]
    auto const &moment() const noexcept {
        return std::get<i>(_mom);
    }
    template <long i> [[nodiscard]]
    auto       &moment()       noexcept {
        return std::get<i>(_mom);
    }

    [[nodiscard]] MomTuple const &moments() const noexcept { return _mom; }
    [[nodiscard]] MomTuple       &moments()       noexcept { return _mom; }

protected:
    // constructor
    //
    explicit _Species() = default;
    explicit _Species(Real const Oc, Real const op, long const Nc);
    _Species &operator=(_Species const&);
    _Species &operator=(_Species &&);
};

// MARK:- pretty print for particle container
//
template <class CharT, class Traits>
decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, std::deque<Particle> const &bucket) {
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

#endif /* SpeciesBase_h */
