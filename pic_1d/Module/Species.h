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
#include "../PlasmaDesc.h"

#include <tuple>

PIC1D_BEGIN_NAMESPACE
/// base class for ion/electron species
///
class Species {
protected:
    using ScalarGrid = GridQ<Scalar, Input::Nx>;
    using VectorGrid = GridQ<Vector, Input::Nx>;
    using TensorGrid = GridQ<Tensor, Input::Nx>;
    using MomTuple = std::tuple<ScalarGrid, VectorGrid, TensorGrid>;
private:
    MomTuple _mom{}; //!< velocity moments at grid points

public:
    // accessors
    //
    [[nodiscard]] virtual PlasmaDesc const* operator->() const noexcept = 0;

    [[nodiscard]] Real charge_density_conversion_factor() const noexcept {
        return ((*this)->op*(*this)->op)*Input::O0/(*this)->Oc;
    }
    [[nodiscard]] Real current_density_conversion_factor() const noexcept {
        return charge_density_conversion_factor()/Input::c;
    }
    [[nodiscard]] Real energy_density_conversion_factor() const noexcept {
        Real const tmp = Input::O0/(*this)->Oc*(*this)->op/Input::c;
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
    virtual ~Species() = default;
    explicit Species() = default;
    Species &operator=(Species const&) noexcept;
    Species &operator=(Species &&) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* Species_h */
