//
//  Species.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "../Utility/GridQ.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../Utility/BorisPush.h"
#include "../InputWrapper.h"

#include <tuple>

PIC1D_BEGIN_NAMESPACE
/// base class for ion/electron species
///
class Species {
public:
    // member variables
    //
    CommonPlasmaDesc param;
protected:
    using MomTuple = std::tuple<GridQ<Scalar>, GridQ<Vector>, GridQ<Tensor>>;
private:
    MomTuple _mom; //!< velocity moments at grid points

public:
    // accessors
    //
    [[nodiscard]] Real charge_density_conversion_factor() const noexcept {
        return (param.op*param.op)*Input::O0/param.Oc;
    }
    [[nodiscard]] Real current_density_conversion_factor() const noexcept {
        return charge_density_conversion_factor()/Input::c;
    }
    [[nodiscard]] Real energy_density_conversion_factor() const noexcept {
        Real const tmp = Input::O0/param.Oc*param.op/Input::c;
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
    explicit Species() = default;
    explicit Species(CommonPlasmaDesc const &param) : param{param}, _mom{} {}
    Species &operator=(Species const&);
    Species &operator=(Species &&);
};
PIC1D_END_NAMESPACE

#endif /* Species_h */
