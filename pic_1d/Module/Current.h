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

PIC1D_BEGIN_NAMESPACE
class PartSpecies;

/// current density
///
class Current : public GridQ<Vector> {
    GridQ<Vector> tmp;

public:
    void reset() noexcept { this->fill(Vector{0}); }
    void smooth() noexcept { _smooth(tmp, *this), this->swap(tmp); }
    Current &operator+=(PartSpecies const &sp) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* Current_h */
