//
//  EField.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef EField_h
#define EField_h

#include "../Utility/GridQ.h"
#include "../Utility/Vector.h"
#include "../Utility/Scalar.h"
#include "../InputWrapper.h"
#include "../Geometry.h"

PIC1D_BEGIN_NAMESPACE
class BField;
class Current;

class EField : public GridQ<Vector, Input::Nx> {
public:
    ParamSet const params;
    Geometry const geomtr;

    explicit EField(ParamSet const&);

    void update(BField const &bfield, Current const &current, Real const dt) noexcept;

private:
    static inline void _update(EField &E, BField const &B, Real const cdtODx, Current const &J, Real const dt) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* EField_h */
