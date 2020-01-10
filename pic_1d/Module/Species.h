//
//  Species.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Species_h
#define Species_h

#include "../Utility/Particle.h"
#include "../Utility/BorisPush.h"
#include "../InputWrapper.h"
#include "../PlasmaDesc.h"
#include "../Geometry.h"

#include <tuple>

PIC1D_BEGIN_NAMESPACE
/// base class for ion/electron species
///
class Species {
public:
    ParamSet const params;
    Geometry const geomtr;
protected:
    using MomTuple = std::tuple<ScalarGrid, VectorGrid, TensorGrid>;
private:
    MomTuple _mom{}; //!< velocity moments at grid points

    template <class T>
    using grid_t = GridQ<T, ScalarGrid::size()>;

public:
    // accessors
    //
    [[nodiscard]] virtual PlasmaDesc const* operator->() const noexcept = 0;

    [[nodiscard]] Real charge_density_conversion_factor() const noexcept {
        return ((*this)->op*(*this)->op)*params.O0/(*this)->Oc;
    }
    [[nodiscard]] Real current_density_conversion_factor() const noexcept {
        return charge_density_conversion_factor()/params.c;
    }
    [[nodiscard]] Real energy_density_conversion_factor() const noexcept {
        Real const tmp = params.O0/(*this)->Oc*(*this)->op/params.c;
        return tmp*tmp;
    }

    // access to i'th velocity moment
    //
    template <long i> [[nodiscard]]
    auto const &moment() const noexcept { return std::get<i>(_mom); }
    template <long i> [[nodiscard]]
    auto       &moment()       noexcept { return std::get<i>(_mom); }
    //
    template <class T> [[nodiscard]]
    auto const &moment() const noexcept { return std::get<grid_t<T>>(_mom); }
    template <class T> [[nodiscard]]
    auto       &moment()       noexcept { return std::get<grid_t<T>>(_mom); }
    //
    [[nodiscard]] MomTuple const &moments() const noexcept { return _mom; }
    [[nodiscard]] MomTuple       &moments()       noexcept { return _mom; }

protected:
    virtual ~Species() = default;
    explicit Species(ParamSet const&);
    Species &operator=(Species const&) noexcept;
    Species &operator=(Species &&) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* Species_h */
