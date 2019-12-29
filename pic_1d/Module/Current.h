//
//  Current.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Current_h
#define Current_h

#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"
#include "../InputWrapper.h"
#include "../Geometry.h"

PIC1D_BEGIN_NAMESPACE
class Species;

/// current density
///
class Current : public GridQ<Vector, Input::Nx> {
    using VectorGrid = GridQ<Vector, Input::Nx>;
    VectorGrid tmp;
    ParamSet const params;
    Geometry const geomtr;

public:
    explicit Current(ParamSet const&);

    void reset() noexcept { this->fill(Vector{0}); }
    void smooth() noexcept { _smooth(tmp, *this), this->swap(tmp); }
    Current &operator+=(Species const &sp) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* Current_h */
